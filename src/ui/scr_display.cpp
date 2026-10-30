#include <stdio.h>

#include "board/display.h"
#include "fonts/symbols.h"
#include "rows.h"
#include "screens.h"
#include "settings/settings.h"
#include "theme.h"
#include "ui.h"
#include "text_tr.h"

namespace scr_display {

namespace {

constexpr uint8_t TOUCH_CONFIRM_S = 10;
const uint16_t kHomeTimeouts[] = {30, 60, 120, 300, 900};
const uint8_t kCardSecs[] = {3, 5, 8, 10, 15, 30};
const uint8_t kEyeSleep[] = {0, 1, 2, 5, 10, 15, 30};

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_bri = nullptr;
lv_obj_t* s_briVal = nullptr;
lv_obj_t* s_home = nullptr;
lv_obj_t* s_card = nullptr;
lv_obj_t* s_sleep = nullptr;
lv_obj_t* s_touch = nullptr;
lv_obj_t* s_overlay = nullptr;
lv_obj_t* s_overlayText = nullptr;
lv_timer_t* s_timer = nullptr;
uint8_t s_touchOld = 0;
uint8_t s_touchNew = 0;
uint8_t s_left = 0;
bool s_pending = false;
bool s_dragging = false;

using theme::setText;

template <typename T, size_t N>
T cycle(const T (&opts)[N], T cur, int dir) {
  size_t idx = N - 1;
  for (size_t i = 0; i < N; i++) {
    if (opts[i] >= cur) {
      idx = i;
      break;
    }
  }
  int32_t n = static_cast<int32_t>(idx) + dir;
  if (n < 0) n = 0;
  if (n >= static_cast<int32_t>(N)) n = static_cast<int32_t>(N) - 1;
  return opts[n];
}

void refresh() {
  const Settings& st = ui::data().settings;
  char buf[24];
  if (!s_dragging) lv_slider_set_value(s_bri, st.brightness, LV_ANIM_OFF);
  snprintf(buf, sizeof(buf), "%%%u", static_cast<unsigned>(s_dragging ? lv_slider_get_value(s_bri) : st.brightness));
  setText(s_briVal, buf);
  if (st.homeTimeoutS < 60) snprintf(buf, sizeof(buf), rc::L("%u sn", "%u s"), static_cast<unsigned>(st.homeTimeoutS));
  else snprintf(buf, sizeof(buf), rc::L("%u dk", "%u min"), static_cast<unsigned>(st.homeTimeoutS / 60));
  setText(s_home, buf);
  snprintf(buf, sizeof(buf), rc::L("%u sn", "%u s"), static_cast<unsigned>(st.cardSeconds));
  setText(s_card, buf);
  if (st.eyeSleepMin) snprintf(buf, sizeof(buf), rc::L("%u dk", "%u min"), static_cast<unsigned>(st.eyeSleepMin));
  else snprintf(buf, sizeof(buf), "%s", rc::L("Kapalı", "Off"));
  setText(s_sleep, buf);
  snprintf(buf, sizeof(buf), rc::L("Yön %u", "Mode %u"), static_cast<unsigned>(st.touchRot));
  rows::setNavValue(s_touch, buf);
}

void closeOverlay() {
  if (s_timer) lv_timer_delete(s_timer);
  s_timer = nullptr;
  if (s_overlay) lv_obj_delete_async(s_overlay);
  s_overlay = nullptr;
  s_overlayText = nullptr;
}

void revert() {
  if (!s_pending) return;
  s_pending = false;
  display::setTouchRotation(s_touchOld);
}

void overlayText() {
  if (!s_overlayText) return;
  char buf[128];
  snprintf(buf, sizeof(buf), rc::L("Dokunmatik yönü %u denendi.\nAşağıdaki düğmeye dokunabiliyorsan onayla.\n%u sn içinde geri alınacak.", "Trying touch orientation %u.\nIf you can tap the button below, confirm.\nReverting in %u s."),
           static_cast<unsigned>(s_touchNew), static_cast<unsigned>(s_left));
  lv_label_set_text(s_overlayText, buf);
}

void tick(lv_timer_t* t) {
  (void)t;
  if (s_left > 0) s_left--;
  if (s_left == 0) {
    revert();
    closeOverlay();
    ui::toast(rc::L("Dokunmatik yönü geri alındı", "Touch orientation reverted"));
    return;
  }
  overlayText();
}

void confirm(lv_event_t* e) {
  (void)e;
  if (!s_pending) return;
  s_pending = false;
  uint8_t rot = s_touchNew;
  closeOverlay();
  Settings st = settings::get();
  st.touchRot = rot;
  settings::set(st);
  ui::toast(rc::L("Dokunmatik yönü kaydedildi", "Touch orientation saved"));
}

void touchClick(lv_event_t* e) {
  (void)e;
  closeOverlay();
  s_touchOld = ui::data().settings.touchRot;
  s_touchNew = static_cast<uint8_t>((s_touchOld + 1) & 7);
  s_left = TOUCH_CONFIRM_S;
  s_pending = true;
  display::setTouchRotation(s_touchNew);
  s_overlay = theme::makeBox(lv_layer_top());
  lv_obj_set_size(s_overlay, theme::SW, theme::SH);
  lv_obj_set_style_bg_color(s_overlay, theme::col(theme::C_BG), 0);
  lv_obj_set_style_bg_opa(s_overlay, LV_OPA_90, 0);
  lv_obj_add_flag(s_overlay, LV_OBJ_FLAG_CLICKABLE);
  s_overlayText = theme::makeLabel(s_overlay, theme::fontM(), theme::C_TEXT);
  lv_label_set_long_mode(s_overlayText, LV_LABEL_LONG_MODE_WRAP);
  lv_obj_set_width(s_overlayText, theme::FW(290));
  lv_obj_set_style_text_align(s_overlayText, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(s_overlayText, LV_ALIGN_TOP_MID, 0, theme::S(12));
  lv_obj_t* ok = theme::makeButton(s_overlay, rc::L(RS_OK "  Evet, doğru", RS_OK "  Yes, that's right"), theme::fontL());
  lv_obj_set_size(ok, theme::S(220), theme::S(84));
  lv_obj_align(ok, LV_ALIGN_CENTER, 0, theme::S(44));
  lv_obj_set_style_border_color(ok, theme::col(theme::C_EYE), 0);
  lv_obj_add_event_cb(ok, confirm, LV_EVENT_CLICKED, nullptr);
  overlayText();
  s_timer = lv_timer_create(tick, 1000, nullptr);
}

void briEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* s = lv_event_get_target_obj(e);
  if (code == LV_EVENT_PRESSED) s_dragging = true;
  if (code == LV_EVENT_VALUE_CHANGED) {
    display::setBrightnessPct(static_cast<uint8_t>(lv_slider_get_value(s)));
    char buf[8];
    snprintf(buf, sizeof(buf), "%%%u", static_cast<unsigned>(lv_slider_get_value(s)));
    setText(s_briVal, buf);
  }
  if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
    s_dragging = false;
    Settings st = settings::get();
    st.brightness = static_cast<uint8_t>(lv_slider_get_value(s));
    settings::set(st);
  }
}

void homeStep(lv_event_t* e) {
  Settings st = settings::get();
  st.homeTimeoutS = cycle(kHomeTimeouts, st.homeTimeoutS, rows::stepDir(e));
  settings::set(st);
}

void cardStep(lv_event_t* e) {
  Settings st = settings::get();
  st.cardSeconds = cycle(kCardSecs, st.cardSeconds, rows::stepDir(e));
  settings::set(st);
}

void sleepStep(lv_event_t* e) {
  Settings st = settings::get();
  st.eyeSleepMin = cycle(kEyeSleep, st.eyeSleepMin, rows::stepDir(e));
  settings::set(st);
}

void back(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_SETTINGS, -1);
}

