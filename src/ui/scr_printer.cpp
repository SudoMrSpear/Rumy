#include <stdio.h>
#include <string.h>

#include "clock/clock.h"
#include "draw_util.h"
#include "fonts/symbols.h"
#include "k2.h"
#include "media/camera.h"
#include "media/media.h"
#include "screens.h"
#include "text_tr.h"
#include "theme.h"
#include "timeutil.h"
#include "ui.h"

namespace scr_printer {

namespace {

using theme::FW;
using theme::S;

constexpr int32_t CAM_X = S(8);
constexpr int32_t CAM_Y = S(8);
constexpr int32_t CAM_W = cfg::CAM_W;
constexpr int32_t CAM_H = cfg::CAM_H;
constexpr int32_t COL_X = S(178);
constexpr int32_t COL_W = FW(136);
constexpr int32_t INFO_H = S(122);
constexpr int32_t BOT_Y = S(134);
constexpr int32_t BOT_H = S(104);
constexpr int32_t EDGE = S(8);

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_cam = nullptr;
lv_obj_t* s_info = nullptr;
lv_obj_t* s_bottom = nullptr;
lv_obj_t* s_frame = nullptr;
lv_timer_t* s_camTimer = nullptr;
bool s_camLive = false;
int8_t s_border = -1;

lv_color_t C(uint32_t hex) { return theme::col(hex); }

const PrinterState& P() { return ui::data().printer; }

int32_t textW(const lv_font_t* f, const char* s) {
  lv_point_t p;
  lv_text_get_size(&p, s, f, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_EXPAND);
  return p.x;
}

void fit(const lv_font_t* f, const char* src, int32_t maxW, char* out, size_t cap) {
  snprintf(out, cap, "%s", src);
  if (textW(f, out) <= maxW) return;
  size_t n = strlen(out);
  while (n > 0) {
    n--;
    while (n > 0 && (static_cast<uint8_t>(out[n]) & 0xC0) == 0x80) n--;
    if (n + 4 > cap) continue;
    memcpy(out + n, "\xE2\x80\xA6", 4);
    if (textW(f, out) <= maxW) return;
  }
}

rc::K2Phase phase() {
  const PrinterState& p = P();
  if (!p.connected) return rc::K2_OFFLINE;
  return static_cast<rc::K2Phase>(p.phase);
}

bool problem() { return P().connected && P().problem; }

uint32_t phaseColor(rc::K2Phase ph) {
  if (problem()) return theme::C_NEON_RED;
  switch (ph) {
    case rc::K2_PRINTING: return theme::C_NEON_GREEN;
    case rc::K2_PAUSED: return theme::C_NEON_YELLOW;
    case rc::K2_DONE: return theme::C_NEON_CYAN;
    case rc::K2_STOPPED: return theme::C_NEON_ORANGE;
    case rc::K2_SELFTEST: return theme::C_NEON_PURPLE;
    case rc::K2_OFFLINE: return theme::C_NEON_RED;
    default: return theme::C_TEXT2;
  }
}

bool hasJob(rc::K2Phase ph) {
  return ph == rc::K2_PRINTING || ph == rc::K2_PAUSED || ph == rc::K2_ERROR || ph == rc::K2_DONE ||
         ph == rc::K2_STOPPED || (problem() && P().st.file[0]);
}

void temp(char* buf, size_t cap, int16_t now10, int16_t target10) {
  if (now10 == rc::K2_NO_TEMP) {
    snprintf(buf, cap, "--");
    return;
  }
  int32_t a = (now10 + (now10 >= 0 ? 5 : -5)) / 10;
  if (target10 > 0 && target10 != rc::K2_NO_TEMP) {
    snprintf(buf, cap, "%ld/%ld°", static_cast<long>(a), static_cast<long>((target10 + 5) / 10));
  } else {
    snprintf(buf, cap, "%ld°", static_cast<long>(a));
  }
}

void drawCam(lv_event_t* e) {
  if (cam::live()) return;
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  du::box(layer, c.x1, c.y1, c.x2, c.y2, C(theme::C_SURFACE), LV_OPA_COVER, S(10));
  const lv_font_t* fl = theme::fontL();
  const lv_font_t* fs = theme::fontS();
  int32_t y = c.y1 + S(28);
  du::text(layer, fl, C(theme::C_TEXT3), LV_TEXT_ALIGN_CENTER, c.x1, y, c.x2, RS_CAMERA);
  bool link = media::state().link;
  du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, y + S(38), c.x2, rc::L("Kamera", "Camera"));
  du::text(layer, fs, C(theme::C_TEXT3), LV_TEXT_ALIGN_CENTER, c.x1, y + S(54), c.x2,
           link ? rc::L("görüntü bekleniyor", "waiting for image") : rc::L("bilgisayar köprüsü gerekli", "computer bridge required"));
}

void drawBadge(lv_layer_t* layer, int32_t x, int32_t y, int32_t x2, rc::K2Phase ph) {
  const lv_font_t* fm = theme::fontM();
  lv_color_t col = C(phaseColor(ph));
  char buf[48];
  if (problem()) {
    snprintf(buf, sizeof(buf), RS_WARNING " %s", rc::k2Runout(P().st) ? rc::L("Filament bitti", "Out of filament") : rc::L("Hata", "Error"));
    du::text(layer, fm, col, LV_TEXT_ALIGN_LEFT, x, y, x2, buf);
    return;
  }
  du::box(layer, x, y + S(6), x + S(8), y + S(14), col, LV_OPA_COVER, LV_RADIUS_CIRCLE);
  du::text(layer, fm, col, LV_TEXT_ALIGN_LEFT, x + S(13), y, x2, rc::k2PhaseTr(ph));
}

void drawProblemBox(lv_layer_t* layer, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
  const PrinterState& p = P();
  const lv_font_t* fs = theme::fontS();
  du::box(layer, x1, y1, x2, y2, C(0x3A0A10), LV_OPA_COVER, S(6));
  du::box(layer, x1, y1, x2, y1, C(theme::C_NEON_RED), LV_OPA_COVER, 0);
  char a[48];
  char b[80];
  if (rc::k2Runout(p.st)) {
    snprintf(a, sizeof(a), "%s", rc::L("Filament bitti,", "Out of filament,"));
    snprintf(b, sizeof(b), "%s", rc::L("baskı duraklatıldı", "print paused"));
  } else {
    snprintf(a, sizeof(a), "%s", rc::L("Yazıcı hata bildirdi", "Printer reported an error"));
    if (p.st.errText[0]) {
      snprintf(b, sizeof(b), rc::L("Kod %ld · %s", "Code %ld · %s"), static_cast<long>(p.st.errCode), p.st.errText);
    } else if (p.st.errKey) {
      snprintf(b, sizeof(b), rc::L("Kod %ld (%ld)", "Code %ld (%ld)"), static_cast<long>(p.st.errCode), static_cast<long>(p.st.errKey));
    } else {
      snprintf(b, sizeof(b), rc::L("Kod %ld", "Code %ld"), static_cast<long>(p.st.errCode));
    }
  }
  char fitted[80];
  fit(fs, b, x2 - x1 - S(12), fitted, sizeof(fitted));
  du::text(layer, fs, C(theme::C_WHITE), LV_TEXT_ALIGN_LEFT, x1 + S(6), y1 + S(5), x2, a);
  du::text(layer, fs, C(theme::C_WHITE), LV_TEXT_ALIGN_LEFT, x1 + S(6), y1 + S(20), x2, fitted);
}

void drawInfo(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const PrinterState& p = P();
  const lv_font_t* fs = theme::fontS();
  const lv_font_t* fm = theme::fontM();
  int32_t x = c.x1;
  int32_t w = c.x2 - c.x1;
  char buf[64];
  char out[64];
  snprintf(buf, sizeof(buf), "%s · Creality K2", p.st.host[0] ? p.st.host : rc::L("Yazıcı", "Printer"));
  fit(fs, buf, w, out, sizeof(out));
  du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x, c.y1, c.x2, out);
  rc::K2Phase ph = phase();
  drawBadge(layer, x, c.y1 + S(18), c.x2, ph);
  char name[48];
  char mat[12];
  rc::k2JobName(p.st.file, name, sizeof(name), mat, sizeof(mat));
  if (ph == rc::K2_OFFLINE) {
    snprintf(buf, sizeof(buf), "%u.%u.%u.%u", static_cast<unsigned>(p.ip & 0xFF), static_cast<unsigned>((p.ip >> 8) & 0xFF),
             static_cast<unsigned>((p.ip >> 16) & 0xFF), static_cast<unsigned>((p.ip >> 24) & 0xFF));
    du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, x, c.y1 + S(42), c.x2, rc::L("Ulaşılamıyor", "Unreachable"));
    du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x, c.y1 + S(64), c.x2, buf);
    du::text(layer, fs, C(theme::C_TEXT3), LV_TEXT_ALIGN_LEFT, x, c.y1 + S(82), c.x2, rc::L("Açık ve aynı ağda mı?", "Is it on and on the same network?"));
    return;
  }
  if (name[0] && hasJob(ph)) {
    fit(fm, name, w, out, sizeof(out));
    du::text(layer, fm, C(theme::C_WHITE), LV_TEXT_ALIGN_LEFT, x, c.y1 + S(42), c.x2, out);
    if (p.st.totalLayer) {
      snprintf(buf, sizeof(buf), rc::L("%s%sKatman %u / %u", "%s%sLayer %u / %u"), mat, mat[0] ? " · " : "", static_cast<unsigned>(p.st.layer),
               static_cast<unsigned>(p.st.totalLayer));
    } else {
      snprintf(buf, sizeof(buf), "%s", mat);
    }
    fit(fs, buf, w, out, sizeof(out));
    du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x, c.y1 + S(64), c.x2, out);
  } else {
    du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, x, c.y1 + S(42), c.x2, rc::L("Baskı yok", "No print"));
    du::text(layer, fs, C(theme::C_TEXT3), LV_TEXT_ALIGN_LEFT, x, c.y1 + S(64), c.x2, rc::L("Yazıcı hazır bekliyor", "Printer is ready"));
  }
  if (problem()) {
    drawProblemBox(layer, x - S(2), c.y1 + S(82), c.x2 - S(2), c.y2);
    return;
  }
  char a[24];
  char b[24];
  temp(a, sizeof(a), p.st.nozzle10, p.st.nozzleTarget10);
  du::text(layer, fm, C(theme::C_NEON_ORANGE), LV_TEXT_ALIGN_LEFT, x, c.y1 + S(84), x + S(14), RS_THERMO);
  snprintf(buf, sizeof(buf), rc::L("Nozul %s", "Nozzle %s"), a);
  du::text(layer, fs, C(theme::C_WHITE), LV_TEXT_ALIGN_LEFT, x + S(16), c.y1 + S(86), c.x2, buf);
  temp(a, sizeof(a), p.st.bed10, rc::K2_NO_TEMP);
  temp(b, sizeof(b), p.st.box10, rc::K2_NO_TEMP);
  snprintf(buf, sizeof(buf), rc::L("Tabla %s · Kabin %s", "Bed %s · Chamber %s"), a, b);
  fit(fs, buf, w - S(16), out, sizeof(out));
  du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x + S(16), c.y1 + S(102), c.x2, out);
}

