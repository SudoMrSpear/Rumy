#include "portal.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <string.h>

#include "board/display.h"
#include "board/lgfx_cyd.h"
#include "config.h"
#include "diag/diag.h"
#include "portal_font.h"
#include "settings/provinces.h"
#include "settings/settings.h"
#include "text_tr.h"

namespace portal {

namespace {

constexpr uint16_t C_BG = 0x0000;
constexpr uint32_t PORTAL_RETRY_PAUSE_MS = 1500;

volatile uint8_t s_reason = 0;

void onDisconnect(arduino_event_id_t, arduino_event_info_t info) { s_reason = info.wifi_sta_disconnected.reason; }

const char* reasonText(uint8_t r) {
  switch (r) {
    case WIFI_REASON_NO_AP_FOUND: return rc::L("Ağ bulunamadı (2.4 GHz olmalı)", "Network not found (must be 2.4 GHz)");
    case WIFI_REASON_AUTH_EXPIRE:
    case WIFI_REASON_AUTH_FAIL:
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_MIC_FAILURE: return rc::L("Şifre yanlış olabilir", "Password may be wrong");
    case 0: return rc::L("Kurulum tamamlanmadı", "Setup not completed");
    default: return rc::L("Bağlanamadı, tekrar dene", "Couldn't connect, try again");
  }
}

uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) { return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)); }

void eyes(LgfxCyd& g, int32_t x, int32_t y) {
  uint16_t glow = rgb(6, 60, 72);
  uint16_t eye = rgb(25, 211, 240);
  g.fillRoundRect(x - 3, y - 3, 32, 38, 11, glow);
  g.fillRoundRect(x + 42 - 3, y - 3, 32, 38, 11, glow);
  g.fillRoundRect(x, y, 26, 32, 8, eye);
  g.fillRoundRect(x + 42, y, 26, 32, 8, eye);
}

void draw(const char* status, uint16_t statusColor) {
  LgfxCyd& g = display::gfx();
  g.startWrite();
  g.fillScreen(C_BG);
  eyes(g, 14, 14);
  g.loadFont(PORTAL_FONT_VLW);
  g.setTextColor(rgb(242, 245, 249), C_BG);
  g.setCursor(102, 16);
  g.print("Rumy");
  g.setTextColor(rgb(154, 165, 180), C_BG);
  g.setCursor(102, 38);
  g.print(rc::L("İlk kurulum", "First setup"));
  g.setTextColor(rgb(242, 245, 249), C_BG);
  g.setCursor(10, 66);
  g.print(rc::L("1. Telefonla \"Rumy-Kurulum\"", "1. On your phone, join"));
  g.setCursor(26, 86);
  g.print(rc::L("ağına bağlan (QR)", "\"Rumy-Kurulum\" (or scan QR)"));
  g.setCursor(10, 110);
  g.print(rc::L("2. Açılan sayfada ev", "2. On the page that opens,"));
  g.setCursor(26, 130);
  g.print(rc::L("WiFi'ni seç", "pick your home WiFi"));
  g.setCursor(10, 154);
  g.print(rc::L("3. Adını ve şehrini yaz,", "3. Enter your name and city,"));
  g.setCursor(26, 174);
  g.print(rc::L("Kaydet'e bas", "then tap Save"));
  g.setTextColor(rgb(94, 105, 120), C_BG);
  g.setCursor(10, 198);
  g.print(rc::L("Sayfa açılmazsa: 192.168.4.1", "If no page opens: 192.168.4.1"));
  g.setTextColor(statusColor, C_BG);
  g.setCursor(10, 220);
  g.print(status);
  g.unloadFont();
  g.fillRoundRect(214, 64, 100, 100, 6, rgb(255, 255, 255));
  g.qrcode("WIFI:T:nopass;S:Rumy-Kurulum;;", 218, 68, 92);
  g.endWrite();
}

char* buildDatalist() {
  size_t cap = 32;
  for (int i = 0; i < provinces::COUNT; i++) cap += strlen(provinces::at(i).name) + 20;
  char* html = static_cast<char*>(malloc(cap));
  if (!html) return nullptr;
  size_t k = 0;
  k += static_cast<size_t>(snprintf(html + k, cap - k, "<datalist id='iller'>"));
  for (int i = 0; i < provinces::COUNT && k < cap; i++) {
    k += static_cast<size_t>(snprintf(html + k, cap - k, "<option value='%s'>", provinces::at(i).name));
  }
  if (k < cap) snprintf(html + k, cap - k, "</datalist>");
  return html;
}

}

void run() {
  display::begin(cfg::TP_DEFAULT_ROTATION);
  display::setFlip(settings::get().flip);
  display::setBrightnessPct(70);
  draw(rc::L("Kurulum bekleniyor…", "Waiting for setup…"), rgb(25, 211, 240));
  diag::note("portal", "WiFi kurulumu: %s agina baglanin", cfg::PORTAL_AP);

  char* datalist = buildDatalist();
  Settings cur = settings::get();
  WiFi.onEvent(onDisconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  for (;;) {
    s_reason = 0;
    WiFiManager wm;
    wm.setDebugOutput(false);
    wm.setTitle("Rumy");
    wm.setDarkMode(true);
    wm.setHostname("rumy");
    wm.setMinimumSignalQuality(8);
    wm.setConfigPortalTimeout(0);
    wm.setConnectTimeout(25);
    wm.setBreakAfterConfig(true);
    wm.setShowInfoErase(false);
    wm.setShowInfoUpdate(false);
    wm.setCountry("TR");
    std::vector<const char*> menu = {"wifi", "exit"};
    wm.setMenu(menu);
    WiFiManagerParameter pName("rname", rc::L("Adın", "Your name"), cur.name, 23, "autocomplete='given-name'");
    WiFiManagerParameter pCity("rcity", rc::L("Şehir (il)", "City (province)"), cur.city, 31, "list='iller' autocomplete='off'");
    WiFiManagerParameter pList(datalist ? datalist : "");
    wm.addParameter(&pName);
    wm.addParameter(&pCity);
    wm.addParameter(&pList);

    bool ok = wm.startConfigPortal(cfg::PORTAL_AP);
    Settings s = settings::get();
    const char* name = pName.getValue();
    const char* city = pCity.getValue();
    if (name) snprintf(s.name, sizeof(s.name), "%s", name);
    int idx = city ? provinces::findByName(city) : -1;
    if (idx >= 0) {
      const provinces::Province& p = provinces::at(idx);
      snprintf(s.city, sizeof(s.city), "%s", p.name);
      s.lat = p.lat;
      s.lon = p.lon;
      s.province = p.plate;
    }
    cur = s;
    if (ok && WiFi.status() == WL_CONNECTED) {
      s.wifiConfigured = 1;
      settings::set(s);
      draw(rc::L("Bağlandı! Yeniden başlatılıyor…", "Connected! Restarting…"), rgb(61, 220, 132));
      diag::note("portal", "kurulum tamam");
      free(datalist);
      vTaskDelay(pdMS_TO_TICKS(1200));
      ESP.restart();
    }
    settings::set(s);
    uint8_t reason = s_reason;
    draw(reasonText(reason), rgb(255, 176, 32));
    diag::note("portal", "baglanti basarisiz (neden %u), portal yeniden aciliyor", static_cast<unsigned>(reason));
    vTaskDelay(pdMS_TO_TICKS(PORTAL_RETRY_PAUSE_MS));
  }
}

}
