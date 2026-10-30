#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "color.h"
#include "draw_util.h"
#include "fonts/symbols.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"
#include "wiz/wiz.h"
#include "wiz_proto.h"
#include "text_tr.h"

namespace scr_light {

namespace {

constexpr uint32_t ADOPT_QUIET_MS = 1500;
constexpr int SCENE_COUNT = 8;
using theme::FW;
using theme::R;
using theme::S;

constexpr int32_t SLIDER_X = S(38);
constexpr int32_t SLIDER_W = FW(214);
constexpr int32_t SLIDER_H = S(14);
constexpr int32_t HUE_Y = S(58);
constexpr int32_t DIM_Y = S(92);
constexpr int32_t TEMP_Y = S(126);
constexpr int32_t KNOB_RING = S(3);
constexpr int32_t SWATCH = S(18);
constexpr int32_t SCENE_Y = S(152);
constexpr int32_t SCENE_GAP = S(4);
constexpr int32_t SCENE_W = (FW(308) - 3 * SCENE_GAP) / 4;
constexpr int32_t SCENE_H = S(40);
constexpr int32_t SCENE_OVERLAP = S(2);

struct Scene {
  const char* name;
  const char* icon;
  uint32_t swatch;
  uint8_t mode;
  uint8_t sceneId;
  uint8_t dim;
  uint8_t reserved;
  uint16_t value;
};

const Scene kScenes[SCENE_COUNT] = {
    {"Sabah", RS_SUN, 0xD6E8FF, rc::PILOT_TEMP, 0, 100, 0, 6500},
    {"Çalışma", RS_LAPTOP, 0xFFE4CE, rc::PILOT_TEMP, 0, 100, 0, 5000},
    {"Okuma", RS_BOOK, 0xFFD1A3, rc::PILOT_TEMP, 0, 80, 0, 4000},
    {"Yemek", RS_UTENSILS, 0xFFB46B, rc::PILOT_TEMP, 0, 80, 0, 3000},
    {"Akşam", RS_COUCH, 0xFFA957, rc::PILOT_TEMP, 0, 50, 0, 2700},
    {"Film", RS_FILM, 0x6A5CFF, rc::PILOT_SCENE, 18, 40, 0, 0},
    {"Mum", RS_FIRE, 0xFF8A2A, rc::PILOT_SCENE, 29, 50, 0, 0},
    {"Gece", RS_MOON, 0xFF6A1A, rc::PILOT_SCENE, 14, 10, 0, 0},
};

const char* const kSceneNamesEn[SCENE_COUNT] = {"Morning", "Work", "Reading", "Dinner",
                                                "Evening", "Movie", "Candle", "Night"};

lv_grad_dsc_t s_hueGrad;
lv_grad_dsc_t s_tempGrad;
bool s_gradReady = false;

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_bulbBg = nullptr;
lv_obj_t* s_bulb = nullptr;
lv_obj_t* s_swatch = nullptr;
lv_obj_t* s_name = nullptr;
lv_obj_t* s_status = nullptr;
lv_obj_t* s_dot = nullptr;
lv_obj_t* s_power = nullptr;
lv_obj_t* s_powerIcon = nullptr;
lv_obj_t* s_hue = nullptr;
lv_obj_t* s_dim = nullptr;
lv_obj_t* s_temp = nullptr;
lv_obj_t* s_dimVal = nullptr;
lv_obj_t* s_tempVal = nullptr;
lv_obj_t* s_scenes = nullptr;
lv_obj_t* s_cover = nullptr;
rc::Pilot s_want = {};
uint32_t s_lastUserMs = 0;
bool s_dragging = false;
int8_t s_pressed = -1;
int8_t s_active = -1;

using theme::setText;

void knobStyle(lv_obj_t* s, bool active, lv_color_t fill) {
  lv_obj_set_style_bg_color(s, active ? fill : theme::col(theme::C_BG), LV_PART_KNOB);
  lv_obj_set_style_border_color(s, theme::col(active ? theme::C_TEXT : theme::C_TEXT2), LV_PART_KNOB);
  lv_obj_set_style_border_width(s, active ? KNOB_RING : S(2), LV_PART_KNOB);
}

void buildGradients() {
  if (s_gradReady) return;
  s_gradReady = true;
  memset(&s_hueGrad, 0, sizeof(s_hueGrad));
  static const uint32_t kHue[7] = {0xFF0000, 0xFFFF00, 0x00FF00, 0x00FFFF, 0x0000FF, 0xFF00FF, 0xFF0000};
  s_hueGrad.dir = LV_GRAD_DIR_HOR;
  s_hueGrad.stops_count = 7;
  for (int i = 0; i < 7; i++) {
    s_hueGrad.stops[i].color = theme::col(kHue[i]);
    s_hueGrad.stops[i].opa = LV_OPA_COVER;
    s_hueGrad.stops[i].frac = static_cast<uint8_t>(i * 255 / 6);
  }
  memset(&s_tempGrad, 0, sizeof(s_tempGrad));
  s_tempGrad.dir = LV_GRAD_DIR_HOR;
  s_tempGrad.stops_count = 3;
  const uint16_t kK[3] = {rc::WIZ_TEMP_MIN, 4000, rc::WIZ_TEMP_MAX};
  for (int i = 0; i < 3; i++) {
    rc::Rgb c = rc::kelvinToRgb(kK[i]);
    s_tempGrad.stops[i].color = lv_color_make(c.r, c.g, c.b);
    s_tempGrad.stops[i].opa = LV_OPA_COVER;
    s_tempGrad.stops[i].frac = static_cast<uint8_t>(i * 255 / 2);
  }
}

rc::Pilot defaults() {
  rc::Pilot p = rc::pilotTemp(2700, 60);
  p.hue = 30;
  return p;
}

lv_color_t pilotLv(const rc::Pilot& p) {
  rc::Rgb c = rc::pilotColor(p);
  return lv_color_make(c.r, c.g, c.b);
}

int8_t activeScene(const rc::Pilot& p) {
  if (!p.valid || !p.on) return -1;
  for (int i = 0; i < SCENE_COUNT; i++) {
    const Scene& s = kScenes[i];
    if (s.mode != p.mode || s.dim != p.dim) continue;
    if (s.mode == rc::PILOT_SCENE && s.sceneId == p.scene) return static_cast<int8_t>(i);
    if (s.mode == rc::PILOT_TEMP && s.value == p.temp) return static_cast<int8_t>(i);
    if (s.mode == rc::PILOT_RGB && s.value == p.hue) return static_cast<int8_t>(i);
  }
  return -1;
}

void refreshControls() {
  const rc::Pilot& p = s_want;
  bool on = p.valid && p.on;
  lv_color_t lc = pilotLv(p);
  lv_obj_set_style_bg_color(s_power, on ? lc : theme::col(theme::C_SURFACE2), 0);
  lv_obj_set_style_text_color(s_powerIcon, on ? lv_color_black() : theme::col(theme::C_TEXT2), 0);
  rc::Rgb hc = rc::hsvToRgb(p.hue % 360, 255, 255);
  knobStyle(s_hue, p.mode == rc::PILOT_RGB, lv_color_make(hc.r, hc.g, hc.b));
  rc::Rgb tc = rc::kelvinToRgb(p.mode == rc::PILOT_TEMP ? p.temp : static_cast<uint16_t>(lv_slider_get_value(s_temp)));
  knobStyle(s_temp, p.mode == rc::PILOT_TEMP, lv_color_make(tc.r, tc.g, tc.b));
  lv_color_t fill = on ? lc : theme::col(theme::C_TEXT3);
  knobStyle(s_dim, true, fill);
  lv_obj_set_style_bg_color(s_dim, fill, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(s_bulbBg, on ? lc : theme::col(theme::C_SURFACE2), 0);
  lv_obj_set_style_bg_opa(s_bulbBg, on ? LV_OPA_20 : LV_OPA_COVER, 0);
  lv_obj_set_style_text_color(s_bulb, on ? lc : theme::col(theme::C_TEXT3), 0);
  lv_obj_set_style_bg_color(s_swatch, on ? lc : theme::col(theme::C_SURFACE2), 0);
  char buf[16];
  snprintf(buf, sizeof(buf), "%%%u", static_cast<unsigned>(p.dim));
  setText(s_dimVal, buf);
  if (p.mode == rc::PILOT_TEMP) snprintf(buf, sizeof(buf), "%u K", static_cast<unsigned>(p.temp));
  else if (p.mode == rc::PILOT_SCENE) snprintf(buf, sizeof(buf), "%s", rc::L("Sahne", "Scene"));
  else snprintf(buf, sizeof(buf), "%s", rc::L("Renk", "Color"));
  setText(s_tempVal, buf);
  int8_t act = activeScene(p);
  if (act != s_active) {
    s_active = act;
    lv_obj_invalidate(s_scenes);
  }
}

void syncSliders() {
  const rc::Pilot& p = s_want;
  lv_slider_set_value(s_dim, p.dim < rc::WIZ_DIM_MIN ? rc::WIZ_DIM_MIN : p.dim, LV_ANIM_OFF);
  lv_slider_set_value(s_hue, p.hue % 360, LV_ANIM_OFF);
  if (p.mode == rc::PILOT_TEMP && p.temp) lv_slider_set_value(s_temp, p.temp, LV_ANIM_OFF);
}

void adopt(bool force) {
  const BulbState& b = ui::data().bulb;
  if (!b.pilot.valid) return;
  if (!force && (s_dragging || millis() - s_lastUserMs < ADOPT_QUIET_MS)) return;
  rc::Pilot p = b.pilot;
  if (p.mode != rc::PILOT_RGB) p.hue = s_want.valid ? s_want.hue : 30;
  s_want = p;
  syncSliders();
  refreshControls();
}

void pairClick(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_PAIR, 1);
}

void showCover(bool show) {
  if (show && !s_cover) {
    s_cover = theme::makeBox(s_scr);
    lv_obj_set_size(s_cover, theme::SW, theme::SH - S(44));
    lv_obj_set_pos(s_cover, 0, S(44));
    lv_obj_set_style_bg_color(s_cover, theme::col(theme::C_BG), 0);
    lv_obj_set_style_bg_opa(s_cover, LV_OPA_90, 0);
    lv_obj_add_flag(s_cover, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t* msg = theme::makeLabel(s_cover, theme::fontM(), theme::C_TEXT);
    lv_label_set_text_static(msg, rc::L("Henüz bir WiZ ampul eşleştirilmedi.", "No WiZ bulb paired yet."));
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, -S(30));
    lv_obj_t* pb = theme::makeButton(s_cover, rc::L(RS_SEARCH "  Ampul bul ve eşleştir", RS_SEARCH "  Find and pair a bulb"), theme::fontM());
    lv_obj_set_size(pb, S(240), S(44));
    lv_obj_align(pb, LV_ALIGN_CENTER, 0, S(20));
    lv_obj_set_style_border_color(pb, theme::col(theme::C_EYE), 0);
    lv_obj_add_event_cb(pb, pairClick, LV_EVENT_CLICKED, nullptr);
  } else if (!show && s_cover) {
    lv_obj_delete(s_cover);
    s_cover = nullptr;
  }
}

void updateStatus() {
  const ui::Data& d = ui::data();
  const BulbState& b = d.bulb;
  bool paired = d.settings.bulbMac[0] != 0;
  setText(s_name, d.settings.bulbName[0] ? d.settings.bulbName : rc::L("WiZ ampul", "WiZ bulb"));
  char buf[48];
  uint32_t dot = theme::C_DANGER;
  if (!paired) {
    snprintf(buf, sizeof(buf), "%s", rc::L("Eşleştirilmedi", "Not paired"));
  } else if (b.online && b.writeDenied) {
    snprintf(buf, sizeof(buf), "%s", rc::L("Komut izni yok · WiZ", "No control permission · WiZ"));
    dot = theme::C_WARN;
  } else if (b.online) {
    snprintf(buf, sizeof(buf), rc::L("Çevrimiçi · %u.%u.%u.%u", "Online · %u.%u.%u.%u"), static_cast<unsigned>(b.ip & 0xFF),
             static_cast<unsigned>((b.ip >> 8) & 0xFF), static_cast<unsigned>((b.ip >> 16) & 0xFF),
             static_cast<unsigned>((b.ip >> 24) & 0xFF));
    dot = theme::C_OK;
  } else if (b.found) {
    snprintf(buf, sizeof(buf), "%s", rc::L("Yanıt vermiyor", "Not responding"));
    dot = theme::C_WARN;
  } else {
    snprintf(buf, sizeof(buf), "%s", rc::L("Ağda aranıyor…", "Searching network…"));
    dot = theme::C_WARN;
  }
  setText(s_status, buf);
  lv_obj_set_style_bg_color(s_dot, theme::col(dot), 0);
  showCover(!paired);
}

void userChanged(bool final) {
  s_lastUserMs = millis();
  wiz::setDesired(s_want, final);
  refreshControls();
}

void sliderEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* s = lv_event_get_target_obj(e);
  if (code == LV_EVENT_PRESSED) {
    s_dragging = true;
    wiz::setInteracting(true);
    return;
  }
  bool final = code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST;
  if (code != LV_EVENT_VALUE_CHANGED && !final) return;
  int32_t v = lv_slider_get_value(s);
  uint8_t dim = static_cast<uint8_t>(lv_slider_get_value(s_dim));
  if (s == s_hue) {
    s_want = rc::pilotRgb(static_cast<uint16_t>(v), dim);
  } else if (s == s_temp) {
    uint16_t hue = s_want.hue;
    s_want = rc::pilotTemp(static_cast<uint16_t>(v), dim);
    s_want.hue = hue;
  } else {
    if (!s_want.valid) s_want = defaults();
    s_want.valid = 1;
    s_want.on = 1;
    s_want.dim = dim;
  }
  userChanged(final);
  if (final) {
    s_dragging = false;
    wiz::setInteracting(false);
  }
}

void powerClick(lv_event_t* e) {
  (void)e;
  if (!s_want.valid) s_want = defaults();
  s_want.valid = 1;
  s_want.on = s_want.on ? 0 : 1;
  userChanged(true);
}

void applyScene(int idx) {
  if (idx < 0 || idx >= SCENE_COUNT) return;
  const Scene& s = kScenes[idx];
  uint16_t hue = s_want.hue;
  if (s.mode == rc::PILOT_RGB) {
    s_want = rc::pilotRgb(s.value, s.dim);
  } else if (s.mode == rc::PILOT_TEMP) {
    s_want = rc::pilotTemp(s.value, s.dim);
    s_want.hue = hue;
  } else {
    s_want = rc::pilotTemp(2700, s.dim);
    s_want.mode = rc::PILOT_SCENE;
    s_want.scene = s.sceneId;
    s_want.temp = 0;
    s_want.hue = hue;
  }
  syncSliders();
  userChanged(true);
}

int sceneAt(lv_obj_t* obj) {
  lv_point_t p;
  lv_indev_get_point(lv_indev_active(), &p);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  int32_t x = p.x - c.x1;
  int32_t y = p.y - c.y1;
  if (x < 0 || y < 0) return -1;
  int col = static_cast<int>(x / (SCENE_W + SCENE_GAP));
  int row = static_cast<int>(y / (SCENE_H + SCENE_GAP));
  if (col > 3 || row > 1) return -1;
  return row * 4 + col;
}

void sceneEvent(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  if (code == LV_EVENT_PRESSED || code == LV_EVENT_PRESSING) {
    int8_t idx = static_cast<int8_t>(sceneAt(obj));
    if (idx != s_pressed) {
      s_pressed = idx;
      lv_obj_invalidate(obj);
    }
  } else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
    s_pressed = -1;
    lv_obj_invalidate(obj);
  } else if (code == LV_EVENT_CLICKED) {
    applyScene(sceneAt(obj));
  }
}

