#include "net/net.h"

#include <Arduino.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdlib.h>
#include <string.h>

#include "app/config.h"
#include "app/mem.h"
#include "app/store.h"
#include "clock/clock.h"
#include "curl_min.h"
#include "diag/diag.h"
#include "events/events.h"
#include "net/fetch.h"
#include "net/http_min.h"
#include "match/board.h"
#include "match/leagues.h"
#include "match/match.h"
#include "printer/printer.h"
#include "quake/quake.h"
#include "weather/weather.h"
#include "webui/webui.h"
#include "text_tr.h"

namespace http {

const char* errText(int code) {
  switch (code) {
    case E_CONNECT: return rc::L("bağlanamadı", "connect failed");
    case E_TIMEOUT: return rc::L("zaman aşımı", "timeout");
    case E_PROTO: return "protokol";
    case E_WRITE: return rc::L("gönderilemedi", "send failed");
    case E_TLS_MEM: return rc::L("TLS için bellek yok", "no memory for TLS");
    case E_PARSE: return "JSON";
    case E_NO_WIFI: return rc::L("WiFi yok", "no WiFi");
    case E_BUILD: return "istek";
    case E_ENCODING: return rc::L("sıkıştırma", "compression");
    default: return code >= 200 ? "HTTP" : "?";
  }
}

}

