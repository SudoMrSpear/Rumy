#include <Arduino.h>
#include <Preferences.h>
#include <SD.h>
#include <SPI.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <mbedtls/sha256.h>
#include <stdio.h>
#include <string.h>

#include "app/config.h"
#include "app/portal_font.h"
#include "board/lgfx_cyd.h"
#include "fw_image.h"

namespace {

constexpr int32_t W = 320;
constexpr int32_t BAR_X = 40;
constexpr int32_t BAR_Y = 134;
constexpr int32_t BAR_W = 240;
constexpr int32_t BAR_H = 10;
constexpr int COPY_TRIES = 3;
constexpr uint32_t SHOW_MS = 1500;
constexpr uint32_t FAIL_MS = 6000;

LgfxCyd s_lcd;
SPIClass s_spi(VSPI);
uint8_t s_buf[4096];
int s_shown = -1;
bool s_en = true;

const char* tx(const char* tr, const char* en) { return s_en ? en : tr; }

uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) { return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)); }

const uint16_t C_BG = 0;
const uint16_t C_TEXT = rgb(242, 245, 249);
const uint16_t C_QUIET = rgb(154, 165, 180);
const uint16_t C_DIM = rgb(94, 105, 120);
const uint16_t C_EYE = rgb(25, 211, 240);
const uint16_t C_GLOW = rgb(6, 60, 72);
const uint16_t C_BAR = rgb(27, 34, 43);
const uint16_t C_RED = rgb(255, 64, 64);
const uint16_t C_GREEN = rgb(43, 255, 106);
const uint16_t C_RED_GLOW = rgb(74, 14, 14);
const uint16_t C_GREEN_GLOW = rgb(11, 74, 31);

void eyes(uint16_t color, uint16_t glow) {
  const int32_t x = 127;
  const int32_t y = 40;
  s_lcd.fillRoundRect(x - 3, y - 3, 32, 38, 11, glow);
  s_lcd.fillRoundRect(x + 42 - 3, y - 3, 32, 38, 11, glow);
  s_lcd.fillRoundRect(x, y, 26, 32, 8, color);
  s_lcd.fillRoundRect(x + 42, y, 26, 32, 8, color);
}

void line(int32_t y, const char* text, uint16_t color) {
  s_lcd.fillRect(0, y, W, 24, C_BG);
  s_lcd.setTextColor(color, C_BG);
  s_lcd.drawString(text, W / 2, y);
}

void frame(const char* title, uint16_t titleColor, const char* note) {
  s_lcd.fillScreen(C_BG);
  if (titleColor == C_RED) eyes(C_RED, C_RED_GLOW);
  else if (titleColor == C_GREEN) eyes(C_GREEN, C_GREEN_GLOW);
  else eyes(C_EYE, C_GLOW);
  line(98, title, titleColor);
  line(206, note, C_DIM);
  s_shown = -1;
}

void progress(const char* step, uint32_t done, uint32_t total) {
  int pct = total ? static_cast<int>(static_cast<uint64_t>(done) * 100 / total) : 0;
  if (pct == s_shown) return;
  s_shown = pct;
  s_lcd.fillRoundRect(BAR_X, BAR_Y, BAR_W, BAR_H, BAR_H / 2, C_BAR);
  int32_t w = BAR_W * pct / 100;
  if (w > 0) s_lcd.fillRoundRect(BAR_X, BAR_Y, w < BAR_H ? BAR_H : w, BAR_H, BAR_H / 2, C_EYE);
  char text[48];
  snprintf(text, sizeof(text), "%s · %%%d", step, pct);
  line(154, text, C_QUIET);
}

const esp_partition_t* appPart() {
  return esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, rc::FW_LABEL_APP);
}

bool mountSd() {
  s_spi.begin(cfg::PIN_SD_SCK, cfg::PIN_SD_MISO, cfg::PIN_SD_MOSI, cfg::PIN_SD_CS);
  if (SD.begin(cfg::PIN_SD_CS, s_spi, cfg::SD_HZ, "/sd", 1, false)) return true;
  SD.end();
  return SD.begin(cfg::PIN_SD_CS, s_spi, cfg::SD_HZ_SLOW, "/sd", 1, false);
}

bool readOffer(rc::FwOffer& out) {
  File f = SD.open(rc::FW_SHA_FILE, FILE_READ);
  if (!f) return false;
  char text[96];
  size_t n = f.read(reinterpret_cast<uint8_t*>(text), sizeof(text) - 1);
  f.close();
  while (n && (text[n - 1] == '\n' || text[n - 1] == '\r')) n--;
  return rc::parseFwShaFile(text, n, out);
}

