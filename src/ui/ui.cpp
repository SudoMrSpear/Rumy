#include "ui.h"

#include <Arduino.h>
#include <esp_random.h>
#include <string.h>

#include "agents/agents.h"
#include "agents_ui.h"
#include "update_ui.h"
#include "app/config.h"
#include "app/mem.h"
#include "app/store.h"
#include "bench.h"
#include "board/display.h"
#include "clock/clock.h"
#include "diag/diag.h"
#include "icons.h"
#include "lvmem.h"
#include "match_ui.h"
#include "media/media.h"
#include "openmeteo.h"
#include "phone_app.h"
#include "rumy_prof.h"
#include "screens.h"
#include "slide.h"
#include "theme.h"
#include "timeutil.h"
#include "text_tr.h"

namespace ui {

namespace {

Data s_data;
uint32_t s_ver[store::COUNT] = {};
uint32_t s_settingsVer = 0;
ScreenId s_cur = SCR_NONE;
ScreenId s_prev = SCR_HOME;
lv_obj_t* s_home = nullptr;
uint32_t s_animEnd = 0;
uint32_t s_lastPoll = 0;
uint32_t s_lastSec = 0;
uint32_t s_lastStats = 0;
int16_t s_lastMinute = -1;
bool s_night = false;
bool s_nightAwake = false;
uint8_t s_brightnessApplied = 0xFF;
uint8_t s_touchRotApplied = 0xFF;
int32_t s_eventShownDay = -1;
uint32_t s_alertQuakeTime = 0;
uint32_t s_alertUntilMs = 0;
uint32_t s_alertStartMs = 0;
#if RUMY_ENDURANCE
uint32_t s_enduranceNext = 0;
uint8_t s_enduranceIdx = 0;
#endif
uint32_t s_frames = 0;
uint32_t s_fps = 0;
uint32_t s_fpsMark = 0;
ScreenId s_pending = SCR_NONE;
int s_pendingDir = 0;
uint32_t s_createUs = 0;
bool s_wasTouching = false;
uint32_t s_mediaVer = 0;
uint32_t s_agentsVer = 0;
bool s_mediaPlaying = false;
uint32_t s_lowMemRefusals = 0;
uint32_t s_lowMemNoteMs = 0;
uint32_t s_mediaPausedAt = 0;
uint32_t s_printerWakeUntil = 0;
bool s_printerProblem = false;
int32_t s_printerErrSeen = 0;

void updateScreen(ScreenId id, uint32_t mask) {
  switch (id) {
    case SCR_WEATHER: scr_weather::update(mask); break;
    case SCR_LIGHT: scr_light::update(mask); break;
    case SCR_SETTINGS: scr_settings::update(mask); break;
    case SCR_DIAG: scr_diag::update(mask); break;
    case SCR_PAIR: scr_pair::update(mask); break;
    case SCR_LOCATION: scr_location::update(mask); break;
    case SCR_NIGHT: scr_night::update(mask); break;
    case SCR_INFO: scr_info::update(mask); break;
    case SCR_EVENT: scr_event::update(mask); break;
    case SCR_DISPLAY_CFG: scr_display::update(mask); break;
    case SCR_NIGHT_CFG: scr_nightcfg::update(mask); break;
    case SCR_MEDIA: scr_media::update(mask); break;
    case SCR_PRINTER: scr_printer::update(mask); break;
    case SCR_TOUCH_CAL: scr_touchcal::update(mask); break;
    case SCR_MATCH: scr_match::update(mask); break;
    case SCR_PLAYLIST: scr_playlist::update(mask); break;
    case SCR_FOOTBALL: scr_football::update(mask); break;
    case SCR_LEAGUE: scr_league::update(mask); break;
    case SCR_AGENTS: scr_agents::update(mask); break;
    case SCR_PHONE: scr_phone::update(mask); break;
    default: break;
  }
}

lv_obj_t* createScreen(ScreenId id) {
  switch (id) {
    case SCR_HOME: return s_home;
    case SCR_WEATHER: return scr_weather::create();
    case SCR_LIGHT: return scr_light::create();
    case SCR_SETTINGS: return scr_settings::create();
    case SCR_DIAG: return scr_diag::create();
    case SCR_PAIR: return scr_pair::create();
    case SCR_LOCATION: return scr_location::create();
    case SCR_NIGHT: return scr_night::create();
    case SCR_INFO: return scr_info::create();
    case SCR_EVENT: return scr_event::create();
    case SCR_DISPLAY_CFG: return scr_display::create();
    case SCR_NIGHT_CFG: return scr_nightcfg::create();
    case SCR_MEDIA: return scr_media::create();
    case SCR_PRINTER: return scr_printer::create();
    case SCR_TOUCH_CAL: return scr_touchcal::create();
    case SCR_MATCH: return scr_match::create();
    case SCR_PLAYLIST: return scr_playlist::create();
    case SCR_FOOTBALL: return scr_football::create();
    case SCR_LEAGUE: return scr_league::create();
    case SCR_AGENTS: return scr_agents::create();
    case SCR_PHONE: return scr_phone::create();
    default: return nullptr;
  }
}

void dispatch(uint32_t mask) {
  if (!mask) return;
  scr_home::update(mask);
  if (s_cur != SCR_HOME && s_cur != SCR_NONE) updateScreen(s_cur, mask);
}

void applySettings() {
  const Settings& st = s_data.settings;
  if (st.touchRot != s_touchRotApplied) {
    s_touchRotApplied = st.touchRot;
    display::setTouchRotation(st.touchRot);
  }
}

bool inHours(uint16_t now, uint16_t start, uint16_t end) {
  if (start == end) return false;
  if (start < end) return now >= start && now < end;
  return now >= start || now < end;
}

bool computeNight() {
  const Settings& st = s_data.settings;
  if (!clk::synced()) return false;
  if (rc::sleepActive(settings::sleepUntil(), clk::nowUtc())) return true;
  if (st.nightMode == NIGHT_OFF) return false;
  if (st.nightMode == NIGHT_SUN && s_data.daily.valid) {
    uint32_t now = clk::nowUtc();
    int32_t today = clk::todayDayNum();
    for (int i = 0; i < s_data.daily.count; i++) {
      const rc::WxDay& d = s_data.daily.days[i];
      if (d.sunrise && d.sunset && rc::localDayNum(d.sunrise) == today) return now >= d.sunset || now < d.sunrise;
    }
  }
  return inHours(clk::minuteOfDay(), st.nightStart, st.nightEnd);
}

void applyBrightness() {
  const Settings& st = s_data.settings;
  uint8_t want = (s_night && !s_nightAwake) ? st.nightBrightness : st.brightness;
  if (want != s_brightnessApplied) {
    s_brightnessApplied = want;
    display::setBrightnessPct(want);
  }
}

void mediaChanged() {
  const media::State& m = media::state();
  bool playing = m.active && m.playing;
  bool started = playing && !s_mediaPlaying;
  s_mediaPlaying = playing;
  if (playing) s_mediaPausedAt = 0;
  else if (!s_mediaPausedAt) s_mediaPausedAt = millis() | 1u;
  if (s_pending != SCR_NONE || bench::active()) return;
  bool sleeping = s_night && !s_nightAwake;
  if (started && s_cur == SCR_HOME && !sleeping && !display::touching()) go(SCR_MEDIA, -1);
  else if (!m.link && (s_cur == SCR_MEDIA || s_cur == SCR_PLAYLIST)) go(SCR_HOME, 1);
}

void printerChanged() {
  const PrinterState& p = s_data.printer;
  bool problem = p.connected && p.problem;
  int32_t sig = problem ? (p.st.errCode ? p.st.errCode : -1) : 0;
  bool fresh = problem && (!s_printerProblem || sig != s_printerErrSeen);
  s_printerProblem = problem;
  s_printerErrSeen = sig;
  if (!fresh || bench::active()) return;
  diag::note("ui", "yazici sorunu: kod %d, ekran aciliyor", static_cast<int>(p.st.errCode));
  if (s_night) {
    s_printerWakeUntil = (millis() + cfg::PRINTER_ERROR_WAKE_MS) | 1u;
    wake();
  }
  if (s_cur != SCR_PRINTER) go(SCR_PRINTER, -1);
}

bool printerWakeActive() { return s_printerWakeUntil && static_cast<int32_t>(s_printerWakeUntil - millis()) > 0; }

void pollData() {
  uint32_t mask = 0;
  if (store::getT(store::FORECAST, s_data.fc, s_ver[store::FORECAST])) mask |= M_FORECAST;
  if (store::getT(store::DAILY, s_data.daily, s_ver[store::DAILY])) mask |= M_DAILY;
  if (store::getT(store::AIR, s_data.air, s_ver[store::AIR])) mask |= M_AIR;
  if (store::getT(store::MARINE, s_data.sea, s_ver[store::MARINE])) mask |= M_MARINE;
  if (store::getT(store::QUAKE, s_data.quake, s_ver[store::QUAKE])) mask |= M_QUAKE;
  if (store::getT(store::EVENTS, s_data.events, s_ver[store::EVENTS])) mask |= M_EVENTS;
  if (store::getT(store::BULB, s_data.bulb, s_ver[store::BULB])) mask |= M_BULB;
  if (store::getT(store::NET, s_data.net, s_ver[store::NET])) mask |= M_NET;
  if (store::getT(store::PRINTER, s_data.printer, s_ver[store::PRINTER])) {
    mask |= M_PRINTER;
    printerChanged();
  }
  if (store::getT(store::MATCH, s_data.match, s_ver[store::MATCH])) mask |= M_MATCH;
  if (store::getT(store::BOARD, s_data.board, s_ver[store::BOARD])) mask |= M_BOARD;
  if (store::getT(store::VIEW, s_data.view, s_ver[store::VIEW])) mask |= M_VIEW;
  uint32_t sv = store::version(store::SETTINGS);
  if (sv != s_settingsVer) {
    s_settingsVer = sv;
    s_data.settings = settings::get();
    theme::setTeamColors(s_data.settings.teamC1, s_data.settings.teamC2);
    applySettings();
    s_brightnessApplied = 0xFF;
    mask |= M_SETTINGS;
  }
  uint32_t mv = media::version();
  if (mv != s_mediaVer) {
    s_mediaVer = mv;
    mask |= M_MEDIA;
    mediaChanged();
  }
  uint32_t av = agents::version();
  if (av != s_agentsVer) {
    s_agentsVer = av;
    mask |= M_AGENTS;
    agents_ui::changed();
  }
  if (mask & M_QUAKE) {
    const rc::Quake& q = s_data.quake.latest;
    uint32_t now = clk::nowUtc();
    if (q.timeUtc && q.mag10 >= cfg::QUAKE_ALERT_MAG10 && q.timeUtc != s_alertQuakeTime && now &&
        now - q.timeUtc < cfg::QUAKE_ALERT_PIN_S) {
      s_alertQuakeTime = q.timeUtc;
      s_alertUntilMs = millis() + (cfg::QUAKE_ALERT_PIN_S - (now - q.timeUtc)) * 1000UL;
      s_alertStartMs = millis();
      if (s_alertStartMs == 0) s_alertStartMs = 1;
      wake();
    }
  }
  dispatch(mask);
  if (mask & (M_MATCH | M_BOARD)) match_ui::changed();
}

void publishStats() {
  LvglStats st;
  memset(&st, 0, sizeof(st));
  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  st.total = mon.total_size;
  st.used = mon.total_size - mon.free_size;
  st.maxUsed = mon.max_used;
  st.biggestFree = mon.free_biggest_size;
  st.usedPct = mon.used_pct;
  st.fragPct = mon.frag_pct;
  st.uiStackFree = uxTaskGetStackHighWaterMark(nullptr);
  st.fps = s_fps;
  store::putT(store::LVGL, st);
}

void perSecond() {
  uint32_t mask = M_SECOND;
  if (clk::synced()) {
    rc::LocalTime lt = clk::local();
    if (lt.minute != s_lastMinute) {
      s_lastMinute = lt.minute;
      mask |= M_MINUTE;
    }
  }
  bool night = computeNight();
  bool touchedLately = (millis() - display::lastTouchMs() < cfg::NIGHT_WAKE_MS) && display::lastTouchMs() != 0;
  bool awake = night && (touchedLately || printerWakeActive());
  if (night != s_night || awake != s_nightAwake) {
    s_night = night;
    s_nightAwake = awake;
    mask |= M_NIGHT;
  }
  applyBrightness();

  bool sleeping = s_night && !s_nightAwake && RUMY_ENDURANCE == 0;
  bool autoNav = !bench::active();
  if (autoNav && sleeping && s_cur != SCR_NIGHT && s_pending == SCR_NONE) go(SCR_NIGHT, 0);
  if (autoNav && !sleeping && s_cur == SCR_NIGHT && s_pending == SCR_NONE) go(SCR_HOME, 0);

  uint32_t inactive = lv_display_get_inactive_time(nullptr);
  if (autoNav && s_cur == SCR_MEDIA && !s_mediaPlaying && s_mediaPausedAt &&
      millis() - s_mediaPausedAt > cfg::MEDIA_PAUSED_HOME_MS && inactive > cfg::MEDIA_PAUSED_HOME_MS &&
      s_pending == SCR_NONE) {
    go(SCR_HOME, 1);
  }
  bool keepPrinter = s_cur == SCR_PRINTER && printerActive();
  if (autoNav && s_cur != SCR_HOME && s_cur != SCR_NONE && s_cur != SCR_NIGHT && s_cur != SCR_MEDIA && !keepPrinter &&
      !display::touching() && inactive > static_cast<uint32_t>(s_data.settings.homeTimeoutS) * 1000UL &&
      RUMY_ENDURANCE == 0) {
    go(SCR_HOME, -1);
  }

  if (autoNav && clk::synced() && s_data.events.todayCount > 0 && s_cur == SCR_HOME && !(s_night && !s_nightAwake)) {
    int32_t today = clk::todayDayNum();
    if (s_data.events.computedDay == today && s_eventShownDay != today) {
      s_eventShownDay = today;
      go(SCR_EVENT, 0);
    }
  }

  uint32_t now = millis();
  if (now - s_fpsMark >= 1000) {
    s_fps = s_frames;
    s_frames = 0;
    s_fpsMark = now;
  }
  if (now - s_lastStats >= 2000) {
    s_lastStats = now;
    publishStats();
  }

#if RUMY_ENDURANCE
  if (static_cast<int32_t>(now - s_enduranceNext) >= 0) {
    s_enduranceNext = now + cfg::ENDURANCE_SCREEN_MS;
    static const ScreenId kCycle[] = {SCR_HOME, SCR_WEATHER, SCR_LIGHT, SCR_SETTINGS, SCR_DIAG};
    s_enduranceIdx = static_cast<uint8_t>((s_enduranceIdx + 1) % (sizeof(kCycle) / sizeof(kCycle[0])));
    ScreenId next = kCycle[s_enduranceIdx];
    go(next, next == SCR_HOME ? -1 : 1);
  } else if (s_cur == SCR_WEATHER) {
    scr_weather::enduranceStep();
  }
#endif

  dispatch(mask);
  match_ui::tick();
  agents_ui::tick();
  update_ui::tick();
}

void gestureCb(lv_event_t* e) {
  (void)e;
  if (static_cast<int32_t>(lv_tick_get() - s_animEnd) < 0) return;
  lv_dir_t d = lv_indev_get_gesture_dir(lv_indev_active());
  ScreenId cur = s_cur;
  if (cur == SCR_EVENT) {
    go(SCR_HOME, 0);
    return;
  }
  if (cur == SCR_LEAGUE) {
    if (d == LV_DIR_RIGHT) go(SCR_FOOTBALL, -1);
    return;
  }
  bool printer = s_data.printer.configured != 0;
  if (cur == SCR_FOOTBALL) {
    if (d == LV_DIR_LEFT) go(SCR_HOME, 1);
    else if (d == LV_DIR_RIGHT && media::state().link) go(SCR_MEDIA, -1);
    else if (d == LV_DIR_RIGHT && printer) go(SCR_PRINTER, -1);
    return;
  }
  if (cur == SCR_MEDIA) {
    if (d == LV_DIR_LEFT) go(SCR_FOOTBALL, 1);
    else if (d == LV_DIR_RIGHT && printer) go(SCR_PRINTER, -1);
    return;
  }
  if (cur == SCR_PRINTER) {
    if (d == LV_DIR_LEFT) go(media::state().link ? SCR_MEDIA : SCR_FOOTBALL, 1);
    return;
  }
  if (cur == SCR_HOME && d == LV_DIR_RIGHT) {
    go(SCR_FOOTBALL, -1);
    return;
  }
  if (cur < SCR_MAIN_COUNT) {
    if (d == LV_DIR_LEFT && cur + 1 < SCR_MAIN_COUNT) go(static_cast<ScreenId>(cur + 1), 1);
    else if (d == LV_DIR_RIGHT && cur > 0) go(static_cast<ScreenId>(cur - 1), -1);
  } else if (d == LV_DIR_RIGHT) {
    back();
  }
}

void flushWatch(lv_event_t* e) {
  (void)e;
  s_frames++;
  bench::frameReady();
}

}

void begin() {
  lv_rand_set_seed(esp_random());
  theme::init();
  s_data.settings = settings::get();
  theme::setTeamColors(s_data.settings.teamC1, s_data.settings.teamC2);
  s_settingsVer = store::version(store::SETTINGS);
  s_touchRotApplied = s_data.settings.touchRot;
  rc::clearForecast(s_data.fc);
  rc::clearDaily(s_data.daily);
  s_data.air.eaqi = rc::NAU16;
  s_data.sea.sst10 = rc::NA16;
  rc::k2Clear(s_data.printer.st);
  pollData();
  s_home = scr_home::create();
  lv_display_add_event_cb(lv_display_get_default(), flushWatch, LV_EVENT_REFR_READY, nullptr);
  lv_screen_load(s_home);
  s_cur = SCR_HOME;
  scr_home::setVisible(true);
  dispatch(M_ALL);
  s_brightnessApplied = 0xFF;
  applyBrightness();
#if RUMY_ENDURANCE
  s_enduranceNext = millis() + cfg::ENDURANCE_SCREEN_MS;
#endif
}

uint32_t screenNeed(ScreenId id) {
  switch (id) {
    case SCR_WEATHER: return 10240;
    case SCR_LIGHT: return 8704;
    case SCR_SETTINGS: return 13824;
    case SCR_DISPLAY_CFG: return 10752;
    case SCR_NIGHT_CFG: return 10240;
    case SCR_LOCATION: return 5632;
    case SCR_PAIR: return 4608;
    case SCR_DIAG: return 6144;
    case SCR_INFO: return 4608;
    case SCR_MEDIA: return 6656;
    case SCR_PRINTER: return 3072;
    case SCR_NIGHT: return 3584;
    case SCR_EVENT: return 3584;
    case SCR_MATCH: return 4096;
    case SCR_PLAYLIST: return 4096;
    case SCR_FOOTBALL: return 3072;
    case SCR_LEAGUE: return 4096;
    case SCR_AGENTS: return 3072;
    case SCR_PHONE: return 6144;
    default: return 0;
  }
}

bool roomFor(ScreenId id) {
  uint32_t need = screenNeed(id);
  if (!need) return true;
  lvmem::Snap m = lvmem::snap();
  if (m.free >= need && m.biggest >= cfg::LVGL_MIN_BLOCK) return true;
  s_lowMemRefusals++;
  uint32_t now = millis();
  if (s_lowMemNoteMs && now - s_lowMemNoteMs < cfg::LVGL_REFUSE_NOTE_MS) return false;
  s_lowMemNoteMs = now | 1u;
  diag::note("ui", "bellek yetersiz: ekran %u acilmadi (bos %u, en buyuk %u, gerek %u)", static_cast<unsigned>(id),
             static_cast<unsigned>(m.free), static_cast<unsigned>(m.biggest), static_cast<unsigned>(need));
  if (m.free >= cfg::LVGL_TOAST_NEED) toast(rc::L("Bellek yetersiz, ekran açılamadı", "Not enough memory to open the screen"));
  return false;
}

void doGo(ScreenId id, int dir) {
  if (id == s_cur || id == SCR_NONE) return;
  uint32_t t0 = micros();
  ScreenId prev = s_cur;
  lv_obj_t* old = lv_screen_active();
  if (prev != SCR_HOME && prev != SCR_NONE && old && old != s_home) lv_obj_delete(old);
  if (!roomFor(id)) {
    if (prev == SCR_HOME) return;
    id = SCR_HOME;
  }
  RUMY_PROF_BEGIN("ekran_olustur");
  lv_obj_t* scr = createScreen(id);
  RUMY_PROF_END("ekran_olustur");
  if (!scr) {
    if (prev == SCR_HOME) return;
    id = SCR_HOME;
    scr = s_home;
  }
  if (dir > 0 && prev != SCR_NONE && prev != SCR_NIGHT && prev != SCR_EVENT) s_prev = prev;
  if (id == SCR_HOME || id < SCR_MAIN_COUNT) s_prev = SCR_HOME;
  s_cur = id;
  RUMY_PROF_BEGIN("ekran_guncelle");
  if (id != SCR_HOME) updateScreen(id, M_ALL);
  scr_home::setVisible(id == SCR_HOME);
  if (id == SCR_HOME) scr_home::update(M_ALL);
  RUMY_PROF_END("ekran_guncelle");
  s_createUs = micros() - t0;
  if (dir != 0) slide::run(scr, dir);
  else lv_screen_load(scr);
  s_animEnd = lv_tick_get() + 40;
}

void loop() {
  if (s_pending != SCR_NONE && static_cast<int32_t>(lv_tick_get() - s_animEnd) >= 0) {
    ScreenId id = s_pending;
    s_pending = SCR_NONE;
    doGo(id, s_pendingDir);
  }
  uint32_t t0 = micros();
  uint32_t wait = lv_timer_handler();
  bench::handlerDone(micros() - t0);
  bench::tick();
  bool touching = display::touching();
  if (touching && !s_wasTouching && s_cur == SCR_HOME) scr_home::onTouch();
  s_wasTouching = touching;
  uint32_t now = millis();
  if (now - s_lastPoll >= cfg::UI_DATA_POLL_MS) {
    s_lastPoll = now;
    pollData();
  }
  if (now - s_lastSec >= 1000) {
    s_lastSec = now;
    perSecond();
  }
  if (wait > 10) wait = 10;
  if (wait == 0) wait = 1;
  vTaskDelay(pdMS_TO_TICKS(wait));
}

void go(ScreenId id, int dir) {
  if (id == SCR_NONE) return;
  s_pending = id;
  s_pendingDir = dir;
}

void back() {
  ScreenId to = s_prev;
  if (to == s_cur || to == SCR_NONE || to == SCR_NIGHT || to == SCR_EVENT) to = SCR_HOME;
  go(to, -1);
}

ScreenId current() { return s_cur; }

bool busy() { return s_pending != SCR_NONE || static_cast<int32_t>(lv_tick_get() - s_animEnd) < 0; }

uint32_t lastCreateUs() { return s_createUs; }

uint32_t lowMemRefusals() { return s_lowMemRefusals; }

const Data& data() { return s_data; }

bool night() { return s_night; }

bool nightAwake() { return s_nightAwake; }

void wake() {
  lv_display_trigger_activity(nullptr);
  s_nightAwake = s_night;
  applyBrightness();
}

void toast(const char* text) {
  lv_obj_t* box = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(box);
  lv_obj_set_style_bg_color(box, theme::col(theme::C_SURFACE2), 0);
  lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(box, theme::RADIUS_M, 0);
  lv_obj_set_style_border_width(box, 1, 0);
  lv_obj_set_style_border_color(box, theme::col(theme::C_EYE), 0);
  lv_obj_set_style_pad_hor(box, theme::S(12), 0);
  lv_obj_set_style_pad_ver(box, theme::S(8), 0);
  lv_obj_set_size(box, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_max_width(box, theme::FW(296), 0);
  lv_obj_align(box, LV_ALIGN_BOTTOM_MID, 0, -theme::S(10));
  lv_obj_remove_flag(box, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_t* l = theme::makeLabel(box, theme::fontM(), theme::C_TEXT);
  lv_label_set_long_mode(l, LV_LABEL_LONG_MODE_WRAP);
  lv_obj_set_style_max_width(l, theme::FW(272), 0);
  lv_label_set_text(l, text);
  lv_timer_t* t = lv_timer_create(
      [](lv_timer_t* tm) {
        lv_obj_t* o = static_cast<lv_obj_t*>(lv_timer_get_user_data(tm));
        lv_obj_delete(o);
      },
      2600, box);
  lv_timer_set_repeat_count(t, 1);
}

void attachGestures(lv_obj_t* scr) { lv_obj_add_event_cb(scr, gestureCb, LV_EVENT_GESTURE, nullptr); }

bool quakeAlertActive() { return s_alertUntilMs && static_cast<int32_t>(s_alertUntilMs - millis()) > 0; }

bool quakeSurprise() { return s_alertStartMs && millis() - s_alertStartMs < 30000UL; }

bool printerActive() {
  const PrinterState& p = s_data.printer;
  if (!p.connected) return false;
  return p.problem || p.phase == rc::K2_PRINTING || p.phase == rc::K2_PAUSED || p.phase == rc::K2_SELFTEST;
}

bool rainSoon() {
  const rc::Forecast& f = s_data.fc;
  if (!f.valid) return false;
  for (int i = 0; i < f.m15.count && i < rc::MINUTELY; i++) {
    if (f.m15.precip100[i] != rc::NAU16 && f.m15.precip100[i] >= 10) return true;
  }
  for (int i = 0; i < 3 && i < f.hourCount; i++) {
    if (f.hours[i].pop != rc::NA8 && f.hours[i].pop >= 60) return true;
  }
  return false;
}

}
