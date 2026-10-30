#include <Arduino.h>
#include <esp_system.h>
#include <stdio.h>

#include "app/app.h"
#include "app/config.h"
#include "diag/diag.h"
#include "fonts/symbols.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"
#include "text_tr.h"

namespace scr_info {

namespace {

const char* const kLicenses =
    "VERİ KAYNAKLARI\n"
    "Hava, deniz, hava kalitesi: Open-Meteo.com (CC BY 4.0)\n"
    "Deprem verisi: T.C. İçişleri Bakanlığı AFAD\n"
    "Yedek deprem verisi: Boğaziçi Ünv. Kandilli Rasathanesi\n"
    "Resmî tatiller: Nager.Date (MIT)\n"
    "Dinî bayram tablosu: Diyanet İşleri Başkanlığı\n"
    "\n"
    "YAZILIM\n"
    "LVGL 9.5 (MIT)\n"
    "LovyanGFX (FreeBSD)\n"
    "ArduinoJson 7 (MIT)\n"
    "WiFiManager (MIT)\n"
    "Arduino-ESP32 (LGPL-2.1)\n"
    "ESP-IDF, mbedTLS, lwIP (Apache-2.0 / BSD)\n"
    "\n"
    "GÖRSELLER VE FONTLAR\n"
    "Meteocons, Bas Milius (MIT)\n"
    "Inter, Rasmus Andersson (SIL OFL 1.1)\n"
    "Font Awesome Free (SIL OFL 1.1, CC BY 4.0)\n"
    "\n"
    "Rumy'nin gözleri ve arayüzü bu proje için çizildi.";

const char* const kLicensesEn =
    "DATA SOURCES\n"
    "Weather, sea, air quality: Open-Meteo.com (CC BY 4.0)\n"
    "Earthquake data: AFAD, Turkish Ministry of Interior\n"
    "Backup quake data: Boğaziçi Univ. Kandilli Observatory\n"
    "Public holidays: Nager.Date (MIT)\n"
    "Religious holiday table: Diyanet (Presidency of Religious Affairs)\n"
    "\n"
    "SOFTWARE\n"
    "LVGL 9.5 (MIT)\n"
    "LovyanGFX (FreeBSD)\n"
    "ArduinoJson 7 (MIT)\n"
    "WiFiManager (MIT)\n"
    "Arduino-ESP32 (LGPL-2.1)\n"
    "ESP-IDF, mbedTLS, lwIP (Apache-2.0 / BSD)\n"
    "\n"
    "IMAGES AND FONTS\n"
    "Meteocons, Bas Milius (MIT)\n"
    "Inter, Rasmus Andersson (SIL OFL 1.1)\n"
    "Font Awesome Free (SIL OFL 1.1, CC BY 4.0)\n"
    "\n"
    "Rumy's eyes and interface were drawn for this project.";

lv_obj_t* s_scr = nullptr;

void backClick(lv_event_t* e) {
  (void)e;
  ui::back();
}

void deleted(lv_event_t* e) {
  (void)e;
  s_scr = nullptr;
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
  lv_obj_set_style_pad_bottom(s_scr, theme::S(12), 0);
  lv_obj_set_style_pad_row(s_scr, theme::S(6), 0);

  lv_obj_t* head = theme::makeBox(s_scr);
  lv_obj_set_size(head, theme::FW(308), theme::S(40));
  lv_obj_t* bk = theme::makeButton(head, RS_LEFT, theme::fontM());
  lv_obj_set_size(bk, theme::S(44), theme::S(36));
  lv_obj_set_pos(bk, 0, theme::S(2));
  lv_obj_add_event_cb(bk, backClick, LV_EVENT_CLICKED, nullptr);
  lv_obj_t* title = theme::makeLabel(head, theme::fontL(), theme::C_TEXT);
  lv_label_set_text_static(title, rc::L("Hakkında", "About"));
  lv_obj_set_pos(title, theme::S(56), theme::S(6));

  lv_obj_t* card = theme::makeCard(s_scr, theme::FW(308), LV_SIZE_CONTENT);
  lv_obj_set_style_pad_all(card, theme::S(10), 0);
  lv_obj_t* dev = theme::makeLabel(card, theme::fontS(), theme::C_TEXT);
  lv_obj_set_width(dev, theme::FW(288));
  lv_label_set_long_mode(dev, LV_LABEL_LONG_MODE_WRAP);
  uint64_t mac = ESP.getEfuseMac();
  char buf[320];
#if RUMY_PI
  snprintf(buf, sizeof(buf),
           rc::L("Rumy %s · panel %s\nRaspberry Pi Zero W · ARMv6 · %u MHz\nUygulama %u KB\n"
                 "MAC %02X:%02X:%02X:%02X:%02X:%02X\nAçılış #%u · %s",
                 "Rumy %s · panel %s\nRaspberry Pi Zero W · ARMv6 · %u MHz\nApp %u KB\n"
                 "MAC %02X:%02X:%02X:%02X:%02X:%02X\nBoot #%u · %s"),
           RUMY_VERSION, RUMY_PANEL_NAME, static_cast<unsigned>(ESP.getCpuFreqMHz()),
           static_cast<unsigned>(ESP.getSketchSize() / 1024U), static_cast<unsigned>(mac & 0xFF),
           static_cast<unsigned>((mac >> 8) & 0xFF), static_cast<unsigned>((mac >> 16) & 0xFF),
           static_cast<unsigned>((mac >> 24) & 0xFF), static_cast<unsigned>((mac >> 32) & 0xFF),
           static_cast<unsigned>((mac >> 40) & 0xFF), static_cast<unsigned>(app::bootCount()), diag::resetReason());
#else
  snprintf(buf, sizeof(buf),
           rc::L("Rumy %s · panel %s\nESP32-2432S028R (CYD)\nESP32 rev %u · %u MHz · flash %u MB\n"
                 "Uygulama %u KB · boş alan %u KB\nMAC %02X:%02X:%02X:%02X:%02X:%02X\nAçılış #%u · %s",
                 "Rumy %s · panel %s\nESP32-2432S028R (CYD)\nESP32 rev %u · %u MHz · flash %u MB\n"
                 "App %u KB · free %u KB\nMAC %02X:%02X:%02X:%02X:%02X:%02X\nBoot #%u · %s"),
           RUMY_VERSION, RUMY_PANEL_NAME, static_cast<unsigned>(ESP.getChipRevision()),
           static_cast<unsigned>(ESP.getCpuFreqMHz()), static_cast<unsigned>(ESP.getFlashChipSize() / 1048576U),
           static_cast<unsigned>(ESP.getSketchSize() / 1024U), static_cast<unsigned>(ESP.getFreeSketchSpace() / 1024U),
           static_cast<unsigned>(mac & 0xFF), static_cast<unsigned>((mac >> 8) & 0xFF),
           static_cast<unsigned>((mac >> 16) & 0xFF), static_cast<unsigned>((mac >> 24) & 0xFF),
           static_cast<unsigned>((mac >> 32) & 0xFF), static_cast<unsigned>((mac >> 40) & 0xFF),
           static_cast<unsigned>(app::bootCount()), diag::resetReason());
#endif
  lv_label_set_text(dev, buf);

  lv_obj_t* lic = theme::makeCard(s_scr, theme::FW(308), LV_SIZE_CONTENT);
  lv_obj_set_style_pad_all(lic, theme::S(10), 0);
  lv_obj_t* l = theme::makeLabel(lic, theme::fontS(), theme::C_TEXT2);
  lv_obj_set_width(l, theme::FW(288));
  lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_WRAP);
  lv_label_set_text_static(l, rc::L(kLicenses, kLicensesEn));
  return s_scr;
}

void update(uint32_t mask) { (void)mask; }

}
