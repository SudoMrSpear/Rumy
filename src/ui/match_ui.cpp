#include "match_ui.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "agents_ui.h"
#include "app/config.h"
#include "bench.h"
#include "clock/clock.h"
#include "draw_util.h"
#include "fonts/symbols.h"
#include "goal_screen.h"
#include "match_art.h"
#include "rumy/rumy.h"
#include "text_tr.h"
#include "theme.h"
#include "timeutil.h"
#include "ui.h"
#include "wiz/wiz.h"

namespace match_ui {

namespace {

using theme::CX;
using theme::FW;
using theme::S;

constexpr int32_t TOAST_X = S(3);
constexpr int32_t TOAST_Y = S(2);
constexpr int32_t TOAST_W = FW(314);
constexpr int32_t TOAST_H = S(68);
constexpr uint32_t TOAST_IN_MS = 300;
constexpr uint32_t TOAST_OUT_MS = 220;

uint32_t s_eventId = 0;
uint16_t s_shown = 0;
lv_obj_t* s_box = nullptr;
lv_obj_t* s_out = nullptr;
bool s_goal = false;
uint32_t s_until = 0;
uint8_t s_goals = goal_screen::V_COUNT;
int8_t s_force = -1;
rc::MatchEvent s_ev;

enum Note : uint8_t { N_NONE = 0, N_KICKOFF, N_HALF, N_FULL, N_CANCEL };

uint8_t s_phase = rc::MP_NONE;
uint8_t s_note = N_NONE;
uint8_t s_noteShown = N_NONE;
uint8_t s_react = rumy::NORMAL;
uint32_t s_reactUntil = 0;
uint8_t s_lastHome = 0;
uint8_t s_lastAway = 0;
uint32_t s_dropAt = 0;
bool s_cancelPending = false;
bool s_cancelOurs = false;
uint32_t s_boardDay = 0;
uint16_t s_boardShown = 0;
rc::BoardGoal s_bgoal;
rc::BoardGame s_bgame;

lv_color_t C(uint32_t hex) { return theme::col(hex); }

const rc::Match& M() { return ui::data().match; }

bool eventHome(const rc::Match& m, const rc::MatchEvent& e) { return (e.ours != 0) == (m.usHome == 1); }

void drawToast(lv_event_t* ev) {
  lv_obj_t* obj = lv_event_get_current_target_obj(ev);
  lv_layer_t* layer = lv_event_get_layer(ev);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const rc::Match& m = M();
  const rc::MatchEvent& e = s_ev;
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fs = theme::fontS();
  bool home = eventHome(m, e);
  const rc::MatchSide& team = home ? m.home : m.away;
  lv_color_t teamCol = C(e.ours ? theme::team() : theme::C_TEXT2);
  int32_t ix = c.x1 + S(12);
  int32_t tx = c.x1 + S(40);
  int32_t r = c.x2 - S(12);
  char title[64];
  if (e.kind == rc::MK_SUB) {
    drawIcon(layer, e, ix, c.y1 + S(6), S(20));
    snprintf(title, sizeof(title), rc::L("Oyuncu değişikliği · %s", "Substitution · %s"), e.minute);
    du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, tx, c.y1 + S(5), r, title);
    du::text(layer, fm, C(theme::C_NEON_GREEN), LV_TEXT_ALIGN_LEFT, tx, c.y1 + S(21), tx + S(16), "\xE2\x86\x91");
    du::text(layer, fm, teamCol, LV_TEXT_ALIGN_RIGHT, tx, c.y1 + S(21), r, team.abbr);
    {
      du::Clip in(layer, tx + S(17), c.y1 + S(21), r - S(44), c.y1 + S(41));
      du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, tx + S(17), c.y1 + S(21), r - S(44), e.player);
    }
    if (e.player2[0]) {
      du::text(layer, fs, C(theme::C_NEON_RED), LV_TEXT_ALIGN_LEFT, tx + S(2), c.y1 + S(44), tx + S(16), "\xE2\x86\x93");
      du::Clip out(layer, tx + S(17), c.y1 + S(44), r - S(44), c.y1 + S(62));
      du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, tx + S(17), c.y1 + S(44), r - S(44), e.player2);
    }
    return;
  }
  drawIcon(layer, e, ix, c.y1 + S(8), S(22));
  bool goal = isGoal(e.kind);
  if (e.kind == rc::MK_OWN_GOAL) snprintf(title, sizeof(title), rc::L("Kendi kalesine gol · %s", "Own goal · %s"), e.minute);
  else if (e.kind == rc::MK_PEN_GOAL) snprintf(title, sizeof(title), rc::L("%s penaltı golü · %s", "%s penalty goal · %s"), team.name, e.minute);
  else if (goal) snprintf(title, sizeof(title), rc::L("%s golü · %s", "%s goal · %s"), team.name, e.minute);
  else if (e.kind == rc::MK_RED) snprintf(title, sizeof(title), rc::L("Kırmızı kart · %s", "Red card · %s"), e.minute);
  else snprintf(title, sizeof(title), rc::L("Sarı kart · %s", "Yellow card · %s"), e.minute);
  char fitted[72];
  const char* tail = strstr(title, goal && e.kind != rc::MK_OWN_GOAL ? (e.kind == rc::MK_PEN_GOAL ? rc::L(" penaltı golü", " penalty goal") : rc::L(" golü", " goal")) : " · ");
  du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, tx, c.y1 + S(8), r,
           du::fit(fm, title, r - tx, fitted, sizeof(fitted), 0, tail ? static_cast<size_t>(tail - title) : SIZE_MAX));
  char right[24];
  if (goal) scoreText(m, right, sizeof(right));
  else snprintf(right, sizeof(right), "%s", team.abbr);
  int32_t rw = 0;
  {
    lv_point_t p;
    lv_text_get_size(&p, right, fm, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_EXPAND);
    rw = p.x;
  }
  du::text(layer, fm, goal ? C(theme::C_TEXT) : teamCol, LV_TEXT_ALIGN_RIGHT, tx, c.y1 + S(34), r, right);
  du::Clip clip(layer, tx, c.y1 + S(34), r - rw - S(10), c.y1 + S(56));
  du::text(layer, fm, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, tx, c.y1 + S(34), r - rw - S(10), e.player);
}