bool verify(const rc::FwOffer& o) {
  File f = SD.open(rc::FW_FILE, FILE_READ);
  if (!f) return false;
  if (f.size() != o.size) {
    f.close();
    return false;
  }
  mbedtls_sha256_context ctx;
  mbedtls_sha256_context body;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_init(&body);
  mbedtls_sha256_starts_ret(&ctx, 0);
  mbedtls_sha256_starts_ret(&body, 0);
  uint8_t tail[rc::FW_SHA_BYTES] = {};
  uint8_t head[rc::FW_HEAD_BYTES] = {};
  uint32_t done = 0;
  while (done < o.size) {
    int n = f.read(s_buf, sizeof(s_buf));
    if (n <= 0) break;
    size_t k = static_cast<size_t>(n);
    if (done < sizeof(head)) memcpy(head + done, s_buf, k < sizeof(head) - done ? k : sizeof(head) - done);
    mbedtls_sha256_update_ret(&ctx, s_buf, k);
    size_t b = rc::fwBodyBytes(done, k, o.size);
    if (b) mbedtls_sha256_update_ret(&body, s_buf, b);
    if (b < k) memcpy(tail + (done + b - (o.size - rc::FW_SHA_BYTES)), s_buf + b, k - b);
    done += static_cast<uint32_t>(k);
    progress(tx("Doğrulanıyor", "Verifying"), done, o.size);
  }
  f.close();
  uint8_t sum[rc::FW_SHA_BYTES];
  uint8_t own[rc::FW_SHA_BYTES];
  mbedtls_sha256_finish_ret(&ctx, sum);
  mbedtls_sha256_finish_ret(&body, own);
  mbedtls_sha256_free(&ctx);
  mbedtls_sha256_free(&body);
  rc::FwInfo info;
  return done == o.size && !memcmp(sum, o.sha, sizeof(sum)) && !memcmp(own, tail, sizeof(own)) &&
         rc::fwImageInfo(head, sizeof(head), info);
}

bool copy(const esp_partition_t* dst, const rc::FwOffer& o) {
  File f = SD.open(rc::FW_FILE, FILE_READ);
  if (!f) return false;
  esp_ota_handle_t h = 0;
  if (esp_ota_begin(dst, o.size, &h) != ESP_OK) {
    f.close();
    return false;
  }
  uint32_t done = 0;
  bool ok = true;
  while (done < o.size) {
    int n = f.read(s_buf, sizeof(s_buf));
    if (n <= 0 || esp_ota_write(h, s_buf, static_cast<size_t>(n)) != ESP_OK) {
      ok = false;
      break;
    }
    done += static_cast<uint32_t>(n);
    progress(tx("Yazılıyor", "Writing"), done, o.size);
  }
  f.close();
  if (esp_ota_end(h) != ESP_OK) ok = false;
  return ok && esp_ota_set_boot_partition(dst) == ESP_OK;
}

void restartAfter(uint32_t ms) {
  vTaskDelay(pdMS_TO_TICKS(ms));
  esp_restart();
}

void giveUp(const esp_partition_t* app, const char* why, bool appIntact) {
  bool back = appIntact && app && esp_ota_set_boot_partition(app) == ESP_OK;
  frame(tx("Güncelleme yapılamadı", "Update failed"), C_RED, back ? tx("Eski yazılımla açılıyor", "Starting the old firmware") : tx("USB ile yükleyin", "Flash over USB"));
  line(154, why, C_TEXT);
  if (back) restartAfter(FAIL_MS);
  for (;;) vTaskDelay(pdMS_TO_TICKS(1000));
}

}

void setup() {
  Preferences prefs;
  prefs.begin("rumy", true);
  uint8_t flip = prefs.getUChar("flip", 0);
  s_en = prefs.getUChar("dil", 1) == 1;
  prefs.end();
  s_lcd.init();
  s_lcd.setRotation(flip ? 3 : 1);
  s_lcd.startWrite();
  s_lcd.writeCommand(0x37);
  s_lcd.writeData16(0);
  s_lcd.endWrite();
  s_lcd.setBrightness(160);
  s_lcd.loadFont(PORTAL_FONT_VLW);
  s_lcd.setTextDatum(lgfx::top_center);
  frame(tx("Rumy güncelleniyor", "Updating Rumy"), C_TEXT, tx("Kapatmayın", "Don't power off"));
  const esp_partition_t* app = appPart();
  if (!app) giveUp(app, tx("Ana bölüm bulunamadı", "App partition not found"), false);
  rc::FwOffer offer;
  if (!mountSd()) giveUp(app, tx("SD kart okunamadı", "Couldn't read SD card"), true);
  if (!readOffer(offer) || !SD.exists(rc::FW_FILE)) giveUp(app, tx("Güncelleme dosyası yok", "No update file"), true);
  if (offer.size < rc::FW_HEAD_BYTES || offer.size > app->size) giveUp(app, tx("Dosya boyutu geçersiz", "Invalid file size"), true);
  if (!verify(offer)) {
    SD.remove(rc::FW_FILE);
    SD.remove(rc::FW_SHA_FILE);
    giveUp(app, tx("Dosya bozuk", "File is corrupted"), true);
  }
  for (int i = 0; i < COPY_TRIES; i++) {
    if (copy(app, offer)) {
      SD.remove(rc::FW_FILE);
      SD.remove(rc::FW_SHA_FILE);
      frame(tx("Güncelleme tamam", "Update complete"), C_GREEN, tx("Rumy yeniden başlıyor", "Rumy is restarting"));
      progress(tx("Kuruldu", "Installed"), 1, 1);
      restartAfter(SHOW_MS);
    }
  }
  giveUp(app, tx("Ana bölüme yazılamadı", "Couldn't write the app partition"), false);
}

void loop() { vTaskDelay(pdMS_TO_TICKS(1000)); }