namespace net {

namespace {

using JobFn = int16_t (*)(FetchStat&);

struct Job {
  uint8_t needsTime;
  uint16_t firstS;
  uint32_t periodS;
  JobFn fn;
  const char* name;
  uint32_t (*periodFn)();
};

uint32_t periodOf(const Job& job) { return job.periodFn ? job.periodFn() : job.periodS; }

const Job kJobs[SRC_COUNT] = {
    {0, 3, cfg::PERIOD_FORECAST_S, weather::fetchWeather, "hava+7gun", nullptr},
    {0, 30, cfg::PERIOD_HOURLY_S, weather::fetchMarine, "deniz", nullptr},
    {0, 45, cfg::PERIOD_HOURLY_S, weather::fetchAir, "havakalitesi", nullptr},
    {1, 60, cfg::PERIOD_QUAKE_S, quake::fetch, "afad", nullptr},
    {1, 90, cfg::PERIOD_HOLIDAY_S, events::fetchHolidays, "nager", nullptr},
    {1, 120, cfg::MATCH_SCHED_S, match::fetch, "mac", match::periodS},
    {1, 140, cfg::BOARD_IDLE_S, board::fetch, "lig", board::periodS},
    {1, 5, cfg::LEAGUE_IDLE_S, leagues::fetch, "ligsayfa", leagues::periodS},
};

constexpr uint32_t TIME_WAIT_RETRY_MS = 5000;
constexpr size_t BODY_MAX = 512U * 1024U;
constexpr long CONNECT_MS = cfg::HTTP_CONNECT_MS;
constexpr long TOTAL_MS = cfg::HTTP_CONNECT_MS + cfg::HTTP_TIMEOUT_MS;

TaskHandle_t s_task = nullptr;
rc::JsonArena s_arena;
NetStatus s_status;
uint32_t s_nextMs[SRC_COUNT] = {};
uint32_t s_backoffS[SRC_COUNT] = {};
bool s_scheduled = false;
bool s_wasUp = false;
uint32_t s_lastStatusMs = 0;
portMUX_TYPE s_mux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t s_refreshMask = 0;
volatile bool s_locationChanged = false;
uint32_t s_unsynced = 0;
CURL* s_curl = nullptr;
char* s_body = nullptr;
size_t s_bodyLen = 0;
char s_agent[48];

bool due(uint32_t now, uint32_t at) { return static_cast<int32_t>(now - at) >= 0; }

size_t onBody(char* p, size_t size, size_t n, void*) {
  size_t k = size * n;
  if (!s_body || s_bodyLen + k > BODY_MAX) return 0;
  memcpy(s_body + s_bodyLen, p, k);
  s_bodyLen += k;
  return k;
}

int16_t mapCurl(CURLcode c) {
  switch (c) {
    case curlc::E_OPERATION_TIMEDOUT: return http::E_TIMEOUT;
    case curlc::E_BAD_CONTENT_ENCODING: return http::E_ENCODING;
    case curlc::E_FILESIZE_EXCEEDED:
    case curlc::E_WRITE_ERROR: return http::E_PROTO;
    default: return http::E_CONNECT;
  }
}

void scheduleAll(uint32_t now) {
  for (int i = 0; i < SRC_COUNT; i++) {
    if (!s_scheduled || due(now, s_nextMs[i])) s_nextMs[i] = now + kJobs[i].firstS * 1000UL;
  }
  s_scheduled = true;
}

void publishStatus(uint32_t now) {
  s_lastStatusMs = now;
  bool up = WiFi.status() == WL_CONNECTED;
  s_status.wifi = up ? 1 : 0;
  s_status.timeSynced = clk::synced() ? 1 : 0;
  s_status.rssi = up ? WiFi.RSSI() : 0;
  s_status.ip = up ? static_cast<uint32_t>(WiFi.localIP()) : 0;
  s_status.webActive = webui::running() ? 1 : 0;
  s_status.webPin = webui::pin();
  for (int i = 0; i < SRC_COUNT; i++) {
    int32_t left = static_cast<int32_t>(s_nextMs[i] - now);
    s_status.src[i].nextInS = left > 0 ? static_cast<uint32_t>(left) / 1000U : 0;
  }
  store::putT(store::NET, s_status);
}

void wifiManage(uint32_t now) {
  bool up = WiFi.status() == WL_CONNECTED;
  if (up && !s_wasUp) {
    scheduleAll(now);
    IPAddress ip = WiFi.localIP();
    diag::note("net", "ag bagli %u.%u.%u.%u rssi %d", static_cast<unsigned>(ip[0]), static_cast<unsigned>(ip[1]),
               static_cast<unsigned>(ip[2]), static_cast<unsigned>(ip[3]), static_cast<int>(WiFi.RSSI()));
    publishStatus(now);
  }
  if (!up && s_wasUp) {
    diag::note("net", "ag koptu");
    publishStatus(now);
  }
  s_wasUp = up;
}

void handleRequests(uint32_t now) {
  uint32_t mask;
  bool loc;
  portENTER_CRITICAL(&s_mux);
  mask = s_refreshMask;
  loc = s_locationChanged;
  s_refreshMask = 0;
  s_locationChanged = false;
  portEXIT_CRITICAL(&s_mux);
  if (loc) {
    weather::clearAll();
    mask |= (1u << SRC_FORECAST) | (1u << SRC_MARINE) | (1u << SRC_AIR) | (1u << SRC_QUAKE);
    diag::note("net", "konum degisti, veriler yenileniyor");
  }
  for (int i = 0; i < SRC_COUNT; i++) {
    if (mask & (1u << i)) {
      s_nextMs[i] = now;
      s_backoffS[i] = 0;
    }
  }
}

void runJob(int i, uint32_t now) {
  const Job& job = kJobs[i];
  SourceStat& ss = s_status.src[i];
  if (job.needsTime && !clk::synced()) {
    s_nextMs[i] = now + TIME_WAIT_RETRY_MS;
    return;
  }
  FetchStat st;
  memset(&st, 0, sizeof(st));
  int16_t code = job.fn(st);
  uint32_t end = millis();
  if (code == 0) {
    s_nextMs[i] = end + periodOf(job) * 1000UL;
    return;
  }
  uint32_t utc = clk::synced() ? clk::nowUtc() : 0;
  if (code == 200 && !utc) s_unsynced |= 1u << i;
  ss.lastTryUtc = utc;
  ss.lastCode = code;
  ss.durMs = st.durMs;
  ss.bytes = st.bytes;
  ss.via = st.via;
  if (code == 200) {
    ss.lastOk = 1;
    ss.lastOkUtc = utc;
    s_backoffS[i] = 0;
    s_nextMs[i] = end + periodOf(job) * 1000UL;
    if (i == SRC_FORECAST) s_status.lastFullOkUtc = utc;
  } else {
    ss.lastOk = 0;
    if (ss.fails < 0xFFFF) ss.fails++;
    uint32_t b = s_backoffS[i] ? s_backoffS[i] * 2 : cfg::RETRY_MIN_S;
    if (b > cfg::RETRY_MAX_S) b = cfg::RETRY_MAX_S;
    s_backoffS[i] = b;
    s_nextMs[i] = end + b * 1000UL;
    diag::note("net", "%s hata %d (%s), %us sonra", job.name, static_cast<int>(code), http::errText(code),
               static_cast<unsigned>(b));
  }
  publishStatus(end);
}

void runDue(uint32_t now) {
  for (int i = 0; i < SRC_COUNT; i++) {
    if (!due(now, s_nextMs[i])) continue;
    runJob(i, now);
    return;
  }
}

void task(void*) {
  for (;;) {
    uint32_t now = millis();
    wifiManage(now);
    handleRequests(now);
    webui::service(now);
    printer::service(millis());
    events::tick();
    if (s_unsynced && clk::synced()) {
      for (int i = 0; i < SRC_COUNT; i++) {
        if (s_unsynced & (1u << i)) s_nextMs[i] = millis();
      }
      s_unsynced = 0;
    }
    if (s_wasUp && s_scheduled) runDue(millis());
    now = millis();
    if (now - s_lastStatusMs >= cfg::NET_STATUS_MS) publishStatus(now);
    vTaskDelay(pdMS_TO_TICKS(webui::busy() ? 5 : 40));
  }
}

}

rc::JsonArena& arena() { return s_arena; }

bool tlsAllowed() { return true; }

int16_t get(const char* host, const char* path, bool, ParseFn parse, void* ctx, FetchStat& st) {
  memset(&st, 0, sizeof(st));
  st.via = VIA_TLS;
  uint32_t t0 = millis();
  if (WiFi.status() != WL_CONNECTED) {
    st.code = http::E_NO_WIFI;
    return st.code;
  }
  char url[1024];
  int n = snprintf(url, sizeof(url), "https://%s%s", host, path);
  if (n <= 0 || static_cast<size_t>(n) >= sizeof(url) || !s_body) {
    st.code = http::E_BUILD;
    return st.code;
  }
  if (!s_curl) s_curl = curl_easy_init();
  if (!s_curl) {
    st.code = http::E_CONNECT;
    return st.code;
  }
  s_bodyLen = 0;
  curl_easy_setopt(s_curl, curlc::URL, url);
  curl_easy_setopt(s_curl, curlc::WRITEFUNCTION, onBody);
  curl_easy_setopt(s_curl, curlc::WRITEDATA, nullptr);
  curl_easy_setopt(s_curl, curlc::USERAGENT, s_agent);
  curl_easy_setopt(s_curl, curlc::ACCEPT_ENCODING, "");
  curl_easy_setopt(s_curl, curlc::FOLLOWLOCATION, 1L);
  curl_easy_setopt(s_curl, curlc::NOSIGNAL, 1L);
  curl_easy_setopt(s_curl, curlc::IPRESOLVE, curlc::IPRESOLVE_V4);
  curl_easy_setopt(s_curl, curlc::MAXFILESIZE, static_cast<long>(BODY_MAX));
  curl_easy_setopt(s_curl, curlc::CONNECTTIMEOUT_MS, CONNECT_MS);
  curl_easy_setopt(s_curl, curlc::TIMEOUT_MS, TOTAL_MS);
  CURLcode rc = curl_easy_perform(s_curl);
  long status = 0;
  curl_easy_getinfo(s_curl, curlc::RESPONSE_CODE, &status);
  int16_t code;
  if (rc != curlc::OK) {
    code = mapCurl(rc);
    diag::note("net", "%s: %s", host, curl_easy_strerror(rc));
  } else if (status != 200) {
    code = static_cast<int16_t>(status);
  } else {
    s_arena.reset();
    rc::MemSource src(s_body, s_bodyLen);
    code = parse(src, ctx) ? 200 : http::E_PARSE;
  }
  s_arena.reset();
  st.bytes = static_cast<uint32_t>(s_bodyLen);
  st.durMs = millis() - t0;
  st.code = code;
  return code;
}

void begin() {
  if (s_task) return;
  memset(&s_status, 0, sizeof(s_status));
  s_arena.attach(mem::jsonArena(), cfg::JSON_ARENA_BYTES);
  snprintf(s_agent, sizeof(s_agent), "Rumy/%s (Raspberry Pi)", RUMY_VERSION);
  s_body = static_cast<char*>(malloc(BODY_MAX));
  curl_global_init(curlc::GLOBAL_DEFAULT);
  clk::begin();
  xTaskCreatePinnedToCore(task, "net", cfg::NET_STACK, nullptr, cfg::NET_PRIO, &s_task, cfg::NET_CORE);
}

void refreshAll() {
  portENTER_CRITICAL(&s_mux);
  s_refreshMask = (1u << SRC_COUNT) - 1u;
  portEXIT_CRITICAL(&s_mux);
}

void refresh(NetSource s) {
  if (s >= SRC_COUNT) return;
  portENTER_CRITICAL(&s_mux);
  s_refreshMask = s_refreshMask | (1u << s);
  portEXIT_CRITICAL(&s_mux);
}

void locationChanged() {
  portENTER_CRITICAL(&s_mux);
  s_locationChanged = true;
  portEXIT_CRITICAL(&s_mux);
}

bool online() { return WiFi.status() == WL_CONNECTED; }

uint32_t stackFree() { return s_task ? uxTaskGetStackHighWaterMark(s_task) : 0; }

}
