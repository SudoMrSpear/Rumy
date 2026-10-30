#include <stdio.h>

#include "fonts/symbols.h"
#include "rows.h"
#include "screens.h"
#include "settings/settings.h"
#include "text_tr.h"
#include "theme.h"
#include "ui.h"

namespace scr_nightcfg {

namespace {

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_modes[3] = {};
lv_obj_t* s_bri = nullptr;
lv_obj_t* s_briVal = nullptr;
lv_obj_t* s_start = nullptr;
lv_obj_t* s_end = nullptr;

using theme::setText;

void refresh() {
  const Settings& st = ui::data().settings;
  char buf[16];
  for (int i = 0; i < 3; i++) {
    bool on = st.nightMode == i;
    lv_obj_set_style_border_color(s_modes[i], theme::col(on ? theme::C_EYE : theme::C_BORDER), 0);
    lv_obj_set_style_border_width(s_modes[i], on ? theme::S(2) : theme::S(1), 0);
  }
  lv_slider_set_value(s_bri, st.nightBrightness, LV_ANIM_OFF);
  snprintf(buf, sizeof(buf), "%%%u", static_cast<unsigned>(st.nightBrightness));
  setText(s_briVal, buf);
  rc::formatHm(buf, sizeof(buf), static_cast<uint8_t>(st.nightStart / 60), static_cast<uint8_t>(st.nightStart % 60));
  setText(s_start, buf);
  rc::formatHm(buf, sizeof(buf), static_cast<uint8_t>(st.nightEnd / 60), static_cast<uint8_t>(st.nightEnd % 60));
  setText(s_end, buf);
}

void modeClick(lv_event_t* e) {
  Settings st = settings::get();
  st.nightMode = static_cast<uint8_t>(reinterpret_cast<intptr_t>(lv_event_get_user_data(e)));
  settings::set(st);
}

void briEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* s = lv_event_get_target_obj(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%%%u", static_cast<unsigned>(lv_slider_get_value(s)));
    setText(s_briVal, buf);
  }
  if (code != LV_EVENT_RELEASED && code != LV_EVENT_PRESS_LOST) return;
  Settings st = settings::get();
  st.nightBrightness = static_cast<uint8_t>(lv_slider_get_value(s));
  settings::set(st);
}

uint16_t step(uint16_t v, int dir) {
  int32_t n = static_cast<int32_t>(v) + dir * 30;
  n = ((n % 1440) + 1440) % 1440;
  return static_cast<uint16_t>(n - n % 30);
}

void startStep(lv_event_t* e) {
  Settings st = settings::get();
  st.nightStart = step(st.nightStart, rows::stepDir(e));
  settings::set(st);
}

void endStep(lv_event_t* e) {
  Settings st = settings::get();
  st.nightEnd = step(st.nightEnd, rows::stepDir(e));
  settings::set(st);
}

void back(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_SETTINGS, -1);
}

void deleted(lv_event_t* e) {
  (void)e;
  s_scr = s_bri = s_briVal = s_start = s_end = nullptr;
  for (int i = 0; i < 3; i++) s_modes[i] = nullptr;
}

}

lv_obj_t* create() {
  s_scr = rows::screen();
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  rows::header(s_scr, rc::L("Gece modu", "Night mode"), false, back);
  lv_obj_t* r = rows::row(s_scr, rc::L(RS_MOON "  Ne zaman?", RS_MOON "  When?"), theme::S(82));
  static const char* const kModes[3] = {rc::L("Kapalı", "Off"), rc::L("Güneşe göre", "By sun"), rc::L("Saatlere göre", "By hours")};
  for (int i = 0; i < 3; i++) {
    lv_obj_t* b = theme::makeButton(r, kModes[i], theme::fontS());
    const int32_t bw = (rows::W - theme::S(8) - theme::S(12) - 2 * theme::S(6)) / 3;
    lv_obj_set_size(b, bw, theme::S(34));
    lv_obj_set_pos(b, theme::S(8) + i * (bw + theme::S(6)), theme::S(40));
    lv_obj_add_event_cb(b, modeClick, LV_EVENT_CLICKED, reinterpret_cast<void*>(static_cast<intptr_t>(i)));
    s_modes[i] = b;
  }
  r = rows::row(s_scr, rc::L(RS_EYE "  Gece parlaklığı", RS_EYE "  Night brightness"), theme::S(66));
  s_briVal = rows::value(r);
  s_bri = rows::slider(r, 1, 40, briEvent);
  r = rows::row(s_scr, rc::L(RS_CLOCK "  Başlangıç", RS_CLOCK "  Start"), rows::H);
  s_start = rows::stepper(r, startStep);
  r = rows::row(s_scr, rc::L(RS_CLOCK "  Bitiş", RS_CLOCK "  End"), rows::H);
  s_end = rows::stepper(r, endStep);
  lv_obj_t* hint = rows::section(s_scr, rc::L("Güneşe göre: gün batımından doğumuna kadar. Gece ekranı kısılır, Rumy uyur; dokununca 30 sn uyanır.", "By sun: from sunset to sunrise. At night the screen dims and Rumy sleeps; a tap wakes it for 30 s."));
  lv_label_set_long_mode(hint, LV_LABEL_LONG_MODE_WRAP);
  refresh();
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & ui::M_SETTINGS) refresh();
}

}
