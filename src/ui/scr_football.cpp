#include <stdio.h>
#include <string.h>

#include "clock/clock.h"
#include "draw_util.h"
#include "fonts/symbols.h"
#include "match/leagues.h"
#include "match_art.h"
#include "match_ui.h"
#include "screens.h"
#include "text_tr.h"
#include "theme.h"
#include "timeutil.h"
#include "ui.h"

namespace {

using theme::S;

lv_color_t C(uint32_t hex) { return theme::col(hex); }

void coords(lv_event_t* e, lv_area_t& c) { lv_obj_get_coords(lv_event_get_current_target_obj(e), &c); }

lv_obj_t* drawObj(lv_obj_t* parent, int32_t x, int32_t y, int32_t w, int32_t h, lv_event_cb_t draw,
                  lv_event_cb_t click) {
  lv_obj_t* o = theme::makeBox(parent);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_add_event_cb(o, draw, LV_EVENT_DRAW_MAIN, nullptr);
  if (click) {
    lv_obj_add_flag(o, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(o, click, LV_EVENT_CLICKED, nullptr);
  } else {
    lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
  }
  return o;
}

int32_t clickY(lv_event_t* e, const lv_area_t& c) {
  lv_point_t pt;
  lv_indev_get_point(lv_indev_active(), &pt);
  return pt.y - c.y1;
}

const uint8_t kLeagues[] = {rc::ML_SUPER_LIG, rc::ML_UCL,       rc::ML_EPL,   rc::ML_LALIGA,
                            rc::ML_SERIE_A,   rc::ML_BUNDESLIGA, rc::ML_LIGUE1};
constexpr int LEAGUES = sizeof(kLeagues) / sizeof(kLeagues[0]);

}

namespace scr_football {

namespace {

constexpr int32_t TITLE_H = S(30);
constexpr int32_t ROW_H = S(30);

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_list = nullptr;

void drawTitle(lv_event_t* e) {
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  coords(e, c);
  const lv_font_t* fm = theme::fontM();
  int32_t y = c.y1 + (TITLE_H - lv_font_get_line_height(fm)) / 2;
  du::text(layer, fm, C(theme::team()), LV_TEXT_ALIGN_LEFT, c.x1 + S(12), y, c.x1 + S(36), RS_FUTBOL);
  du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, c.x1 + S(38), y, c.x2, rc::L("Futbol", "Football"));
}

void status(uint8_t league, char* out, size_t cap, uint32_t& color) {
  out[0] = 0;
  color = theme::C_TEXT3;
  const rc::Board& b = ui::data().board;
  if (!b.valid || static_cast<int32_t>(b.day) != clk::todayDayNum()) return;
  int live = 0;
  int all = 0;
  for (int i = 0; i < b.n; i++) {
    if (b.games[i].league != league) continue;
    all++;
    live += b.games[i].phase == rc::MP_LIVE || b.games[i].phase == rc::MP_HALF;
  }
  if (live) {
    snprintf(out, cap, rc::L("%d canlı", "%d live"), live);
    color = theme::C_NEON_RED;
  } else if (all) {
    snprintf(out, cap, rc::L("Bugün %d maç", "%d matches today"), all);
    color = theme::C_TEXT2;
  }
}

void drawList(lv_event_t* e) {
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  coords(e, c);
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fs = theme::fontS();
  for (int i = 0; i < LEAGUES; i++) {
    int32_t y = c.y1 + i * ROW_H;
    if (!du::visible(layer, y, y + ROW_H)) continue;
    du::box(layer, c.x1 + S(8), y, c.x2 - S(8), y, C(theme::C_SURFACE2), LV_OPA_COVER, 0);
    match_ui::drawLeague(layer, kLeagues[i], c.x1 + S(12), y, ROW_H, C(theme::C_TEXT2));
    du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, c.x1 + S(40), y + (ROW_H - lv_font_get_line_height(fm)) / 2,
             c.x2 - S(110), rc::matchLeagueTr(kLeagues[i]));
    char buf[24];
    uint32_t col;
    status(kLeagues[i], buf, sizeof(buf), col);
    int32_t ty = y + (ROW_H - lv_font_get_line_height(fs)) / 2;
    du::text(layer, fs, C(col), LV_TEXT_ALIGN_RIGHT, c.x1, ty, c.x2 - S(28), buf);
    du::text(layer, fs, C(theme::C_TEXT3), LV_TEXT_ALIGN_RIGHT, c.x1, ty, c.x2 - S(12), RS_RIGHT);
  }
}

void listClick(lv_event_t* e) {
  lv_area_t c;
  coords(e, c);
  int32_t y = clickY(e, c);
  if (y < 0) return;
  int i = static_cast<int>(y / ROW_H);
  if (i >= LEAGUES) return;
  leagues::open(kLeagues[i]);
  ui::go(ui::SCR_LEAGUE, 1);
}

void deleted(lv_event_t* e) {
  (void)e;
  s_scr = s_list = nullptr;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  ui::attachGestures(s_scr);
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  drawObj(s_scr, 0, 0, theme::SW, TITLE_H, drawTitle, nullptr);
  s_list = drawObj(s_scr, 0, TITLE_H, theme::SW, theme::SH - TITLE_H, drawList, listClick);
  return s_scr;
}

void update(uint32_t mask) {
  if (s_scr && (mask & (ui::M_BOARD | ui::M_MINUTE))) lv_obj_invalidate(s_list);
}

}

