#include <Arduino.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "app/app.h"
#include "app/config.h"
#include "app/mem.h"
#include "app/store.h"
#include "board/display.h"
#include "board/sdcard.h"
#include "clock/clock.h"
#include "diag/diag.h"
#include "fonts/symbols.h"
#include "net/net.h"
#include "draw_util.h"
#include "screens.h"
#include "text_tr.h"
#include "theme.h"
#include "ui.h"
#include "wiz/wiz.h"

namespace scr_diag {

namespace {

const char* const kKeys =
    "Sürüm\nÇalışma süresi\nAçılış\nHeap boş\nHeap en düşük\nEn büyük blok\nHeap eğilimi\nLVGL havuzu\nLVGL tepe\n"
    "FPS\nYığın UI/ağ/WiZ\nTLS tepe\nWiFi\nSD kart\nSaat\nHava + 7 gün\nDeniz\n"
    "Hava kalitesi\nDeprem\nTatiller\nAmpul";

const char* const kKeysEn =
    "Version\nUptime\nBoot\nHeap free\nHeap min\nLargest block\nHeap trend\nLVGL pool\nLVGL peak\n"
    "FPS\nStack UI/net/WiZ\nTLS peak\nWiFi\nSD card\nClock\nWeather + 7 days\nSea\n"
    "Air quality\nEarthquake\nHolidays\nBulb";

constexpr int32_t VAL_W = theme::FW(188);

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_vals = nullptr;
uint32_t s_lastMs = 0;

struct Buf {
  char* p;
  size_t left;
  void add(const char* fmt, ...) {
    if (left < 2) return;
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(p, left, fmt, ap);
    va_end(ap);
    if (n < 0) return;
    size_t k = static_cast<size_t>(n) < left ? static_cast<size_t>(n) : left - 1;
    p += k;
    left -= k;
  }
};

void kb(Buf& b, uint32_t bytes) {
  if (bytes >= 10U * 1024U * 1024U) {
    b.add("%u.%u MB", static_cast<unsigned>(bytes >> 20), static_cast<unsigned>((bytes & 0xFFFFFU) * 10U >> 20));
    return;
  }
  b.add("%u.%u KB", static_cast<unsigned>(bytes / 1024), static_cast<unsigned>((bytes % 1024) * 10 / 1024));
}

void source(Buf& b, const SourceStat& s, uint32_t now) {
  if (!s.lastTryUtc) {
    b.add("bekliyor\n");
    return;
  }
  char ago[24];
  char text[96];
  Buf line{text, sizeof(text)};
  if (s.lastOkUtc) {
    rc::agoTr(now > s.lastOkUtc ? now - s.lastOkUtc : 0, ago, sizeof(ago));
    if (s.lastOk) line.add("OK %s · %u ms", ago, static_cast<unsigned>(s.durMs));
    else line.add(rc::L("ESKİ %s", "OLD %s"), ago);
  } else {
    line.add(rc::L("HATA", "ERROR"));
  }
  if (!s.lastOk) line.add(rc::L(" · kod %d", " · code %d"), static_cast<int>(s.lastCode));
  if (s.fails) line.add(rc::L(" · %u hata", " · %u errors"), static_cast<unsigned>(s.fails));
  if (s.deferred && s.lastOk) line.add(rc::L(" · %u ert.", " · %u def."), static_cast<unsigned>(s.deferred));
  char fitted[100];
  b.add("%s\n", du::fit(theme::fontS(), text, VAL_W, fitted, sizeof(fitted)));
}

void fill() {
  char text[1100];
  Buf b{text, sizeof(text)};
  const ui::Data& d = ui::data();
  LvglStats lv;
  store::peekT(store::LVGL, lv);
  uint32_t now = clk::nowUtc();
  uint32_t up = diag::uptimeS();
  char dur[24];

  b.add("%s · %s\n", RUMY_VERSION, RUMY_PANEL_NAME);
  rc::durationTr(up, dur, sizeof(dur));
  b.add("%s\n", dur);
  b.add("#%u · %s\n", static_cast<unsigned>(app::bootCount()), diag::resetReason());
  kb(b, mem::freeHeap());
  b.add("\n");
  kb(b, mem::minFreeHeap());
  b.add("\n");
  kb(b, mem::largestBlock());
  b.add("\n");
  int32_t trend = diag::heapTrendPerHour();
  b.add(rc::L("%+d B/sa\n", "%+d B/h\n"), static_cast<int>(trend));
  kb(b, lv.used);
  b.add(" / ");
  kb(b, lv.total);
  b.add(rc::L(" · parça %%%u\n", " · frag %u%%\n"), static_cast<unsigned>(lv.fragPct));
  kb(b, lv.maxUsed);
  b.add(rc::L(" · blok ", " · block "));
  kb(b, lv.biggestFree);
  b.add("\n");
  b.add("%u\n", static_cast<unsigned>(lv.fps));
  b.add("%u / %u / %u B\n", static_cast<unsigned>(lv.uiStackFree), static_cast<unsigned>(net::stackFree()),
        static_cast<unsigned>(wiz::stackFree()));
  kb(b, d.net.tlsPeak);
  b.add(rc::L(" · erteleme %u\n", " · deferred %u\n"), static_cast<unsigned>(d.net.tlsDeferred));
  if (d.net.wifi) {
    b.add("%d dBm · %u.%u.%u.%u\n", static_cast<int>(d.net.rssi), static_cast<unsigned>(d.net.ip & 0xFF),
          static_cast<unsigned>((d.net.ip >> 8) & 0xFF), static_cast<unsigned>((d.net.ip >> 16) & 0xFF),
          static_cast<unsigned>((d.net.ip >> 24) & 0xFF));
  } else {
    b.add(rc::L("bağlı değil\n", "not connected\n"));
  }
  if (sdcard::mounted()) {
    b.add(rc::L("%u/%u MB · y%u o%u y.hata %u\n", "%u/%u MB · w%u r%u w.err %u\n"), static_cast<unsigned>(sdcard::usedBytes() / 1048576ULL),
          static_cast<unsigned>(sdcard::totalBytes() / 1048576ULL), static_cast<unsigned>(sdcard::writes()),
          static_cast<unsigned>(sdcard::readErrors()), static_cast<unsigned>(sdcard::writeErrors()));
  } else {
    b.add(rc::L("takılı değil\n", "not inserted\n"));
  }
  b.add("%s\n", clk::synced() ? rc::L("NTP eşitli", "NTP synced") : rc::L("eşitlenmedi", "not synced"));
  source(b, d.net.src[SRC_FORECAST], now);
  source(b, d.net.src[SRC_MARINE], now);
  source(b, d.net.src[SRC_AIR], now);
  source(b, d.net.src[SRC_QUAKE], now);
  source(b, d.net.src[SRC_HOLIDAY], now);
  if (!d.settings.bulbMac[0]) b.add(rc::L("eşleşmedi", "not paired"));
  else b.add("%s · %s", d.settings.bulbMac, d.bulb.online ? rc::L("çevrimiçi", "online") : (d.bulb.found ? rc::L("yanıtsız", "no response") : rc::L("aranıyor", "searching")));
  lv_label_set_text(s_vals, text);
}

void sdTest(lv_event_t* e) {
  (void)e;
  uint32_t ok = 0;
  bool pass = sdcard::selfTest(20, ok);
  char buf[64];
  snprintf(buf, sizeof(buf), rc::L("SD testi: %u/20 %s", "SD test: %u/20 %s"), static_cast<unsigned>(ok), pass ? rc::L("başarılı", "passed") : rc::L("BAŞARISIZ", "FAILED"));
  ui::toast(buf);
}

void serialDump(lv_event_t* e) {
  (void)e;
  diag::printReport(Serial);
  ui::toast(rc::L("Rapor seri porta yazıldı (460800)", "Report written to serial (460800)"));
}

void rebootClick(lv_event_t* e) {
  (void)e;
  ui::toast(rc::L("Yeniden başlatılıyor…", "Restarting…"));
  app::request(app::ACT_REBOOT);
}

void backClick(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_SETTINGS, -1);
}

void deleted(lv_event_t* e) {
  (void)e;
  s_scr = nullptr;
  s_vals = nullptr;
}

lv_obj_t* smallButton(lv_obj_t* parent, const char* text, int32_t x, lv_event_cb_t cb) {
  lv_obj_t* b = theme::makeButton(parent, text, theme::fontS());
  lv_obj_set_size(b, (theme::FW(308) - 2 * theme::S(10)) / 3, theme::S(36));
  lv_obj_set_pos(b, x, 0);
  lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, nullptr);
  return b;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  ui::attachGestures(s_scr);
  lv_obj_add_flag(s_scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(s_scr, LV_DIR_VER);
  theme::styleScrollbar(s_scr);
  lv_obj_set_flex_flow(s_scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(s_scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_top(s_scr, theme::S(6), 0);
  lv_obj_set_style_pad_bottom(s_scr, theme::S(10), 0);
  lv_obj_set_style_pad_row(s_scr, theme::S(6), 0);

  lv_obj_t* head = theme::makeBox(s_scr);
  lv_obj_set_size(head, theme::FW(308), theme::S(40));
  lv_obj_t* back = theme::makeButton(head, RS_LEFT, theme::fontM());
  lv_obj_set_size(back, theme::S(44), theme::S(36));
  lv_obj_set_pos(back, 0, theme::S(2));
  lv_obj_add_event_cb(back, backClick, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* title = theme::makeLabel(head, theme::fontL(), theme::C_TEXT);
  lv_label_set_text_static(title, rc::L("Tanılama", "Diagnostics"));
  lv_obj_set_pos(title, theme::S(56), theme::S(6));

  lv_obj_t* card = theme::makeCard(s_scr, theme::FW(308), LV_SIZE_CONTENT);
  lv_obj_set_style_pad_all(card, theme::S(8), 0);
  lv_obj_t* keys = theme::makeLabel(card, theme::fontS(), theme::C_TEXT2);
  lv_label_set_text_static(keys, rc::L(kKeys, kKeysEn));
  lv_obj_set_pos(keys, 0, 0);
  lv_obj_set_style_text_line_space(keys, theme::S(3), 0);
  lv_obj_t* vbox = theme::makeBox(card);
  lv_obj_set_pos(vbox, theme::S(104), 0);
  lv_obj_set_size(vbox, VAL_W, LV_SIZE_CONTENT);
  s_vals = theme::makeLabel(vbox, theme::fontS(), theme::C_TEXT);
  lv_obj_set_style_text_line_space(s_vals, theme::S(3), 0);

  lv_obj_t* row = theme::makeBox(s_scr);
  lv_obj_set_size(row, theme::FW(308), theme::S(36));
  smallButton(row, rc::L(RS_SD "  SD testi", RS_SD "  SD test"), 0, sdTest);
  smallButton(row, rc::L(RS_CHART "  Seri rapor", RS_CHART "  Serial report"), (theme::FW(308) - 2 * theme::S(10)) / 3 + theme::S(10), serialDump);
  lv_obj_t* rb = smallButton(row, rc::L(RS_REFRESH "  Başlat", RS_REFRESH "  Restart"), 2 * ((theme::FW(308) - 2 * theme::S(10)) / 3 + theme::S(10)),
                                rebootClick);
  lv_obj_set_style_text_color(rb, theme::col(theme::C_WARN), 0);

  fill();
  s_lastMs = millis();
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask == ui::M_ALL || millis() - s_lastMs >= 2000) {
    s_lastMs = millis();
    fill();
  }
}

}