void drawBottom(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const PrinterState& p = P();
  const lv_font_t* fs = theme::fontS();
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fl = theme::fontL();
  rc::K2Phase ph = phase();
  bool bad = problem();
  bool job = hasJob(ph);
  int32_t x1 = c.x1;
  int32_t x2 = c.x2;
  char buf[48];
  char dur[24];
  int32_t barY = c.y1 + S(38);
  if (!job) {
    du::box(layer, x1, barY, x2, barY + S(11), C(theme::C_BAR_BG), LV_OPA_COVER, S(6));
    const char* hint = p.configured ? (ph == rc::K2_OFFLINE ? rc::L("Bağlantı yeniden deneniyor", "Retrying connection") : rc::L("Şu an yazdırılan bir şey yok", "Nothing printing right now"))
                                    : rc::L("Yazıcı ayarlanmadı", "Printer not set up");
    du::text(layer, fm, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x1, c.y1 + S(6), x2, hint);
  } else {
    uint32_t pctColor = bad ? theme::C_NEON_RED : (ph == rc::K2_DONE ? theme::C_NEON_GREEN : theme::C_NEON_CYAN);
    snprintf(buf, sizeof(buf), "%%%u", static_cast<unsigned>(p.st.progress));
    du::text(layer, fl, C(pctColor), LV_TEXT_ALIGN_LEFT, x1, c.y1 + S(2), x2, buf);
    du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x1 + textW(fl, buf) + S(8), c.y1 + S(12), x2, rc::L("tamamlandı", "complete"));
    const char* right = nullptr;
    uint32_t rightColor = theme::C_NEON_YELLOW;
    char when[24];
    if (bad || ph == rc::K2_PAUSED) {
      right = rc::L("Bekliyor", "Waiting");
      rightColor = bad ? theme::C_NEON_RED : theme::C_NEON_YELLOW;
    } else if (ph == rc::K2_DONE) {
      right = rc::L("Bitti", "Done");
      rightColor = theme::C_NEON_GREEN;
    } else if (ph == rc::K2_STOPPED) {
      right = rc::L("Durduruldu", "Stopped");
      rightColor = theme::C_NEON_ORANGE;
    } else if (ph == rc::K2_PRINTING && clk::synced() && p.st.leftS) {
      uint32_t end = clk::nowUtc() + p.st.leftS;
      rc::LocalTime lt = rc::toLocal(end);
      char hm[8];
      rc::formatHm(hm, sizeof(hm), lt.hour, lt.minute);
      int32_t days = rc::localDayNum(end) - clk::todayDayNum();
      if (days == 0) snprintf(when, sizeof(when), rc::L("Bitiş %s", "Done at %s"), hm);
      else if (days == 1) snprintf(when, sizeof(when), rc::L("Bitiş yarın %s", "Done tomorrow %s"), hm);
      else snprintf(when, sizeof(when), rc::L("Bitiş %s %s", "Done %s %s"), rc::dayShortTr(lt.wday), hm);
      right = when;
    }
    if (right) du::text(layer, fm, C(rightColor), LV_TEXT_ALIGN_RIGHT, x1, c.y1 + S(6), x2, right);
    du::box(layer, x1, barY, x2, barY + S(11), C(theme::C_BAR_BG), LV_OPA_COVER, S(6));
    int32_t w = (x2 - x1) * p.st.progress / 100;
    if (w > 0) {
      if (w < S(12)) w = S(12);
      lv_color_t a = C(bad ? theme::C_NEON_RED : (ph == rc::K2_DONE ? theme::C_NEON_GREEN : theme::C_NEON_BLUE));
      lv_color_t b = C(bad ? theme::C_NEON_RED : (ph == rc::K2_DONE ? theme::C_NEON_GREEN : theme::C_NEON_CYAN));
      du::hgrad(layer, x1, barY, x1 + w, barY + S(11), a, b, S(6));
    }
    int32_t ty = barY + S(20);
    if (ph == rc::K2_DONE) {
      rc::durationTr(p.st.jobS, dur, sizeof(dur));
      snprintf(buf, sizeof(buf), rc::L("Süre %s", "Time %s"), dur);
      du::text(layer, fm, C(theme::C_WHITE), LV_TEXT_ALIGN_LEFT, x1, ty, x2, buf);
    } else {
      du::text(layer, fm, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x1, ty, x1 + S(18), RS_CLOCK);
      if (p.st.leftS) {
        rc::durationTr(p.st.leftS, dur, sizeof(dur));
        snprintf(buf, sizeof(buf), rc::L("Kalan %s", "Left %s"), dur);
      } else {
        snprintf(buf, sizeof(buf), "%s", rc::L("Kalan süre hesaplanıyor", "Estimating time left"));
      }
      du::text(layer, fm, C(theme::C_WHITE), LV_TEXT_ALIGN_LEFT, x1 + S(20), ty, x2, buf);
      rc::durationTr(p.st.jobS, dur, sizeof(dur));
      snprintf(buf, sizeof(buf), rc::L("Geçen %s", "Elapsed %s"), dur);
      du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_RIGHT, x1, ty + S(4), x2, buf);
    }
  }
  const char* back = media::state().link ? rc::L("Sola kaydır: Spotify", "Swipe left: Spotify") : rc::L("Sola kaydır: ana ekran", "Swipe left: home");
  du::text(layer, fs, C(theme::C_TEXT3), LV_TEXT_ALIGN_LEFT, x1, c.y2 - lv_font_get_line_height(fs) + 1, x2, back);
}