namespace scr_league {

namespace {

constexpr int32_t HEAD_H = S(36);
constexpr int32_t BACK_W = S(200);
constexpr int32_t BTN_W = S(104);
constexpr int32_t BTN_H = S(26);
constexpr int32_t BTN_X = theme::SW - S(8) - BTN_W;
constexpr int32_t BTN_Y = (HEAD_H - BTN_H) / 2;
constexpr int32_t BAR_H = S(26);
constexpr int32_t ARROW_W = S(48);
constexpr int32_t VIEW_Y = HEAD_H + BAR_H;
constexpr int32_t VIEW_H = theme::SH - VIEW_Y;
constexpr int32_t GAME_H = S(24);
constexpr int32_t TROW_H = S(21);
constexpr int32_t ZONE_H = S(18);
constexpr int32_t X_RANK = S(28);
constexpr int32_t X_NAME = S(34);
constexpr int32_t X_O = S(176);
constexpr int32_t X_G = S(198);
constexpr int32_t X_B = S(220);
constexpr int32_t X_M = S(242);
constexpr int32_t X_AV = S(278);
constexpr int32_t X_P = S(310);

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_btn = nullptr;
lv_obj_t* s_bar = nullptr;
lv_obj_t* s_cont = nullptr;
lv_obj_t* s_list = nullptr;
uint8_t s_shownKind = rc::VK_NONE;

const rc::LeagueView& V() { return ui::data().view; }

bool table() { return leagues::tableMode(); }

bool fresh() {
  const rc::LeagueView& v = V();
  return v.league == leagues::league() && v.kind == (table() ? rc::VK_TABLE : rc::VK_DAY) && v.state != rc::VS_LOADING;
}

bool pending() { return V().req != leagues::request(); }

void drawBack(lv_event_t* e) {
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  coords(e, c);
  const lv_font_t* fm = theme::fontM();
  int32_t y = c.y1 + (HEAD_H - lv_font_get_line_height(fm)) / 2;
  du::text(layer, fm, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, c.x1 + S(8), y, c.x1 + S(26), RS_LEFT);
  int32_t w = match_ui::drawLeague(layer, leagues::league(), c.x1 + S(26), c.y1, HEAD_H, C(theme::C_TEXT2));
  char fitted[40];
  int32_t x = c.x1 + S(32) + w;
  du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, x, y, c.x2,
           du::fit(fm, rc::matchLeagueTr(leagues::league()), c.x2 - x, fitted, sizeof(fitted)));
}

void drawButton(lv_event_t* e) {
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  coords(e, c);
  int32_t r = BTN_H / 2;
  du::box(layer, c.x1, c.y1, c.x2, c.y2, C(theme::team()), LV_OPA_COVER, r);
  du::box(layer, c.x1 + 1, c.y1 + 1, c.x2 - 1, c.y2 - 1, C(theme::teamBg()), LV_OPA_COVER, r - 1);
  const lv_font_t* fs = theme::fontS();
  int32_t y = c.y1 + (BTN_H - lv_font_get_line_height(fs)) / 2;
  const char* icon = table() ? RS_FUTBOL : RS_LIST;
  const char* text = table() ? rc::L("Maçlar", "Matches") : rc::L("Puan durumu", "Standings");
  int32_t w = du::width(fs, icon) + S(5) + du::width(fs, text);
  int32_t x = c.x1 + (BTN_W - w) / 2;
  du::text(layer, fs, C(theme::team()), LV_TEXT_ALIGN_LEFT, x, y, c.x2, icon);
  du::text(layer, fs, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, x + du::width(fs, icon) + S(5), y, c.x2, text);
}

void dayLabel(uint32_t ymd, char* out, size_t cap) {
  if (!ymd) {
    snprintf(out, cap, "…");
    return;
  }
  int y = static_cast<int>(ymd / 10000u);
  int m = static_cast<int>(ymd / 100u % 100u);
  int d = static_cast<int>(ymd % 100u);
  int32_t days = rc::daysFromCivil(y, m, d);
  int32_t diff = days - clk::todayDayNum();
  char date[32];
  snprintf(date, sizeof(date), "%d %s", d, rc::monthNameTr(static_cast<uint8_t>(m)));
  if (diff == 0) snprintf(out, cap, rc::L("Bugün · %s", "Today · %s"), date);
  else if (diff == -1) snprintf(out, cap, rc::L("Dün · %s", "Yesterday · %s"), date);
  else if (diff == 1) snprintf(out, cap, rc::L("Yarın · %s", "Tomorrow · %s"), date);
  else snprintf(out, cap, "%s %s", rc::dayShortTr(rc::weekdayFromDays(days)), date);
}

void drawBar(lv_event_t* e) {
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  coords(e, c);
  const lv_font_t* fs = theme::fontS();
  const lv_font_t* fm = theme::fontM();
  du::box(layer, c.x1 + S(8), c.y1, c.x2 - S(8), c.y1, C(theme::C_SURFACE2), LV_OPA_COVER, 0);
  if (table()) {
    lv_color_t dim = C(theme::C_TEXT3);
    int32_t y = c.y1 + (BAR_H - lv_font_get_line_height(fs)) / 2;
    du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, y, c.x1 + X_RANK, "#");
    du::text(layer, fs, dim, LV_TEXT_ALIGN_LEFT, c.x1 + X_NAME, y, c.x1 + X_O - S(24), rc::L("Takım", "Team"));
    du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, y, c.x1 + X_O, rc::L("O", "P"));
    du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, y, c.x1 + X_G, rc::L("G", "W"));
    du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, y, c.x1 + X_B, rc::L("B", "D"));
    du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, y, c.x1 + X_M, rc::L("M", "L"));
    du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, y, c.x1 + X_AV, rc::L("AV", "GD"));
    du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, y, c.x1 + X_P, rc::L("P", "Pts"));
    return;
  }
  int32_t y = c.y1 + (BAR_H - lv_font_get_line_height(fm)) / 2;
  du::text(layer, fm, C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, y, c.x1 + ARROW_W, RS_LEFT);
  du::text(layer, fm, C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x2 - ARROW_W, y, c.x2, RS_RIGHT);
  char label[48];
  dayLabel(V().league == leagues::league() ? V().ymd : 0, label, sizeof(label));
  du::text(layer, fm, C(pending() ? theme::C_TEXT3 : theme::C_TEXT), LV_TEXT_ALIGN_CENTER, c.x1 + ARROW_W, y,
           c.x2 - ARROW_W, label);
}

