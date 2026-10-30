#include "settings.h"

#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <string.h>

#include "afad.h"
#include "app/config.h"
#include "app/store.h"
#include "k2.h"
#include "pairing.h"
#include "provinces.h"
#include "text_tr.h"

namespace settings {

namespace {

const char* const kNs = "rumy";
Settings s_cur;
uint32_t s_boots = 0;
volatile uint32_t s_sleepUntil = 0;
SemaphoreHandle_t s_mutex = nullptr;
StaticSemaphore_t s_mutexBuf;

void lock() {
  if (!s_mutex) s_mutex = xSemaphoreCreateMutexStatic(&s_mutexBuf);
  xSemaphoreTake(s_mutex, portMAX_DELAY);
}

void unlock() { xSemaphoreGive(s_mutex); }

void copyStr(char* dst, size_t cap, const char* src) { rc::utf8Copy(dst, cap, src ? src : "", src ? strlen(src) : 0); }

Settings defaults() {
  Settings s;
  memset(&s, 0, sizeof(s));
  copyStr(s.city, sizeof(s.city), cfg::DEFAULT_CITY);
  s.lat = cfg::DEFAULT_LAT;
  s.lon = cfg::DEFAULT_LON;
  s.province = cfg::DEFAULT_PROVINCE;
  s.brightness = 80;
  s.nightMode = NIGHT_HOURS;
  s.nightBrightness = 4;
  s.nightStart = cfg::NIGHT_START_DEFAULT_MIN;
  s.nightEnd = 7 * 60;
  s.homeTimeoutS = cfg::HOME_TIMEOUT_DEFAULT_S;
  s.cardSeconds = cfg::CARD_SECONDS_DEFAULT;
  s.maintReboot = 0;
  s.touchRot = cfg::TP_DEFAULT_ROTATION;
  s.flip = 0;
  s.wifiConfigured = 0;
  s.quakeMinMag10 = 20;
  s.eyeSleepMin = cfg::EYE_SLEEP_DEFAULT_MIN;
  s.teamId = 0;
  s.wizAgent = 1;
  s.wizGoal = 1;
  s.wizOnlyOn = 1;
  s.goalSl = 1;
  s.goalUcl = 1;
  s.football = 1;
  s.lang = rc::LANG_EN;
  return s;
}

void readStr(Preferences& p, const char* key, char* dst, size_t cap) {
  if (p.isKey(key)) p.getString(key, dst, cap);
}

float readFloat(Preferences& p, const char* key, float def) { return p.isKey(key) ? p.getFloat(key, def) : def; }

void save(const Settings& s) {
  Preferences p;
  if (!p.begin(kNs, false)) return;
  p.putString("name", s.name);
  p.putString("city", s.city);
  p.putFloat("lat", s.lat);
  p.putFloat("lon", s.lon);
  p.putUChar("prov", s.province);
  p.putUChar("bri", s.brightness);
  p.putUChar("nmode", s.nightMode);
  p.putUChar("nbri", s.nightBrightness);
  p.putUShort("nstart", s.nightStart);
  p.putUShort("nend", s.nightEnd);
  p.putUShort("home", s.homeTimeoutS);
  p.putUChar("card", s.cardSeconds);
  p.putUChar("maint", s.maintReboot);
  p.putUChar("trot", s.touchRot);
  p.putUChar("flip", s.flip);
  p.putUChar("wifi", s.wifiConfigured);
  p.putUChar("qmag", s.quakeMinMag10);
  p.putUChar("esleep", s.eyeSleepMin);
  p.putUInt("team", s.teamId);
  p.putString("bmac", s.bulbMac);
  p.putString("bname", s.bulbName);
  p.putString("phost", s.printerHost);
  p.putUChar("wagent", s.wizAgent);
  p.putUChar("wgoal", s.wizGoal);
  p.putUChar("wonly", s.wizOnlyOn);
  p.putUChar("gsl", s.goalSl);
  p.putUChar("gucl", s.goalUcl);
  p.putUChar("fut", s.football);
  p.putUChar("dil", s.lang);
  p.putUInt("tc1", s.teamC1);
  p.putUInt("tc2", s.teamC2);
  p.end();
}

}

bool sane(Settings& s) {
  bool ok = true;
  s.name[sizeof(s.name) - 1] = 0;
  s.city[sizeof(s.city) - 1] = 0;
  s.bulbMac[sizeof(s.bulbMac) - 1] = 0;
  s.bulbName[sizeof(s.bulbName) - 1] = 0;
  s.printerHost[sizeof(s.printerHost) - 1] = 0;
  if (s.printerHost[0] && !rc::k2HostValid(s.printerHost)) {
    s.printerHost[0] = 0;
    ok = false;
  }
  if (!(s.lat >= -90.0f && s.lat <= 90.0f) || !(s.lon >= -180.0f && s.lon <= 180.0f)) {
    s.lat = cfg::DEFAULT_LAT;
    s.lon = cfg::DEFAULT_LON;
    ok = false;
  }
  if (s.brightness < 5) s.brightness = 5;
  if (s.brightness > 100) s.brightness = 100;
  if (s.nightBrightness < 1) s.nightBrightness = 1;
  if (s.nightBrightness > 40) s.nightBrightness = 40;
  if (s.nightMode > NIGHT_HOURS) s.nightMode = NIGHT_OFF;
  if (s.nightStart >= 1440) s.nightStart = cfg::NIGHT_START_DEFAULT_MIN;
  if (s.nightEnd >= 1440) s.nightEnd = 7 * 60;
  if (s.homeTimeoutS < 15) s.homeTimeoutS = 15;
  if (s.homeTimeoutS > 900) s.homeTimeoutS = 900;
  if (s.cardSeconds < 3) s.cardSeconds = 3;
  if (s.cardSeconds > 30) s.cardSeconds = 30;
  if (s.touchRot > 7) s.touchRot = 0;
  if (s.flip > 1) s.flip = 0;
  if (s.quakeMinMag10 < 10 || s.quakeMinMag10 > 50) s.quakeMinMag10 = 20;
  if (s.eyeSleepMin > 60) s.eyeSleepMin = cfg::EYE_SLEEP_DEFAULT_MIN;
  if (s.wizAgent > 1) s.wizAgent = 1;
  if (s.wizGoal > 1) s.wizGoal = 1;
  if (s.wizOnlyOn > 1) s.wizOnlyOn = 1;
  if (s.goalSl > 1) s.goalSl = 1;
  if (s.goalUcl > 1) s.goalUcl = 1;
  if (s.football > 1) s.football = 1;
  if (s.lang > rc::LANG_EN) s.lang = rc::LANG_EN;
  s.teamC1 &= 0xFFFFFF;
  s.teamC2 &= 0xFFFFFF;
  if (s.bulbMac[0]) {
    char m[13];
    if (rc::macNormalize(s.bulbMac, m)) memcpy(s.bulbMac, m, 13);
    else s.bulbMac[0] = 0;
  }
  if (!s.city[0]) copyStr(s.city, sizeof(s.city), cfg::DEFAULT_CITY);
  return ok;
}

void load() {
  Settings s = defaults();
  Preferences p;
  if (p.begin(kNs, false)) {
    readStr(p, "name", s.name, sizeof(s.name));
    readStr(p, "city", s.city, sizeof(s.city));
    s.lat = readFloat(p, "lat", s.lat);
    s.lon = readFloat(p, "lon", s.lon);
    s.province = p.getUChar("prov", s.province);
    s.brightness = p.getUChar("bri", s.brightness);
    s.nightMode = p.getUChar("nmode", s.nightMode);
    s.nightBrightness = p.getUChar("nbri", s.nightBrightness);
    s.nightStart = p.getUShort("nstart", s.nightStart);
    s.nightEnd = p.getUShort("nend", s.nightEnd);
    s.homeTimeoutS = p.getUShort("home", s.homeTimeoutS);
    s.cardSeconds = p.getUChar("card", s.cardSeconds);
    s.maintReboot = p.getUChar("maint", s.maintReboot);
    s.touchRot = p.getUChar("trot", s.touchRot);
    s.flip = p.getUChar("flip", s.flip);
    s.wifiConfigured = p.getUChar("wifi", 0);
    s.quakeMinMag10 = p.getUChar("qmag", s.quakeMinMag10);
    s.eyeSleepMin = p.getUChar("esleep", s.eyeSleepMin);
    s.teamId = p.getUInt("team", s.teamId);
    s.wizAgent = p.getUChar("wagent", s.wizAgent);
    s.wizGoal = p.getUChar("wgoal", s.wizGoal);
    s.wizOnlyOn = p.getUChar("wonly", s.wizOnlyOn);
    s.goalSl = p.getUChar("gsl", s.goalSl);
    s.goalUcl = p.getUChar("gucl", s.goalUcl);
    s.football = p.getUChar("fut", s.football);
    s.lang = p.getUChar("dil", s.lang);
    s.teamC1 = p.getUInt("tc1", s.teamC1);
    s.teamC2 = p.getUInt("tc2", s.teamC2);
    s_sleepUntil = p.getUInt("uyku", 0);
    readStr(p, "bmac", s.bulbMac, sizeof(s.bulbMac));
    readStr(p, "bname", s.bulbName, sizeof(s.bulbName));
    readStr(p, "phost", s.printerHost, sizeof(s.printerHost));
    s_boots = p.getUInt("boots", 0);
    p.end();
  }
  sane(s);
  rc::setLang(s.lang);
  lock();
  s_cur = s;
  unlock();
}

Settings get() {
  lock();
  Settings s = s_cur;
  unlock();
  return s;
}

void set(const Settings& in) {
  Settings s = in;
  sane(s);
  lock();
  bool changed = memcmp(&s, &s_cur, sizeof(s)) != 0;
  if (changed) s_cur = s;
  unlock();
  if (changed) {
    save(s);
    store::bump(store::SETTINGS);
  }
}

void setBulb(const char* mac, const char* name) {
  Settings s = get();
  copyStr(s.bulbMac, sizeof(s.bulbMac), mac);
  if (name) copyStr(s.bulbName, sizeof(s.bulbName), name);
  set(s);
}

void setWifiConfigured(bool v) {
  Settings s = get();
  s.wifiConfigured = v ? 1 : 0;
  set(s);
}

void factoryReset() {
  Preferences p;
  if (p.begin(kNs, false)) {
    p.clear();
    p.end();
  }
  lock();
  s_cur = defaults();
  unlock();
  store::bump(store::SETTINGS);
}

uint32_t version() { return store::version(store::SETTINGS); }

uint32_t bootCount() { return s_boots; }

void bumpBootCount() {
  Preferences p;
  if (!p.begin(kNs, false)) return;
  s_boots = p.getUInt("boots", 0) + 1;
  p.putUInt("boots", s_boots);
  p.end();
}

uint32_t sleepUntil() { return s_sleepUntil; }

void setSleepUntil(uint32_t utc) {
  if (utc == s_sleepUntil) return;
  s_sleepUntil = utc;
  Preferences p;
  if (p.begin(kNs, false)) {
    p.putUInt("uyku", utc);
    p.end();
  }
  store::bump(store::SETTINGS);
}

}