void sceneDraw(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  for (int i = 0; i < SCENE_COUNT; i++) {
    int32_t x = c.x1 + (i % 4) * (SCENE_W + SCENE_GAP);
    int32_t y = c.y1 + (i / 4) * (SCENE_H + SCENE_GAP);
    bool act = i == s_active;
    lv_color_t sw = theme::col(kScenes[i].swatch);
    lv_color_t surf = theme::col(theme::C_SURFACE2);
    lv_color_t border = act ? sw : theme::col(theme::C_BORDER);
    lv_color_t bg = i == s_pressed ? theme::col(theme::C_BORDER) : act ? lv_color_mix(sw, surf, 46) : surf;
    int32_t bw = act ? S(2) : S(1);
    du::box(layer, x, y, x + SCENE_W - 1, y + SCENE_H - 1, border, LV_OPA_COVER, theme::RADIUS_S);
    du::box(layer, x + bw, y + bw, x + SCENE_W - 1 - bw, y + SCENE_H - 1 - bw, bg, LV_OPA_COVER,
            theme::RADIUS_S - bw);
    const lv_font_t* fi = theme::fontM();
    const lv_font_t* fn = theme::fontS();
    int32_t lhI = lv_font_get_line_height(fi);
    int32_t block = lhI + lv_font_get_line_height(fn) - SCENE_OVERLAP;
    int32_t iy = y + (SCENE_H - block) / 2;
    du::text(layer, fi, sw, LV_TEXT_ALIGN_CENTER, x, iy, x + SCENE_W - 1, kScenes[i].icon);
    du::text(layer, fn, theme::col(theme::C_TEXT), LV_TEXT_ALIGN_CENTER, x, iy + lhI - SCENE_OVERLAP, x + SCENE_W - 1,
             rc::L(kScenes[i].name, kSceneNamesEn[i]));
  }
}