void drawMessage(lv_layer_t* layer, const lv_area_t& c, const char* text) {
  const lv_font_t* fm = theme::fontM();
  du::text(layer, fm, C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, c.y1 + S(40), c.x2, text);
}

void drawTableRows(lv_layer_t* layer, const lv_area_t& c, const rc::Standings& t) {
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fs = theme::fontS();
  int32_t tm = (TROW_H - lv_font_get_line_height(fm)) / 2;
  int32_t ts = (TROW_H - lv_font_get_line_height(fs)) / 2;
  for (int i = 0; i < t.n; i++) {
    const rc::StandRow& r = t.rows[i];
    int32_t y = c.y1 + i * TROW_H;
    if (!du::visible(layer, y, y + TROW_H)) continue;
    bool ours = r.flags & rc::BF_OURS;
    if (ours) du::box(layer, c.x1 + S(8), y + 1, c.x2 - S(4), y + TROW_H - 2, C(theme::teamBg()), LV_OPA_COVER, S(5));
    if (r.zone && r.zone <= t.nz)
      du::box(layer, c.x1 + S(3), y + S(3), c.x1 + S(5), y + TROW_H - S(4), theme::col(t.zones[r.zone - 1].color),
              LV_OPA_COVER, 1);
    lv_color_t main = C(ours ? theme::team() : theme::C_TEXT);
    lv_color_t dim = C(ours ? theme::C_TEXT : theme::C_TEXT2);
    char buf[12];
    snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(r.rank));
    du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, y + ts, c.x1 + X_RANK, buf);
    char fitted[32];
    du::text(layer, fm, main, LV_TEXT_ALIGN_LEFT, c.x1 + X_NAME, y + tm, c.x1 + X_O - S(22),
             du::fit(fm, r.name, X_O - S(22) - X_NAME, fitted, sizeof(fitted)));
    const struct {
      int32_t x;
      int v;
    } cols[] = {{X_O, r.played}, {X_G, r.wins}, {X_B, r.ties}, {X_M, r.losses}};
    for (const auto& k : cols) {
      snprintf(buf, sizeof(buf), "%d", k.v);
      du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, y + ts, c.x1 + k.x, buf);
    }
    snprintf(buf, sizeof(buf), r.gd > 0 ? "+%d" : "%d", static_cast<int>(r.gd));
    du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, y + ts, c.x1 + X_AV, buf);
    snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(r.points));
    du::text(layer, fm, main, LV_TEXT_ALIGN_RIGHT, c.x1, y + tm, c.x1 + X_P, buf);
  }
  int32_t y = c.y1 + t.n * TROW_H + S(6);
  for (int i = 0; i < t.nz; i++, y += ZONE_H) {
    if (!du::visible(layer, y, y + ZONE_H)) continue;
    int32_t ty = y + (ZONE_H - lv_font_get_line_height(fs)) / 2;
    du::box(layer, c.x1 + S(12), y + S(5), c.x1 + S(20), y + ZONE_H - S(5), theme::col(t.zones[i].color), LV_OPA_COVER,
            S(2));
    du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, c.x1 + S(28), ty, c.x2 - S(8), t.zones[i].text);
  }
}

