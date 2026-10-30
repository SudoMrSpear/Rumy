#include "agents_ui.h"

#include <stdio.h>
#include <string.h>

#include "agents/agents.h"
#include "app/config.h"
#include "bench.h"
#include "draw_util.h"
#include "match_ui.h"
#include "rumy/rumy.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"
#include "wiz/wiz.h"
#include "text_tr.h"

namespace agents_ui {

namespace {

using theme::FW;
using theme::S;

constexpr int32_t TOAST_X = S(3);
constexpr int32_t TOAST_Y = S(2);
constexpr int32_t TOAST_W = FW(314);
constexpr int32_t TOAST_H = S(68);
constexpr uint32_t TOAST_IN_MS = 300;
constexpr uint32_t TOAST_OUT_MS = 220;
constexpr int QUEUE = 4;
constexpr uint32_t NOTE_GAP_MS = 60000;
constexpr uint32_t ALERT_GAP_MS = 30000;

rc::Agents s_prev;
rc::AgentNote s_q[QUEUE];
int s_qn = 0;
rc::AgentNote s_note;
lv_obj_t* s_box = nullptr;
lv_obj_t* s_out = nullptr;
uint32_t s_until = 0;
uint8_t s_react = rumy::NORMAL;
uint32_t s_reactUntil = 0;
uint32_t s_gapKey[rc::AGENT_ROWS];
uint32_t s_gapAt[rc::AGENT_ROWS];
int s_gapNext = 0;
uint32_t s_alertAt = 0;

lv_color_t C(uint32_t hex) { return theme::col(hex); }

void line(lv_layer_t* layer, int32_t x1, int32_t y1, int32_t x2, int32_t y2, lv_color_t c, int32_t w) {
  lv_draw_line_dsc_t d;
  lv_draw_line_dsc_init(&d);
  d.p1.x = x1;
  d.p1.y = y1;
  d.p2.x = x2;
  d.p2.y = y2;
  d.color = c;
  d.width = w;
  d.round_start = 1;
  d.round_end = 1;
  lv_draw_line(layer, &d);
}

int32_t wave(uint32_t ms, uint32_t period, int32_t lo, int32_t hi) {
  int32_t s = lv_trigo_sin(static_cast<int16_t>((ms % period) * 360U / period));
  return lo + (hi - lo) * (s + LV_TRIGO_SIN_MAX) / (2 * LV_TRIGO_SIN_MAX);
}

void drawToast(lv_event_t* ev) {
  lv_obj_t* obj = lv_event_get_current_target_obj(ev);
  lv_layer_t* layer = lv_event_get_layer(ev);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const rc::AgentRow& r = s_note.row;
  const lv_font_t* fm = theme::fontM();
  bool wait = s_note.kind == rc::AN_WAIT;
  drawBeacon(layer, c.x1 + S(22), c.y1 + S(22), S(11), wait ? rc::AG_WAIT : rc::AG_WORK, 0);
  int32_t tx = c.x1 + S(42);
  int32_t rx = c.x2 - S(12);
  char head[40];
  snprintf(head, sizeof(head), "%s %s", rc::agentName(r.agent), wait ? rc::L("seni bekliyor", "is waiting for you") : rc::L("işini bitirdi", "finished its task"));
  du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, tx, c.y1 + S(8), rx, head);
  const lv_font_t* fs = theme::fontS();
  char fitted[48];
  int32_t px = tx + du::width(fm, head) + S(10);
  du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_RIGHT, px, c.y1 + S(11), rx,
           du::fit(fs, r.project, rx - px, fitted, sizeof(fitted)));
  char tail[56];
  if (wait) {
    snprintf(tail, sizeof(tail), "%s", r.detail[0] ? r.detail : rc::L("Cevabını bekliyor", "Waiting for your answer"));
  } else {
    char span[16];
    rc::agentSpanTr(r.workS, span, sizeof(span));
    snprintf(tail, sizeof(tail), rc::L("%s sürdü", "took %s"), span);
  }
  du::text(layer, fm, C(wait ? color(rc::AG_WAIT) : theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, tx, c.y1 + S(34), rx,
           du::fit(fm, tail, rx - tx, fitted, sizeof(fitted)));
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

int32_t away() { return -(TOAST_Y + TOAST_H + S(6)); }

void outDone(lv_anim_t* a) {
  lv_obj_t* obj = static_cast<lv_obj_t*>(lv_anim_get_user_data(a));
  if (obj == s_out) s_out = nullptr;
  lv_obj_delete_async(obj);
}

void boxClicked(lv_event_t* ev) {
  (void)ev;
  bool wait = s_note.kind == rc::AN_WAIT;
  close();
  if (wait && ui::current() != ui::SCR_AGENTS) ui::go(ui::SCR_AGENTS, -1);
}

void show(const rc::AgentNote& n) {
  s_note = n;
  bool wait = n.kind == rc::AN_WAIT;
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
  lv_obj_set_style_border_color(s_box, C(color(wait ? rc::AG_WAIT : rc::AG_WORK)), 0);
  lv_obj_add_event_cb(s_box, drawToast, LV_EVENT_DRAW_MAIN, nullptr);
  lv_obj_set_style_translate_y(s_box, away(), 0);
  slide(s_box, away(), 0, TOAST_IN_MS, lv_anim_path_ease_out, nullptr);
  s_until = (lv_tick_get() + (wait ? cfg::AGENT_WAIT_TOAST_MS : cfg::AGENT_DONE_TOAST_MS)) | 1u;
  if (wait) {
    scr_home::glance();
    if (!s_alertAt || lv_tick_get() - s_alertAt >= ALERT_GAP_MS) {
      s_alertAt = lv_tick_get() | 1u;
      wiz::alert();
    }
  } else {
    s_react = rumy::HAPPY;
    s_reactUntil = (lv_tick_get() + cfg::AGENT_REACT_MS) | 1u;
  }
}

bool cooled(const rc::AgentNote& n) {
  if (n.kind != rc::AN_WAIT) return true;
  uint32_t now = lv_tick_get();
  for (int i = 0; i < rc::AGENT_ROWS; i++) {
    if (s_gapKey[i] == n.row.key && s_gapAt[i] && now - s_gapAt[i] < NOTE_GAP_MS) return false;
  }
  s_gapKey[s_gapNext] = n.row.key;
  s_gapAt[s_gapNext] = now | 1u;
  s_gapNext = (s_gapNext + 1) % rc::AGENT_ROWS;
  return true;
}

void next() {
  if (s_box || s_out || !s_qn || match_ui::busy()) return;
  rc::AgentNote n = s_q[0];
  s_qn--;
  memmove(&s_q[0], &s_q[1], sizeof(s_q[0]) * static_cast<size_t>(s_qn));
  if (ui::night() && !ui::nightAwake()) return;
  if (bench::active()) return;
  show(n);
}

}

uint32_t color(uint8_t status) {
  switch (status) {
    case rc::AG_WAIT: return theme::C_NEON_RED;
    case rc::AG_WORK: return theme::C_NEON_GREEN;
    case rc::AG_IDLE: return theme::C_NEON_YELLOW;
    default: return theme::C_TEXT3;
  }
}

bool animated(uint8_t status) { return status == rc::AG_WORK || status == rc::AG_WAIT; }

void drawBeacon(lv_layer_t* layer, int32_t cx, int32_t cy, int32_t r, uint8_t status, uint32_t ms) {
  lv_color_t c = C(color(status));
  int32_t rot = 0;
  int32_t scale = 100;
  int32_t glow = status == rc::AG_NONE ? 0 : 36;
  if (status == rc::AG_WORK) {
    rot = static_cast<int32_t>((ms * 60U / 1000U) % 360U);
    scale = wave(ms, 1600, 84, 100);
  } else if (status == rc::AG_WAIT) {
    glow = wave(ms, 1100, 30, 120);
  }
  if (glow) {
    int32_t g = r + S(4);
    du::box(layer, cx - g, cy - g, cx + g, cy + g, c, static_cast<lv_opa_t>(glow / 3), LV_RADIUS_CIRCLE);
    int32_t h = r * 3 / 4;
    du::box(layer, cx - h, cy - h, cx + h, cy + h, c, static_cast<lv_opa_t>(glow / 2), LV_RADIUS_CIRCLE);
  }
  int32_t w = r / 3 < 2 ? 2 : r / 3;
  for (int k = 0; k < 8; k++) {
    int32_t len = (k & 1 ? r * 55 / 100 : r) * scale / 100;
    int16_t a = static_cast<int16_t>((rot + k * 45) % 360);
    int32_t x = cx + len * lv_trigo_cos(a) / LV_TRIGO_SIN_MAX;
    int32_t y = cy + len * lv_trigo_sin(a) / LV_TRIGO_SIN_MAX;
    line(layer, cx, cy, x, y, c, w);
  }
}

void lineText(const rc::AgentRow& r, char* out, size_t cap) {
  char span[16];
  rc::agentSpanTr(agents::spanS(r), span, sizeof(span));
  if (r.status == rc::AG_IDLE) {
    if (r.detail[0]) {
      snprintf(out, cap, rc::L("%s · %s önce", "%s · %s ago"), r.detail, span);
    } else if (r.workS >= 60) {
      char work[16];
      rc::agentSpanTr(r.workS, work, sizeof(work));
      snprintf(out, cap, rc::L("Bitti · %s önce · %s sürdü", "Done · %s ago · took %s"), span, work);
    } else {
      snprintf(out, cap, rc::L("Bitti · %s önce", "Done · %s ago"), span);
    }
    return;
  }
  const char* what = r.detail[0] ? r.detail : r.status == rc::AG_WAIT ? rc::L("Seni bekliyor", "Waiting for you") : rc::L("Çalışıyor", "Working");
  snprintf(out, cap, "%s · %s", what, span);
}

void summary(const rc::Agents& a, char* out, size_t cap) {
  out[0] = 0;
  size_t n = 0;
  const struct {
    uint8_t count;
    const char* word;
  } parts[] = {{a.wait, rc::L("bekliyor", "waiting")}, {a.work, rc::L("çalışıyor", "working")}, {a.idle, rc::L("bitti", "done")}};
  for (const auto& p : parts) {
    if (!p.count || n >= cap) continue;
    n += static_cast<size_t>(snprintf(out + n, cap - n, "%s%u %s", n ? " · " : "", static_cast<unsigned>(p.count), p.word));
  }
}

void changed() {
  const rc::Agents& a = agents::state();
  rc::AgentNote notes[rc::AGENT_ROWS];
  int k = rc::agentsNotes(s_prev, a, cfg::AGENT_DONE_MIN_S, notes, rc::AGENT_ROWS);
  s_prev = a;
  for (int i = 0; i < k; i++) {
    if (!cooled(notes[i])) continue;
    if (s_qn >= QUEUE) {
      memmove(&s_q[0], &s_q[1], sizeof(s_q[0]) * (QUEUE - 1));
      s_qn = QUEUE - 1;
    }
    s_q[s_qn++] = notes[i];
  }
  next();
}

void tick() {
  if (s_box && s_until && static_cast<int32_t>(s_until - lv_tick_get()) <= 0) close();
  next();
}

void close() {
  s_until = 0;
  if (!s_box) return;
  if (s_out) lv_obj_delete(s_out);
  s_out = s_box;
  s_box = nullptr;
  lv_obj_remove_flag(s_out, LV_OBJ_FLAG_CLICKABLE);
  slide(s_out, lv_obj_get_style_translate_y(s_out, LV_PART_MAIN), away(), TOAST_OUT_MS, lv_anim_path_ease_in, outDone);
}

bool busy() { return s_box || s_out; }

bool reaction(uint8_t& mood) {
  if (!s_reactUntil) return false;
  if (static_cast<int32_t>(s_reactUntil - lv_tick_get()) <= 0) {
    s_reactUntil = 0;
    return false;
  }
  mood = s_react;
  return true;
}

}