void homeClick(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_HOME, -1);
}

void deleted(lv_event_t* e) {
  (void)e;
  if (s_dragging) wiz::setInteracting(false);
  s_dragging = false;
  s_pressed = -1;
  s_scr = s_bulbBg = s_bulb = s_swatch = s_name = s_status = s_dot = s_power = s_powerIcon = nullptr;
  s_hue = s_dim = s_temp = nullptr;
  s_dimVal = s_tempVal = s_scenes = s_cover = nullptr;
}

lv_obj_t* slider(int32_t y, int32_t lo, int32_t hi) {
  lv_obj_t* s = lv_slider_create(s_scr);
  theme::styleSlider(s);
  lv_obj_set_style_pad_all(s, S(5), LV_PART_KNOB);
  lv_obj_set_pos(s, SLIDER_X, y);
  lv_obj_set_size(s, SLIDER_W, SLIDER_H);
  lv_slider_set_range(s, lo, hi);
  lv_obj_add_event_cb(s, sliderEvent, LV_EVENT_ALL, nullptr);
  return s;
}

lv_obj_t* rowIcon(int32_t y, const char* sym) {
  const lv_font_t* f = theme::fontM();
  lv_obj_t* l = theme::makeLabel(s_scr, f, theme::C_TEXT2);
  lv_label_set_text_static(l, sym);
  lv_obj_set_width(l, S(26));
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(l, S(5), y + (SLIDER_H - lv_font_get_line_height(f)) / 2);
  return l;
}