void drawList(lv_event_t* e) {
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  coords(e, c);
  const rc::LeagueView& v = V();
  if (!fresh()) {
    drawMessage(layer, c, rc::L("Yükleniyor…", "Loading…"));
    return;
  }
  if (v.state == rc::VS_ERROR) {
    drawMessage(layer, c, rc::L("Veri alınamadı, birazdan yeniden denenecek", "Couldn't load data, retrying soon"));
    return;
  }
  if (v.state == rc::VS_EMPTY) {
    drawMessage(layer, c, table() ? rc::L("Puan durumu yok", "No standings") : rc::L("Bu tarihte maç yok", "No matches on this date"));
    return;
  }
  if (table()) {
    drawTableRows(layer, c, v.table);
    return;
  }
  for (int i = 0; i < v.day.n; i++) {
    int32_t y = c.y1 + i * GAME_H;
    if (du::visible(layer, y, y + GAME_H)) match_ui::drawGameRow(layer, c.x1, c.x2, y, GAME_H, v.day.games[i]);
  }
}

void layoutList() {
  const rc::LeagueView& v = V();
  int32_t h = VIEW_H;
  if (fresh() && v.state == rc::VS_OK) {
    if (table()) h = v.table.n * TROW_H + S(6) + v.table.nz * ZONE_H + S(6);
    else h = v.day.n * GAME_H + S(6);
  }
  if (h < VIEW_H) h = VIEW_H;
  if (lv_obj_get_height(s_list) != h) lv_obj_set_height(s_list, h);
  uint8_t kind = table() ? rc::VK_TABLE : rc::VK_DAY;
  if (kind != s_shownKind) {
    s_shownKind = kind;
    lv_obj_scroll_to_y(s_cont, 0, LV_ANIM_OFF);
  }
}

void refresh() {
  layoutList();
  lv_obj_invalidate(s_btn);
  lv_obj_invalidate(s_bar);
  lv_obj_invalidate(s_list);
}

void backClick(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_FOOTBALL, -1);
}

void buttonClick(lv_event_t* e) {
  (void)e;
  leagues::showTable(!table());
  refresh();
}

void barClick(lv_event_t* e) {
  if (table()) return;
  lv_area_t c;
  coords(e, c);
  lv_point_t pt;
  lv_indev_get_point(lv_indev_active(), &pt);
  if (pt.x < c.x1 + ARROW_W * 2) leagues::step(-1);
  else if (pt.x > c.x2 - ARROW_W * 2) leagues::step(1);
  else return;
  refresh();
}

void deleted(lv_event_t* e) {
  (void)e;
  leagues::close();
  s_scr = s_btn = s_bar = s_cont = s_list = nullptr;
  s_shownKind = rc::VK_NONE;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  ui::attachGestures(s_scr);
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  drawObj(s_scr, 0, 0, BACK_W, HEAD_H, drawBack, backClick);
  s_btn = drawObj(s_scr, BTN_X, BTN_Y, BTN_W, BTN_H, drawButton, buttonClick);
  s_bar = drawObj(s_scr, 0, HEAD_H, theme::SW, BAR_H, drawBar, barClick);
  s_cont = theme::makeBox(s_scr);
  lv_obj_set_pos(s_cont, 0, VIEW_Y);
  lv_obj_set_size(s_cont, theme::SW, VIEW_H);
  lv_obj_add_flag(s_cont, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(s_cont, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(s_cont, LV_SCROLLBAR_MODE_OFF);
  s_list = drawObj(s_cont, 0, 0, theme::SW, VIEW_H, drawList, nullptr);
  s_shownKind = table() ? rc::VK_TABLE : rc::VK_DAY;
  layoutList();
  return s_scr;
}

void update(uint32_t mask) {
  if (s_scr && (mask & (ui::M_VIEW | ui::M_MINUTE))) refresh();
}

}
