#include "webui.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <WiFi.h>
#if !RUMY_PI
#include <ESPmDNS.h>
#endif
#include <esp_random.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "afad.h"
#include "app/app.h"
#include "app/config.h"
#include "app/mem.h"
#include "app/shared.h"
#include "app/store.h"
#include "board/sdcard.h"
#include "clock/clock.h"
#include "diag/diag.h"
#include "events/events.h"
#include "events_calc.h"
#include "json_arena.h"
#include "match/match.h"
#include "net/net.h"
#include "phone_app.h"
#include "rumy/look.h"
#include "settings/provinces.h"
#include "settings/settings.h"
#include "web_assets.h"
#include "web_faces.h"
#include "web_icon.h"

namespace webui {

namespace {

constexpr uint32_t READ_TIMEOUT_MS = 3000;
constexpr uint8_t MAX_BAD = 5;
constexpr uint32_t LOCK_MS = 60000;
constexpr size_t OUT_OFFSET = 4096;
const char* const kNs = "telefon";

WiFiServer s_server(cfg::WEB_PORT);
volatile bool s_want = true;
bool s_running = false;
volatile uint32_t s_lastActivity = 0;
uint8_t s_bad = 0;
uint32_t s_lockUntil = 0;
volatile bool s_pairing = false;
volatile uint16_t s_code = 0;
volatile bool s_forgetReq = false;
volatile uint8_t s_phones = 0;
char s_tokens[rc::PHONE_MAX][rc::PHONE_TOKEN_HEX + 1];
uint8_t s_nextSlot = 0;
bool s_loaded = false;
bool s_mdns = false;
uint32_t s_pairs = 0;
uint32_t s_requests = 0;
uint32_t s_denied = 0;

struct Request {
  char method[8];
  char path[64];
  char token[rc::PHONE_TOKEN_HEX + 1];
  int32_t contentLength;
  size_t bodyLen;
  char* body;
};

struct Out {
  char* p;
  size_t left;
  char* start;
  bool overflow;
  void add(const char* fmt, ...) {
    if (left < 2) {
      overflow = true;
      return;
    }
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(p, left, fmt, ap);
    va_end(ap);
    if (n < 0 || static_cast<size_t>(n) >= left) {
      overflow = true;
      p += left - 1;
      left = 1;
      return;
    }
    p += n;
    left -= static_cast<size_t>(n);
  }
  size_t len() const { return static_cast<size_t>(p - start); }
};

uint16_t newCode() { return static_cast<uint16_t>(1000 + esp_random() % 9000); }

void countPhones() {
  uint8_t n = 0;
  for (uint8_t i = 0; i < rc::PHONE_MAX; i++) {
    if (s_tokens[i][0]) n++;
  }
  s_phones = n;
}

void loadTokens() {
  s_loaded = true;
  memset(s_tokens, 0, sizeof(s_tokens));
  Preferences p;
  if (p.begin(kNs, true)) {
    for (uint8_t i = 0; i < rc::PHONE_MAX; i++) {
      char key[3] = {'t', static_cast<char>('0' + i), 0};
      if (p.isKey(key)) p.getString(key, s_tokens[i], sizeof(s_tokens[i]));
      if (!rc::phoneTokenWellFormed(s_tokens[i])) s_tokens[i][0] = 0;
    }
    s_nextSlot = static_cast<uint8_t>(p.getUChar("next", 0) % rc::PHONE_MAX);
    p.end();
  }
  countPhones();
}

void forgetAll() {
  memset(s_tokens, 0, sizeof(s_tokens));
  s_nextSlot = 0;
  Preferences p;
  if (p.begin(kNs, false)) {
    p.clear();
    p.end();
  }
  countPhones();
  diag::note("web", "esli telefonlar silindi");
}

void addToken(char* out) {
  uint8_t slot = rc::PHONE_MAX;
  for (uint8_t i = 0; i < rc::PHONE_MAX; i++) {
    if (!s_tokens[i][0]) {
      slot = i;
      break;
    }
  }
  if (slot == rc::PHONE_MAX) {
    slot = s_nextSlot;
    s_nextSlot = static_cast<uint8_t>((s_nextSlot + 1) % rc::PHONE_MAX);
  }
  for (size_t k = 0; k < rc::PHONE_TOKEN_HEX; k += 8) {
    snprintf(out + k, 9, "%08lx", static_cast<unsigned long>(esp_random()));
  }
  out[rc::PHONE_TOKEN_HEX] = 0;
  memcpy(s_tokens[slot], out, rc::PHONE_TOKEN_HEX + 1);
  Preferences p;
  if (p.begin(kNs, false)) {
    char key[3] = {'t', static_cast<char>('0' + slot), 0};
    p.putString(key, s_tokens[slot]);
    p.putUChar("next", s_nextSlot);
    p.end();
  }
  countPhones();
}

bool tokenKnown(const char* t) {
  for (uint8_t i = 0; i < rc::PHONE_MAX; i++) {
    if (rc::phoneTokenEqual(t, s_tokens[i])) return true;
  }
  return false;
}

void start() {
  s_server.begin();
  s_server.setNoDelay(true);
  s_running = true;
  s_bad = 0;
  s_lockUntil = 0;
#if !RUMY_PI
  s_mdns = MDNS.begin("rumy");
  if (s_mdns) MDNS.addService("http", "tcp", cfg::WEB_PORT);
#endif
  IPAddress ip = WiFi.localIP();
  diag::note("web", "acik http://%u.%u.%u.%u:%u mdns %d telefon %u", static_cast<unsigned>(ip[0]),
             static_cast<unsigned>(ip[1]), static_cast<unsigned>(ip[2]), static_cast<unsigned>(ip[3]),
             static_cast<unsigned>(cfg::WEB_PORT), s_mdns ? 1 : 0, static_cast<unsigned>(s_phones));
}

void stop() {
  s_server.end();
#if !RUMY_PI
  if (s_mdns) MDNS.end();
#endif
  s_mdns = false;
  s_running = false;
  diag::note("web", "kapandi");
}

bool readLine(WiFiClient& c, char* out, size_t cap, uint32_t deadline) {
  size_t k = 0;
  for (;;) {
    if (c.available() > 0) {
      int ch = c.read();
      if (ch < 0) continue;
      if (ch == '\n') break;
      if (ch == '\r') continue;
      if (k + 1 < cap) out[k++] = static_cast<char>(ch);
      continue;
    }
    if (!c.connected() || static_cast<int32_t>(millis() - deadline) > 0) return false;
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  out[k] = 0;
  return true;
}

bool readBody(WiFiClient& c, char* out, size_t n, uint32_t deadline) {
  size_t k = 0;
  while (k < n) {
    int avail = c.available();
    if (avail > 0) {
      int r = c.read(reinterpret_cast<uint8_t*>(out + k), n - k);
      if (r > 0) k += static_cast<size_t>(r);
      continue;
    }
    if (!c.connected() || static_cast<int32_t>(millis() - deadline) > 0) return false;
    vTaskDelay(pdMS_TO_TICKS(2));
  }
  return true;
}

bool headerIs(const char* line, const char* name, const char** value) {
  size_t n = strlen(name);
  if (strncasecmp(line, name, n) != 0) return false;
  const char* v = line + n;
  while (*v == ' ' || *v == '\t') v++;
  *value = v;
  return true;
}

void writeAll(WiFiClient& c, const uint8_t* data, size_t len) {
  size_t off = 0;
  uint32_t deadline = millis() + READ_TIMEOUT_MS * 2;
  while (off < len && c.connected()) {
    size_t chunk = len - off > 1024 ? 1024 : len - off;
    size_t w = c.write(data + off, chunk);
    if (w == 0) {
      if (static_cast<int32_t>(millis() - deadline) > 0) return;
      vTaskDelay(pdMS_TO_TICKS(2));
      continue;
    }
    off += w;
  }
}

void respond(WiFiClient& c, int code, const char* type, const char* body, size_t len, bool gzip) {
  const char* text = code == 200 ? "OK" : code == 400 ? "Bad Request" : code == 403 ? "Forbidden" : code == 404 ? "Not Found"
                     : code == 409 ? "Conflict" : code == 429 ? "Too Many Requests" : code == 413 ? "Payload Too Large"
                     : "Error";
  char head[288];
  int n = snprintf(head, sizeof(head),
                   "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %u\r\n%sCache-Control: %s\r\n"
                   "X-Content-Type-Options: nosniff\r\nConnection: close\r\n\r\n",
                   code, text, type, static_cast<unsigned>(len), gzip ? "Content-Encoding: gzip\r\n" : "",
                   strncmp(type, "image/", 6) == 0 ? "max-age=86400" : "no-store");
  if (n > 0) writeAll(c, reinterpret_cast<const uint8_t*>(head), static_cast<size_t>(n));
  if (len) writeAll(c, reinterpret_cast<const uint8_t*>(body), len);
}

void json(WiFiClient& c, int code, const char* body) {
  respond(c, code, "application/json; charset=utf-8", body, strlen(body), false);
}

void escaped(Out& o, const char* s) {
  char tmp[112];
  rc::jsonEscape(s, tmp, sizeof(tmp));
  o.add("\"%s\"", tmp);
}

void sendState(WiFiClient& c) {
  char* buf = reinterpret_cast<char*>(mem::jsonArena());
  Out o{buf, cfg::JSON_ARENA_BYTES, buf, false};
  Settings s = settings::get();
  IPAddress ip = WiFi.localIP();
  o.add("{\"v\":\"%s\",\"panel\":\"%s\",\"up\":%u,\"heap\":%u,\"minHeap\":%u,\"rssi\":%d,\"ip\":\"%u.%u.%u.%u\","
        "\"sd\":%s,\"boots\":%u,\"s\":{",
        RUMY_VERSION, RUMY_PANEL_NAME, static_cast<unsigned>(diag::uptimeS()), static_cast<unsigned>(mem::freeHeap()),
        static_cast<unsigned>(mem::minFreeHeap()), static_cast<int>(WiFi.RSSI()), static_cast<unsigned>(ip[0]),
        static_cast<unsigned>(ip[1]), static_cast<unsigned>(ip[2]), static_cast<unsigned>(ip[3]),
        sdcard::mounted() ? "true" : "false", static_cast<unsigned>(app::bootCount()));
  o.add("\"name\":");
  escaped(o, s.name);
  o.add(",\"city\":");
  escaped(o, s.city);
  o.add(",\"lat\":%.4f,\"lon\":%.4f,\"prov\":%u,\"bri\":%u,\"nmode\":%u,\"nbri\":%u,\"nstart\":%u,\"nend\":%u,"
        "\"home\":%u,\"card\":%u,\"esleep\":%u,\"maint\":%u,\"qmag\":%u,\"bmac\":\"%s\",\"bname\":",
        static_cast<double>(s.lat), static_cast<double>(s.lon), static_cast<unsigned>(s.province),
        static_cast<unsigned>(s.brightness), static_cast<unsigned>(s.nightMode), static_cast<unsigned>(s.nightBrightness),
        static_cast<unsigned>(s.nightStart), static_cast<unsigned>(s.nightEnd), static_cast<unsigned>(s.homeTimeoutS),
        static_cast<unsigned>(s.cardSeconds), static_cast<unsigned>(s.eyeSleepMin), static_cast<unsigned>(s.maintReboot),
        static_cast<unsigned>(s.quakeMinMag10), s.bulbMac);
  escaped(o, s.bulbName);
  o.add(",\"phost\":");
  escaped(o, s.printerHost);
  o.add(",\"team\":%lu,\"wagent\":%u,\"wgoal\":%u,\"wonly\":%u,\"gsl\":%u,\"gucl\":%u,\"fut\":%u,"
        "\"tc1\":\"%06lx\",\"tc2\":\"%06lx\",\"lang\":%u}",
        static_cast<unsigned long>(s.teamId), static_cast<unsigned>(s.wizAgent), static_cast<unsigned>(s.wizGoal),
        static_cast<unsigned>(s.wizOnlyOn), static_cast<unsigned>(s.goalSl), static_cast<unsigned>(s.goalUcl),
        static_cast<unsigned>(s.football), static_cast<unsigned long>(s.teamC1), static_cast<unsigned long>(s.teamC2),
        static_cast<unsigned>(s.lang));
  BulbState b;
  memset(&b, 0, sizeof(b));
  store::peekT(store::BULB, b);
  o.add(",\"bulb\":{\"online\":%u,\"on\":%u}", static_cast<unsigned>(b.online),
        static_cast<unsigned>(b.pilot.valid && b.pilot.on));
  uint32_t now = clk::nowUtc();
  uint32_t until = settings::sleepUntil();
  bool sleeping = clk::synced() && rc::sleepActive(until, now);
  uint32_t wake = sleeping ? (clk::minuteOfDay() + (until - now + 59) / 60) % 1440 : s.nightEnd;
  look::Pick lk = look::shown();
  o.add(",\"look\":{\"m\":%u,\"a\":%u,\"f\":%u,\"mine\":%u}", static_cast<unsigned>(lk.mood),
        static_cast<unsigned>(lk.acc), static_cast<unsigned>(lk.fx), static_cast<unsigned>(lk.mine));
  o.add(",\"sleep\":%s,\"wake\":%u,\"phones\":%u,\"ev\":", sleeping ? "true" : "false",
        static_cast<unsigned>(wake), static_cast<unsigned>(s_phones));
  size_t n = events::personalJson(o.p, o.left);
  if (!n) o.add("[]");
  else {
    o.p += n;
    o.left -= n;
  }
  o.add("}");
  if (o.overflow) {
    json(c, 500, "{\"error\":\"buyuk\"}");
    return;
  }
  respond(c, 200, "application/json; charset=utf-8", buf, o.len(), false);
}

void sendProvinces(WiFiClient& c) {
  char* buf = reinterpret_cast<char*>(mem::jsonArena());
  Out o{buf, cfg::JSON_ARENA_BYTES, buf, false};
  o.add("[");
  for (int i = 0; i < provinces::COUNT; i++) {
    const provinces::Province& p = provinces::at(i);
    o.add("%s[%u,\"%s\",%.3f,%.3f]", i ? "," : "", static_cast<unsigned>(p.plate), p.name, static_cast<double>(p.lat),
          static_cast<double>(p.lon));
  }
  o.add("]");
  if (o.overflow) {
    json(c, 500, "{\"error\":\"buyuk\"}");
    return;
  }
  respond(c, 200, "application/json; charset=utf-8", buf, o.len(), false);
}

void copyField(JsonVariantConst v, char* dst, size_t cap) {
  if (!v.is<const char*>()) return;
  const char* s = v.as<const char*>();
  rc::utf8Copy(dst, cap, s, strlen(s));
}

template <typename T>
void numField(JsonVariantConst v, T& dst) {
  if (v.is<float>() || v.is<int>()) dst = static_cast<T>(v.as<float>());
}

bool hexField(JsonVariantConst v, uint32_t& dst) { return rc::hexColorParse(v.as<const char*>(), dst); }

bool applySettings(const char* body, size_t len) {
  rc::JsonArena a(mem::jsonArena() + OUT_OFFSET, cfg::JSON_ARENA_BYTES - OUT_OFFSET);
  JsonDocument doc(&a);
  if (deserializeJson(doc, body, len)) return false;
  Settings old = settings::get();
  Settings s = old;
  copyField(doc["name"], s.name, sizeof(s.name));
  copyField(doc["city"], s.city, sizeof(s.city));
  copyField(doc["bname"], s.bulbName, sizeof(s.bulbName));
  copyField(doc["phost"], s.printerHost, sizeof(s.printerHost));
  numField(doc["lat"], s.lat);
  numField(doc["lon"], s.lon);
  numField(doc["prov"], s.province);
  numField(doc["bri"], s.brightness);
  numField(doc["nmode"], s.nightMode);
  numField(doc["nbri"], s.nightBrightness);
  numField(doc["nstart"], s.nightStart);
  numField(doc["nend"], s.nightEnd);
  numField(doc["home"], s.homeTimeoutS);
  numField(doc["card"], s.cardSeconds);
  numField(doc["esleep"], s.eyeSleepMin);
  numField(doc["maint"], s.maintReboot);
  numField(doc["qmag"], s.quakeMinMag10);
  numField(doc["team"], s.teamId);
  numField(doc["wagent"], s.wizAgent);
  numField(doc["wgoal"], s.wizGoal);
  numField(doc["wonly"], s.wizOnlyOn);
  numField(doc["gsl"], s.goalSl);
  numField(doc["gucl"], s.goalUcl);
  numField(doc["fut"], s.football);
  numField(doc["lang"], s.lang);
  bool c1 = hexField(doc["tc1"], s.teamC1);
  bool c2 = hexField(doc["tc2"], s.teamC2);
  if (s.teamId != old.teamId && !(c1 && c2)) s.teamC1 = s.teamC2 = 0;
  doc.clear();
  settings::sane(s);
  settings::set(s);
  if (s.teamId != old.teamId || s.football != old.football) match::refresh();
  if (s.football != old.football) net::refresh(SRC_BOARD);
  if (s.lang != old.lang) app::request(app::ACT_REBOOT, 1500);
  bool moved = s.lat != old.lat || s.lon != old.lon || strcmp(s.city, old.city) != 0;
  if (moved) net::locationChanged();
  else if (s.quakeMinMag10 != old.quakeMinMag10) net::refresh(SRC_QUAKE);
  return true;
}

bool locked(WiFiClient& c) {
  if (s_lockUntil && static_cast<int32_t>(s_lockUntil - millis()) > 0) {
    json(c, 429, "{\"error\":\"kilit\"}");
    return true;
  }
  s_lockUntil = 0;
  return false;
}

void strike() {
  s_denied++;
  if (++s_bad >= MAX_BAD) {
    s_bad = 0;
    s_lockUntil = (millis() + LOCK_MS) | 1u;
    diag::note("web", "hatali deneme, 60 sn kilit");
  }
}

bool authOk(WiFiClient& c, const Request& r) {
  if (locked(c)) return false;
  if (tokenKnown(r.token)) {
    s_bad = 0;
    return true;
  }
  strike();
  json(c, 403, "{\"error\":\"eslesme\"}");
  return false;
}

void pairPhone(WiFiClient& c, const Request& r) {
  if (locked(c)) return;
  if (!s_pairing || !s_code) {
    json(c, 409, "{\"error\":\"kapali\"}");
    return;
  }
  rc::JsonArena a(mem::jsonArena() + OUT_OFFSET, cfg::JSON_ARENA_BYTES - OUT_OFFSET);
  JsonDocument doc(&a);
  uint16_t v = 0;
  bool ok = !deserializeJson(doc, r.body, r.bodyLen) && rc::pairCodeParse(doc["code"] | "", v) && v == s_code;
  doc.clear();
  if (!ok) {
    strike();
    json(c, 403, "{\"error\":\"kod\"}");
    return;
  }
  s_bad = 0;
  char tok[rc::PHONE_TOKEN_HEX + 1];
  addToken(tok);
  s_code = newCode();
  s_pairs++;
  char out[64];
  snprintf(out, sizeof(out), "{\"token\":\"%s\"}", tok);
  json(c, 200, out);
  diag::note("web", "telefon eslesti, esli %u", static_cast<unsigned>(s_phones));
}

bool applyLook(const char* body, size_t len) {
  rc::JsonArena a(mem::jsonArena() + OUT_OFFSET, cfg::JSON_ARENA_BYTES - OUT_OFFSET);
  JsonDocument doc(&a);
  if (deserializeJson(doc, body, len)) return false;
  bool automatic = doc["auto"] | false;
  int m = doc["mood"] | -1;
  int acc = doc["acc"] | 0;
  int fx = doc["fx"] | 0;
  doc.clear();
  if (automatic) {
    look::clear();
    return true;
  }
  if (m < 0 || m > 255 || acc < 0 || acc > 255 || fx < 0 || fx > 255) return false;
  return look::set(static_cast<uint8_t>(m), static_cast<uint8_t>(acc), static_cast<uint8_t>(fx));
}

bool applySleep(const char* body, size_t len) {
  rc::JsonArena a(mem::jsonArena() + OUT_OFFSET, cfg::JSON_ARENA_BYTES - OUT_OFFSET);
  JsonDocument doc(&a);
  if (deserializeJson(doc, body, len)) return false;
  bool on = doc["on"] | false;
  doc.clear();
  if (!on) {
    settings::setSleepUntil(0);
    diag::note("web", "uyku kapatildi");
    return true;
  }
  if (!clk::synced()) return false;
  rc::LocalTime lt = clk::local();
  uint32_t until = rc::sleepUntilUtc(clk::nowUtc(), clk::minuteOfDay(), lt.second, settings::get().nightEnd);
  settings::setSleepUntil(until);
  diag::note("web", "uyku %lu sn", static_cast<unsigned long>(until - clk::nowUtc()));
  return true;
}

void route(WiFiClient& c, Request& r) {
  bool get = strcmp(r.method, "GET") == 0;
  bool post = strcmp(r.method, "POST") == 0;
  if (get && (strcmp(r.path, "/") == 0 || strcmp(r.path, "/index.html") == 0)) {
    respond(c, 200, "text/html; charset=utf-8", reinterpret_cast<const char*>(WEB_INDEX_GZ), WEB_INDEX_GZ_LEN, true);
    return;
  }
  if (get && strcmp(r.path, "/yuzler.png") == 0) {
    respond(c, 200, "image/png", reinterpret_cast<const char*>(WEB_FACES_PNG), WEB_FACES_PNG_LEN, false);
    return;
  }
  if (get && strcmp(r.path, "/icon.png") == 0) {
    respond(c, 200, "image/png", reinterpret_cast<const char*>(WEB_ICON_PNG), WEB_ICON_PNG_LEN, false);
    return;
  }
  if (get && strcmp(r.path, "/api/provinces") == 0) {
    sendProvinces(c);
    return;
  }
  if (post && strcmp(r.path, "/api/pair") == 0) {
    pairPhone(c, r);
    return;
  }
  if (strncmp(r.path, "/api/", 5) != 0) {
    json(c, 404, "{\"error\":\"yok\"}");
    return;
  }
  if (!authOk(c, r)) return;
  if (get && strcmp(r.path, "/api/state") == 0) {
    sendState(c);
  } else if (post && strcmp(r.path, "/api/settings") == 0) {
    bool ok = applySettings(r.body, r.bodyLen);
    json(c, ok ? 200 : 400, ok ? "{\"ok\":true}" : "{\"error\":\"gecersiz\"}");
  } else if (post && strcmp(r.path, "/api/events") == 0) {
    bool ok = events::setPersonal(r.body, r.bodyLen);
    json(c, ok ? 200 : 400, ok ? "{\"ok\":true}" : "{\"error\":\"kaydedilemedi\"}");
  } else if (post && strcmp(r.path, "/api/refresh") == 0) {
    net::refreshAll();
    json(c, 200, "{\"ok\":true}");
  } else if (post && strcmp(r.path, "/api/reboot") == 0) {
    json(c, 200, "{\"ok\":true}");
    app::request(app::ACT_REBOOT, 1500);
  } else if (post && strcmp(r.path, "/api/look") == 0) {
    bool ok = applyLook(r.body, r.bodyLen);
    json(c, ok ? 200 : 400, ok ? "{\"ok\":true}" : "{\"error\":\"gecersiz\"}");
  } else if (post && strcmp(r.path, "/api/sleep") == 0) {
    bool ok = applySleep(r.body, r.bodyLen);
    json(c, ok ? 200 : 400, ok ? "{\"ok\":true}" : "{\"error\":\"saat\"}");
  } else if (post && strcmp(r.path, "/api/ping") == 0) {
    json(c, 200, "{\"ok\":true}");
  } else {
    json(c, 404, "{\"error\":\"yok\"}");
  }
}

void handle(WiFiClient& c) {
  uint32_t deadline = millis() + READ_TIMEOUT_MS;
  char line[192];
  if (!readLine(c, line, sizeof(line), deadline)) return;
  Request r;
  memset(&r, 0, sizeof(r));
  r.contentLength = 0;
  char* sp1 = strchr(line, ' ');
  if (!sp1) return;
  *sp1 = 0;
  char* target = sp1 + 1;
  char* sp2 = strchr(target, ' ');
  if (sp2) *sp2 = 0;
  char* q = strchr(target, '?');
  if (q) *q = 0;
  snprintf(r.method, sizeof(r.method), "%.7s", line);
  snprintf(r.path, sizeof(r.path), "%.63s", target);
  for (int guard = 0; guard < 40; guard++) {
    if (!readLine(c, line, sizeof(line), deadline)) return;
    if (!line[0]) break;
    const char* v;
    if (headerIs(line, "content-length:", &v)) r.contentLength = static_cast<int32_t>(atol(v));
    else if (headerIs(line, "x-token:", &v)) snprintf(r.token, sizeof(r.token), "%.32s", v);
  }
  if (r.contentLength < 0 || static_cast<size_t>(r.contentLength) > cfg::WEB_BODY_MAX) {
    json(c, 413, "{\"error\":\"buyuk\"}");
    return;
  }
  r.body = reinterpret_cast<char*>(mem::jsonArena());
  r.bodyLen = static_cast<size_t>(r.contentLength);
  if (r.bodyLen && !readBody(c, r.body, r.bodyLen, deadline + READ_TIMEOUT_MS)) return;
  r.body[r.bodyLen] = 0;
  s_lastActivity = millis() | 1u;
  s_requests++;
  route(c, r);
}

}

void setEnabled(bool on) { s_want = on; }

bool enabled() { return s_want; }

bool running() { return s_running; }

bool busy() {
  uint32_t last = s_lastActivity;
  return s_running && last && millis() - last < cfg::WEB_ACTIVE_MS;
}

void service(uint32_t nowMs) {
  (void)nowMs;
  if (!s_loaded) loadTokens();
  if (s_forgetReq) {
    s_forgetReq = false;
    forgetAll();
  }
  bool wifi = WiFi.status() == WL_CONNECTED;
  if (s_want && !s_running && wifi) start();
  if (s_running && (!s_want || !wifi)) stop();
  if (!s_running) return;
  WiFiClient c = s_server.available();
  if (!c) return;
  handle(c);
  c.flush();
  c.stop();
}

uint16_t pin() { return s_pairing ? s_code : 0; }

uint32_t lastActivityMs() { return s_lastActivity; }

void setPairing(bool on) {
  if (on && !s_pairing) s_code = newCode();
  if (!on) s_code = 0;
  s_pairing = on;
}

uint16_t pairCode() { return s_code; }

uint8_t phones() { return s_phones; }

void forgetPhones() {
  s_forgetReq = true;
  s_phones = 0;
}

bool mdnsUp() { return s_mdns; }

void report(Print& out) {
  out.printf("[web] %s · mdns %d · telefon %u · istek %lu · eslesme %lu · ret %lu%s\n",
             s_running ? "acik" : (s_want ? "bekliyor" : "kapali"), s_mdns ? 1 : 0, static_cast<unsigned>(s_phones),
             static_cast<unsigned long>(s_requests), static_cast<unsigned long>(s_pairs),
             static_cast<unsigned long>(s_denied), s_pairing ? " · eslestirme acik" : "");
}

}
