#include "k2.h"

#include <ArduinoJson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "afad.h"
#include "text_tr.h"

namespace rc {

namespace {

const char* const kKeys[] = {"ModeCode", "state",          "printProgress",  "printLeftTime", "printJobTime",
                             "printFileName", "layer",     "TotalLayer",     "nozzleTemp",    "targetNozzleTemp",
                             "bedTemp0", "targetBedTemp0", "boxTemp",        "withSelfTest",  "materialStatus",
                             "hostname", "model",          "printStartTime", "curFeedratePct"};

bool number(JsonVariantConst v, float& out) {
  if (v.is<const char*>()) {
    const char* s = v.as<const char*>();
    char* end = nullptr;
    float f = strtof(s, &end);
    if (end == s) return false;
    out = f;
    return true;
  }
  if (v.is<float>() || v.is<int>() || v.is<long>()) {
    out = v.as<float>();
    return true;
  }
  return false;
}

bool integer(JsonVariantConst v, int32_t& out) {
  if (v.is<int32_t>()) {
    out = v.as<int32_t>();
    return true;
  }
  if (v.is<const char*>()) {
    const char* s = v.as<const char*>();
    char* end = nullptr;
    long l = strtol(s, &end, 10);
    if (end == s) return false;
    out = static_cast<int32_t>(l);
    return true;
  }
  float f;
  if (!number(v, f)) return false;
  out = static_cast<int32_t>(f < 0 ? f - 0.5f : f + 0.5f);
  return true;
}

bool unsignedInt(JsonVariantConst v, uint32_t& out) {
  if (v.is<uint32_t>()) {
    out = v.as<uint32_t>();
    return true;
  }
  int32_t i;
  if (!integer(v, i)) return false;
  out = static_cast<uint32_t>(i < 0 ? 0 : i);
  return true;
}

int16_t tenths(float f) {
  float t = f * 10.0f + (f < 0 ? -0.5f : 0.5f);
  if (t > 32000.0f) t = 32000.0f;
  if (t < -32000.0f) t = -32000.0f;
  return static_cast<int16_t>(t);
}

template <typename T>
T clampTo(int32_t v, int32_t lo, int32_t hi) {
  if (v < lo) v = lo;
  if (v > hi) v = hi;
  return static_cast<T>(v);
}

void text(JsonVariantConst v, char* dst, size_t cap) {
  if (!v.is<const char*>()) return;
  const char* s = v.as<const char*>();
  utf8Copy(dst, cap, s, strlen(s));
}

const char* baseName(const char* path) {
  const char* slash = strrchr(path, '/');
  return slash ? slash + 1 : path;
}

bool endsWith(const char* s, size_t n, const char* tail) {
  size_t k = strlen(tail);
  if (n < k) return false;
  for (size_t i = 0; i < k; i++) {
    char a = s[n - k + i];
    if (a >= 'A' && a <= 'Z') a = static_cast<char>(a - 'A' + 'a');
    if (a != tail[i]) return false;
  }
  return true;
}

bool durationToken(const char* s, size_t n) {
  if (!n) return false;
  bool digit = false;
  for (size_t i = 0; i < n; i++) {
    char c = s[i];
    if (c >= '0' && c <= '9') digit = true;
    else if (c != 'd' && c != 'h' && c != 'm' && c != 's') return false;
  }
  return digit;
}

}

void k2Clear(K2Status& st) {
  memset(&st, 0, sizeof(st));
  st.state = -1;
  st.nozzle10 = K2_NO_TEMP;
  st.nozzleTarget10 = K2_NO_TEMP;
  st.bed10 = K2_NO_TEMP;
  st.bedTarget10 = K2_NO_TEMP;
  st.box10 = K2_NO_TEMP;
}

bool k2Parse(ByteSource& src, JsonArena& arena, K2Status& st, uint32_t& present) {
  present = 0;
  arena.reset();
  JsonDocument filter(&arena);
  for (const char* k : kKeys) filter[k] = true;
  filter["err"]["errcode"] = true;
  filter["err"]["key"] = true;
  filter["err"]["value"] = true;
  if (filter.overflowed()) {
    filter.clear();
    arena.reset();
    return false;
  }
  JsonDocument doc(&arena);
  DeserializationError e = deserializeJson(doc, src, DeserializationOption::Filter(filter));
  if (e || !doc.is<JsonObjectConst>()) {
    doc.clear();
    filter.clear();
    arena.reset();
    return false;
  }
  JsonObjectConst o = doc.as<JsonObjectConst>();
  int32_t i;
  float f;
  const char* mode = o["ModeCode"] | "";
  if (!strcmp(mode, "heart_beat")) present |= K2F_HEARTBEAT;
  if (integer(o["state"], i)) {
    st.state = clampTo<int16_t>(i, -1, 32000);
    present |= K2F_STATE;
  }
  if (integer(o["printProgress"], i)) {
    st.progress = clampTo<uint8_t>(i, 0, 100);
    present |= K2F_PROGRESS;
  }
  if (unsignedInt(o["printLeftTime"], st.leftS)) present |= K2F_LEFT;
  if (unsignedInt(o["printJobTime"], st.jobS)) present |= K2F_JOB;
  if (unsignedInt(o["printStartTime"], st.startUtc)) present |= K2F_START;
  if (o["printFileName"].is<const char*>()) {
    const char* path = o["printFileName"].as<const char*>();
    const char* b = baseName(path);
    utf8Copy(st.file, sizeof(st.file), b, strlen(b));
    present |= K2F_FILE;
  }
  if (integer(o["layer"], i)) {
    st.layer = clampTo<uint16_t>(i, 0, 65535);
    present |= K2F_LAYER;
  }
  if (integer(o["TotalLayer"], i)) {
    st.totalLayer = clampTo<uint16_t>(i, 0, 65535);
    present |= K2F_TOTAL_LAYER;
  }
  if (number(o["nozzleTemp"], f)) {
    st.nozzle10 = tenths(f);
    present |= K2F_NOZZLE;
  }
  if (number(o["targetNozzleTemp"], f)) {
    st.nozzleTarget10 = tenths(f);
    present |= K2F_NOZZLE_TARGET;
  }
  if (number(o["bedTemp0"], f)) {
    st.bed10 = tenths(f);
    present |= K2F_BED;
  }
  if (number(o["targetBedTemp0"], f)) {
    st.bedTarget10 = tenths(f);
    present |= K2F_BED_TARGET;
  }
  if (number(o["boxTemp"], f)) {
    st.box10 = tenths(f);
    present |= K2F_BOX;
  }
  if (integer(o["withSelfTest"], i)) {
    st.selfTest = clampTo<uint8_t>(i, 0, 255);
    present |= K2F_SELFTEST;
  }
  if (integer(o["materialStatus"], i)) {
    st.runout = i == 1 ? 1 : 0;
    present |= K2F_MATERIAL;
  }
  if (integer(o["curFeedratePct"], i)) {
    st.speedPct = clampTo<uint8_t>(i, 0, 255);
    present |= K2F_SPEED;
  }
  if (o["hostname"].is<const char*>()) {
    text(o["hostname"], st.host, sizeof(st.host));
    present |= K2F_HOST;
  }
  if (o["model"].is<const char*>()) {
    text(o["model"], st.model, sizeof(st.model));
    present |= K2F_MODEL;
  }
  JsonVariantConst err = o["err"];
  if (err.is<JsonObjectConst>()) {
    int32_t code = 0;
    int32_t key = 0;
    integer(err["errcode"], code);
    integer(err["key"], key);
    st.errCode = code;
    st.errKey = key;
    st.errText[0] = 0;
    text(err["value"], st.errText, sizeof(st.errText));
    present |= K2F_ERR;
  } else if (integer(err, i)) {
    st.errCode = i;
    st.errKey = 0;
    st.errText[0] = 0;
    present |= K2F_ERR;
  }
  st.seen |= present & ~K2F_HEARTBEAT;
  doc.clear();
  filter.clear();
  arena.reset();
  return true;
}

K2Phase k2Phase(const K2Status& st) {
  if (!(st.seen & K2F_STATE)) return K2_IDLE;
  if (st.errCode != 0) return K2_ERROR;
  if (st.selfTest > 0 && st.selfTest < 100) return K2_SELFTEST;
  switch (st.state) {
    case 1: return K2_PRINTING;
    case 2: return K2_DONE;
    case 4: return K2_STOPPED;
    case 5: return K2_PAUSED;
    default: return K2_IDLE;
  }
}

bool k2Runout(const K2Status& st) { return st.runout && (st.state == 1 || st.state == 5); }

bool k2Problem(const K2Status& st) { return k2Phase(st) == K2_ERROR || k2Runout(st); }

void k2JobName(const char* file, char* name, size_t cap, char* material, size_t mcap) {
  if (cap) name[0] = 0;
  if (mcap) material[0] = 0;
  if (!file || !cap) return;
  const char* b = baseName(file);
  size_t n = strlen(b);
  if (endsWith(b, n, ".gcode")) n -= 6;
  else if (endsWith(b, n, ".gco")) n -= 4;
  static const char* const kModels[] = {".stl_", ".3mf_", ".obj_", ".step_", ".stp_"};
  size_t cut = n;
  const char* rest = nullptr;
  size_t restN = 0;
  for (const char* m : kModels) {
    size_t k = strlen(m);
    for (size_t i = 0; i + k <= n; i++) {
      bool hit = true;
      for (size_t j = 0; j < k && hit; j++) {
        char c = b[i + j];
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
        hit = c == m[j];
      }
      if (hit && i < cut) {
        cut = i;
        rest = b + i + k;
        restN = n - i - k;
      }
    }
  }
  if (rest && mcap) {
    size_t mlen = 0;
    while (mlen < restN && rest[mlen] != '_') mlen++;
    bool tail = mlen == restN || durationToken(rest + mlen + 1, restN - mlen - 1);
    if (mlen && tail) utf8Copy(material, mcap, rest, mlen);
  } else if (!rest) {
    size_t us = n;
    while (us > 0 && b[us - 1] != '_') us--;
    if (us > 0 && durationToken(b + us, n - us)) {
      cut = us - 1;
      size_t ms = cut;
      while (ms > 0 && b[ms - 1] != '_') ms--;
      if (ms > 0 && mcap) {
        utf8Copy(material, mcap, b + ms, cut - ms);
        cut = ms - 1;
      }
    }
  }
  utf8Copy(name, cap, b, cut);
  for (char* p = name; *p; p++) {
    if (*p == '_') *p = ' ';
  }
}

size_t k2BuildStatusRequest(char* out, size_t cap) {
  int n = snprintf(out, cap, "{\"method\":\"get\",\"params\":{\"ReqPrinterPara\":1}}");
  if (n < 0 || static_cast<size_t>(n) >= cap) return 0;
  return static_cast<size_t>(n);
}

bool k2HostValid(const char* host) {
  if (!host) return false;
  const char* p = host;
  for (int part = 0; part < 4; part++) {
    if (part > 0 && *p++ != '.') return false;
    const char* start = p;
    int value = 0;
    while (*p >= '0' && *p <= '9' && p - start < 3) value = value * 10 + (*p++ - '0');
    if (p == start || value > 255 || (p - start > 1 && *start == '0') || (*p >= '0' && *p <= '9')) return false;
  }
  return *p == 0;
}

const char* k2PhaseTr(K2Phase p) {
  switch (p) {
    case K2_OFFLINE: return rc::L("Bağlantı yok", "Offline");
    case K2_PRINTING: return rc::L("Yazdırılıyor", "Printing");
    case K2_PAUSED: return rc::L("Duraklatıldı", "Paused");
    case K2_DONE: return rc::L("Tamamlandı", "Complete");
    case K2_STOPPED: return rc::L("Durduruldu", "Stopped");
    case K2_ERROR: return rc::L("Hata", "Error");
    case K2_SELFTEST: return rc::L("Kalibrasyon", "Calibrating");
    default: return rc::L("Hazır", "Ready");
  }
}

}
