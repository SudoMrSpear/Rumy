#include <Arduino.h>
#include <stdio.h>
#include <string.h>

#include "fonts/symbols.h"
#include "pairing.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"
#include "wiz/wiz.h"
#include "text_tr.h"

namespace scr_pair {

namespace {

constexpr int MAX_ROWS = rc::BulbList::CAP;

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_status = nullptr;
lv_obj_t* s_list = nullptr;
lv_obj_t* s_forget = nullptr;
lv_obj_t* s_rows[MAX_ROWS] = {};
uint32_t s_sig = 0;
char s_macs[MAX_ROWS][13];

using theme::setText;

void formatMac(const char* mac, char* out, size_t cap) {
  if (strlen(mac) != 12) {
    snprintf(out, cap, "%s", mac);
    return;
  }
  snprintf(out, cap, "%.2s:%.2s:%.2s:%.2s:%.2s:%.2s", mac, mac + 2, mac + 4, mac + 6, mac + 8, mac + 10);
}

void formatIp(uint32_t ip, char* out, size_t cap) {
  snprintf(out, cap, "%u.%u.%u.%u", static_cast<unsigned>(ip & 0xFF), static_cast<unsigned>((ip >> 8) & 0xFF),
           static_cast<unsigned>((ip >> 16) & 0xFF), static_cast<unsigned>((ip >> 24) & 0xFF));
}

void rowClick(lv_event_t* e) {
  intptr_t idx = reinterpret_cast<intptr_t>(lv_event_get_user_data(e));
  if (idx < 0 || idx >= MAX_ROWS || !s_macs[idx][0]) return;
  wiz::pair(s_macs[idx]);
  ui::toast(rc::L("Ampul eşleştirildi · kısa bir yanıp sönme ile tanıyabilirsin", "Bulb paired · it blinks briefly so you can spot it"));
}

void rescan(lv_event_t* e) {
  (void)e;
  wiz::scan();
  s_sig = 0xFFFFFFFFu;
}

void forget(lv_event_t* e) {
  (void)e;
  wiz::forget();
  ui::toast(rc::L("Eşleşme kaldırıldı", "Pairing removed"));
}

void back(lv_event_t* e) {
  (void)e;
  ui::back();
}

uint32_t signature(const ui::Data& d) {
  uint32_t s = 2166136261u;
  auto mix = [&s](const void* p, size_t n) {
    const uint8_t* b = static_cast<const uint8_t*>(p);
    for (size_t i = 0; i < n; i++) s = (s ^ b[i]) * 16777619u;
  };
  mix(&d.bulb.candidateCount, 1);
  for (int i = 0; i < d.bulb.candidateCount && i < MAX_ROWS; i++) {
    mix(d.bulb.candidates[i].mac, 12);
    mix(&d.bulb.candidates[i].ip, 4);
  }
  mix(d.settings.bulbMac, 12);
  mix(&d.bulb.online, 1);
  return s;
}

void rebuild() {
  const ui::Data& d = ui::data();
  uint32_t sig = signature(d);
  if (sig == s_sig) return;
  s_sig = sig;
  for (int i = 0; i < MAX_ROWS; i++) {
    if (s_rows[i]) lv_obj_delete(s_rows[i]);
    s_rows[i] = nullptr;
    s_macs[i][0] = 0;
  }
  int n = d.bulb.candidateCount < MAX_ROWS ? d.bulb.candidateCount : MAX_ROWS;
  for (int i = 0; i < n; i++) {
    const rc::FoundBulb& fb = d.bulb.candidates[i];
    memcpy(s_macs[i], fb.mac, 13);
    s_macs[i][12] = 0;
    bool mine = rc::macEqual(fb.mac, d.settings.bulbMac);
    lv_obj_t* r = theme::makeCard(s_list, theme::FW(308), theme::S(48));
    lv_obj_add_flag(r, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_style(r, theme::stylePressed(), LV_STATE_PRESSED);
    lv_obj_add_event_cb(r, rowClick, LV_EVENT_CLICKED, reinterpret_cast<void*>(static_cast<intptr_t>(i)));
    if (mine) lv_obj_set_style_border_color(r, theme::col(theme::C_EYE), 0);
    lv_obj_t* ic = theme::makeLabel(r, theme::fontL(), mine ? theme::C_EYE : theme::C_TEXT2);
    lv_label_set_text_static(ic, mine ? RS_OK : RS_BULB);
    lv_obj_set_pos(ic, theme::S(12), theme::S(12));
    char buf[64];
    formatMac(fb.mac, buf, sizeof(buf));
    lv_obj_t* t = theme::makeLabel(r, theme::fontM(), theme::C_TEXT);
    lv_label_set_text(t, buf);
    lv_obj_set_pos(t, theme::S(48), theme::S(5));
    char ip[20];
    formatIp(fb.ip, ip, sizeof(ip));
    snprintf(buf, sizeof(buf), "%s%s", ip, mine ? rc::L("  ·  eşleşmiş", "  ·  paired") : rc::L("  ·  dokun ve eşleştir", "  ·  tap to pair"));
    lv_obj_t* sub = theme::makeLabel(r, theme::fontS(), mine ? theme::C_EYE : theme::C_TEXT2);
    lv_label_set_text(sub, buf);
    lv_obj_set_pos(sub, theme::S(48), theme::S(27));
    s_rows[i] = r;
  }
}

void updateStatus() {
  const ui::Data& d = ui::data();
  char buf[64];
  if (d.bulb.scanning) snprintf(buf, sizeof(buf), "%s", rc::L(RS_SEARCH "  Ağda WiZ ampul aranıyor…", RS_SEARCH "  Searching for WiZ bulbs…"));
  else if (d.bulb.candidateCount == 0) snprintf(buf, sizeof(buf), "%s", rc::L("Ampul bulunamadı · ampul açık ve aynı ağda mı?", "No bulb found · is it on and on the same network?"));
  else snprintf(buf, sizeof(buf), rc::L("%u ampul bulundu · birine dokun", "%u bulbs found · tap one"), static_cast<unsigned>(d.bulb.candidateCount));
  setText(s_status, buf);
  if (d.settings.bulbMac[0]) lv_obj_remove_flag(s_forget, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(s_forget, LV_OBJ_FLAG_HIDDEN);
}

void deleted(lv_event_t* e) {
  (void)e;
  s_scr = s_status = s_list = s_forget = nullptr;
  for (int i = 0; i < MAX_ROWS; i++) s_rows[i] = nullptr;
}

}

lv_obj_t* create() {
  s_sig = 0;
  memset(s_macs, 0, sizeof(s_macs));
  s_scr = theme::makeScreen();
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  ui::attachGestures(s_scr);

  lv_obj_t* bk = theme::makeButton(s_scr, RS_LEFT, theme::fontM());
  lv_obj_set_size(bk, theme::S(44), theme::S(36));
  lv_obj_set_pos(bk, theme::S(6), theme::S(4));
  lv_obj_add_event_cb(bk, back, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* title = theme::makeLabel(s_scr, theme::fontL(), theme::C_TEXT);
  lv_label_set_text_static(title, rc::L("Ampul eşleştirme", "Bulb pairing"));
  lv_obj_set_pos(title, theme::S(60), theme::S(9));

  s_status = theme::makeLabel(s_scr, theme::fontS(), theme::C_TEXT2);
  lv_obj_set_pos(s_status, theme::S(10), theme::S(44));
  lv_obj_set_width(s_status, theme::FW(300));
  theme::oneLine(s_status);

  s_list = theme::makeBox(s_scr);
  lv_obj_set_pos(s_list, 0, theme::S(62));
  lv_obj_set_size(s_list, theme::SW, theme::S(128));
  lv_obj_add_flag(s_list, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(s_list, LV_DIR_VER);
  theme::styleScrollbar(s_list);
  lv_obj_set_flex_flow(s_list, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(s_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_row(s_list, theme::S(6), 0);

  lv_obj_t* scan = theme::makeButton(s_scr, rc::L(RS_REFRESH "  Yeniden ara", RS_REFRESH "  Search again"), theme::fontM());
  const int32_t bw = (theme::FW(308) - theme::S(8)) / 2;
  lv_obj_set_size(scan, bw, theme::S(40));
  lv_obj_set_pos(scan, theme::S(6), theme::S(196));
  lv_obj_add_event_cb(scan, rescan, LV_EVENT_CLICKED, nullptr);
  s_forget = theme::makeButton(s_scr, rc::L(RS_TRASH "  Unut", RS_TRASH "  Forget"), theme::fontM());
  lv_obj_set_size(s_forget, bw, theme::S(40));
  lv_obj_set_pos(s_forget, theme::S(6) + bw + theme::S(8), theme::S(196));
  lv_obj_set_style_text_color(s_forget, theme::col(theme::C_DANGER), 0);
  lv_obj_add_event_cb(s_forget, forget, LV_EVENT_CLICKED, nullptr);

  wiz::scan();
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & (ui::M_BULB | ui::M_SETTINGS)) {
    updateStatus();
    rebuild();
  }
}

}
