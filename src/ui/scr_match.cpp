#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "draw_util.h"
#include "match_art.h"
#include "match_ui.h"
#include "screens.h"
#include "clock/clock.h"
#include "fonts/symbols.h"
#include "theme.h"
#include "timeutil.h"
#include "ui.h"
#include "text_tr.h"

namespace scr_match {

namespace {

using theme::FW;
using theme::S;

constexpr int32_t HEAD_Y = S(3);
constexpr int32_t HEAD_H = S(20);
constexpr int32_t BOX_X = S(8);
constexpr int32_t BOX_Y = S(24);
constexpr int32_t BOX_W = FW(304);
constexpr int32_t BOX_H = S(59);
constexpr int32_t LIST_Y = S(90);
constexpr int32_t TABLE_Y = S(88);
constexpr int32_t LINE_H = S(20);
constexpr int32_t GRID_HEAD = S(15);
constexpr int32_t GRID_ROW = S(18);
constexpr int32_t ROW_H = S(29);
constexpr int32_t MIN_X = S(44);
constexpr int32_t ICON_X = S(52);
constexpr int32_t NAME_X = S(80);
constexpr int32_t BOARD_GAP = S(10);
constexpr int32_t BOARD_HEAD = S(24);
constexpr int32_t BOARD_ROW = S(24);

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_head = nullptr;
lv_obj_t* s_box = nullptr;
lv_obj_t* s_list = nullptr;
lv_obj_t* s_table = nullptr;
lv_obj_t* s_board = nullptr;
int s_style = 0;

lv_color_t C(uint32_t hex) { return theme::col(hex); }

const rc::Match& M() { return ui::data().match; }

void drawHead(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const rc::Match& m = M();
  char buf[64];
  if (!m.valid) snprintf(buf, sizeof(buf), "%s", rc::L("Maç", "Match"));
  else if (m.venue[0]) snprintf(buf, sizeof(buf), "%s · %s", rc::matchLeagueTr(m.league), m.venue);
  else snprintf(buf, sizeof(buf), "%s", rc::matchLeagueTr(m.league));
  const lv_font_t* fs = theme::fontS();
  int32_t h = lv_area_get_height(&c);
  const lv_image_dsc_t* logo = m.valid ? match_ui::leagueLogo(m.league) : nullptr;
  int32_t gap = logo ? LEAGUE_PX + S(6) : 0;
  char fitted[64];
  const char* txt = du::fit(fs, buf, lv_area_get_width(&c) - gap, fitted, sizeof(fitted));
  int32_t x = c.x1 + (lv_area_get_width(&c) - gap - du::width(fs, txt)) / 2;
  if (logo) du::image(layer, logo, x, c.y1 + (h - LEAGUE_PX) / 2);
  du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x + gap, c.y1 + (h - lv_font_get_line_height(fs)) / 2, c.x2,
           txt);
}

void drawBox(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const rc::Match& m = M();
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fl = theme::fontL();
  if (!m.valid) {
    du::box(layer, c.x1, c.y1, c.x2, c.y2, C(theme::C_SURFACE2), LV_OPA_COVER, S(14));
    int32_t y = c.y1 + (lv_area_get_height(&c) - lv_font_get_line_height(fm)) / 2;
    du::text(layer, fm, C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, y, c.x2, rc::L("Yaklaşan maç bulunamadı", "No upcoming match found"));
    return;
  }
  du::box(layer, c.x1, c.y1, c.x2, c.y2, C(theme::team()), LV_OPA_COVER, S(14));
  du::box(layer, c.x1 + 1, c.y1 + 1, c.x2 - 1, c.y2 - 1, C(theme::teamBg()), LV_OPA_COVER, S(13));
  bool homeOurs = m.usHome == 1;
  int32_t y1 = c.y1 + S(8);
  int32_t y2 = c.y1 + S(30);
  int32_t sx = c.x1 + S(192);
  {
    int32_t nw = sx - S(24) - (c.x1 + S(12));
    char fitted[40];
    du::text(layer, fm, C(homeOurs ? theme::team() : theme::C_TEXT), LV_TEXT_ALIGN_LEFT, c.x1 + S(12), y1,
             sx - S(24), du::fit(fm, m.home.name, nw, fitted, sizeof(fitted)));
    du::text(layer, fm, C(homeOurs ? theme::C_TEXT : theme::team()), LV_TEXT_ALIGN_LEFT, c.x1 + S(12), y2,
             sx - S(24), du::fit(fm, m.away.name, nw, fitted, sizeof(fitted)));
  }
  char buf[24];
  if (m.phase != rc::MP_PRE) {
    snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(m.homeScore));
    du::text(layer, fm, C(homeOurs ? theme::team() : theme::C_TEXT), LV_TEXT_ALIGN_RIGHT, c.x1, y1, sx, buf);
    snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(m.awayScore));
    du::text(layer, fm, C(homeOurs ? theme::C_TEXT : theme::team()), LV_TEXT_ALIGN_RIGHT, c.x1, y2, sx, buf);
  }
  int32_t ly = c.y1 + (lv_area_get_height(&c) - lv_font_get_line_height(fl)) / 2;
  int32_t rx = c.x2 - S(12);
  if (m.phase == rc::MP_LIVE) {
    int32_t dy = c.y1 + lv_area_get_height(&c) / 2;
    du::box(layer, sx + S(22), dy - S(4), sx + S(29), dy + S(3), C(theme::C_NEON_RED), LV_OPA_COVER, LV_RADIUS_CIRCLE);
    du::text(layer, fl, C(theme::C_TEXT), LV_TEXT_ALIGN_RIGHT, c.x1, ly, rx, m.clock);
  } else if (m.phase == rc::MP_HALF) {
    du::text(layer, fl, C(theme::C_TEXT2), LV_TEXT_ALIGN_RIGHT, c.x1, ly, rx, rc::L("İY", "HT"));
  } else if (m.phase == rc::MP_POST) {
    du::text(layer, fl, C(theme::C_TEXT2), LV_TEXT_ALIGN_RIGHT, c.x1, ly, rx, "MS");
  } else {
    match_ui::kickoffText(m, buf, sizeof(buf));
    char* sp = strrchr(buf, ' ');
    if (sp) *sp++ = 0;
    du::text(layer, theme::fontS(), C(theme::C_TEXT2), LV_TEXT_ALIGN_RIGHT, c.x1, y1, rx, buf);
    du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_RIGHT, c.x1, y2, rx, sp ? sp : "");
  }
}

