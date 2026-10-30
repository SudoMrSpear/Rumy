#include <stdio.h>
#include <string.h>

#include "app/config.h"
#include "draw_util.h"
#include "fonts/symbols.h"
#include "media/media.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"
#include "text_tr.h"

namespace scr_media {

namespace {

using theme::CX;
using theme::FW;
using theme::S;

constexpr int32_t ART_X = S(10);
constexpr int32_t ART_Y = S(10);
constexpr int32_t ART = cfg::MEDIA_ART;
constexpr int32_t COL_X = S(164);
constexpr int32_t COL_W = FW(148);
constexpr int32_t BAR_Y = S(164);
constexpr int32_t BAR_H = S(28);
constexpr int32_t BAR_PAD = S(10);
constexpr int32_t TRACK_H = S(5);
constexpr int32_t KNOB_R = S(5);
constexpr int32_t KNOB_DRAG_R = S(7);
constexpr int32_t HALO_R = S(10);
constexpr int32_t BTN_Y = S(196);
constexpr int32_t LIST_BTN = S(32);
constexpr int32_t LIST_INSET = S(5);
constexpr uint32_t C_SPOTIFY = 0x1ED760;

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_art = nullptr;
lv_obj_t* s_head = nullptr;
lv_obj_t* s_title = nullptr;
lv_obj_t* s_artist = nullptr;
lv_obj_t* s_album = nullptr;
lv_obj_t* s_bar = nullptr;
lv_obj_t* s_play = nullptr;
lv_obj_t* s_playIcon = nullptr;
lv_obj_t* s_btn[4] = {};
lv_obj_t* s_listBtn = nullptr;
int32_t s_dragMs = -1;
uint32_t s_volUntil = 0;
int8_t s_volSeen = -1;
uint32_t s_volPressAt = 0;
uint32_t s_volStepAt = 0;

lv_color_t C(uint32_t hex) { return theme::col(hex); }

uint32_t accent() {
  uint32_t a = media::state().accent;
  return a ? a : C_SPOTIFY;
}

void fmtTime(char* buf, size_t n, uint32_t ms) {
  uint32_t s = ms / 1000;
  snprintf(buf, n, "%lu:%02lu", static_cast<unsigned long>(s / 60), static_cast<unsigned long>(s % 60));
}

void drawArt(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const lv_image_dsc_t* img = media::art();
  if (img && media::state().active) {
    int32_t x = c.x1 + (lv_area_get_width(&c) - static_cast<int32_t>(img->header.w)) / 2;
    int32_t y = c.y1 + (lv_area_get_height(&c) - static_cast<int32_t>(img->header.h)) / 2;
    du::image(layer, img, x, y);
    return;
  }
  du::box(layer, c.x1, c.y1, c.x2, c.y2, C(theme::C_SURFACE), LV_OPA_COVER, S(12));
  const lv_font_t* f = theme::fontL();
  int32_t y = c.y1 + (lv_area_get_height(&c) - lv_font_get_line_height(f)) / 2;
  du::text(layer, f, C(accent()), LV_TEXT_ALIGN_CENTER, c.x1, y, c.x2, RS_MUSIC);
}

void drawHead(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const media::State& m = media::state();
  const char* app = m.app[0] ? m.app : rc::L("Müzik", "Music");
  bool spotify = !strncmp(app, "Spotify", 7);
  du::text(layer, theme::fontM(), C(spotify ? C_SPOTIFY : accent()), LV_TEXT_ALIGN_LEFT, c.x1, c.y1, c.x1 + S(20),
           spotify ? RS_SPOTIFY : RS_MUSIC);
  char buf[40];
  snprintf(buf, sizeof(buf), "%s%s", app, m.active && !m.playing ? rc::L(" · duraklatıldı", " · paused") : "");
  du::text(layer, theme::fontS(), C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, c.x1 + S(22), c.y1 + S(2), c.x2, buf);
}

bool volShown() { return s_volUntil && static_cast<int32_t>(s_volUntil - lv_tick_get()) > 0; }

void showVol() {
  s_volUntil = (lv_tick_get() + cfg::MEDIA_VOL_SHOW_MS) | 1u;
  if (s_bar) lv_obj_invalidate(s_bar);
}

void drawVol(lv_layer_t* layer, int32_t x1, int32_t x2, int32_t ty) {
  int8_t v = media::state().vol;
  if (v > 0) {
    int32_t w = (x2 - x1 + 1) * v / 100;
    du::box(layer, x1, ty, x1 + LV_MAX(w, TRACK_H) - 1, ty + TRACK_H, C(theme::C_TEXT), LV_OPA_COVER, S(3));
  }
  const lv_font_t* f = theme::fontS();
  du::text(layer, f, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x1, ty + S(10), x2, rc::L(RS_VOL_UP "  Spotify sesi", RS_VOL_UP "  Spotify volume"));
  char buf[8];
  if (v >= 0) snprintf(buf, sizeof(buf), "%%%d", v);
  else snprintf(buf, sizeof(buf), "--");
  du::text(layer, f, C(theme::C_TEXT), LV_TEXT_ALIGN_RIGHT, x1, ty + S(10), x2, buf);
}

void drawBar(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const media::State& m = media::state();
  int32_t x1 = c.x1 + BAR_PAD;
  int32_t x2 = c.x2 - BAR_PAD;
  int32_t ty = c.y1 + BAR_PAD;
  du::box(layer, x1, ty, x2, ty + TRACK_H, C(theme::C_BAR_BG), LV_OPA_COVER, S(3));
  if (volShown() && s_dragMs < 0) {
    drawVol(layer, x1, x2, ty);
    return;
  }
  if (!m.active) return;
  bool drag = s_dragMs >= 0;
  uint32_t pos = drag ? static_cast<uint32_t>(s_dragMs) : media::positionMs();
  if (m.durMs) {
    if (pos > m.durMs) pos = m.durMs;
    int32_t w = static_cast<int32_t>(static_cast<uint64_t>(x2 - x1 + 1) * pos / m.durMs);
    if (w > 0) du::box(layer, x1, ty, x1 + w - 1, ty + TRACK_H, C(accent()), LV_OPA_COVER, S(3));
    int32_t kx = x1 + w;
    int32_t ky = ty + TRACK_H / 2;
    if (drag) du::box(layer, kx - HALO_R, ky - HALO_R, kx + HALO_R, ky + HALO_R, C(accent()), LV_OPA_40, HALO_R);
    int32_t r = drag ? KNOB_DRAG_R : KNOB_R;
    du::box(layer, kx - r, ky - r, kx + r, ky + r, C(theme::C_TEXT), LV_OPA_COVER, r);
  }
  char a[12], b[12];
  fmtTime(a, sizeof(a), pos);
  fmtTime(b, sizeof(b), m.durMs);
  du::text(layer, theme::fontS(), C(drag ? accent() : theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x1, ty + S(10), x2, a);
  if (m.durMs) du::text(layer, theme::fontS(), C(theme::C_TEXT2), LV_TEXT_ALIGN_RIGHT, x1, ty + S(10), x2, b);
}

int32_t msAt(lv_obj_t* obj, uint32_t durMs) {
  lv_point_t p;
  lv_indev_get_point(lv_indev_active(), &p);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  int32_t x1 = c.x1 + BAR_PAD;
  int32_t x2 = c.x2 - BAR_PAD;
  int32_t x = p.x < x1 ? x1 : (p.x > x2 ? x2 : p.x);
  return static_cast<int32_t>(static_cast<uint64_t>(x - x1) * durMs / static_cast<uint32_t>(x2 - x1));
}

void barCb(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_event_code_t code = lv_event_get_code(e);
  const media::State& m = media::state();
  if (code == LV_EVENT_PRESSED && s_volUntil) {
    s_volUntil = 0;
    lv_obj_invalidate(obj);
  }
  if (!m.link || !m.active || !m.durMs || code == LV_EVENT_PRESS_LOST) {
    if (s_dragMs >= 0) lv_obj_invalidate(obj);
    s_dragMs = -1;
    return;
  }
  if (code == LV_EVENT_RELEASED) {
    if (s_dragMs >= 0) media::seek(static_cast<uint32_t>(s_dragMs));
    s_dragMs = -1;
    lv_obj_invalidate(obj);
    return;
  }
  int32_t ms = msAt(obj, m.durMs);
  if (ms != s_dragMs) {
    s_dragMs = ms;
    lv_obj_invalidate(obj);
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

lv_obj_t* textLabel(int32_t y, int32_t h, const lv_font_t* font, uint32_t color) {
  lv_obj_t* l = theme::makeLabel(s_scr, font, color);
  lv_obj_set_pos(l, COL_X, y);
  lv_obj_set_size(l, COL_W, h);
  lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_DOTS);
  return l;
}

media::Cmd eventCmd(lv_event_t* e) {
  return static_cast<media::Cmd>(reinterpret_cast<uintptr_t>(lv_event_get_user_data(e)));
}

bool isVol(media::Cmd cmd) { return cmd == media::CMD_VOL_UP || cmd == media::CMD_VOL_DOWN; }

void buttonCb(lv_event_t* e) { media::send(eventCmd(e)); }

void volCb(lv_event_t* e) {
  uint32_t now = lv_tick_get();
  if (lv_event_get_code(e) == LV_EVENT_PRESSED) {
    s_volPressAt = now;
  } else if (lv_tick_diff(now, s_volPressAt) < cfg::MEDIA_VOL_HOLD_MS ||
             lv_tick_diff(now, s_volStepAt) < cfg::MEDIA_VOL_REPEAT_MS) {
    return;
  }
  s_volStepAt = now;
  media::send(eventCmd(e));
  showVol();
}

lv_obj_t* button(int32_t cx, int32_t size, const char* icon, media::Cmd cmd, lv_obj_t** iconOut) {
  lv_obj_t* b = theme::makeBox(s_scr);
  lv_obj_set_size(b, size, size);
  lv_obj_set_pos(b, cx - size / 2, BTN_Y + (S(40) - size) / 2);
  lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(b, C(theme::C_BG), 0);
  lv_obj_set_style_border_width(b, S(2), 0);
  lv_obj_set_style_border_color(b, C(theme::C_BORDER), 0);
  lv_obj_add_style(b, theme::stylePressed(), LV_STATE_PRESSED);
  lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_ext_click_area(b, S(10));
  void* ud = reinterpret_cast<void*>(static_cast<uintptr_t>(cmd));
  if (isVol(cmd)) {
    lv_obj_add_event_cb(b, volCb, LV_EVENT_PRESSED, ud);
    lv_obj_add_event_cb(b, volCb, LV_EVENT_PRESSING, ud);
  } else {
    lv_obj_add_event_cb(b, buttonCb, LV_EVENT_CLICKED, ud);
  }
  lv_obj_t* l = theme::makeLabel(b, size > S(40) ? theme::fontL() : theme::fontM(), theme::C_TEXT);
  lv_label_set_text_static(l, icon);
  lv_obj_center(l);
  if (iconOut) *iconOut = l;
  return b;
}

void listClick(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_PLAYLIST, -1);
}

lv_obj_t* listButton() {
  lv_obj_t* b = theme::makeBox(s_scr);
  lv_obj_set_size(b, LIST_BTN, LIST_BTN);
  lv_obj_set_pos(b, ART_X + ART - LIST_BTN - LIST_INSET, ART_Y + ART - LIST_BTN - LIST_INSET);
  lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(b, C(theme::C_BG), 0);
  lv_obj_set_style_border_width(b, S(2), 0);
  lv_obj_set_style_border_color(b, C(theme::C_BORDER), 0);
  lv_obj_add_style(b, theme::stylePressed(), LV_STATE_PRESSED);
  lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_ext_click_area(b, S(8));
  lv_obj_add_event_cb(b, listClick, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* l = theme::makeLabel(b, theme::fontM(), theme::C_TEXT);
  lv_label_set_text_static(l, RS_LIST);
  lv_obj_center(l);
  return b;
}

void refresh() {
  const media::State& m = media::state();
  uint32_t acc = accent();
  if (!m.link) {
    lv_label_set_text_static(s_title, rc::L("Bilgisayar bağlı değil", "Computer not connected"));
    lv_label_set_text_static(s_artist, "spotify_bridge.py");
    lv_label_set_text_static(s_album, RUMY_PI ? rc::L("bilgisayarda köprüyü --pi ile çalıştır", "run the bridge with --pi on your computer")
                                              : rc::L("USB'den bağlı bilgisayarda köprüyü çalıştır", "run the bridge on the USB-connected computer"));
  } else if (!m.active) {
    lv_label_set_text_static(s_title, rc::L("Müzik çalmıyor", "No music playing"));
    lv_label_set_text_static(s_artist, rc::L("Spotify'da bir şarkı aç", "Play a song in Spotify"));
    lv_label_set_text_static(s_album, "");
  } else {
    char buf[sizeof(m.title)];
    lv_label_set_text(s_title, theme::printable(lv_obj_get_style_text_font(s_title, LV_PART_MAIN), m.title, buf,
                                                sizeof(buf), rc::L("Bilinmeyen ad", "Unknown title")));
    lv_label_set_text(s_artist, theme::printable(lv_obj_get_style_text_font(s_artist, LV_PART_MAIN), m.artist, buf,
                                                 sizeof(buf), ""));
    lv_label_set_text(s_album, theme::printable(lv_obj_get_style_text_font(s_album, LV_PART_MAIN), m.album, buf,
                                                sizeof(buf), ""));
  }
  lv_obj_set_style_text_color(s_artist, C(acc), 0);
  lv_obj_set_style_bg_color(s_play, C(acc), 0);
  lv_obj_set_style_border_color(s_play, C(acc), 0);
  lv_label_set_text_static(s_playIcon, m.active && m.playing ? RS_PAUSE : RS_PLAY);
  lv_obj_set_style_text_color(s_playIcon, C(theme::C_BG), 0);
  for (lv_obj_t* b : s_btn) lv_obj_set_style_border_color(b, lv_color_mix(C(acc), C(theme::C_BORDER), 110), 0);
  lv_obj_set_style_border_color(s_listBtn, C(acc), 0);
  if (m.vol != s_volSeen) {
    if (m.vol >= 0 && s_volSeen >= 0) showVol();
    s_volSeen = m.vol;
  }
  lv_obj_invalidate(s_art);
  lv_obj_invalidate(s_head);
  lv_obj_invalidate(s_bar);
}

void deleted(lv_event_t* e) {
  (void)e;
  s_scr = s_art = s_head = s_title = s_artist = s_album = s_bar = s_play = s_playIcon = nullptr;
  for (lv_obj_t*& b : s_btn) b = nullptr;
  s_listBtn = nullptr;
  s_dragMs = -1;
  s_volUntil = 0;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  ui::attachGestures(s_scr);
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);

  s_art = drawObj(ART_X, ART_Y, ART, ART, drawArt);
  s_head = drawObj(COL_X, S(12), COL_W, S(20), drawHead);
  s_title = textLabel(S(34), S(58), theme::fontL(), theme::C_TEXT);
  s_artist = textLabel(S(98), S(20), theme::fontM(), theme::C_TEXT2);
  s_album = textLabel(S(122), S(32), theme::fontS(), theme::C_TEXT3);
  s_bar = drawObj(ART_X - BAR_PAD, BAR_Y - BAR_PAD, theme::SW - 2 * (ART_X - BAR_PAD), BAR_H + BAR_PAD, drawBar);
  lv_obj_add_flag(s_bar, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(s_bar, LV_OBJ_FLAG_PRESS_LOCK);
  lv_obj_remove_flag(s_bar, LV_OBJ_FLAG_GESTURE_BUBBLE);
  lv_obj_add_event_cb(s_bar, barCb, LV_EVENT_PRESSED, nullptr);
  lv_obj_add_event_cb(s_bar, barCb, LV_EVENT_PRESSING, nullptr);
  lv_obj_add_event_cb(s_bar, barCb, LV_EVENT_RELEASED, nullptr);
  lv_obj_add_event_cb(s_bar, barCb, LV_EVENT_PRESS_LOST, nullptr);

  s_btn[0] = button(CX(36), S(38), RS_VOL_DOWN, media::CMD_VOL_DOWN, nullptr);
  s_btn[1] = button(CX(98), S(40), RS_PREV, media::CMD_PREV, nullptr);
  s_play = button(CX(160), S(46), RS_PLAY, media::CMD_PLAY_PAUSE, &s_playIcon);
  s_btn[2] = button(CX(222), S(40), RS_NEXT, media::CMD_NEXT, nullptr);
  s_btn[3] = button(CX(284), S(38), RS_VOL_UP, media::CMD_VOL_UP, nullptr);
  lv_obj_set_pos(s_play, CX(160) - S(23), BTN_Y - S(3));
  s_listBtn = listButton();
  s_volSeen = media::state().vol;
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & (ui::M_MEDIA | ui::M_SETTINGS)) refresh();
  if ((mask & ui::M_SECOND) && media::state().playing) lv_obj_invalidate(s_bar);
  if ((mask & ui::M_SECOND) && s_volUntil && !volShown()) {
    s_volUntil = 0;
    lv_obj_invalidate(s_bar);
  }
}

}
