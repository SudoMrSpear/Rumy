#include <stdio.h>

#include "clock/clock.h"
#include "k2.h"
#include "media/media.h"
#include "rumy/rumy.h"
#include "screens.h"
#include "text_tr.h"
#include "theme.h"
#include "timeutil.h"
#include "ui.h"

namespace scr_night {

namespace {

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_face = nullptr;
lv_obj_t* s_clock = nullptr;
lv_obj_t* s_date = nullptr;
lv_obj_t* s_printer = nullptr;

void wakeCb(lv_event_t* e) {
  (void)e;
  ui::wake();
}

void updateClock() {
  char buf[40];
  if (!clk::synced()) {
    theme::setText(s_clock, "--:--");
    theme::setText(s_date, "");
    return;
  }
  rc::LocalTime lt = clk::local();
  rc::formatHm(buf, sizeof(buf), lt.hour, lt.minute);
  theme::setText(s_clock, buf);
  snprintf(buf, sizeof(buf), "%s, %u %s", rc::dayNameTr(lt.wday), static_cast<unsigned>(lt.day),
           rc::monthNameTr(lt.month));
  theme::setText(s_date, buf);
}

void updatePrinter() {
  const PrinterState& p = ui::data().printer;
  char buf[64];
  buf[0] = 0;
  uint32_t color = theme::C_TEXT3;
  if (p.connected && p.problem) {
    if (rc::k2Runout(p.st)) snprintf(buf, sizeof(buf), "%s", rc::L("Yazıcı: filament bitti", "Printer: out of filament"));
    else snprintf(buf, sizeof(buf), rc::L("Yazıcı hatası · kod %ld", "Printer error · code %ld"), static_cast<long>(p.st.errCode));
    color = theme::C_NEON_RED;
  } else if (p.connected && (p.phase == rc::K2_PRINTING || p.phase == rc::K2_PAUSED)) {
    if (p.phase == rc::K2_PRINTING && p.st.leftS && clk::synced()) {
      rc::LocalTime lt = rc::toLocal(clk::nowUtc() + p.st.leftS);
      char hm[8];
      rc::formatHm(hm, sizeof(hm), lt.hour, lt.minute);
      snprintf(buf, sizeof(buf), rc::L("Baskı %%%u · bitiş %s", "Print %u%% · done at %s"), static_cast<unsigned>(p.st.progress), hm);
    } else {
      snprintf(buf, sizeof(buf), rc::L("Baskı %%%u · %s", "Print %u%% · %s"), static_cast<unsigned>(p.st.progress),
               rc::k2PhaseTr(static_cast<rc::K2Phase>(p.phase)));
    }
  }
  lv_obj_set_style_text_color(s_printer, theme::col(color), 0);
  theme::setText(s_printer, buf);
}

void deleted(lv_event_t* e) {
  (void)e;
  s_scr = s_face = s_clock = s_date = s_printer = nullptr;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  lv_obj_add_event_cb(s_scr, wakeCb, LV_EVENT_PRESSED, nullptr);

  s_face = rumy::create(s_scr, theme::SW, theme::S(150), theme::HOME_EYE);
  lv_obj_set_pos(s_face, 0, theme::S(14));
  lv_obj_remove_flag(s_face, LV_OBJ_FLAG_CLICKABLE);
  rumy::setColor(s_face, lv_color_mix(theme::col(theme::C_EYE), theme::col(theme::C_BG), 150));
  rumy::setMood(s_face, rumy::ASLEEP);
  rumy::setAnimated(s_face, true);

  s_clock = theme::makeLabel(s_scr, theme::fontL(), theme::C_TEXT3);
  lv_obj_set_width(s_clock, theme::SW);
  lv_obj_set_style_text_align(s_clock, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(s_clock, 0, theme::S(176));
  s_date = theme::makeLabel(s_scr, theme::fontS(), theme::C_TEXT3);
  lv_obj_set_width(s_date, theme::SW);
  lv_obj_set_style_text_align(s_date, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(s_date, 0, theme::S(206));
  s_printer = theme::makeLabel(s_scr, theme::fontS(), theme::C_TEXT3);
  lv_obj_set_width(s_printer, theme::SW);
  lv_obj_set_style_text_align(s_printer, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(s_printer, 0, theme::S(222));
  updateClock();
  updatePrinter();
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & (ui::M_MINUTE | ui::M_NET)) updateClock();
  if (mask & (ui::M_MINUTE | ui::M_PRINTER)) updatePrinter();
  if (mask & ui::M_MEDIA) {
    const media::State& m = media::state();
    lv_color_t dim = theme::col(theme::C_BG);
    rumy::setMusic(s_face, m.link && m.active && m.playing, lv_color_mix(theme::col(theme::C_NEON_PINK), dim, 150),
                   lv_color_mix(theme::col(theme::C_NEON_YELLOW), dim, 150));
  }
}

}