int32_t tableH() {
  const rc::Table& t = M().table;
  if (!M().valid || !t.valid || !s_style) return 0;
  return s_style == 1 ? LINE_H : GRID_HEAD + t.n * GRID_ROW + S(4);
}

void drawTableLine(lv_layer_t* layer, const lv_area_t& c, const rc::Table& t) {
  const lv_font_t* fs = theme::fontS();
  const rc::TableRow& r = t.rows[t.ours];
  char buf[96];
  snprintf(buf, sizeof(buf), rc::L("%u. sıra · %u puan · %uG %uB %uM · averaj %+d", "#%u · %u pts · %uW %uD %uL · GD %+d"), static_cast<unsigned>(r.rank),
           static_cast<unsigned>(r.points), static_cast<unsigned>(t.wins), static_cast<unsigned>(t.ties),
           static_cast<unsigned>(t.losses), static_cast<int>(t.gd));
  int32_t gap = match_ui::leagueLogo(t.league) ? LEAGUE_PX + S(6) : 0;
  char fitted[96];
  const char* txt = du::fit(fs, buf, lv_area_get_width(&c) - gap - S(16), fitted, sizeof(fitted));
  int32_t x = c.x1 + (lv_area_get_width(&c) - gap - du::width(fs, txt)) / 2;
  int32_t h = lv_area_get_height(&c);
  if (gap) du::image(layer, match_ui::leagueLogo(t.league), x, c.y1 + (h - LEAGUE_PX) / 2);
  du::text(layer, fs, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, x + gap, c.y1 + (h - lv_font_get_line_height(fs)) / 2, c.x2,
           txt);
}