int ourGoals(const rc::Match& m) { return m.usHome == 1 ? m.homeScore : m.awayScore; }

int theirGoals(const rc::Match& m) { return m.usHome == 1 ? m.awayScore : m.homeScore; }

void drawNote(lv_event_t* ev) {
  lv_obj_t* obj = lv_event_get_current_target_obj(ev);
  lv_layer_t* layer = lv_event_get_layer(ev);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const rc::Match& m = M();
  const lv_font_t* fm = theme::fontM();
  int32_t tx = c.x1 + S(40);
  int32_t r = c.x2 - S(12);
  drawLeague(layer, m.league, c.x1 + S(11), c.y1 + S(8), S(22), C(theme::team()));
  const char* title = s_noteShown == N_KICKOFF ? rc::L("Maç başladı", "Kick-off")
                      : s_noteShown == N_HALF  ? rc::L("İlk yarı bitti", "Half-time")
                      : s_noteShown == N_CANCEL ? rc::L("Gol iptal edildi", "Goal disallowed")
                                                : rc::L("Maç bitti", "Full time");
  du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, tx, c.y1 + S(8), r, title);
  char score[24];
  scoreText(m, score, sizeof(score));
  int32_t sw = du::width(fm, score);
  du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_RIGHT, tx, c.y1 + S(34), r, score);
  const char* left = rc::matchLeagueTr(m.league);
  uint32_t col = theme::C_TEXT2;
  if (s_noteShown == N_HALF) left = rc::L("Devre arası", "Half-time break");
  char who[40];
  if (s_noteShown == N_CANCEL) {
    bool home = s_cancelOurs == (m.usHome == 1);
    snprintf(who, sizeof(who), rc::L("%s golü", "%s goal"), home ? m.home.name : m.away.name);
    left = who;
    col = s_cancelOurs ? theme::C_TEXT2 : theme::C_NEON_GREEN;
  }
  if (s_noteShown == N_FULL) {
    int d = ourGoals(m) - theirGoals(m);
    left = d > 0 ? rc::L("Kazandık!", "We won!") : d == 0 ? rc::L("Berabere", "Draw") : rc::L("Kaybettik", "We lost");
    if (d > 0) col = theme::team();
  }
  char fitted[48];
  du::text(layer, fm, C(col), LV_TEXT_ALIGN_LEFT, tx, c.y1 + S(34), r - sw - S(10),
           du::fit(fm, left, r - sw - S(10) - tx, fitted, sizeof(fitted)));
}

