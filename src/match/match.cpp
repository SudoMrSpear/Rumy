#include "match.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "app/config.h"
#include "app/shared.h"
#include "app/store.h"
#include "clock/clock.h"
#include "diag/diag.h"
#include "espn.h"
#include "fast.h"
#include "livescore.h"
#include "match/board.h"
#include "ui/goal_screen.h"
#include "ui/match_ui.h"
#include "net/net.h"
#include "settings/settings.h"
#include "text_tr.h"

namespace match {

namespace {

enum DemoCmd : uint8_t { D_NONE = 0, D_PRE, D_LIVE, D_GOAL, D_OPP, D_YELLOW, D_RED, D_SUB, D_HALF, D_POST, D_CANCEL, D_OFF };

rc::Match s_m;
rc::EspnPage s_page;
rc::MatchStatus s_status;
char s_path[160];
char s_name[rc::MATCH_PLAYER];
uint32_t s_team = 0;
uint32_t s_nextSchedUtc = 0;
uint32_t s_bytes = 0;
uint32_t s_requests = 0;
uint32_t s_errors = 0;
int16_t s_lastCode = 0;
uint8_t s_demoMin = 0;
volatile uint8_t s_demoCmd = D_NONE;
volatile uint32_t s_demoUntil = 0;
volatile bool s_refresh = false;
bool s_catchUp = false;
rc::LsMatch s_ls;
rc::LsEvent s_lsEvent;
uint32_t s_fastEvent = 0;
uint32_t s_lsEid = 0;
uint8_t s_lsSwap = 0;
uint32_t s_lsTryMs = 0;
uint8_t s_fastHome = 0;
uint8_t s_fastAway = 0;
uint32_t s_fastAt = 0;
uint32_t s_bridgeAt = 0;
uint32_t s_lsRequests = 0;
uint32_t s_lsErrors = 0;
uint32_t s_fastGoals = 0;
int16_t s_lsCode = 0;

bool demoActive() { return s_demoUntil && static_cast<int32_t>(s_demoUntil - millis()) > 0; }

bool parseSchedule(rc::ByteSource& src, void*) {
  return rc::espnParseSchedule(src, net::arena(), s_team, clk::nowUtc(), s_m);
}

bool parseStatus(rc::ByteSource& src, void*) { return rc::espnParseStatus(src, net::arena(), s_status); }

bool parsePlays(rc::ByteSource& src, void*) { return rc::espnParsePlays(src, net::arena(), s_page); }

bool parseLsLive(rc::ByteSource& src, void*) {
  return rc::lsFindLive(src, net::arena(), s_m.home.name, s_m.away.name, s_lsEvent);
}

bool parseLsIncidents(rc::ByteSource& src, void*) { return rc::lsParseIncidents(src, net::arena(), s_ls); }

bool parseAthlete(rc::ByteSource& src, void*) {
  return rc::espnParseAthlete(src, net::arena(), s_name, sizeof(s_name));
}

int16_t request(const char* host, net::ParseFn fn, net::FetchStat& total) {
  net::FetchStat st;
  int16_t code = net::get(host, s_path, false, fn, nullptr, st);
  total.bytes += st.bytes;
  total.durMs += st.durMs;
  total.code = code;
  s_bytes += st.bytes;
  s_requests++;
  s_lastCode = code;
  if (code != 200) s_errors++;
  return code;
}

void publish() {
  s_m.fetchedUtc = clk::synced() ? clk::nowUtc() : 0;
  store::putT(store::MATCH, s_m);
}

bool fresh(uint32_t at, uint32_t ms) { return at && millis() - at < ms; }

bool fastFresh() { return fresh(s_fastAt, cfg::FAST_FRESH_MS); }

bool bridgeFresh() { return fresh(s_bridgeAt, cfg::FAST_BRIDGE_MS); }

void fastReset() {
  s_fastEvent = s_m.eventId;
  s_lsEid = 0;
  s_lsSwap = 0;
  s_lsTryMs = 0;
  s_fastAt = 0;
  s_bridgeAt = 0;
}

bool fastScore(uint8_t home, uint8_t away) {
  s_fastHome = home;
  s_fastAway = away;
  s_fastAt = millis() | 1u;
  bool changed = home != s_m.homeScore || away != s_m.awayScore;
  if (home < s_m.homeScore || away < s_m.awayScore) {
    int dropped = rc::matchDropAbove(s_m, home, away);
    diag::note("mac", "hizli kaynak skoru dusurdu: %u-%u (%d gol silindi)", static_cast<unsigned>(home),
               static_cast<unsigned>(away), dropped);
  }
  s_m.homeScore = home;
  s_m.awayScore = away;
  return changed;
}

bool fastGoal(uint8_t kind, bool homeSide, uint8_t home, uint8_t away, const char* minute, const char* player,
              const char* from) {
  bool added = rc::matchAddFast(s_m, kind, homeSide, home, away, minute, player);
  if (added) {
    s_fastGoals++;
    diag::note("mac", "hizli gol (%s): %s %s %u-%u", from, minute, player, static_cast<unsigned>(home),
               static_cast<unsigned>(away));
  }
  return fastScore(home, away) || added;
}

bool live() { return s_m.valid && (s_m.phase == rc::MP_LIVE || s_m.phase == rc::MP_HALF); }

bool drainFast() {
  bool changed = false;
  fast::Msg msg;
  while (fast::take(msg)) {
    if (!s_m.valid || msg.espnId != s_m.eventId) {
      board::fastMsg(msg);
      continue;
    }
    if (s_fastEvent != s_m.eventId) fastReset();
    s_bridgeAt = millis() | 1u;
    if (msg.type == fast::T_GOAL) changed = fastGoal(msg.kind, msg.homeSide, msg.home, msg.away, msg.minute,
                                                     msg.player, "kopru") || changed;
    else changed = fastScore(msg.home, msg.away) || changed;
  }
  return changed;
}

void pollLiveScore(net::FetchStat& st) {
  if (bridgeFresh() || !net::tlsAllowed()) return;
  if (s_fastEvent != s_m.eventId) fastReset();
  if (!s_lsEid) {
    if (s_lsTryMs && static_cast<int32_t>(millis() - s_lsTryMs) < 0) return;
    s_lsTryMs = millis() + cfg::LS_RETRY_S * 1000UL;
    rc::lsLivePath(s_path, sizeof(s_path));
    net::FetchStat one;
    s_lsCode = net::get(rc::LS_HOST, s_path, true, parseLsLive, nullptr, one);
    st.bytes += one.bytes;
    s_lsRequests++;
    if (s_lsCode != 200) {
      s_lsErrors++;
      return;
    }
    s_lsEid = s_lsEvent.eid;
    s_lsSwap = s_lsEvent.swapped;
    diag::note("mac", "livescore eslesti: %lu%s (uyum %u)", static_cast<unsigned long>(s_lsEid),
               s_lsSwap ? " ters" : "", static_cast<unsigned>(s_lsEvent.quality));
  }
  rc::lsIncidentsPath(s_path, sizeof(s_path), s_lsEid);
  net::FetchStat one;
  s_lsCode = net::get(rc::LS_HOST, s_path, true, parseLsIncidents, nullptr, one);
  st.bytes += one.bytes;
  s_lsRequests++;
  if (s_lsCode != 200) {
    s_lsErrors++;
    return;
  }
  for (int i = 0; i < s_ls.n; i++) {
    const rc::LsGoal& g = s_ls.goals[i];
    uint8_t h = s_lsSwap ? g.away : g.home;
    uint8_t a = s_lsSwap ? g.home : g.away;
    bool homeSide = s_lsSwap ? !g.homeSide : g.homeSide;
    fastGoal(g.kind, homeSide, h, a, g.minute, g.player, "kart");
  }
  fastScore(s_lsSwap ? s_ls.away : s_ls.home, s_lsSwap ? s_ls.home : s_ls.away);
}

int16_t pollPlays(net::FetchStat& st) {
  bool rebuilt = false;
  bool caught = false;
  if (!s_m.live) s_catchUp = true;
  int page = s_m.plays ? (s_m.plays - 1) / rc::ESPN_PAGE + 1 : 1;
  for (int k = 0; k < cfg::MATCH_PAGES_PER_RUN; k++) {
    rc::espnPlaysPath(s_path, sizeof(s_path), s_m.eventId, page);
    int16_t code = request(rc::ESPN_CORE_HOST, parsePlays, st);
    if (code != 200) {
      if (s_catchUp) s_m.seedSeq = s_m.seq;
      return code;
    }
    if (s_page.count < s_m.plays && !rebuilt) {
      diag::note("mac", "olay sayisi %u -> %u, liste yeniden kuruluyor", static_cast<unsigned>(s_m.plays),
                 static_cast<unsigned>(s_page.count));
      s_m.plays = 0;
      s_m.n = 0;
      s_m.homeScore = 0;
      s_m.awayScore = 0;
      rebuilt = true;
      s_catchUp = true;
      page = 1;
      continue;
    }
    rc::matchApply(s_m, s_page);
    if (s_page.pageCount <= s_page.pageIndex || s_page.n < rc::ESPN_PAGE) {
      caught = true;
      break;
    }
    page = s_page.pageIndex + 1;
  }
  if (s_catchUp) s_m.seedSeq = s_m.seq;
  s_m.live = 1;
  if (caught) s_catchUp = false;
  return 200;
}

void resolveNames(net::FetchStat& st) {
  for (int k = 0; k < cfg::MATCH_NAMES_PER_RUN; k++) {
    rc::MatchEvent* e = rc::matchFindOff(s_m);
    if (!e) return;
    rc::espnAthletePath(s_path, sizeof(s_path), e->offAthlete);
    e->offAthlete = 0;
    if (request(rc::ESPN_CORE_HOST, parseAthlete, st) == 200) snprintf(e->player2, sizeof(e->player2), "%s", s_name);
  }
}

int16_t pollLive(uint32_t now, net::FetchStat& st) {
  rc::espnStatusPath(s_path, sizeof(s_path), s_m.eventId);
  int16_t code = request(rc::ESPN_CORE_HOST, parseStatus, st);
  if (code != 200) return code;
  memcpy(s_m.clock, s_status.clock, sizeof(s_m.clock));
  uint8_t prev = s_m.phase;
  if (s_status.phase == rc::MP_PRE) {
    s_m.phase = rc::MP_PRE;
    publish();
    return code;
  }
  code = pollPlays(st);
  s_m.phase = s_status.phase;
  if (code == 200) resolveNames(st);
  if (live()) pollLiveScore(st);
  if (fastFresh()) {
    rc::matchDropAbove(s_m, s_fastHome, s_fastAway);
    s_m.homeScore = s_fastHome;
    s_m.awayScore = s_fastAway;
  }
  if (s_m.phase == rc::MP_POST && prev != rc::MP_POST) {
    s_m.endUtc = now;
    diag::note("mac", "bitti: %s %u-%u %s", s_m.home.abbr, static_cast<unsigned>(s_m.homeScore),
               static_cast<unsigned>(s_m.awayScore), s_m.away.abbr);
  }
  publish();
  return code;
}

uint32_t clampPeriod(uint32_t s) {
  if (s < 60) return 60;
  return s > cfg::MATCH_SCHED_S ? cfg::MATCH_SCHED_S : s;
}

uint32_t computePeriod(uint32_t now) {
  if (demoActive()) return 30;
  if (!s_team) return 3600;
  if (!s_m.valid) return cfg::MATCH_SCHED_S;
  if (s_m.phase == rc::MP_LIVE || s_m.phase == rc::MP_HALF) return cfg::MATCH_LIVE_S;
  if (s_m.phase == rc::MP_PRE) {
    if (now + cfg::MATCH_LEAD_S >= s_m.kickoffUtc) return cfg::MATCH_PRE_POLL_S;
    uint32_t until = s_m.kickoffUtc - cfg::MATCH_LEAD_S - now;
    uint32_t sched = s_nextSchedUtc > now ? s_nextSchedUtc - now : 60;
    return clampPeriod(until < sched ? until : sched);
  }
  if (s_m.phase == rc::MP_POST) {
    uint32_t end = s_m.endUtc + cfg::MATCH_AFTER_S;
    return clampPeriod(end > now ? end - now : 0);
  }
  return cfg::MATCH_SCHED_S;
}

void demoBase(uint32_t now, bool pre) {
  rc::matchClear(s_m);
  s_m.valid = 1;
  s_m.eventId = 1;
  s_m.teamId = s_team ? s_team : 436;
  s_m.usHome = 1;
  s_m.league = rc::ML_UCL;
  s_m.home.id = s_m.teamId;
  bool fb = s_m.teamId == 436;
  snprintf(s_m.home.name, sizeof(s_m.home.name), "%s", fb ? "Fenerbahçe" : rc::L("Takımım", "My team"));
  snprintf(s_m.home.abbr, sizeof(s_m.home.abbr), "%s", fb ? "FB" : "TKM");
  s_m.away.id = 362;
  snprintf(s_m.away.name, sizeof(s_m.away.name), "Aston Villa");
  snprintf(s_m.away.abbr, sizeof(s_m.away.abbr), "AVL");
  snprintf(s_m.venue, sizeof(s_m.venue), "Ülker Stadyumu");
  s_demoMin = 21;
  s_m.kickoffUtc = pre ? now + 3u * 3600u : now - 21u * 60u;
  s_m.phase = pre ? rc::MP_PRE : rc::MP_LIVE;
  s_m.live = 1;
  snprintf(s_m.clock, sizeof(s_m.clock), "%u'", static_cast<unsigned>(s_demoMin));
}

void demoEvent(uint8_t kind, bool ours, const char* player, const char* player2) {
  if (s_m.phase != rc::MP_LIVE) s_m.phase = rc::MP_LIVE;
  s_demoMin = static_cast<uint8_t>(s_demoMin + 6 < 90 ? s_demoMin + 6 : 90);
  bool goal = kind == rc::MK_GOAL || kind == rc::MK_PEN_GOAL || kind == rc::MK_OWN_GOAL;
  if (goal && ours == (s_m.usHome == 1)) s_m.homeScore++;
  else if (goal) s_m.awayScore++;
  rc::MatchEvent e;
  memset(&e, 0, sizeof(e));
  e.id = 1000u + s_m.seq;
  e.kind = kind;
  e.ours = ours ? 1 : 0;
  e.home = s_m.homeScore;
  e.away = s_m.awayScore;
  snprintf(e.minute, sizeof(e.minute), "%u'", static_cast<unsigned>(s_demoMin));
  snprintf(e.player, sizeof(e.player), "%s", player);
  snprintf(e.player2, sizeof(e.player2), "%s", player2);
  e.seq = ++s_m.seq;
  if (s_m.n >= rc::MATCH_EVENTS) {
    memmove(&s_m.ev[0], &s_m.ev[1], sizeof(s_m.ev[0]) * (rc::MATCH_EVENTS - 1));
    s_m.n = rc::MATCH_EVENTS - 1;
  }
  s_m.ev[s_m.n++] = e;
  memcpy(s_m.clock, e.minute, sizeof(s_m.clock));
}

void applyDemo() {
  uint8_t cmd = s_demoCmd;
  s_demoCmd = D_NONE;
  if (cmd == D_OFF) {
    s_demoUntil = 0;
    rc::matchClear(s_m);
    s_nextSchedUtc = 0;
    s_team = 0;
    publish();
    return;
  }
  s_demoUntil = (millis() + cfg::MATCH_DEMO_MS) | 1u;
  uint32_t now = clk::synced() ? clk::nowUtc() : 0;
  if (cmd == D_PRE || cmd == D_LIVE || !s_m.valid || s_m.eventId != 1) demoBase(now, cmd == D_PRE);
  switch (cmd) {
    case D_GOAL: {
      static const char* const kScorers[] = {"Kerem Aktürkoğlu", "Mattéo Guendouzi", "Marco Asensio", "Jhon Durán"};
      demoEvent(rc::MK_GOAL, true, kScorers[(s_m.homeScore + s_m.awayScore) % 4], "");
      break;
    }
    case D_OPP: demoEvent(rc::MK_GOAL, false, "Ollie Watkins", ""); break;
    case D_YELLOW: demoEvent(rc::MK_YELLOW, true, "Mason Greenwood", ""); break;
    case D_RED: demoEvent(rc::MK_RED, false, "Ezri Konsa", ""); break;
    case D_SUB: demoEvent(rc::MK_SUB, true, "Kerem Aktürkoğlu", "İrfan Can Kahveci"); break;
    case D_HALF:
      s_m.phase = rc::MP_HALF;
      snprintf(s_m.clock, sizeof(s_m.clock), "45'");
      break;
    case D_CANCEL:
      for (int i = s_m.n - 1; i >= 0; i--) {
        uint8_t k = s_m.ev[i].kind;
        if (k != rc::MK_GOAL && k != rc::MK_PEN_GOAL && k != rc::MK_OWN_GOAL) continue;
        bool home = s_m.ev[i].ours == (s_m.usHome == 1);
        if (home && s_m.homeScore) s_m.homeScore--;
        else if (!home && s_m.awayScore) s_m.awayScore--;
        memmove(&s_m.ev[i], &s_m.ev[i + 1], sizeof(s_m.ev[0]) * static_cast<size_t>(s_m.n - 1 - i));
        s_m.n--;
        break;
      }
      break;
    case D_POST:
      s_m.phase = rc::MP_POST;
      s_m.endUtc = now;
      snprintf(s_m.clock, sizeof(s_m.clock), "90'");
      break;
    default: break;
  }
  publish();
}

}

int16_t fetch(net::FetchStat& st) {
  memset(&st, 0, sizeof(st));
  if (s_demoCmd != D_NONE) applyDemo();
  if (demoActive()) return 0;
  Settings set = settings::get();
  uint32_t team = set.football ? set.teamId : 0;
  if (team != s_team) {
    s_team = team;
    rc::matchClear(s_m);
    s_nextSchedUtc = 0;
    publish();
  }
  if (!team || !clk::synced()) return 0;
  if (drainFast()) publish();
  uint32_t now = clk::nowUtc();
  bool forced = s_refresh;
  s_refresh = false;
  bool live = s_m.valid && (s_m.phase == rc::MP_LIVE || s_m.phase == rc::MP_HALF);
  bool post = s_m.valid && s_m.phase == rc::MP_POST;
  bool window = s_m.valid && s_m.phase == rc::MP_PRE && now + cfg::MATCH_LEAD_S >= s_m.kickoffUtc;
  bool stale = s_m.valid && now > s_m.kickoffUtc + cfg::MATCH_MAX_S + cfg::MATCH_AFTER_S;
  if ((live || window) && !stale) return pollLive(now, st);
  bool due = !s_m.valid || forced || stale || (post ? now >= s_m.endUtc + cfg::MATCH_AFTER_S : now >= s_nextSchedUtc);
  if (!due) return 0;
  rc::espnSchedulePath(s_path, sizeof(s_path), team);
  int16_t code = request(rc::ESPN_SITE_HOST, parseSchedule, st);
  if (code == 200) {
    s_nextSchedUtc = now + cfg::MATCH_SCHED_S;
    publish();
  }
  return code;
}

uint32_t periodS() { return computePeriod(clk::synced() ? clk::nowUtc() : 0); }

void refresh() {
  s_refresh = true;
  net::refresh(SRC_MATCH);
}

bool demo(const char* what) {
  if (!strncmp(what, "gol ", 4) && what[4] >= '1' && what[4] <= '0' + goal_screen::V_COUNT && !what[5]) {
    match_ui::forceGoalVariant(static_cast<int8_t>(what[4] - '1'));
    s_demoCmd = D_GOAL;
    net::refresh(SRC_MATCH);
    return true;
  }
  static const struct {
    const char* name;
    uint8_t cmd;
  } kCmds[] = {{"once", D_PRE},      {"basla", D_LIVE}, {"gol", D_GOAL},   {"rakip", D_OPP},
               {"sari", D_YELLOW},   {"kirmizi", D_RED}, {"degisiklik", D_SUB}, {"devre", D_HALF},
               {"bitti", D_POST},    {"iptal", D_CANCEL}, {"kapat", D_OFF}};
  for (const auto& c : kCmds) {
    if (!strcmp(what, c.name)) {
      s_demoCmd = c.cmd;
      net::refresh(SRC_MATCH);
      return true;
    }
  }
  return false;
}

void report(Print& out) {
  static const char* const kPhase[] = {"yok", "once", "canli", "devre", "bitti"};
  const rc::Match& m = s_m;
  Settings set = settings::get();
  uint32_t team = set.teamId;
  if (!set.football && !demoActive()) {
    out.println("[mac] mac takibi kapali (futbol ac)");
    return;
  }
  if (!team && !demoActive()) {
    out.println("[mac] takim secili degil (uygulamadan Futbol ya da takim <espn-no>)");
    return;
  }
  uint32_t now = clk::synced() ? clk::nowUtc() : 0;
  long kick = m.kickoffUtc && now ? static_cast<long>(m.kickoffUtc) - static_cast<long>(now) : 0;
  out.printf("[mac] takim %lu%s · %s · %s %u-%u %s · %s · baslama %+ld dk · olay %u · istek %lu · hata %lu · %lu B · "
             "son kod %d · sonraki %lu sn\n",
             static_cast<unsigned long>(team), demoActive() ? " (deneme)" : "",
             m.valid ? kPhase[m.phase < 5 ? m.phase : 0] : "mac yok", m.home.abbr,
             static_cast<unsigned>(m.homeScore), static_cast<unsigned>(m.awayScore), m.away.abbr, m.clock, kick / 60,
             static_cast<unsigned>(m.n), static_cast<unsigned long>(s_requests), static_cast<unsigned long>(s_errors),
             static_cast<unsigned long>(s_bytes), static_cast<int>(s_lastCode),
             static_cast<unsigned long>(computePeriod(now)));
  out.printf("[hizli] kaynak %s · livescore %lu%s · istek %lu · hata %lu · son kod %d · hizli gol %lu · kopru mesaji %lu\n",
             bridgeFresh() ? "kopru" : (s_lsEid && fastFresh()) ? "kart" : "yok", static_cast<unsigned long>(s_lsEid),
             s_lsSwap ? " (ters)" : "", static_cast<unsigned long>(s_lsRequests), static_cast<unsigned long>(s_lsErrors),
             static_cast<int>(s_lsCode), static_cast<unsigned long>(s_fastGoals),
             static_cast<unsigned long>(fast::received()));
}

}
