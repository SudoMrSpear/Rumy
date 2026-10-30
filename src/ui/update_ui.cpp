#include "update_ui.h"

#include <lvgl.h>
#include <stdio.h>
#include <string.h>

#include "media/fwupdate.h"
#include "theme.h"
#include "ui.h"
#include "text_tr.h"

namespace update_ui {

namespace {

using theme::S;

constexpr uint32_t C_GLOW = 0x063C48;
constexpr int32_t EYE_W = S(26);
constexpr int32_t EYE_H = S(32);
constexpr int32_t EYE_GAP = S(42);
constexpr int32_t EYE_Y = S(40);
constexpr int32_t GLOW = S(3);
constexpr int32_t BAR_W = S(240);
constexpr int32_t BAR_H = S(10);

lv_obj_t* s_box = nullptr;
lv_obj_t* s_eye[2] = {nullptr, nullptr};
lv_obj_t* s_glow[2] = {nullptr, nullptr};
lv_obj_t* s_title = nullptr;
lv_obj_t* s_bar = nullptr;
lv_obj_t* s_line = nullptr;
lv_obj_t* s_note = nullptr;
fwupdate::Phase s_shown = fwupdate::FW_IDLE;
int s_pct = -1;

lv_obj_t* rect(lv_obj_t* parent, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color) {
  lv_obj_t* o = lv_obj_create(parent);
  lv_obj_remove_style_all(o);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_set_style_radius(o, r, 0);
  lv_obj_set_style_bg_color(o, theme::col(color), 0);
  lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
  return o;
}

lv_obj_t* label(const lv_font_t* font, uint32_t color, int32_t y) {
  lv_obj_t* l = theme::makeLabel(s_box, font, color);
  lv_obj_set_width(l, theme::SW);
  lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_pos(l, 0, y);
  return l;
}

void build() {
  s_box = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(s_box);
  lv_obj_set_size(s_box, theme::SW, theme::SH);
  lv_obj_set_style_bg_color(s_box, theme::col(theme::C_BG), 0);
  lv_obj_set_style_bg_opa(s_box, LV_OPA_COVER, 0);
  lv_obj_add_flag(s_box, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_remove_flag(s_box, LV_OBJ_FLAG_SCROLLABLE);
  int32_t x0 = (theme::SW - EYE_GAP - EYE_W) / 2;
  for (int i = 0; i < 2; i++) {
    int32_t x = x0 + i * EYE_GAP;
    s_glow[i] = rect(s_box, x - GLOW, EYE_Y - GLOW, EYE_W + 2 * GLOW, EYE_H + 2 * GLOW, S(11), C_GLOW);
    s_eye[i] = rect(s_box, x, EYE_Y, EYE_W, EYE_H, S(8), theme::C_EYE);
  }
  s_title = label(theme::fontM(), theme::C_TEXT, S(98));
  s_bar = lv_bar_create(s_box);
  lv_obj_remove_style_all(s_bar);
  lv_obj_set_size(s_bar, BAR_W, BAR_H);
  lv_obj_set_pos(s_bar, (theme::SW - BAR_W) / 2, S(134));
  lv_obj_set_style_radius(s_bar, BAR_H / 2, 0);
  lv_obj_set_style_radius(s_bar, BAR_H / 2, LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(s_bar, theme::col(theme::C_BAR_BG), 0);
  lv_obj_set_style_bg_opa(s_bar, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(s_bar, theme::col(theme::C_EYE), LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(s_bar, LV_OPA_COVER, LV_PART_INDICATOR);
  lv_bar_set_range(s_bar, 0, 100);
  s_line = label(theme::fontS(), theme::C_TEXT2, S(154));
  s_note = label(theme::fontS(), theme::C_TEXT3, S(206));
  s_shown = fwupdate::FW_IDLE;
  s_pct = -1;
  ui::wake();
}

void close() {
  if (s_box) lv_obj_delete(s_box);
  s_box = nullptr;
  s_shown = fwupdate::FW_IDLE;
}

void eyes(uint32_t color, uint32_t glow) {
  for (int i = 0; i < 2; i++) {
    lv_obj_set_style_bg_color(s_eye[i], theme::col(color), 0);
    lv_obj_set_style_bg_color(s_glow[i], theme::col(glow), 0);
  }
}

const char* reason(const char* why) {
  if (!strcmp(why, "zaman asimi")) return rc::L("Bağlantı koptu", "Connection lost");
  if (!strcmp(why, "okuma")) return rc::L("Bağlantı okunamadı", "Couldn't read the connection");
  if (!strcmp(why, "sha")) return rc::L("Dosya bozuk geldi", "File arrived corrupted");
  if (!strcmp(why, "imaj") || !strcmp(why, "imaj ozeti")) return rc::L("Geçersiz yazılım dosyası", "Invalid firmware file");
  if (!strcmp(why, "guncelleyici imaji")) return rc::L("Yanlış dosya gönderildi", "Wrong file sent");
  if (!strncmp(why, "sd", 2)) return rc::L("SD karta yazılamadı", "Couldn't write to SD card");
  if (!strcmp(why, "acilis bolumu")) return rc::L("Güncelleyici açılamadı", "Couldn't start the updater");
  return why;
}

void mb(uint32_t bytes, char* out, size_t cap) {
  uint32_t tenth = (bytes + 52428) / 104858;
  snprintf(out, cap, "%lu,%lu", static_cast<unsigned long>(tenth / 10), static_cast<unsigned long>(tenth % 10));
}

void showProgress(const fwupdate::Progress& p) {
  int pct = p.total ? static_cast<int>(static_cast<uint64_t>(p.got) * 100 / p.total) : 0;
  if (pct == s_pct) return;
  s_pct = pct;
  lv_bar_set_value(s_bar, pct, LV_ANIM_OFF);
  char got[12];
  char total[12];
  mb(p.got, got, sizeof(got));
  mb(p.total, total, sizeof(total));
  char text[48];
  snprintf(text, sizeof(text), rc::L("%%%d · %s / %s MB", "%d%% · %s / %s MB"), pct, got, total);
  lv_label_set_text(s_line, text);
}

void show(const fwupdate::Progress& p) {
  if (p.phase != s_shown) {
    s_shown = p.phase;
    s_pct = -1;
    lv_obj_set_style_text_color(s_title, theme::col(theme::C_TEXT), 0);
    lv_obj_remove_flag(s_bar, LV_OBJ_FLAG_HIDDEN);
    if (p.phase == fwupdate::FW_RX) {
      eyes(theme::C_EYE, C_GLOW);
      lv_label_set_text(s_title, rc::L("Güncelleme alınıyor", "Receiving update"));
      lv_label_set_text(s_note, rc::L("Kapatmayın · Rumy kendini yeniden başlatacak", "Don't power off · Rumy will restart itself"));
    } else if (p.phase == fwupdate::FW_DONE) {
      eyes(theme::C_NEON_GREEN, 0x0B4A1F);
      lv_label_set_text(s_title, rc::L("Güncelleme alındı", "Update received"));
      lv_bar_set_value(s_bar, 100, LV_ANIM_OFF);
      lv_label_set_text(s_line, rc::L("Yeniden başlıyor…", "Restarting…"));
      lv_label_set_text(s_note, rc::L("Kapatmayın · yeni yazılım kuruluyor", "Don't power off · installing new firmware"));
    } else if (p.phase == fwupdate::FW_FAIL) {
      eyes(theme::C_NEON_RED, 0x4A0E0E);
      lv_obj_set_style_text_color(s_title, theme::col(theme::C_DANGER), 0);
      lv_label_set_text(s_title, rc::L("Güncelleme alınamadı", "Update failed"));
      lv_obj_add_flag(s_bar, LV_OBJ_FLAG_HIDDEN);
      lv_label_set_text(s_line, reason(p.why));
      lv_label_set_text(s_note, rc::L("Eski yazılım çalışmaya devam ediyor", "The old firmware keeps running"));
    }
  }
  if (p.phase == fwupdate::FW_RX) showProgress(p);
  lv_obj_move_foreground(s_box);
}

}

void tick() {
  fwupdate::Progress p = fwupdate::progress();
  if (p.phase == fwupdate::FW_IDLE) {
    if (s_box) close();
    return;
  }
  if (!s_box) {
    if (p.phase == fwupdate::FW_FAIL) return;
    build();
  }
  show(p);
}

bool active() { return s_box != nullptr; }

}