void drawBoardToast(lv_event_t* ev) {
  lv_obj_t* obj = lv_event_get_current_target_obj(ev);
  lv_layer_t* layer = lv_event_get_layer(ev);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const rc::BoardGoal& g = s_bgoal;
  const rc::BoardGame& gm = s_bgame;
  const lv_font_t* fm = theme::fontM();
  int32_t tx = c.x1 + S(40);
  int32_t r = c.x2 - S(12);
  drawLeague(layer, gm.league, c.x1 + S(11), c.y1 + S(8), S(22), C(theme::C_TEXT2));
  char score[16];
  snprintf(score, sizeof(score), " %u\xE2\x80\x93%u ", static_cast<unsigned>(g.home), static_cast<unsigned>(g.away));
  const char* home = gm.home;
  const char* away = gm.away;
  if (du::width(fm, home) + du::width(fm, score) + du::width(fm, away) > r - tx) {
    home = gm.homeAbbr;
    away = gm.awayAbbr;
  }
  lv_color_t hot = C(theme::C_TEXT);
  lv_color_t dim = C(theme::C_TEXT2);
  int32_t x = tx;
  int32_t y = c.y1 + S(8);
  du::text(layer, fm, g.homeSide ? hot : dim, LV_TEXT_ALIGN_LEFT, x, y, r, home);
  x += du::width(fm, home);
  du::text(layer, fm, hot, LV_TEXT_ALIGN_LEFT, x, y, r, score);
  x += du::width(fm, score);
  du::text(layer, fm, g.homeSide ? dim : hot, LV_TEXT_ALIGN_LEFT, x, y, r, away);
  y = c.y1 + S(34);
  int32_t ballW = du::width(fm, RS_FUTBOL) + S(6);
  du::text(layer, fm, dim, LV_TEXT_ALIGN_LEFT, tx, y, tx + ballW, RS_FUTBOL);
  char minute[24];
  if (g.kind == rc::MK_PEN_GOAL) snprintf(minute, sizeof(minute), "pen. %s", g.minute);
  else if (g.kind == rc::MK_OWN_GOAL) snprintf(minute, sizeof(minute), "k.k. %s", g.minute);
  else snprintf(minute, sizeof(minute), "%s", g.minute);
  int32_t mw = du::width(fm, minute);
  du::text(layer, fm, dim, LV_TEXT_ALIGN_RIGHT, tx, y, r, minute);
  char fitted[40];
  int32_t px = tx + ballW;
  du::text(layer, fm, hot, LV_TEXT_ALIGN_LEFT, px, y, r - mw - S(10),
           du::fit(fm, g.player[0] ? g.player : rc::L("Gol", "Goal"), r - mw - S(10) - px, fitted, sizeof(fitted)));
}

void react(uint8_t mood, uint32_t ms) {
  s_react = mood;
  s_reactUntil = (lv_tick_get() + ms) | 1u;
}

void slideY(void* obj, int32_t v) { lv_obj_set_style_translate_y(static_cast<lv_obj_t*>(obj), v, 0); }

void slide(lv_obj_t* obj, int32_t from, int32_t to, uint32_t ms, lv_anim_path_cb_t path, lv_anim_completed_cb_t done) {
  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, obj);
  lv_anim_set_user_data(&a, obj);
  lv_anim_set_values(&a, from, to);
  lv_anim_set_duration(&a, ms);
  lv_anim_set_path_cb(&a, path);
  lv_anim_set_exec_cb(&a, slideY);
  if (done) lv_anim_set_completed_cb(&a, done);
  lv_anim_start(&a);
}

int32_t toastAway() { return -(TOAST_Y + TOAST_H + S(6)); }

