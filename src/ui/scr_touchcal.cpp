#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>

#include "board/display.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"
#include "text_tr.h"

namespace scr_touchcal {

namespace {

constexpr int POINTS = 5;
constexpr int32_t RING = theme::S(26);
constexpr int32_t ARM = theme::S(44);
constexpr uint32_t TICK_MS = 20;
constexpr uint32_t IDLE_MS = 30000;
constexpr uint32_t DONE_MS = 1800;
constexpr int SKIP_SAMPLES = 3;
constexpr int MAX_SAMPLES = 40;
constexpr int MIN_SAMPLES = 3;
constexpr int32_t VERIFY_TOL = theme::S(14);
const int32_t kTx[POINTS] = {theme::S(26), theme::R(294), theme::R(294), theme::S(26), theme::CX(160)};
const int32_t kTy[POINTS] = {theme::S(26), theme::S(26), theme::S(214), theme::S(214), theme::S(120)};

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_ring = nullptr;
lv_obj_t* s_hline = nullptr;
lv_obj_t* s_vline = nullptr;
lv_obj_t* s_text = nullptr;
lv_timer_t* s_timer = nullptr;
int s_idx = 0;
int s_seen = 0;
int s_samples = 0;
int32_t s_sumX = 0;
int32_t s_sumY = 0;
bool s_wasDown = false;
uint32_t s_idleSince = 0;
uint32_t s_doneAt = 0;
int32_t s_rx[POINTS];
int32_t s_ry[POINTS];

void setMessage(const char* msg) {
  char buf[160];
  snprintf(buf, sizeof(buf), "%s\n%d / %d", msg, s_idx + 1, POINTS);
  lv_label_set_text(s_text, buf);
}

void placeTarget() {
  int32_t x = kTx[s_idx], y = kTy[s_idx];
  lv_obj_set_pos(s_ring, x - RING / 2, y - RING / 2);
  lv_obj_set_pos(s_hline, x - ARM / 2, y);
  lv_obj_set_pos(s_vline, x, y - ARM / 2);
  lv_obj_align(s_text, LV_ALIGN_CENTER, 0, s_idx == POINTS - 1 ? theme::S(52) : 0);
}

void restart(const char* msg) {
  display::keepTouchCal(false);
  s_idx = 0;
  placeTarget();
  setMessage(msg);
}

void resetSamples() {
  s_seen = 0;
  s_samples = 0;
  s_sumX = 0;
  s_sumY = 0;
}

void finish(bool ok) {
  s_doneAt = millis();
  lv_obj_add_flag(s_ring, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(s_hline, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(s_vline, LV_OBJ_FLAG_HIDDEN);
  lv_obj_align(s_text, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(s_text, ok ? rc::L("Dokunmatik ayarı kaydedildi", "Touch calibration saved") : rc::L("Değişiklik yapılmadı", "No changes made"));
}

void pointDone() {
  s_rx[s_idx] = s_sumX / s_samples;
  s_ry[s_idx] = s_sumY / s_samples;
  if (s_idx < POINTS - 2) {
    s_idx++;
    placeTarget();
    setMessage(rc::L("Artının tam ortasına dokunup kaldır", "Tap the exact center of the cross and lift"));
    return;
  }
  if (s_idx == POINTS - 2) {
    if (!display::calibrateTouch(kTx, kTy, s_rx, s_ry)) {
      restart(rc::L("Dokunuşlar tutarsız çıktı, baştan alalım", "Taps were inconsistent, let's start over"));
      return;
    }
    s_idx++;
    placeTarget();
    setMessage(rc::L("Son olarak ortadaki artıya dokun", "Finally, tap the cross in the middle"));
    return;
  }
  int32_t x = 0, y = 0;
  display::mapRawTouch(s_rx[s_idx], s_ry[s_idx], x, y);
  if (abs(x - kTx[s_idx]) <= VERIFY_TOL && abs(y - kTy[s_idx]) <= VERIFY_TOL) {
    display::keepTouchCal(true);
    finish(true);
  } else {
    restart(rc::L("Sağlama tutmadı, baştan alalım", "Check failed, let's start over"));
  }
}

void tick(lv_timer_t* t) {
  (void)t;
  uint32_t now = millis();
  if (s_doneAt) {
    if (now - s_doneAt >= DONE_MS) ui::go(ui::SCR_DISPLAY_CFG, -1);
    return;
  }
  int32_t rx = 0, ry = 0;
  bool down = display::rawTouch(rx, ry);
  if (down) {
    s_idleSince = now;
    s_wasDown = true;
    if (++s_seen > SKIP_SAMPLES && s_samples < MAX_SAMPLES) {
      s_sumX += rx;
      s_sumY += ry;
      s_samples++;
    }
    return;
  }
  if (!s_wasDown) {
    if (now - s_idleSince >= IDLE_MS) {
      display::keepTouchCal(false);
      finish(false);
    }
    return;
  }
  s_wasDown = false;
  if (s_samples >= MIN_SAMPLES) pointDone();
  resetSamples();
}

lv_obj_t* line(int32_t w, int32_t h) {
  lv_obj_t* o = theme::makeBox(s_scr);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_bg_color(o, theme::col(theme::C_EYE), 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
  return o;
}

void deleted(lv_event_t* e) {
  (void)e;
  if (s_timer) lv_timer_delete(s_timer);
  s_timer = nullptr;
  if (!s_doneAt) display::keepTouchCal(false);
  s_scr = s_ring = s_hline = s_vline = s_text = nullptr;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  lv_obj_set_style_bg_color(s_scr, lv_color_black(), 0);
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  s_ring = theme::makeBox(s_scr);
  lv_obj_set_size(s_ring, RING, RING);
  lv_obj_set_style_radius(s_ring, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_border_width(s_ring, theme::S(2), 0);
  lv_obj_set_style_border_color(s_ring, theme::col(theme::C_EYE), 0);
  lv_obj_set_style_bg_opa(s_ring, LV_OPA_TRANSP, 0);
  lv_obj_remove_flag(s_ring, LV_OBJ_FLAG_CLICKABLE);
  s_hline = line(ARM, theme::S(2));
  s_vline = line(theme::S(2), ARM);
  s_text = theme::makeLabel(s_scr, theme::fontM(), theme::C_TEXT);
  lv_label_set_long_mode(s_text, LV_LABEL_LONG_MODE_WRAP);
  lv_obj_set_width(s_text, theme::S(240));
  lv_obj_set_style_text_align(s_text, LV_TEXT_ALIGN_CENTER, 0);
  s_idx = 0;
  s_doneAt = 0;
  s_wasDown = false;
  s_idleSince = millis();
  resetSamples();
  placeTarget();
  setMessage(display::touchCalSupported() ? rc::L("Dokunmatik ayarı: artının tam ortasına dokunup kaldır", "Touch calibration: tap the exact center of the cross and lift")
                                          : rc::L("Dokunmatik bulunamadı", "Touch not found"));
  s_timer = lv_timer_create(tick, TICK_MS, nullptr);
  return s_scr;
}

void update(uint32_t mask) { (void)mask; }

}
