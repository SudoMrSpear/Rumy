#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "fonts/symbols.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"
#include "webui/webui.h"
#include "text_tr.h"

namespace scr_phone {

namespace {

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_qr = nullptr;
lv_obj_t* s_code = nullptr;
lv_obj_t* s_addr = nullptr;
lv_obj_t* s_count = nullptr;
lv_obj_t* s_forget = nullptr;
uint32_t s_ip = 0;
uint16_t s_shown = 0;

using theme::setText;

void back(lv_event_t* e) {
  (void)e;
  ui::back();
}

void forget(lv_event_t* e) {
  (void)e;
  webui::forgetPhones();
  ui::toast(rc::L("Eşli telefonlar unutuldu", "Paired phones forgotten"));
}

void step(lv_obj_t* parent, int32_t y, const char* num, const char* text) {
  lv_obj_t* n = theme::makeLabel(parent, theme::fontS(), theme::C_EYE);
  lv_label_set_text_static(n, num);
  lv_obj_set_pos(n, theme::S(150), y);
  lv_obj_t* t = theme::makeLabel(parent, theme::fontS(), theme::C_TEXT);
  lv_label_set_text_static(t, text);
  lv_obj_set_pos(t, theme::S(164), y);
  lv_obj_set_width(t, theme::FW(150));
}

void refresh() {
  const ui::Data& d = ui::data();
  bool mdns = webui::mdnsUp();
  uint32_t key = d.net.ip ^ (mdns ? 0x80000000u : 0u);
  if (key != s_ip) {
    s_ip = key;
    char ip[20];
    snprintf(ip, sizeof(ip), "%u.%u.%u.%u", static_cast<unsigned>(d.net.ip & 0xFF),
             static_cast<unsigned>((d.net.ip >> 8) & 0xFF), static_cast<unsigned>((d.net.ip >> 16) & 0xFF),
             static_cast<unsigned>((d.net.ip >> 24) & 0xFF));
    char url[40];
    snprintf(url, sizeof(url), "http://%s/", mdns ? "rumy.local" : ip);
    lv_qrcode_update(s_qr, url, strlen(url));
    char buf[48];
    if (mdns) snprintf(buf, sizeof(buf), "rumy.local  ·  %s", ip);
    else snprintf(buf, sizeof(buf), "%s", ip);
    setText(s_addr, buf);
  }
  uint16_t code = webui::pairCode();
  if (code != s_shown) {
    s_shown = code;
    char buf[8];
    snprintf(buf, sizeof(buf), "%04u", static_cast<unsigned>(code));
    setText(s_code, buf);
  }
  uint8_t n = webui::phones();
  char buf[40];
  if (n) snprintf(buf, sizeof(buf), rc::L(RS_OK "  %u telefon eşli", RS_OK "  %u phones paired"), static_cast<unsigned>(n));
  else snprintf(buf, sizeof(buf), "%s", rc::L("Henüz eşli telefon yok", "No phones paired yet"));
  setText(s_count, buf);
  if (n) lv_obj_remove_flag(s_forget, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(s_forget, LV_OBJ_FLAG_HIDDEN);
}

void deleted(lv_event_t* e) {
  (void)e;
  webui::setPairing(false);
  s_scr = s_qr = s_code = s_addr = s_count = s_forget = nullptr;
}

}

lv_obj_t* create() {
  s_ip = 0xFFFFFFFFu;
  s_shown = 0;
  s_scr = theme::makeScreen();
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  ui::attachGestures(s_scr);

  lv_obj_t* bk = theme::makeButton(s_scr, RS_LEFT, theme::fontM());
  lv_obj_set_size(bk, theme::S(44), theme::S(36));
  lv_obj_set_pos(bk, theme::S(6), theme::S(4));
  lv_obj_add_event_cb(bk, back, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* title = theme::makeLabel(s_scr, theme::fontL(), theme::C_TEXT);
  lv_label_set_text_static(title, rc::L("Telefon", "Phone"));
  lv_obj_set_pos(title, theme::S(60), theme::S(9));

  lv_obj_t* frame = theme::makeBox(s_scr);
  lv_obj_set_size(frame, theme::S(132), theme::S(132));
  lv_obj_set_pos(frame, theme::S(8), theme::S(48));
  lv_obj_set_style_bg_color(frame, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(frame, theme::S(8), 0);
  s_qr = lv_qrcode_create(frame);
  lv_qrcode_set_size(s_qr, theme::S(116));
  lv_qrcode_set_dark_color(s_qr, lv_color_black());
  lv_qrcode_set_light_color(s_qr, lv_color_white());
  lv_obj_center(s_qr);

  step(s_scr, theme::S(50), "1", rc::L("QR'ı iPhone kamerasıyla okut", "Scan the QR with your iPhone camera"));
  step(s_scr, theme::S(86), "2", rc::L("Paylaş, sonra Ana Ekrana Ekle", "Share, then Add to Home Screen"));
  step(s_scr, theme::S(122), "3", rc::L("Uygulamada bu kodu gir:", "Enter this code in the app:"));
  s_code = theme::makeLabel(s_scr, theme::fontNum(), theme::C_EYE);
  lv_obj_set_pos(s_code, theme::S(162), theme::S(140));

  s_addr = theme::makeLabel(s_scr, theme::fontS(), theme::C_TEXT2);
  lv_obj_set_pos(s_addr, theme::S(10), theme::S(184));
  lv_obj_set_width(s_addr, theme::FW(300));
  theme::oneLine(s_addr);

  s_count = theme::makeLabel(s_scr, theme::fontS(), theme::C_TEXT);
  lv_obj_set_pos(s_count, theme::S(10), theme::S(212));
  s_forget = theme::makeButton(s_scr, rc::L(RS_TRASH "  Unut", RS_TRASH "  Forget"), theme::fontS());
  lv_obj_set_size(s_forget, theme::S(84), theme::S(32));
  lv_obj_set_pos(s_forget, theme::SW - theme::S(90), theme::S(202));
  lv_obj_set_style_text_color(s_forget, theme::col(theme::C_DANGER), 0);
  lv_obj_add_event_cb(s_forget, forget, LV_EVENT_CLICKED, nullptr);

  webui::setPairing(true);
  refresh();
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & (ui::M_NET | ui::M_SETTINGS | ui::M_SECOND)) refresh();
}

}
