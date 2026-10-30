#include <stdio.h>
#include <string.h>

#include "agents/agents.h"
#include "agents_ui.h"
#include "app/config.h"
#include "draw_util.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"
#include "text_tr.h"

namespace scr_agents {

namespace {

using theme::S;

constexpr int32_t TITLE_H = S(34);
constexpr int32_t ROW_H = S(34);
constexpr int32_t BEACON_X = S(20);
constexpr int32_t BEACON_R = S(9);

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_title = nullptr;
lv_obj_t* s_list = nullptr;
lv_timer_t* s_timer = nullptr;
uint8_t s_top = rc::AG_NONE;

lv_color_t C(uint32_t hex) { return theme::col(hex); }

void coords(lv_event_t* e, lv_area_t& c) { lv_obj_get_coords(lv_event_get_current_target_obj(e), &c); }

void drawTitle(lv_event_t* e) {
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  coords(e, c);
  const rc::Agents& a = agents::state();
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fs = theme::fontS();
  uint8_t top = rc::agentsTop(a);
  agents_ui::drawBeacon(layer, c.x1 + BEACON_X, c.y1 + TITLE_H / 2, BEACON_R, top, lv_tick_get());
  du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, c.x1 + S(38), c.y1 + (TITLE_H - lv_font_get_line_height(fm)) / 2,
           c.x2, rc::L("Kod ajanları", "Coding agents"));
  char sum[64];
  agents_ui::summary(a, sum, sizeof(sum));
  du::text(layer, fs, C(agents_ui::color(top)), LV_TEXT_ALIGN_RIGHT, c.x1 + S(150),
           c.y1 + (TITLE_H - lv_font_get_line_height(fs)) / 2, c.x2 - S(12), sum);
}

void drawEmpty(lv_layer_t* layer, const lv_area_t& c, const char* head, const char* hint) {
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fs = theme::fontS();
  int32_t cy = c.y1 + S(60);
  agents_ui::drawBeacon(layer, (c.x1 + c.x2) / 2, cy, S(22), rc::AG_NONE, 0);
  du::text(layer, fm, C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, cy + S(40), c.x2, head);
  du::text(layer, fs, C(theme::C_TEXT3), LV_TEXT_ALIGN_CENTER, c.x1 + S(24), cy + S(66), c.x2 - S(24), hint);
}

void drawList(lv_event_t* e) {
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  coords(e, c);
  const rc::Agents& a = agents::state();
  if (!a.valid) {
    drawEmpty(layer, c, rc::L("Köprü bağlı değil", "Bridge not connected"), rc::L("Bilgisayarda köprü çalışınca görünür", "Appears when the bridge runs on your computer"));
    return;
  }
  if (!a.n) {
    drawEmpty(layer, c, rc::L("Açık ajan oturumu yok", "No open agent sessions"), rc::L("Claude ya da Codex'te bir iş başlatınca görünür", "Appears when you start a task in Claude or Codex"));
    return;
  }
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fs = theme::fontS();
  char text[96], fitted[96];
  for (int i = 0; i < a.n; i++) {
    const rc::AgentRow& r = a.rows[i];
    int32_t y = c.y1 + i * ROW_H;
    if (!du::visible(layer, y, y + ROW_H)) continue;
    du::box(layer, c.x1 + S(8), y, c.x2 - S(8), y, C(theme::C_SURFACE2), LV_OPA_COVER, 0);
    agents_ui::drawBeacon(layer, c.x1 + BEACON_X, y + ROW_H / 2, S(6), r.status, 0);
    int32_t tx = c.x1 + S(38);
    int32_t rx = c.x2 - S(12);
    const char* who = rc::agentName(r.agent);
    du::text(layer, fs, C(theme::C_TEXT3), LV_TEXT_ALIGN_RIGHT, tx, y + S(4), rx, who);
    int32_t px = rx - du::width(fs, who) - S(8);
    du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, tx, y + S(1), px,
             du::fit(fm, r.project, px - tx, fitted, sizeof(fitted)));
    agents_ui::lineText(r, text, sizeof(text));
    uint32_t col = r.status == rc::AG_WAIT ? theme::C_NEON_RED : r.status == rc::AG_WORK ? theme::C_TEXT2 : theme::C_TEXT3;
    du::text(layer, fs, C(col), LV_TEXT_ALIGN_LEFT, tx, y + S(19), rx, du::fit(fs, text, rx - tx, fitted, sizeof(fitted)));
  }
  int more = a.work + a.wait + a.idle - a.n;
  if (more > 0) {
    snprintf(text, sizeof(text), rc::L("+%d oturum daha", "+%d more sessions"), more);
    du::text(layer, fs, C(theme::C_TEXT3), LV_TEXT_ALIGN_RIGHT, c.x1, c.y1 + a.n * ROW_H + S(4), c.x2 - S(12), text);
  }
}

void beaconArea(lv_area_t& out) {
  lv_area_t c;
  lv_obj_get_coords(s_title, &c);
  int32_t g = BEACON_R + S(5);
  out = {c.x1 + BEACON_X - g, c.y1 + TITLE_H / 2 - g, c.x1 + BEACON_X + g, c.y1 + TITLE_H / 2 + g};
}

void frame(lv_timer_t* t) {
  (void)t;
  if (!s_title || !agents_ui::animated(s_top)) return;
  lv_area_t a;
  beaconArea(a);
  lv_obj_invalidate_area(s_title, &a);
}

void deleted(lv_event_t* e) {
  (void)e;
  if (s_timer) lv_timer_delete(s_timer);
  s_timer = nullptr;
  s_scr = s_title = s_list = nullptr;
}

lv_obj_t* drawObj(int32_t y, int32_t h, lv_event_cb_t draw) {
  lv_obj_t* o = theme::makeBox(s_scr);
  lv_obj_set_pos(o, 0, y);
  lv_obj_set_size(o, theme::SW, h);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(o, draw, LV_EVENT_DRAW_MAIN, nullptr);
  return o;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  ui::attachGestures(s_scr);
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  s_title = drawObj(0, TITLE_H, drawTitle);
  s_list = drawObj(TITLE_H, theme::SH - TITLE_H, drawList);
  s_top = rc::agentsTop(agents::state());
  s_timer = lv_timer_create(frame, cfg::AGENT_FRAME_MS, nullptr);
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & ui::M_AGENTS) {
    s_top = rc::agentsTop(agents::state());
    lv_obj_invalidate(s_title);
    lv_obj_invalidate(s_list);
  } else if ((mask & ui::M_SECOND) && agents::state().n) {
    lv_obj_invalidate(s_list);
  }
}

}
