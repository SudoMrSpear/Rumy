#include <stdio.h>

#include "clock/clock.h"
#include "events_calc.h"
#include "rumy/rumy.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"
#include "text_tr.h"

namespace scr_event {

namespace {

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_face = nullptr;
lv_obj_t* s_title = nullptr;
lv_obj_t* s_sub = nullptr;
uint8_t s_index = 0;

void show() {
  const ui::Data& d = ui::data();
  const rc::EventsSnapshot& ev = d.events;
  int32_t today = clk::synced() ? clk::todayDayNum() : ev.computedDay;
  char buf[112];
  if (!ev.count) {
    theme::setText(s_title, rc::L("Yaklaşan önemli gün yok", "No upcoming special days"));
    theme::setText(s_sub, rc::L("Kişisel günlerini web arayüzünden ekleyebilirsin", "You can add your own days in the web app"));
    rumy::setMood(s_face, rumy::NORMAL);
    rumy::setAccessory(s_face, rc::ACC_NONE);
    return;
  }
  if (s_index >= ev.count) s_index = 0;
  const rc::EventOcc& e = ev.items[s_index];
  bool isToday = e.dayNum == today;
  if (isToday) {
    rc::eventGreeting(e, buf, sizeof(buf));
    theme::setText(s_title, buf);
    rc::eventSubtitle(e, buf, sizeof(buf));
    theme::setText(s_sub, buf);
    rumy::setMood(s_face, e.kind == rc::EV_COMMEMORATION ? rumy::CALM : rumy::CELEBRATE);
  } else {
    theme::setText(s_title, e.name);
    rc::eventCountdownText(e, today, buf, sizeof(buf));
    theme::setText(s_sub, buf);
    rumy::setMood(s_face, rumy::HAPPY);
  }
  rumy::setAccessory(s_face, e.accessory);
  lv_obj_set_style_text_color(s_title, theme::col(e.kind == rc::EV_COMMEMORATION ? theme::C_TEXT : theme::C_EYE), 0);
}

void pressed(lv_event_t* e) {
  (void)e;
  const ui::Data& d = ui::data();
  int32_t today = clk::synced() ? clk::todayDayNum() : d.events.computedDay;
  uint8_t next = static_cast<uint8_t>(s_index + 1);
  if (next < d.events.count && next < d.events.todayCount && d.events.items[next].dayNum == today) {
    s_index = next;
    show();
    return;
  }
  ui::go(ui::SCR_HOME, 0);
}

void deleted(lv_event_t* e) {
  (void)e;
  s_scr = s_face = s_title = s_sub = nullptr;
}

}

lv_obj_t* create() {
  s_index = 0;
  s_scr = theme::makeScreen();
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  lv_obj_add_event_cb(s_scr, pressed, LV_EVENT_CLICKED, nullptr);

  s_face = rumy::create(s_scr, theme::SW, theme::S(150), theme::EVENT_EYE);
  lv_obj_set_pos(s_face, 0, theme::S(6));
  lv_obj_remove_flag(s_face, LV_OBJ_FLAG_CLICKABLE);

  s_title = theme::makeLabel(s_scr, theme::fontL(), theme::C_EYE);
  lv_obj_set_width(s_title, theme::FW(304));
  lv_label_set_long_mode(s_title, LV_LABEL_LONG_MODE_WRAP);
  lv_obj_set_style_text_align(s_title, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(s_title, theme::S(8), theme::S(158));
  s_sub = theme::makeLabel(s_scr, theme::fontM(), theme::C_TEXT2);
  lv_obj_set_width(s_sub, theme::FW(304));
  theme::oneLine(s_sub);
  lv_obj_set_style_text_align(s_sub, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(s_sub, LV_ALIGN_BOTTOM_MID, 0, -theme::S(8));
  show();
  rumy::setAnimated(s_face, true);
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & (ui::M_EVENTS | ui::M_MINUTE)) show();
}

}