void drawSetup(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const lv_font_t* fl = theme::fontL();
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fs = theme::fontS();
  int32_t y = c.y1 + S(40);
  du::text(layer, fl, C(theme::C_NEON_CYAN), LV_TEXT_ALIGN_CENTER, c.x1, y, c.x2, RS_CUBE);
  du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_CENTER, c.x1, y + S(40), c.x2, rc::L("Yazıcı ayarlanmadı", "Printer not set up"));
  du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, y + S(66), c.x2,
           rc::L("Web arayüzünde \"3D yazıcı\" bölümüne", "In the web app's \"3D printer\" section,"));
  du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, y + S(82), c.x2, rc::L("Creality K2'nin IP adresini yaz.", "enter the Creality K2's IP address."));
}

void refresh() {
  bool configured = P().configured != 0;
  if (configured) {
    lv_obj_remove_flag(s_cam, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_info, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_bottom, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_frame, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(s_cam, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_info, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_bottom, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_frame, LV_OBJ_FLAG_HIDDEN);
  }
  int8_t bad = problem() ? 1 : 0;
  if (bad != s_border) {
    s_border = bad;
    lv_obj_set_style_border_width(s_scr, bad ? S(2) : 0, 0);
    lv_obj_set_style_border_color(s_scr, C(theme::C_NEON_RED), 0);
  }
  lv_obj_invalidate(s_info);
  lv_obj_invalidate(s_bottom);
}

void syncCam() {
  const PrinterState& p = P();
  cam::want(p.configured && media::state().link, p.ip, CAM_X, CAM_Y);
}

void camTick(lv_timer_t* t) {
  (void)t;
  if (!s_scr) return;
  syncCam();
  bool live = cam::live();
  if (live != s_camLive) {
    s_camLive = live;
    if (!live) lv_obj_invalidate(s_cam);
  }
}

void deleted(lv_event_t* e) {
  (void)e;
  if (s_camTimer) lv_timer_delete(s_camTimer);
  s_camTimer = nullptr;
  cam::want(false, 0, 0, 0);
  s_scr = s_cam = s_info = s_bottom = s_frame = nullptr;
}

lv_obj_t* drawObj(int32_t x, int32_t y, int32_t w, int32_t h, lv_event_cb_t cb) {
  lv_obj_t* o = theme::makeBox(s_scr);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(o, cb, LV_EVENT_DRAW_MAIN, nullptr);
  return o;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  ui::attachGestures(s_scr);
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  s_cam = drawObj(CAM_X, CAM_Y, CAM_W, CAM_H, drawCam);
  s_info = drawObj(COL_X, CAM_Y, COL_W, INFO_H, drawInfo);
  s_bottom = drawObj(EDGE, BOT_Y, theme::SW - 2 * EDGE, BOT_H, drawBottom);
  s_frame = drawObj(0, 0, theme::SW, theme::SH, drawSetup);
  s_border = -1;
  s_camLive = false;
  s_camTimer = lv_timer_create(camTick, 200, nullptr);
  syncCam();
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & (ui::M_PRINTER | ui::M_SETTINGS)) refresh();
  if (mask & ui::M_MINUTE) lv_obj_invalidate(s_bottom);
  if (mask & ui::M_MEDIA) {
    if (!cam::live()) lv_obj_invalidate(s_cam);
    lv_obj_invalidate(s_bottom);
  }
}

}