void calClick(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_TOUCH_CAL, 1);
}

void deleted(lv_event_t* e) {
  (void)e;
  revert();
  closeOverlay();
  s_dragging = false;
  s_scr = s_bri = s_briVal = s_home = s_card = s_sleep = s_touch = nullptr;
}

}

lv_obj_t* create() {
  s_scr = rows::screen();
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  rows::header(s_scr, rc::L("Ekran", "Display"), false, back);
  lv_obj_t* r = rows::row(s_scr, rc::L(RS_SUN "  Parlaklık", RS_SUN "  Brightness"), theme::S(66));
  s_briVal = rows::value(r);
  s_bri = rows::slider(r, 5, 100, briEvent);
  r = rows::row(s_scr, rc::L(RS_HOME "  Ana ekran", RS_HOME "  Home screen"), rows::H);
  s_home = rows::stepper(r, homeStep);
  r = rows::row(s_scr, rc::L(RS_INFO "  Kart süresi", RS_INFO "  Card time"), rows::H);
  s_card = rows::stepper(r, cardStep);
  r = rows::row(s_scr, rc::L(RS_EYE "  Göz uykusu", RS_EYE "  Eye sleep"), rows::H);
  s_sleep = rows::stepper(r, sleepStep);
  rows::nav(s_scr, rc::L(RS_HAND "  Dokunmatik yönü", RS_HAND "  Touch orientation"), touchClick, &s_touch);
  if (display::touchCalSupported()) rows::nav(s_scr, rc::L(RS_HAND "  Dokunmatik ayarı", RS_HAND "  Touch calibration"), calClick, nullptr);
  lv_obj_t* hint = rows::section(s_scr, rc::L("Ana ekran: dokunulmazsa ana ekrana dönme süresi. Göz uykusu: bu süre dokunulmazsa Rumy uyur, dokununca uyanır. Dokunmatik yönü değişince 10 sn içinde onaylamazsan eski haline döner. Dokunmatik ayarı: dokunuşlar düğmeyi ıskalıyorsa artılara dokunarak düzeltir.", "Home screen: how long without touch before returning home. Eye sleep: Rumy sleeps after this long without touch and wakes on tap. A new touch orientation reverts unless you confirm within 10 s. Touch calibration: if taps miss buttons, fix it by tapping the crosses."));
  lv_label_set_long_mode(hint, LV_LABEL_LONG_MODE_WRAP);
  refresh();
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & ui::M_SETTINGS) refresh();
}

}