void drawTableGrid(lv_layer_t* layer, const lv_area_t& c, const rc::Table& t) {
  const lv_font_t* fs = theme::fontS();
  const lv_font_t* fm = theme::fontM();
  int32_t xRank = c.x1 + S(36);
  int32_t xName = c.x1 + S(46);
  int32_t xPlayed = c.x2 - S(70);
  int32_t xPts = c.x2 - S(20);
  lv_color_t dim = C(theme::C_TEXT3);
  if (match_ui::leagueLogo(t.league)) du::image(layer, match_ui::leagueLogo(t.league), c.x1 + S(10), c.y1 - S(2));
  du::text(layer, fs, dim, LV_TEXT_ALIGN_LEFT, xName, c.y1, xPlayed, rc::matchLeagueTr(t.league));
  du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, c.y1, xPlayed, rc::L("O", "P"));
  du::text(layer, fs, dim, LV_TEXT_ALIGN_RIGHT, c.x1, c.y1, xPts, rc::L("P", "Pts"));
  int32_t ty = (GRID_ROW - lv_font_get_line_height(fm)) / 2;
  for (int i = 0; i < t.n; i++) {
    const rc::TableRow& r = t.rows[i];
    int32_t y = c.y1 + GRID_HEAD + i * GRID_ROW;
    bool ours = i == t.ours;
    if (ours) du::box(layer, c.x1 + S(8), y, c.x2 - S(8), y + GRID_ROW - 1, C(theme::teamBg()), LV_OPA_COVER, S(6));
    lv_color_t col = C(ours ? theme::team() : theme::C_TEXT);
    char buf[8];
    snprintf(buf, sizeof(buf), "%u.", static_cast<unsigned>(r.rank));
    du::text(layer, fm, ours ? col : C(theme::C_TEXT2), LV_TEXT_ALIGN_RIGHT, c.x1, y + ty, xRank, buf);
    char fitted[40];
    du::text(layer, fm, col, LV_TEXT_ALIGN_LEFT, xName, y + ty, xPlayed - S(24),
             du::fit(fm, r.name, xPlayed - S(24) - xName, fitted, sizeof(fitted)));
    snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(r.played));
    du::text(layer, fm, C(theme::C_TEXT2), LV_TEXT_ALIGN_RIGHT, c.x1, y + ty, xPlayed, buf);
    snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(r.points));
    du::text(layer, fm, col, LV_TEXT_ALIGN_RIGHT, c.x1, y + ty, xPts, buf);
  }
}

void drawTable(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const rc::Table& t = M().table;
  if (!tableH()) return;
  if (s_style == 1) drawTableLine(layer, c, t);
  else drawTableGrid(layer, c, t);
}