lv_obj_t* valueLabel(int32_t y) {
  const lv_font_t* f = theme::fontS();
  lv_obj_t* l = theme::makeLabel(s_scr, f, theme::C_TEXT);
  lv_obj_set_width(l, S(90));
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_RIGHT, 0);
  lv_obj_set_pos(l, R(222), y + (SLIDER_H - lv_font_get_line_height(f)) / 2);
  return l;
}

}

lv_obj_t* create() {
  buildGradients();
  s_dragging = false;
  s_pressed = -1;
  s_active = -1;
  s_scr = theme::makeScreen();
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  ui::attachGestures(s_scr);

  s_bulbBg = theme::makeBox(s_scr);
  lv_obj_set_size(s_bulbBg, S(36), S(36));
  lv_obj_set_pos(s_bulbBg, S(4), S(4));
  lv_obj_set_style_radius(s_bulbBg, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(s_bulbBg, LV_OPA_20, 0);
  s_bulb = theme::makeLabel(s_bulbBg, theme::fontL(), theme::C_WARN);
  lv_label_set_text_static(s_bulb, RS_BULB);
  lv_obj_center(s_bulb);
  s_name = theme::makeLabel(s_scr, theme::fontM(), theme::C_TEXT);
  lv_obj_set_pos(s_name, S(46), S(4));
  lv_obj_set_width(s_name, FW(150));
  theme::oneLine(s_name);
  s_dot = theme::makeBox(s_scr);
  lv_obj_set_size(s_dot, S(8), S(8));
  lv_obj_set_style_radius(s_dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(s_dot, LV_OPA_COVER, 0);
  lv_obj_set_pos(s_dot, S(47), S(29));
  s_status = theme::makeLabel(s_scr, theme::fontS(), theme::C_TEXT2);
  lv_obj_set_pos(s_status, S(60), S(24));
  lv_obj_set_width(s_status, FW(140));
  theme::oneLine(s_status);
  lv_obj_add_flag(s_status, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_ext_click_area(s_status, S(8));
  lv_obj_add_event_cb(s_status, pairClick, LV_EVENT_LONG_PRESSED, nullptr);

  lv_obj_t* home = theme::makeButton(s_scr, RS_HOME, theme::fontM());
  lv_obj_set_size(home, S(44), S(36));
  lv_obj_set_pos(home, R(206), S(6));
  lv_obj_add_event_cb(home, homeClick, LV_EVENT_CLICKED, nullptr);

  s_power = theme::makeButton(s_scr, nullptr, theme::fontL());
  lv_obj_set_size(s_power, S(58), S(36));
  lv_obj_set_pos(s_power, R(256), S(6));
  lv_obj_add_event_cb(s_power, powerClick, LV_EVENT_CLICKED, nullptr);
  s_powerIcon = theme::makeLabel(s_power, theme::fontL(), theme::C_TEXT);
  lv_label_set_text_static(s_powerIcon, RS_POWER);
  lv_obj_center(s_powerIcon);

  rowIcon(HUE_Y, RS_PALETTE);
  s_hue = slider(HUE_Y, 0, 359);
  lv_obj_set_style_bg_grad(s_hue, &s_hueGrad, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(s_hue, LV_OPA_TRANSP, LV_PART_INDICATOR);
  s_swatch = theme::makeBox(s_scr);
  lv_obj_set_size(s_swatch, SWATCH, SWATCH);
  lv_obj_set_pos(s_swatch, R(292), HUE_Y + (SLIDER_H - SWATCH) / 2);
  lv_obj_set_style_radius(s_swatch, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(s_swatch, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(s_swatch, S(1), 0);
  lv_obj_set_style_border_color(s_swatch, theme::col(theme::C_BORDER), 0);

  rowIcon(DIM_Y, RS_SUN);
  s_dimVal = valueLabel(DIM_Y);
  s_dim = slider(DIM_Y, rc::WIZ_DIM_MIN, rc::WIZ_DIM_MAX);

  rowIcon(TEMP_Y, RS_THERMO);
  s_tempVal = valueLabel(TEMP_Y);
  s_temp = slider(TEMP_Y, rc::WIZ_TEMP_MIN, rc::WIZ_TEMP_MAX);
  lv_obj_set_style_bg_grad(s_temp, &s_tempGrad, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(s_temp, LV_OPA_TRANSP, LV_PART_INDICATOR);

  s_scenes = theme::makeBox(s_scr);
  lv_obj_set_pos(s_scenes, S(6), SCENE_Y);
  lv_obj_set_size(s_scenes, 4 * SCENE_W + 3 * SCENE_GAP, 2 * SCENE_H + SCENE_GAP);
  lv_obj_add_flag(s_scenes, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_remove_flag(s_scenes, LV_OBJ_FLAG_GESTURE_BUBBLE);
  lv_obj_add_event_cb(s_scenes, sceneDraw, LV_EVENT_DRAW_MAIN, nullptr);
  lv_obj_add_event_cb(s_scenes, sceneEvent, LV_EVENT_ALL, nullptr);

  s_want = defaults();
  s_lastUserMs = 0;
  syncSliders();
  adopt(true);
  refreshControls();
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & (ui::M_BULB | ui::M_SETTINGS)) {
    updateStatus();
    adopt(mask == ui::M_ALL);
  }
}

}