void outDone(lv_anim_t* a) {
  lv_obj_t* obj = static_cast<lv_obj_t*>(lv_anim_get_user_data(a));
  if (obj == s_out) s_out = nullptr;
  lv_obj_delete_async(obj);
}

void toastOut() {
  if (!s_box) return;
  if (s_out) lv_obj_delete(s_out);
  s_out = s_box;
  s_box = nullptr;
  lv_obj_remove_flag(s_out, LV_OBJ_FLAG_CLICKABLE);
  slide(s_out, lv_obj_get_style_translate_y(s_out, LV_PART_MAIN), toastAway(), TOAST_OUT_MS, lv_anim_path_ease_in, outDone);
}

void boxClicked(lv_event_t* ev) {
  (void)ev;
  close();
}

uint32_t borderFor(const rc::MatchEvent& e) {
  if (e.kind == rc::MK_YELLOW) return theme::C_NEON_YELLOW;
  if (e.kind == rc::MK_RED) return theme::C_NEON_RED;
  return theme::C_BORDER;
}

uint8_t goalVariant() {
  if (s_force >= 0) {
    uint8_t f = static_cast<uint8_t>(s_force);
    s_force = -1;
    s_goals = f;
    return f;
  }
  bool first = s_goals >= goal_screen::V_COUNT;
  uint8_t v = static_cast<uint8_t>(lv_rand(0, goal_screen::V_COUNT - (first ? 1 : 2)));
  if (!first && v >= s_goals) v++;
  s_goals = v;
  return v;
}

void makeToast(lv_event_cb_t draw, uint32_t border) {
  s_box = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(s_box);
  lv_obj_add_flag(s_box, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_bg_opa(s_box, LV_OPA_COVER, 0);
  lv_obj_add_event_cb(s_box, boxClicked, LV_EVENT_CLICKED, nullptr);
  lv_obj_set_pos(s_box, TOAST_X, TOAST_Y);
  lv_obj_set_size(s_box, TOAST_W, TOAST_H);
  lv_obj_set_style_bg_color(s_box, C(theme::C_SURFACE2), 0);
  lv_obj_set_style_radius(s_box, S(14), 0);
  lv_obj_set_style_border_width(s_box, S(2), 0);
  lv_obj_set_style_border_color(s_box, C(border), 0);
  lv_obj_add_event_cb(s_box, draw, LV_EVENT_DRAW_MAIN, nullptr);
  lv_obj_set_style_translate_y(s_box, toastAway(), 0);
  slide(s_box, toastAway(), 0, TOAST_IN_MS, lv_anim_path_ease_out, nullptr);
  s_until = (lv_tick_get() + cfg::MATCH_TOAST_MS) | 1u;
}

void showNote(uint8_t note) {
  s_noteShown = note;
  const rc::Match& m = M();
  bool won = note == N_FULL && ourGoals(m) > theirGoals(m);
  uint32_t border = won ? theme::team() : theme::C_BORDER;
  if (note == N_CANCEL) border = s_cancelOurs ? theme::C_NEON_RED : theme::C_NEON_GREEN;
  makeToast(drawNote, border);
}

void show(const rc::MatchEvent& e, bool full) {
  s_ev = e;
  if (full) {
    goal_screen::open(M(), e, goalVariant(), boxClicked);
    wiz::celebrate();
    s_goal = true;
    s_until = (lv_tick_get() + cfg::MATCH_GOAL_MS) | 1u;
    return;
  }
  makeToast(drawToast, borderFor(e));
}

void drop() {
  goal_screen::kill();
  s_goal = false;
  if (s_box) lv_obj_delete(s_box);
  if (s_out) lv_obj_delete(s_out);
  s_box = s_out = nullptr;
  s_until = 0;
}

bool nextMatch(bool sleeping) {
  const rc::Match& m = M();
  if (!m.valid || m.eventId != s_eventId) return false;
  for (int i = 0; i < m.n; i++) {
    const rc::MatchEvent& e = m.ev[i];
    if (e.seq <= s_shown || e.seq <= m.seedSeq) continue;
    s_shown = e.seq;
    if (isGoal(e.kind)) {
      if (e.ours) react(rumy::CELEBRATE, cfg::MATCH_REACT_GOAL_MS);
      else react(rumy::SAD, cfg::MATCH_REACT_OPP_MS);
    }
    if (sleeping || bench::active()) continue;
    bool full = isGoal(e.kind) && e.ours;
    if (!full && ui::current() == ui::SCR_MATCH) continue;
    show(e, full);
    return true;
  }
  if (s_cancelPending) {
    s_cancelPending = false;
    if (!sleeping && !bench::active() && ui::current() != ui::SCR_MATCH) {
      showNote(N_CANCEL);
      return true;
    }
  }
  if (s_note == N_NONE) return false;
  uint8_t note = s_note;
  s_note = N_NONE;
  if (note == N_FULL) {
    int d = ourGoals(m) - theirGoals(m);
    if (d > 0) react(rumy::CELEBRATE, cfg::MATCH_REACT_WIN_MS);
    else if (d < 0) react(rumy::SAD, cfg::MATCH_REACT_LOSS_MS);
    else s_reactUntil = 0;
  }
  if (sleeping || bench::active() || ui::current() == ui::SCR_MATCH) return false;
  showNote(note);
  return true;
}

bool leagueWanted(uint8_t league) {
  const Settings& st = ui::data().settings;
  if (!st.football) return false;
  if (league == rc::ML_SUPER_LIG) return st.goalSl != 0;
  if (league == rc::ML_UCL) return st.goalUcl != 0;
  return true;
}

void nextBoard(bool sleeping) {
  const rc::Board& b = ui::data().board;
  if (!b.valid || b.day != s_boardDay) return;
  for (int i = 0; i < b.ng; i++) {
    const rc::BoardGoal& g = b.goals[i];
    if (g.seq <= s_boardShown) continue;
    s_boardShown = g.seq;
    if (sleeping || bench::active() || g.game >= b.n || !leagueWanted(b.games[g.game].league)) continue;
    s_bgoal = g;
    s_bgame = b.games[g.game];
    makeToast(drawBoardToast, theme::C_BORDER);
    return;
  }
}

void next() {
  if (s_goal && !goal_screen::active()) s_goal = false;
  if (s_box || s_out || s_goal || goal_screen::active() || agents_ui::busy()) return;
  bool sleeping = ui::night() && !ui::nightAwake();
  if (nextMatch(sleeping)) return;
  nextBoard(sleeping);
}

void boardChanged() {
  const rc::Board& b = ui::data().board;
  if (!b.valid || b.day != s_boardDay || b.seq < s_boardShown) {
    s_boardDay = b.day;
    s_boardShown = b.seq;
  }
}

}