void drawList(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const rc::Match& m = M();
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fs = theme::fontS();
  if (!m.valid || m.n == 0) {
    char buf[64];
    if (!ui::data().settings.football) snprintf(buf, sizeof(buf), "%s", rc::L("Maç takibi kapalı · uygulamadan açılır", "Match tracking off · enable it in the app"));
    else if (!ui::data().settings.teamId) snprintf(buf, sizeof(buf), "%s", rc::L("Takım seçilmedi · uygulamada Futbol", "No team selected · Football in the app"));
    else if (!m.valid) snprintf(buf, sizeof(buf), "%s", rc::L("ESPN fikstüründe yaklaşan maç yok", "No upcoming match in ESPN fixtures"));
    else if (m.phase == rc::MP_PRE) snprintf(buf, sizeof(buf), "%s", rc::L("Maç başlayınca olaylar burada görünür", "Events appear here once the match starts"));
    else snprintf(buf, sizeof(buf), "%s", rc::L("Henüz gol, kart ya da değişiklik yok", "No goals, cards or subs yet"));
    du::text(layer, fs, C(theme::C_TEXT3), LV_TEXT_ALIGN_CENTER, c.x1, c.y1 + S(12), c.x2, buf);
    return;
  }
  int32_t ty = (ROW_H - lv_font_get_line_height(fm)) / 2;
  for (int k = 0; k < m.n; k++) {
    const rc::MatchEvent& ev = m.ev[m.n - 1 - k];
    int32_t y = c.y1 + k * ROW_H;
    if (!du::visible(layer, y, y + ROW_H)) continue;
    const lv_font_t* fmin = du::width(fm, ev.minute) <= MIN_X - S(4) ? fm : fs;
    int32_t my = y + (ROW_H - lv_font_get_line_height(fmin)) / 2;
    du::text(layer, fmin, C(theme::C_TEXT2), LV_TEXT_ALIGN_RIGHT, c.x1, my, c.x1 + MIN_X, ev.minute);
    match_ui::drawIcon(layer, ev, c.x1 + ICON_X, y, ROW_H);
    char name[2 * rc::MATCH_PLAYER + 4];
    size_t from = 0;
    size_t to = SIZE_MAX;
    if (ev.kind == rc::MK_SUB && ev.player2[0]) {
      snprintf(name, sizeof(name), "%s (%s)", ev.player, ev.player2);
      from = strlen(ev.player) + 2;
      to = strlen(name) - 1;
    } else {
      snprintf(name, sizeof(name), "%s", ev.player);
    }
    char fitted[2 * rc::MATCH_PLAYER + 8];
    du::text(layer, fm, C(ev.ours ? theme::C_TEXT : theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, c.x1 + NAME_X, y + ty,
             c.x2 - S(8), du::fit(fm, name, c.x2 - S(8) - (c.x1 + NAME_X), fitted, sizeof(fitted), from, to));
    du::box(layer, c.x1 + S(10), y + ROW_H - 1, c.x2 - S(10), y + ROW_H - 1, C(theme::C_SURFACE2), LV_OPA_COVER, 0);
  }
}

bool boardShown() {
  const rc::Board& b = ui::data().board;
  return b.valid && b.n && static_cast<int32_t>(b.day) == clk::todayDayNum();
}

int boardLeagues(const rc::Board& b, uint8_t* out, int cap) {
  int k = 0;
  for (int i = 0; i < b.n; i++) {
    bool seen = false;
    for (int j = 0; j < k; j++) seen = seen || out[j] == b.games[i].league;
    if (!seen && k < cap) out[k++] = b.games[i].league;
  }
  return k;
}

int32_t boardH() {
  if (!boardShown()) return 0;
  uint8_t leagues[4];
  int groups = boardLeagues(ui::data().board, leagues, 4);
  return groups * BOARD_HEAD + ui::data().board.n * BOARD_ROW + S(6);
}

void drawBoardHead(lv_layer_t* layer, const lv_area_t& c, int32_t y, const rc::Board& b, uint8_t league) {
  const lv_font_t* fs = theme::fontS();
  int32_t hy = y + (BOARD_HEAD - lv_font_get_line_height(fs)) / 2;
  int32_t lx = c.x1 + S(10);
  if (match_ui::leagueLogo(league)) {
    du::image(layer, match_ui::leagueLogo(league), lx, y + (BOARD_HEAD - LEAGUE_PX) / 2);
    lx += LEAGUE_PX + S(6);
  }
  char buf[48];
  snprintf(buf, sizeof(buf), rc::L("%s · bugün", "%s · today"), rc::matchLeagueTr(league));
  du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, lx, hy, c.x2 - S(70), buf);
  int live = 0;
  int done = 0;
  int all = 0;
  for (int i = 0; i < b.n; i++) {
    const rc::BoardGame& g = b.games[i];
    if (g.league != league) continue;
    all++;
    live += g.phase == rc::MP_LIVE || g.phase == rc::MP_HALF;
    done += g.phase == rc::MP_POST;
  }
  if (live) snprintf(buf, sizeof(buf), rc::L("%d canlı", "%d live"), live);
  else if (done == all) snprintf(buf, sizeof(buf), "bitti");
  else buf[0] = 0;
  du::text(layer, fs, C(live ? theme::C_NEON_RED : theme::C_TEXT3), LV_TEXT_ALIGN_RIGHT, c.x1, hy, c.x2 - S(10), buf);
}

