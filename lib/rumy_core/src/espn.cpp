#include "espn.h"

#include "espn_board.h"
#include "text_tr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "afad.h"
#include "json_stream.h"
#include "timeutil.h"

namespace rc {

using namespace js;

const char* const ESPN_SITE_HOST = "site.api.espn.com";
const char* const ESPN_CORE_HOST = "sports.core.api.espn.com";

namespace {


struct TeamTr {
  const char* espn;
  const char* name;
  const char* abbr;
};

const TeamTr kTeams[] = {
    {"Fenerbahce", "Fenerbahçe", "FB"},
    {"Galatasaray", "Galatasaray", "GS"},
    {"Besiktas", "Beşiktaş", "BJK"},
    {"Trabzonspor", "Trabzonspor", "TS"},
    {"Istanbul Basaksehir", "Başakşehir", "İBFK"},
    {"Caykur Rizespor", "Çaykur Rizespor", "RİZ"},
    {"Eyupspor", "Eyüpspor", "EYP"},
    {"Genclerbirligi", "Gençlerbirliği", "GNÇ"},
    {"Goztepe", "Göztepe", "GÖZ"},
    {"Kasimpasa", "Kasımpaşa", "KAS"},
    {"Konyaspor", "Konyaspor", "KON"},
    {"Samsunspor", "Samsunspor", "SAM"},
    {"Alanyaspor", "Alanyaspor", "ALN"},
    {"Kocaelispor", "Kocaelispor", "KOC"},
    {"Gaziantep FK", "Gaziantep FK", "GFK"},
    {"Erzurum BB", "Erzurumspor", "ERZ"},
    {"Amed SFK", "Amedspor", "AMD"},
    {"Antalyaspor", "Antalyaspor", "ANT"},
    {"Kayserispor", "Kayserispor", "KAY"},
    {"Sivasspor", "Sivasspor", "SVS"},
    {"Karagumruk", "Karagümrük", "FKG"},
    {"Fatih Karagumruk", "Karagümrük", "FKG"},
    {"Ankaragucu", "Ankaragücü", "ANK"},
    {"Hatayspor", "Hatayspor", "HTY"},
    {"Adana Demirspor", "Adana Demirspor", "ADS"},
    {"Bodrum FK", "Bodrum FK", "BOD"},
    {"Istanbulspor", "İstanbulspor", "İST"},
    {"Pendikspor", "Pendikspor", "PEN"},
};

const char* const kLeagues[] = {"Maç",          "Süper Lig",      "Şampiyonlar Ligi", "Avrupa Ligi", "Konferans Ligi",
                                "Türkiye Kupası", "Süper Kupa",    "Hazırlık maçı",    "Premier Lig", "LaLiga",
                                "Serie A",       "Bundesliga",     "Ligue 1"};
const char* const kLeaguesEn[] = {"Match",          "Süper Lig",  "Champions League", "Europa League", "Conference League",
                                  "Turkish Cup",    "Super Cup",  "Friendly",         "Premier League", "LaLiga",
                                  "Serie A",        "Bundesliga", "Ligue 1"};

const char* const kSlugs[] = {"",      "tur.1", "uefa.champions", "uefa.europa", "uefa.europa.conf", "", "", "",
                              "eng.1", "esp.1", "ita.1",          "ger.1",       "fra.1"};

uint8_t kindOf(JsonObjectConst e) {
  if (e["scoringPlay"] | false) {
    if (e["ownGoal"] | false) return MK_OWN_GOAL;
    if (e["penaltyKick"] | false) return MK_PEN_GOAL;
    return MK_GOAL;
  }
  if (e["redCard"] | false) return MK_RED;
  if (e["yellowCard"] | false) return MK_YELLOW;
  if (e["substitution"] | false) return MK_SUB;
  return MK_NONE;
}

bool isGoal(uint8_t kind) { return kind == MK_GOAL || kind == MK_PEN_GOAL || kind == MK_OWN_GOAL; }

}

void matchClear(Match& m) { memset(&m, 0, sizeof(m)); }

size_t espnSchedulePath(char* out, size_t cap, uint32_t teamId) {
  return fin(snprintf(out, cap, "/apis/site/v2/sports/soccer/all/teams/%lu/schedule?fixture=true",
                      static_cast<unsigned long>(teamId)),
             cap);
}

size_t espnStatusPath(char* out, size_t cap, uint32_t eventId) {
  unsigned long id = eventId;
  return fin(snprintf(out, cap, "/v2/sports/soccer/leagues/all/events/%lu/competitions/%lu/status", id, id), cap);
}

size_t espnPlaysPath(char* out, size_t cap, uint32_t eventId, int page) {
  unsigned long id = eventId;
  return fin(snprintf(out, cap, "/v2/sports/soccer/leagues/all/events/%lu/competitions/%lu/plays?limit=%d&page=%d", id,
                      id, ESPN_PAGE, page),
             cap);
}

size_t espnAthletePath(char* out, size_t cap, uint32_t athleteId) {
  return fin(snprintf(out, cap, "/v2/sports/soccer/athletes/%lu", static_cast<unsigned long>(athleteId)), cap);
}

uint8_t espnLeague(const char* slug) {
  if (!slug) return ML_OTHER;
  if (!strcmp(slug, "tur.1")) return ML_SUPER_LIG;
  if (!strcmp(slug, "uefa.champions")) return ML_UCL;
  if (!strcmp(slug, "uefa.europa")) return ML_UEL;
  if (!strcmp(slug, "uefa.europa.conf")) return ML_UECL;
  for (uint8_t i = ML_EPL; i < ML_COUNT; i++) {
    if (!strcmp(slug, kSlugs[i])) return i;
  }
  if (!strncmp(slug, "tur.", 4) && strstr(slug, "super")) return ML_SUPER_CUP;
  if (!strncmp(slug, "tur.", 4) && strstr(slug, "cup")) return ML_CUP;
  if (strstr(slug, "friendly")) return ML_FRIENDLY;
  return ML_OTHER;
}

const char* espnLeagueSlug(uint8_t league) { return league < ML_COUNT ? kSlugs[league] : ""; }

const char* matchLeagueTr(uint8_t league) {
  const char* const* t = langEn() ? kLeaguesEn : kLeagues;
  return league < sizeof(kLeagues) / sizeof(kLeagues[0]) ? t[league] : t[0];
}

bool espnTeamTurkish(const char* name, MatchSide& out) {
  name = name ? name : "";
  for (const TeamTr& t : kTeams) {
    if (!strcmp(name, t.espn)) {
      copyText(out.name, sizeof(out.name), t.name);
      copyText(out.abbr, sizeof(out.abbr), t.abbr);
      return true;
    }
  }
  return false;
}

void espnTeamTr(const char* name, const char* abbr, MatchSide& out) {
  name = name ? name : "";
  if (espnTeamTurkish(name, out)) return;
  copyText(out.name, sizeof(out.name), name);
  copyText(out.abbr, sizeof(out.abbr), abbr && abbr[0] ? abbr : name);
}

void espnMinute(const char* display, char* out, size_t cap) {
  if (!cap) return;
  size_t k = 0;
  for (const char* p = display ? display : ""; *p && k + 1 < cap; p++) {
    if (*p == '\'' && p[1] == '+') continue;
    out[k++] = *p;
  }
  out[k] = 0;
}

namespace {

void lowerRest(char* w, char* e, char*& o) {
  bool first = true;
  for (char* p = w; p < e;) {
    uint8_t c = static_cast<uint8_t>(p[0]);
    if (c == 0xCC && p + 1 < e && static_cast<uint8_t>(p[1]) == 0x87) {
      p += 2;
      continue;
    }
    size_t n = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
    if (p + n > e) n = static_cast<size_t>(e - p);
    if (!first && n == 1 && c >= 'A' && c <= 'Z') {
      *o++ = static_cast<char>(c + 32);
      p++;
      continue;
    }
    if (!first && n == 2) {
      uint8_t d = static_cast<uint8_t>(p[1]);
      if (c == 0xC4 && d == 0xB0) {
        *o++ = 'i';
        p += 2;
        continue;
      }
      if ((c == 0xC4 || c == 0xC5) && d == 0x9E) d = 0x9F;
      else if (c == 0xC3 && (d == 0x87 || d == 0x96 || d == 0x9C)) d = static_cast<uint8_t>(d + 0x20);
      *o++ = static_cast<char>(c);
      *o++ = static_cast<char>(d);
      p += 2;
      continue;
    }
    memmove(o, p, n);
    o += n;
    p += n;
    first = false;
  }
}

void fixDotted(char* s) {
  char* w = s;
  while (*w) {
    char* e = w;
    while (*e && *e != ' ') e++;
    bool dotted = false;
    for (char* p = w; p + 1 < e; p++) dotted = dotted || (static_cast<uint8_t>(p[0]) == 0xCC && static_cast<uint8_t>(p[1]) == 0x87);
    if (dotted) {
      char* o = w;
      lowerRest(w, e, o);
      memmove(o, e, strlen(e) + 1);
      e = o;
    }
    w = *e ? e + 1 : e;
  }
}

}

void espnPlayer(const char* text, char* out, size_t cap) {
  text = text ? text : "";
  const char* cut = strstr(text, " (");
  utf8Copy(out, cap, text, cut ? static_cast<size_t>(cut - text) : strlen(text));
  fixDotted(out);
}

bool espnParseSchedule(ByteSource& src, JsonArena& arena, uint32_t teamId, uint32_t nowUtc, Match& out) {
  matchClear(out);
  arena.reset();
  JsonDocument filter(&arena);
  filter["id"] = true;
  filter["date"] = true;
  filter["league"]["slug"] = true;
  JsonObject comp = filter["competitions"].add<JsonObject>();
  comp["venue"]["fullName"] = true;
  comp["status"]["type"]["state"] = true;
  JsonObject side = comp["competitors"].add<JsonObject>();
  side["id"] = true;
  side["homeAway"] = true;
  side["team"]["displayName"] = true;
  side["team"]["abbreviation"] = true;
  size_t base = arena.mark();
  JsonDocument doc(&arena);
  TopKeys keys(src);
  char key[24];
  bool ok = false;
  while (keys.next(key, sizeof(key))) {
    if (strcmp(key, "events")) continue;
    ok = eachElement(src, arena, doc, filter, base, [&](JsonObjectConst e) {
      uint32_t kick = 0;
      if (!parseIsoDateTime(e["date"] | "", kick)) return;
      JsonObjectConst c = e["competitions"][0];
      const char* state = c["status"]["type"]["state"] | "pre";
      if (!strcmp(state, "post")) return;
      if (nowUtc && kick + 4u * 3600u < nowUtc) return;
      if (out.valid && kick >= out.kickoffUtc) return;
      uint8_t usHome = 2;
      for (JsonObjectConst s : c["competitors"].as<JsonArrayConst>()) {
        bool home = !strcmp(s["homeAway"] | "", "home");
        if (idOf(s["id"]) == teamId) usHome = home ? 1 : 0;
      }
      if (usHome == 2) return;
      matchClear(out);
      out.valid = 1;
      out.teamId = teamId;
      out.usHome = usHome;
      out.eventId = idOf(e["id"]);
      out.kickoffUtc = kick;
      out.league = espnLeague(e["league"]["slug"] | "");
      out.phase = strcmp(state, "in") ? MP_PRE : MP_LIVE;
      copyText(out.venue, sizeof(out.venue), c["venue"]["fullName"] | "");
      for (JsonObjectConst s : c["competitors"].as<JsonArrayConst>()) {
        MatchSide& m = !strcmp(s["homeAway"] | "", "home") ? out.home : out.away;
        m.id = idOf(s["id"]);
        espnTeamTr(s["team"]["displayName"] | "", s["team"]["abbreviation"] | "", m);
      }
    });
    break;
  }
  doc.clear();
  filter.clear();
  arena.reset();
  return ok;
}

bool espnParseStatus(ByteSource& src, JsonArena& arena, MatchStatus& out) {
  memset(&out, 0, sizeof(out));
  arena.reset();
  JsonDocument filter(&arena);
  filter["displayClock"] = true;
  filter["period"] = true;
  filter["type"]["state"] = true;
  filter["type"]["name"] = true;
  JsonDocument doc(&arena);
  bool ok = !deserializeJson(doc, src, DeserializationOption::Filter(filter));
  if (ok) {
    const char* state = doc["type"]["state"] | "";
    const char* name = doc["type"]["name"] | "";
    if (!strcmp(state, "post")) out.phase = MP_POST;
    else if (!strcmp(state, "in")) out.phase = strstr(name, "HALFTIME") ? MP_HALF : MP_LIVE;
    else if (!strcmp(state, "pre")) out.phase = MP_PRE;
    else ok = false;
    out.period = doc["period"] | 0;
    espnMinute(doc["displayClock"] | "", out.clock, sizeof(out.clock));
  }
  doc.clear();
  filter.clear();
  arena.reset();
  return ok;
}

bool espnParsePlays(ByteSource& src, JsonArena& arena, EspnPage& out) {
  memset(&out, 0, sizeof(out));
  arena.reset();
  JsonDocument filter(&arena);
  filter["id"] = true;
  filter["text"] = true;
  filter["clock"]["displayValue"] = true;
  filter["team"]["$ref"] = true;
  filter["homeScore"] = true;
  filter["awayScore"] = true;
  filter["scoringPlay"] = true;
  filter["ownGoal"] = true;
  filter["penaltyKick"] = true;
  filter["redCard"] = true;
  filter["yellowCard"] = true;
  filter["substitution"] = true;
  filter["participants"].add<JsonObject>()["athlete"]["$ref"] = true;
  size_t base = arena.mark();
  JsonDocument doc(&arena);
  TopKeys keys(src);
  char key[24];
  bool items = false;
  bool ok = true;
  while (ok && keys.next(key, sizeof(key))) {
    if (!strcmp(key, "count")) {
      out.count = static_cast<uint16_t>(readUint(src));
    } else if (!strcmp(key, "pageIndex")) {
      out.pageIndex = static_cast<uint8_t>(readUint(src));
    } else if (!strcmp(key, "pageCount")) {
      out.pageCount = static_cast<uint8_t>(readUint(src));
    } else if (!strcmp(key, "items")) {
      items = true;
      ok = eachElement(src, arena, doc, filter, base, [&](JsonObjectConst e) {
        if (out.n >= ESPN_PAGE) return;
        EspnPlay& p = out.items[out.n++];
        memset(&p, 0, sizeof(p));
        p.id = idOf(e["id"]);
        p.kind = kindOf(e);
        p.teamId = idAfter(e["team"]["$ref"] | "", "/teams/");
        p.home = e["homeScore"] | 0;
        p.away = e["awayScore"] | 0;
        espnMinute(e["clock"]["displayValue"] | "", p.minute, sizeof(p.minute));
        espnPlayer(e["text"] | "", p.player, sizeof(p.player));
        if (p.kind == MK_SUB) p.offAthlete = idAfter(e["participants"][1]["athlete"]["$ref"] | "", "/athletes/");
      });
    }
  }
  doc.clear();
  filter.clear();
  arena.reset();
  return ok && items;
}

bool espnParseAthlete(ByteSource& src, JsonArena& arena, char* out, size_t cap) {
  if (cap) out[0] = 0;
  arena.reset();
  JsonDocument filter(&arena);
  filter["displayName"] = true;
  JsonDocument doc(&arena);
  bool ok = !deserializeJson(doc, src, DeserializationOption::Filter(filter));
  if (ok) copyText(out, cap, doc["displayName"] | "");
  doc.clear();
  filter.clear();
  arena.reset();
  return ok && cap && out[0];
}

int matchApply(Match& m, const EspnPage& page) {
  int added = 0;
  int base = (page.pageIndex ? page.pageIndex - 1 : 0) * ESPN_PAGE;
  for (int i = 0; i < page.n; i++) {
    int idx = base + i;
    if (idx < m.plays) continue;
    const EspnPlay& p = page.items[i];
    m.plays = static_cast<uint16_t>(idx + 1);
    uint8_t prevHome = m.homeScore;
    uint8_t prevAway = m.awayScore;
    m.homeScore = p.home;
    m.awayScore = p.away;
    if (p.kind == MK_NONE) continue;
    bool dup = false;
    for (int k = 0; k < m.n; k++) dup = dup || m.ev[k].id == p.id;
    if (dup) continue;
    MatchEvent* fast = isGoal(p.kind) ? matchFindGoal(m, p.home, p.away) : nullptr;
    if (fast && (fast->id & MATCH_FAST_ID) == MATCH_FAST_ID) {
      fast->id = p.id;
      fast->kind = p.kind;
      if (p.player[0]) copyText(fast->player, sizeof(fast->player), p.player);
      continue;
    }
    MatchEvent e;
    memset(&e, 0, sizeof(e));
    e.id = p.id;
    e.kind = p.kind;
    e.home = p.home;
    e.away = p.away;
    e.offAthlete = p.offAthlete;
    copyText(e.minute, sizeof(e.minute), p.minute);
    copyText(e.player, sizeof(e.player), p.player);
    bool homeUp = p.home > prevHome;
    bool awayUp = p.away > prevAway;
    if (isGoal(p.kind) && (homeUp || awayUp)) e.ours = (homeUp ? m.usHome == 1 : m.usHome == 0) ? 1 : 0;
    else if (isGoal(p.kind)) e.ours = ((p.teamId == m.teamId) != (p.kind == MK_OWN_GOAL)) ? 1 : 0;
    else e.ours = p.teamId == m.teamId ? 1 : 0;
    e.seq = ++m.seq;
    if (m.n >= MATCH_EVENTS) {
      memmove(&m.ev[0], &m.ev[1], sizeof(m.ev[0]) * (MATCH_EVENTS - 1));
      m.n = MATCH_EVENTS - 1;
    }
    m.ev[m.n++] = e;
    added++;
  }
  return added;
}

MatchEvent* matchFindGoal(Match& m, uint8_t home, uint8_t away) {
  for (int i = 0; i < m.n; i++) {
    if (isGoal(m.ev[i].kind) && m.ev[i].home == home && m.ev[i].away == away) return &m.ev[i];
  }
  return nullptr;
}

bool matchAddFast(Match& m, uint8_t kind, bool homeSide, uint8_t home, uint8_t away, const char* minute,
                  const char* player) {
  if (!isGoal(kind)) return false;
  MatchEvent* old = matchFindGoal(m, home, away);
  if (old) {
    if (!old->player[0] && player && player[0]) copyText(old->player, sizeof(old->player), player);
    return false;
  }
  MatchEvent e;
  memset(&e, 0, sizeof(e));
  e.id = MATCH_FAST_ID | (static_cast<uint32_t>(home) << 8) | away;
  e.kind = kind;
  e.home = home;
  e.away = away;
  e.ours = homeSide == (m.usHome == 1) ? 1 : 0;
  copyText(e.minute, sizeof(e.minute), minute);
  copyText(e.player, sizeof(e.player), player);
  e.seq = ++m.seq;
  if (m.n >= MATCH_EVENTS) {
    memmove(&m.ev[0], &m.ev[1], sizeof(m.ev[0]) * (MATCH_EVENTS - 1));
    m.n = MATCH_EVENTS - 1;
  }
  m.ev[m.n++] = e;
  m.homeScore = home;
  m.awayScore = away;
  return true;
}

int matchDropAbove(Match& m, uint8_t home, uint8_t away) {
  int dropped = 0;
  for (int i = m.n - 1; i >= 0; i--) {
    const MatchEvent& e = m.ev[i];
    if (!isGoal(e.kind) || (e.home <= home && e.away <= away)) continue;
    memmove(&m.ev[i], &m.ev[i + 1], sizeof(m.ev[0]) * static_cast<size_t>(m.n - 1 - i));
    m.n--;
    dropped++;
  }
  return dropped;
}

MatchEvent* matchFindOff(Match& m) {
  for (int i = 0; i < m.n; i++) {
    if (m.ev[i].kind == MK_SUB && m.ev[i].offAthlete && !m.ev[i].player2[0]) return &m.ev[i];
  }
  return nullptr;
}

void boardClear(Board& b) { memset(&b, 0, sizeof(b)); }

size_t espnBoardPath(char* out, size_t cap, const char* slug, uint32_t yyyymmdd) {
  if (!yyyymmdd) return fin(snprintf(out, cap, "/apis/site/v2/sports/soccer/%s/scoreboard", slug ? slug : ""), cap);
  return fin(snprintf(out, cap, "/apis/site/v2/sports/soccer/%s/scoreboard?dates=%08lu", slug ? slug : "",
                      static_cast<unsigned long>(yyyymmdd)),
             cap);
}

namespace {

constexpr int BOARD_SCORES = 24;

struct BoardScore {
  uint8_t kind;
  uint8_t own;
  uint8_t homeSide;
  char minute[8];
  char player[MATCH_PLAYER];
};

void boardSide(JsonObjectConst team, char* name, uint32_t& id, char* abbr, bool& turkish) {
  MatchSide tr;
  memset(&tr, 0, sizeof(tr));
  id = idOf(team["id"]);
  turkish = espnTeamTurkish(team["displayName"] | "", tr);
  if (turkish) {
    copyText(name, BOARD_NAME, tr.name);
    copyText(abbr, MATCH_ABBR, tr.abbr);
    return;
  }
  const char* shortName = team["shortDisplayName"] | "";
  copyText(name, BOARD_NAME, shortName[0] ? shortName : (team["displayName"] | ""));
  copyText(abbr, MATCH_ABBR, team["abbreviation"] | "");
}

uint8_t boardPhase(JsonObjectConst status) {
  const char* state = status["type"]["state"] | "";
  const char* name = status["type"]["name"] | "";
  if (!strcmp(state, "post")) return MP_POST;
  if (!strcmp(state, "in")) return strstr(name, "HALFTIME") ? MP_HALF : MP_LIVE;
  return MP_PRE;
}

void boardPush(Board& b, const BoardGoal& g) {
  if (b.ng >= BOARD_GOALS) {
    memmove(&b.goals[0], &b.goals[1], sizeof(b.goals[0]) * (BOARD_GOALS - 1));
    b.ng = BOARD_GOALS - 1;
  }
  b.goals[b.ng++] = g;
}

void boardEvent(Board& b, JsonObjectConst e, uint32_t ourTeam, uint8_t league, bool seeding) {
  uint32_t id = idOf(e["id"]);
  JsonObjectConst c = e["competitions"][0];
  BoardGame* g = nullptr;
  for (int i = 0; i < b.n; i++) {
    if (b.games[i].id == id) g = &b.games[i];
  }
  bool fresh = !g;
  if (fresh) {
    if (b.n >= BOARD_GAMES) return;
    g = &b.games[b.n++];
    memset(g, 0, sizeof(*g));
    g->id = id;
  }
  parseIsoDateTime(e["date"] | "", g->kickoffUtc);
  g->phase = boardPhase(e["status"]);
  espnMinute(e["status"]["displayClock"] | "", g->clock, sizeof(g->clock));
  g->flags = 0;
  g->league = league;
  for (JsonObjectConst s : c["competitors"].as<JsonArrayConst>()) {
    bool home = !strcmp(s["homeAway"] | "", "home");
    bool turkish = false;
    uint32_t sid = 0;
    const char* score = s["score"] | "0";
    if (home) {
      boardSide(s["team"], g->home, sid, g->homeAbbr, turkish);
      g->homeId = sid;
      g->homeScore = static_cast<uint8_t>(atoi(score));
      if (turkish) g->flags |= BF_HOME_TR;
    } else {
      boardSide(s["team"], g->away, sid, g->awayAbbr, turkish);
      g->awayId = sid;
      g->awayScore = static_cast<uint8_t>(atoi(score));
      if (turkish) g->flags |= BF_AWAY_TR;
    }
    if (ourTeam && sid == ourTeam) g->flags |= BF_OURS;
  }
  BoardScore sc[BOARD_SCORES];
  int n = 0;
  int own = 0;
  int home = 0;
  for (JsonObjectConst d : c["details"].as<JsonArrayConst>()) {
    if (!(d["scoringPlay"] | false) || (d["shootout"] | false) || n >= BOARD_SCORES) continue;
    BoardScore& s = sc[n++];
    memset(&s, 0, sizeof(s));
    s.own = (d["ownGoal"] | false) ? 1 : 0;
    s.kind = s.own ? MK_OWN_GOAL : (d["penaltyKick"] | false) ? MK_PEN_GOAL : MK_GOAL;
    s.homeSide = idOf(d["team"]["id"]) == g->homeId ? 1 : 0;
    espnMinute(d["clock"]["displayValue"] | "", s.minute, sizeof(s.minute));
    espnPlayer(d["athletesInvolved"][0]["displayName"] | "", s.player, sizeof(s.player));
    own += s.own;
    home += s.homeSide;
  }
  if (own && n == g->homeScore + g->awayScore && home != g->homeScore) {
    for (int i = 0; i < n; i++) {
      if (sc[i].own) sc[i].homeSide = sc[i].homeSide ? 0 : 1;
    }
  }
  int known = g->goals;
  g->goals = static_cast<uint8_t>(n);
  if (fresh && seeding) return;
  if (n <= known || (g->flags & BF_OURS)) return;
  uint8_t h = 0;
  uint8_t a = 0;
  for (int i = 0; i < n; i++) {
    if (sc[i].homeSide) h++;
    else a++;
    if (i < known) continue;
    BoardGoal goal;
    memset(&goal, 0, sizeof(goal));
    goal.seq = ++b.seq;
    goal.game = static_cast<uint8_t>(g - b.games);
    goal.kind = sc[i].kind;
    goal.home = h;
    goal.away = a;
    if (i == n - 1 && h + a < g->homeScore + g->awayScore) {
      goal.home = g->homeScore;
      goal.away = g->awayScore;
    }
    goal.homeSide = sc[i].homeSide;
    copyText(goal.minute, sizeof(goal.minute), sc[i].minute);
    copyText(goal.player, sizeof(goal.player), sc[i].player);
    boardPush(b, goal);
  }
}

}

bool espnParseBoard(ByteSource& src, JsonArena& arena, uint32_t ourTeam, uint8_t league, Board& io) {
  arena.reset();
  JsonDocument filter(&arena);
  filter["id"] = true;
  filter["date"] = true;
  filter["status"]["displayClock"] = true;
  filter["status"]["type"]["state"] = true;
  filter["status"]["type"]["name"] = true;
  JsonObject comp = filter["competitions"].add<JsonObject>();
  JsonObject side = comp["competitors"].add<JsonObject>();
  side["homeAway"] = true;
  side["score"] = true;
  side["team"]["id"] = true;
  side["team"]["displayName"] = true;
  side["team"]["shortDisplayName"] = true;
  side["team"]["abbreviation"] = true;
  JsonObject det = comp["details"].add<JsonObject>();
  det["scoringPlay"] = true;
  det["ownGoal"] = true;
  det["penaltyKick"] = true;
  det["shootout"] = true;
  det["clock"]["displayValue"] = true;
  det["team"]["id"] = true;
  det["athletesInvolved"].add<JsonObject>()["displayName"] = true;
  size_t base = arena.mark();
  JsonDocument doc(&arena);
  TopKeys keys(src);
  char key[24];
  bool ok = false;
  uint8_t bit = static_cast<uint8_t>(1u << (league & 7u));
  bool seeding = !(io.seeded & bit);
  while (keys.next(key, sizeof(key))) {
    if (strcmp(key, "events")) continue;
    ok = eachElement(src, arena, doc, filter, base, [&](JsonObjectConst e) { boardEvent(io, e, ourTeam, league, seeding); });
    break;
  }
  doc.clear();
  filter.clear();
  arena.reset();
  if (ok) {
    io.valid = 1;
    io.seeded |= bit;
  }
  return ok;
}

int boardCount(const Board& b, uint8_t phase) {
  int k = 0;
  for (int i = 0; i < b.n; i++) k += b.games[i].phase == phase;
  return k;
}

uint32_t boardNextKick(const Board& b, uint32_t nowUtc) {
  uint32_t best = 0;
  for (int i = 0; i < b.n; i++) {
    const BoardGame& g = b.games[i];
    if (g.phase != MP_PRE || !g.kickoffUtc) continue;
    uint32_t k = g.kickoffUtc > nowUtc ? g.kickoffUtc : nowUtc;
    if (!best || k < best) best = k;
  }
  return best;
}

bool boardTurkish(const BoardGame& g) { return (g.flags & (BF_HOME_TR | BF_AWAY_TR)) != 0; }

size_t espnStandingsPath(char* out, size_t cap, const char* slug) {
  return fin(snprintf(out, cap, "/apis/v2/sports/soccer/%s/standings", slug ? slug : ""), cap);
}

namespace {

struct ZoneTr {
  const char* espn;
  const char* tr;
};

const ZoneTr kZones[] = {
    {"Champions League", "Şampiyonlar Ligi"},
    {"Champions League qualifying", "Şampiyonlar Ligi ön eleme"},
    {"Europa League", "Avrupa Ligi"},
    {"Europa League qualifying", "Avrupa Ligi ön eleme"},
    {"Conference League", "Konferans Ligi"},
    {"Conference League qualifying", "Konferans Ligi ön eleme"},
    {"Relegation", "Küme düşme"},
    {"Relegated", "Küme düşme"},
    {"Relegation playoff", "Düşme play-off'u"},
    {"Qualifies for round of 16", "Son 16"},
    {"Knockout phase playoffs - seeded", "Play-off (seri başı)"},
    {"Knockout phase playoffs - unseeded", "Play-off"},
    {"Eliminated", "Elendi"},
};


uint32_t hexColor(const char* s) {
  while (s && *s == '#') s++;
  return s ? static_cast<uint32_t>(strtoul(s, nullptr, 16)) & 0xFFFFFFu : 0;
}

int roundInt(float v) { return static_cast<int>(v < 0 ? v - 0.5f : v + 0.5f); }

uint8_t clamp8(int v) { return static_cast<uint8_t>(v < 0 ? 0 : v > 255 ? 255 : v); }

void standRow(Standings& out, JsonObjectConst e, uint32_t ourTeam) {
  if (out.n >= STAND_ROWS) return;
  StandRow& r = out.rows[out.n++];
  memset(&r, 0, sizeof(r));
  JsonObjectConst team = e["team"];
  r.teamId = idOf(team["id"]);
  MatchSide tr;
  memset(&tr, 0, sizeof(tr));
  if (espnTeamTurkish(team["displayName"] | "", tr)) {
    copyText(r.name, sizeof(r.name), tr.name);
    r.flags |= BF_HOME_TR;
  } else {
    const char* shortName = team["shortDisplayName"] | "";
    copyText(r.name, sizeof(r.name), shortName[0] ? shortName : (team["displayName"] | ""));
  }
  if (ourTeam && r.teamId == ourTeam) r.flags |= BF_OURS;
  for (JsonObjectConst st : e["stats"].as<JsonArrayConst>()) {
    const char* name = st["name"] | "";
    int v = roundInt(st["value"] | 0.0f);
    if (!strcmp(name, "rank")) r.rank = clamp8(v);
    else if (!strcmp(name, "gamesPlayed")) r.played = clamp8(v);
    else if (!strcmp(name, "wins")) r.wins = clamp8(v);
    else if (!strcmp(name, "ties")) r.ties = clamp8(v);
    else if (!strcmp(name, "losses")) r.losses = clamp8(v);
    else if (!strcmp(name, "points")) r.points = clamp8(v);
    else if (!strcmp(name, "pointDifferential")) r.gd = static_cast<int16_t>(v);
  }
  if (!r.rank) r.rank = out.n;
  const char* desc = e["note"]["description"] | "";
  if (!desc[0]) return;
  char text[STAND_ZONE_TEXT];
  espnZoneTr(desc, text, sizeof(text));
  for (int i = 0; i < out.nz; i++) {
    if (!strcmp(out.zones[i].text, text)) {
      r.zone = static_cast<uint8_t>(i + 1);
      return;
    }
  }
  if (out.nz >= STAND_ZONES) return;
  StandZone& z = out.zones[out.nz++];
  z.color = hexColor(e["note"]["color"] | "");
  copyText(z.text, sizeof(z.text), text);
  r.zone = out.nz;
}

}

void espnZoneTr(const char* description, char* out, size_t cap) {
  description = description ? description : "";
  for (const ZoneTr& z : kZones) {
    if (!strcmp(description, z.espn)) {
      copyText(out, cap, langEn() ? z.espn : z.tr);
      return;
    }
  }
  copyText(out, cap, description);
}

bool espnParseStandings(ByteSource& src, JsonArena& arena, uint32_t ourTeam, Standings& out) {
  memset(&out, 0, sizeof(out));
  arena.reset();
  JsonDocument filter(&arena);
  filter["team"]["id"] = true;
  filter["team"]["displayName"] = true;
  filter["team"]["shortDisplayName"] = true;
  filter["note"]["color"] = true;
  filter["note"]["description"] = true;
  JsonObject stat = filter["stats"].add<JsonObject>();
  stat["name"] = true;
  stat["value"] = true;
  size_t base = arena.mark();
  JsonDocument doc(&arena);
  bool ok = seekKey(src, "entries") &&
            eachElement(src, arena, doc, filter, base, [&](JsonObjectConst e) { standRow(out, e, ourTeam); });
  doc.clear();
  filter.clear();
  arena.reset();
  for (int i = 1; i < out.n; i++) {
    StandRow r = out.rows[i];
    int j = i - 1;
    while (j >= 0 && out.rows[j].rank > r.rank) {
      out.rows[j + 1] = out.rows[j];
      j--;
    }
    out.rows[j + 1] = r;
  }
  return ok && out.n > 0;
}

}