bool isGoal(uint8_t kind) { return kind == rc::MK_GOAL || kind == rc::MK_PEN_GOAL || kind == rc::MK_OWN_GOAL; }

int32_t drawIcon(lv_layer_t* layer, const rc::MatchEvent& e, int32_t x, int32_t y, int32_t h) {
  if (e.kind == rc::MK_YELLOW || e.kind == rc::MK_RED) {
    int32_t cy = y + (h - S(14)) / 2;
    du::box(layer, x + S(3), cy, x + S(12), cy + S(14) - 1,
            C(e.kind == rc::MK_RED ? theme::C_NEON_RED : theme::C_NEON_YELLOW), LV_OPA_COVER, S(2));
    return S(18);
  }
  const lv_font_t* f = theme::fontM();
  int32_t ty = y + (h - lv_font_get_line_height(f)) / 2;
  if (e.kind == rc::MK_SUB) {
    du::text(layer, f, C(theme::C_NEON_GREEN), LV_TEXT_ALIGN_LEFT, x, ty, x + S(22), RS_EXCHANGE);
  } else {
    du::text(layer, f, C(e.ours ? theme::team() : theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x, ty, x + S(22), RS_FUTBOL);
  }
  return S(22);
}

const char* side(const rc::Match& m, bool home, bool full) {
  const rc::MatchSide& s = home ? m.home : m.away;
  return full ? s.name : s.abbr;
}

void scoreText(const rc::Match& m, char* out, size_t cap) {
  snprintf(out, cap, "%s %u\xE2\x80\x93%u %s", m.home.abbr, static_cast<unsigned>(m.homeScore),
           static_cast<unsigned>(m.awayScore), m.away.abbr);
}

void kickoffText(const rc::Match& m, char* out, size_t cap) {
  rc::LocalTime lt = rc::toLocal(m.kickoffUtc);
  int32_t days = rc::localDayNum(m.kickoffUtc) - clk::todayDayNum();
  const char* day = days == 0 ? rc::L("Bugün", "Today") : days == 1 ? rc::L("Yarın", "Tomorrow") : rc::dayShortTr(lt.wday);
  snprintf(out, cap, "%s %02u:%02u", day, static_cast<unsigned>(lt.hour), static_cast<unsigned>(lt.minute));
}

void changed() {
  boardChanged();
  const rc::Match& m = M();
  if (!m.valid) {
    drop();
    s_eventId = 0;
    s_shown = 0;
    s_phase = rc::MP_NONE;
    s_note = N_NONE;
    s_reactUntil = 0;
    s_dropAt = 0;
    s_cancelPending = false;
    return;
  }
  if (m.eventId != s_eventId) {
    drop();
    s_eventId = m.eventId;
    s_shown = m.seq;
    s_phase = m.phase;
    s_note = N_NONE;
    s_reactUntil = 0;
    s_lastHome = m.homeScore;
    s_lastAway = m.awayScore;
    s_dropAt = 0;
    s_cancelPending = false;
    return;
  }
  if (m.homeScore < s_lastHome || m.awayScore < s_lastAway) {
    if (!s_dropAt) s_dropAt = lv_tick_get() | 1u;
  } else {
    s_dropAt = 0;
    s_lastHome = m.homeScore;
    s_lastAway = m.awayScore;
  }
  if (m.seq < s_shown) s_shown = m.seq;
  if (m.phase != s_phase) {
    bool wasLive = s_phase == rc::MP_LIVE || s_phase == rc::MP_HALF;
    if (s_phase == rc::MP_PRE && m.phase == rc::MP_LIVE) s_note = N_KICKOFF;
    else if (s_phase == rc::MP_LIVE && m.phase == rc::MP_HALF) s_note = N_HALF;
    else if (wasLive && m.phase == rc::MP_POST) s_note = N_FULL;
    s_phase = m.phase;
  }
  next();
}

void checkCancel() {
  if (!s_dropAt || lv_tick_get() - s_dropAt < cfg::MATCH_CANCEL_CONFIRM_MS) return;
  s_dropAt = 0;
  const rc::Match& m = M();
  if (!m.valid || m.eventId != s_eventId) return;
  bool homeDrop = m.homeScore < s_lastHome;
  bool awayDrop = m.awayScore < s_lastAway;
  if (homeDrop || awayDrop) {
    s_cancelOurs = homeDrop ? m.usHome == 1 : m.usHome != 1;
    s_cancelPending = true;
    react(s_cancelOurs ? rumy::SAD : rumy::HAPPY, cfg::MATCH_REACT_OPP_MS);
  }
  s_lastHome = m.homeScore;
  s_lastAway = m.awayScore;
}

void tick() {
  checkCancel();
  if ((s_box || s_goal) && s_until && static_cast<int32_t>(s_until - lv_tick_get()) <= 0) close();
  next();
}

void close() {
  if (s_goal) goal_screen::leave();
  s_goal = false;
  toastOut();
  s_until = 0;
}

void forceGoalVariant(int8_t variant) { s_force = variant < goal_screen::V_COUNT ? variant : -1; }

bool busy() { return s_box || s_goal || goal_screen::active(); }

bool reaction(uint8_t& mood) {
  if (!s_reactUntil) return false;
  if (static_cast<int32_t>(s_reactUntil - lv_tick_get()) <= 0) {
    s_reactUntil = 0;
    return false;
  }
  mood = s_react;
  return true;
}

void drawGameRow(lv_layer_t* layer, int32_t x1, int32_t x2, int32_t y, int32_t h, const rc::BoardGame& g) {
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fs = theme::fontS();
  int32_t xTime = x1 + S(40);
  int32_t mid = x1 + S(176);
  int32_t half = S(22);
  int32_t ty = (h - lv_font_get_line_height(fm)) / 2;
  bool ours = g.flags & rc::BF_OURS;
  bool live = g.phase == rc::MP_LIVE || g.phase == rc::MP_HALF;
  if (ours) du::box(layer, x1 + S(6), y + 1, x2 - S(6), y + h - 2, C(theme::teamBg()), LV_OPA_COVER, S(6));
  else if (g.league != rc::ML_SUPER_LIG && rc::boardTurkish(g))
    du::box(layer, x1 + S(6), y + 1, x2 - S(6), y + h - 2, C(theme::C_SURFACE2), LV_OPA_COVER, S(6));
  char t[12];
  lv_color_t tc = C(theme::C_TEXT3);
  if (g.phase == rc::MP_PRE) {
    rc::LocalTime lt = rc::toLocal(g.kickoffUtc);
    snprintf(t, sizeof(t), "%02u:%02u", static_cast<unsigned>(lt.hour), static_cast<unsigned>(lt.minute));
  } else if (g.phase == rc::MP_HALF) {
    snprintf(t, sizeof(t), "%s", rc::L("İY", "HT"));
    tc = C(theme::C_TEXT2);
  } else if (g.phase == rc::MP_POST) {
    snprintf(t, sizeof(t), "MS");
  } else {
    snprintf(t, sizeof(t), "%s", g.clock);
    tc = C(theme::C_NEON_RED);
  }
  du::text(layer, fs, tc, LV_TEXT_ALIGN_RIGHT, x1, y + (h - lv_font_get_line_height(fs)) / 2, xTime, t);
  lv_color_t nameCol = C(live || ours ? theme::C_TEXT : theme::C_TEXT2);
  lv_color_t homeCol = ours && g.homeId == ui::data().settings.teamId ? C(theme::team()) : nameCol;
  lv_color_t awayCol = ours && g.awayId == ui::data().settings.teamId ? C(theme::team()) : nameCol;
  char fitted[32];
  int32_t hx1 = xTime + S(6);
  du::text(layer, fm, homeCol, LV_TEXT_ALIGN_RIGHT, hx1, y + ty, mid - half - S(4),
           du::fit(fm, g.home, mid - half - S(4) - hx1, fitted, sizeof(fitted)));
  if (g.phase == rc::MP_PRE) snprintf(t, sizeof(t), "-");
  else snprintf(t, sizeof(t), "%u\xE2\x80\x93%u", static_cast<unsigned>(g.homeScore), static_cast<unsigned>(g.awayScore));
  du::text(layer, fm, C(g.phase == rc::MP_PRE ? theme::C_TEXT3 : theme::C_TEXT), LV_TEXT_ALIGN_CENTER, mid - half, y + ty,
           mid + half, t);
  int32_t ax2 = x2 - S(8);
  du::text(layer, fm, awayCol, LV_TEXT_ALIGN_LEFT, mid + half + S(4), y + ty, ax2,
           du::fit(fm, g.away, ax2 - (mid + half + S(4)), fitted, sizeof(fitted)));
}

const lv_image_dsc_t* leagueLogo(uint8_t league) {
  if (league >= rc::ML_SUPER_LIG && league <= rc::ML_SUPER_CUP) return LEAGUE_IMAGES[league - rc::ML_SUPER_LIG];
  if (league >= rc::ML_EPL && league < rc::ML_COUNT) return LEAGUE_IMAGES[rc::ML_SUPER_CUP + league - rc::ML_EPL];
  return nullptr;
}

int32_t drawLeague(lv_layer_t* layer, uint8_t league, int32_t x, int32_t y, int32_t h, lv_color_t fallback) {
  const lv_image_dsc_t* logo = leagueLogo(league);
  if (logo) {
    du::image(layer, logo, x, y + (h - LEAGUE_PX) / 2);
    return LEAGUE_PX;
  }
  const lv_font_t* f = theme::fontM();
  du::text(layer, f, fallback, LV_TEXT_ALIGN_LEFT, x, y + (h - lv_font_get_line_height(f)) / 2, x + S(22), RS_FUTBOL);
  return du::width(f, RS_FUTBOL);
}

}
