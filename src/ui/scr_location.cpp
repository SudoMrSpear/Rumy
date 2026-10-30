#include <stdio.h>
#include <string.h>

#include "fonts/symbols.h"
#include "net/net.h"
#include "screens.h"
#include "settings/provinces.h"
#include "settings/settings.h"
#include "theme.h"
#include "ui.h"
#include "text_tr.h"

namespace scr_location {

namespace {

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_roller = nullptr;
lv_obj_t* s_info = nullptr;

using theme::setText;

void showInfo() {
  int idx = static_cast<int>(lv_roller_get_selected(s_roller));
  const provinces::Province& p = provinces::at(idx);
  char buf[64];
  snprintf(buf, sizeof(buf), rc::L("Plaka %02u\n%.3f K\n%.3f D", "Plate %02u\n%.3f N\n%.3f E"), static_cast<unsigned>(p.plate),
           static_cast<double>(p.lat), static_cast<double>(p.lon));
  setText(s_info, buf);
}

void rollerChanged(lv_event_t* e) {
  (void)e;
  showInfo();
}

void saveClick(lv_event_t* e) {
  (void)e;
  int idx = static_cast<int>(lv_roller_get_selected(s_roller));
  const provinces::Province& p = provinces::at(idx);
  Settings s = settings::get();
  bool changed = s.province != p.plate || strcmp(s.city, p.name) != 0;
  s.province = p.plate;
  s.lat = p.lat;
  s.lon = p.lon;
  snprintf(s.city, sizeof(s.city), "%s", p.name);
  settings::set(s);
  if (changed) {
    net::locationChanged();
    ui::toast(rc::L("Konum kaydedildi · hava durumu yenileniyor", "Location saved · refreshing weather"));
  }
  ui::back();
}

void backClick(lv_event_t* e) {
  (void)e;
  ui::back();
}

void deleted(lv_event_t* e) {
  (void)e;
  s_scr = s_roller = s_info = nullptr;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  ui::attachGestures(s_scr);

  lv_obj_t* bk = theme::makeButton(s_scr, RS_LEFT, theme::fontM());
  lv_obj_set_size(bk, theme::S(44), theme::S(36));
  lv_obj_set_pos(bk, theme::S(6), theme::S(4));
  lv_obj_add_event_cb(bk, backClick, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* title = theme::makeLabel(s_scr, theme::fontL(), theme::C_TEXT);
  lv_label_set_text_static(title, rc::L(RS_PIN "  Konum", RS_PIN "  Location"));
  lv_obj_set_pos(title, theme::S(60), theme::S(9));

  size_t cap = 0;
  for (int i = 0; i < provinces::COUNT; i++) cap += strlen(provinces::at(i).name) + 1;
  char* opts = static_cast<char*>(lv_malloc(cap + 1));
  s_roller = lv_roller_create(s_scr);
  theme::styleRoller(s_roller);
  if (opts) {
    size_t off = 0;
    for (int i = 0; i < provinces::COUNT; i++) {
      const char* n = provinces::at(i).name;
      size_t len = strlen(n);
      memcpy(opts + off, n, len);
      off += len;
      if (i + 1 < provinces::COUNT) opts[off++] = '\n';
    }
    opts[off] = 0;
    lv_roller_set_options(s_roller, opts, LV_ROLLER_MODE_NORMAL);
    lv_free(opts);
  }
  lv_roller_set_visible_row_count(s_roller, 4);
  lv_obj_set_width(s_roller, theme::S(200));
  lv_obj_set_pos(s_roller, theme::S(10), theme::S(48));
  lv_obj_add_event_cb(s_roller, rollerChanged, LV_EVENT_VALUE_CHANGED, nullptr);
  int cur = provinces::indexOfPlate(ui::data().settings.province);
  if (cur < 0) cur = provinces::findByName(ui::data().settings.city);
  if (cur >= 0) lv_roller_set_selected(s_roller, static_cast<uint32_t>(cur), LV_ANIM_OFF);

  s_info = theme::makeLabel(s_scr, theme::fontS(), theme::C_TEXT2);
  lv_obj_set_pos(s_info, theme::S(220), theme::S(60));
  lv_obj_set_width(s_info, theme::FW(96));
  lv_label_set_long_mode(s_info, LV_LABEL_LONG_MODE_WRAP);

  lv_obj_t* hint = theme::makeLabel(s_scr, theme::fontS(), theme::C_TEXT3);
  lv_label_set_text_static(hint, rc::L("İlçe veya tam koordinat için web arayüzünü kullan.", "Use the web app for a district or exact coordinates."));
  lv_label_set_long_mode(hint, LV_LABEL_LONG_MODE_WRAP);
  lv_obj_set_width(hint, theme::FW(96));
  lv_obj_set_pos(hint, theme::S(220), theme::S(110));

  lv_obj_t* save = theme::makeButton(s_scr, rc::L(RS_OK "  Kaydet", RS_OK "  Save"), theme::fontM());
  lv_obj_set_size(save, theme::FW(300), theme::S(42));
  lv_obj_set_pos(save, theme::S(10), theme::S(192));
  lv_obj_set_style_border_color(save, theme::col(theme::C_EYE), 0);
  lv_obj_add_event_cb(save, saveClick, LV_EVENT_CLICKED, nullptr);

  showInfo();
  return s_scr;
}

void update(uint32_t mask) { (void)mask; }

}