void drawBoard(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  if (!boardShown()) return;
  const rc::Board& b = ui::data().board;
  uint8_t leagues[4];
  int groups = boardLeagues(b, leagues, 4);
  int32_t y = c.y1;
  for (int k = 0; k < groups; k++) {
    if (du::visible(layer, y, y + BOARD_HEAD)) drawBoardHead(layer, c, y, b, leagues[k]);
    y += BOARD_HEAD;
    for (int i = 0; i < b.n; i++) {
      if (b.games[i].league != leagues[k]) continue;
      if (du::visible(layer, y, y + BOARD_ROW)) match_ui::drawGameRow(layer, c.x1, c.x2, y, BOARD_ROW, b.games[i]);
      y += BOARD_ROW;
    }
  }
}

lv_obj_t* drawObj(int32_t x, int32_t y, int32_t w, int32_t h, lv_event_cb_t cb) {
  lv_obj_t* o = theme::makeBox(s_scr);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(o, cb, LV_EVENT_DRAW_MAIN, nullptr);
  return o;
}

void layoutList() {
  const rc::Match& m = M();
  int32_t rows = m.valid && m.n ? m.n : 1;
  int32_t h = rows * ROW_H;
  int32_t th = tableH();
  if (lv_obj_get_height(s_table) != (th ? th : 1)) lv_obj_set_height(s_table, th ? th : 1);
  int32_t y = th ? TABLE_Y + th + S(4) : LIST_Y;
  if (lv_obj_get_y(s_list) != y) lv_obj_set_y(s_list, y);
  if (lv_obj_get_height(s_list) != h) lv_obj_set_height(s_list, h);
  int32_t bh = boardH();
  int32_t by = y + h + BOARD_GAP;
  if (lv_obj_get_y(s_board) != by) lv_obj_set_y(s_board, by);
  if (lv_obj_get_height(s_board) != (bh ? bh : 1)) lv_obj_set_height(s_board, bh ? bh : 1);
}

void deleted(lv_event_t* e) {
  (void)e;
  s_scr = s_head = s_box = s_list = s_table = s_board = nullptr;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  ui::attachGestures(s_scr);
  lv_obj_add_flag(s_scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(s_scr, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(s_scr, LV_SCROLLBAR_MODE_OFF);
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  s_head = drawObj(0, HEAD_Y, theme::SW, HEAD_H, drawHead);
  s_box = drawObj(BOX_X, BOX_Y, BOX_W, BOX_H, drawBox);
  s_table = drawObj(0, TABLE_Y, theme::SW, 1, drawTable);
  s_list = drawObj(0, LIST_Y, theme::SW, ROW_H, drawList);
  s_board = drawObj(0, LIST_Y, theme::SW, 1, drawBoard);
  layoutList();
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & (ui::M_MATCH | ui::M_BOARD | ui::M_MINUTE | ui::M_SETTINGS)) {
    layoutList();
    lv_obj_invalidate(s_board);
    lv_obj_invalidate(s_head);
    lv_obj_invalidate(s_box);
    lv_obj_invalidate(s_list);
    lv_obj_invalidate(s_table);
  }
}

void setTableStyle(int style) { s_style = style; }

}
