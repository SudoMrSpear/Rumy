#include <stdio.h>
#include <string.h>

#include "afad.h"
#include "agents/agents.h"
#include "agents_ui.h"
#include "app/config.h"
#include "board/sdcard.h"
#include "clock/clock.h"
#include "color.h"
#include "draw_util.h"
#include "events_calc.h"
#include "fonts/symbols.h"
#include "icons.h"
#include "media/media.h"
#include "record_frames.h"
#include "rumy/look.h"
#include "rumy/rumy.h"
#include "screens.h"
#include "src/misc/lv_text_private.h"
#include "text_tr.h"
#include "match_ui.h"
#include "theme.h"
#include "ui.h"
#include "wind_tr.h"
#include "wiz/wiz.h"
#include "wmo_tr.h"

namespace scr_home {

namespace {

enum Card : uint8_t {
  CARD_WEATHER = 0,
  CARD_WIND,
  CARD_RAIN,
  CARD_SUN,
  CARD_SEA,
  CARD_AIR,
  CARD_EVENT,
  CARD_MATCH,
  CARD_PRINTER,
  CARD_QUAKE,
  CARD_OFFLINE,
  CARD_SD,
  CARD_COUNT,
};

using theme::FW;
using theme::R;
using theme::S;

constexpr int32_t FACE_H = S(88);
constexpr int32_t FACE_EYE = S(52);
constexpr int32_t CLOCK_X = S(6);
constexpr int32_t CLOCK_Y = S(88);
constexpr int32_t CLOCK_W = S(182);
constexpr int32_t CLOCK_H = S(66);
constexpr bool WIDE = theme::SW >= 480;
constexpr int32_t COL_X = S(184);
constexpr int32_t COL_Y = S(95);
constexpr int32_t COL_RIGHT = theme::SW - 1 - S(12);
constexpr int32_t WX_PAD = S(10);
constexpr int32_t WX_X = COL_X - WX_PAD;
constexpr int32_t WX_H = S(96);
constexpr icons::Size WX_ICON = icons::S64;
constexpr icons::Size WX_ICON_CLEAR = icons::S48;
constexpr int32_t WX_ICON_DY = S(13);
constexpr int32_t WX_ICON_LIFT = S(3);
constexpr int32_t WX_TEMP_GAP = WIDE ? S(12) : S(4);
constexpr int32_t WX_TEMP_DY = S(8);
constexpr int32_t WX_HILO_DY = S(40);
constexpr int32_t DATE_Y = S(154);
constexpr int32_t DATE_H = S(29);
constexpr int32_t BARS_Y = S(184);
constexpr int32_t BARS_H = S(26);
constexpr int32_t CARD_Y = S(212);
constexpr int32_t CARD_H = S(26);
constexpr int32_t CARD_TEXT_W = FW(270);
constexpr int32_t CITY_W = S(86);
constexpr int32_t ROW_X = S(6);
constexpr int32_t ROW_W = FW(308);
constexpr int32_t DAY_GAP = S(10);
constexpr int32_t NP_X = COL_X;
constexpr int32_t NP_RIGHT = COL_RIGHT;
constexpr int32_t NP_DISC = RECORD_PX;
constexpr int32_t NP_SHIFT = RECORD_PX + S(5);
constexpr int32_t NP_TEXT_GAP = S(6);
constexpr int32_t NP_DATE_GAP = S(14);
constexpr int32_t DATE_W = NP_X - NP_DATE_GAP - ROW_X;
constexpr int32_t NP_FADE = S(10);
constexpr int32_t NP_LOOP_GAP = S(32);
constexpr int32_t NP_SPEED = S(30);
constexpr int32_t NP_DECIDEG_PER_MS = 2;
constexpr uint32_t NP_FRAME_MS = 33;
constexpr uint32_t NP_HOLD_MS = 1500;
constexpr uint32_t WAKE_MS = 1200;
constexpr int32_t AG_X = S(4);
constexpr int32_t AG_Y = S(26);
constexpr int32_t AG_W = S(58);
constexpr int32_t AG_H = S(28);
constexpr int32_t AG_CX = S(14);
constexpr int32_t AG_R = S(9);

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_face = nullptr;
lv_obj_t* s_city = nullptr;
lv_obj_t* s_bulb = nullptr;
lv_obj_t* s_bulbIcon = nullptr;
lv_obj_t* s_clock = nullptr;
lv_obj_t* s_wx = nullptr;
lv_obj_t* s_date = nullptr;
lv_obj_t* s_bars = nullptr;
lv_obj_t* s_card = nullptr;
lv_obj_t* s_cardIcon = nullptr;
lv_obj_t* s_cardText = nullptr;
lv_obj_t* s_cardLive = nullptr;
lv_timer_t* s_cardTimer = nullptr;
uint8_t s_cardIdx = 0;
uint32_t s_rotation = 0;
bool s_visible = false;
bool s_colon = true;
lv_area_t s_colonArea = {0, 0, -1, -1};
bool s_idleAsleep = false;
uint32_t s_wakeUntil = 0;
lv_obj_t* s_np = nullptr;
lv_timer_t* s_npTimer = nullptr;
bool s_npShown = false;
bool s_npPlaying = false;
bool s_npRunning = false;
bool s_npScrolling = false;
int32_t s_npTitleW = 0;
int32_t s_npChars = 0;
char s_npTitle[sizeof(media::State::title)];
uint8_t s_npByte[sizeof(media::State::title)];
int16_t s_npPos[sizeof(media::State::title)];
int32_t s_npOff = 0;
int32_t s_npAngle = 0;
uint32_t s_npTitleHash = 0;
uint32_t s_npPhaseAt = 0;
uint32_t s_npLastTick = 0;
uint32_t s_npPausedAt = 0;
lv_obj_t* s_ag = nullptr;
lv_timer_t* s_agTimer = nullptr;
uint8_t s_agTop = rc::AG_NONE;
bool s_agRunning = false;

using theme::setText;

lv_color_t C(uint32_t hex) { return theme::col(hex); }

int32_t textW(const lv_font_t* f, const char* s) {
  lv_point_t p;
  lv_text_get_size(&p, s, f, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_EXPAND);
  return p.x;
}

bool printerProblem(const ui::Data& d) { return d.printer.connected && d.printer.problem; }

bool matchLive(const ui::Data& d) {
  return d.match.valid && (d.match.phase == rc::MP_LIVE || d.match.phase == rc::MP_HALF);
}

bool matchSoon(const ui::Data& d) {
  const rc::Match& m = d.match;
  if (!m.valid || !clk::synced()) return false;
  if (m.phase == rc::MP_POST) return m.endUtc && clk::nowUtc() < m.endUtc + cfg::MATCH_AFTER_S;
  if (m.phase != rc::MP_PRE) return false;
  return rc::localDayNum(m.kickoffUtc) == clk::todayDayNum();
}

bool matchPinned(const ui::Data& d) {
  const rc::Match& m = d.match;
  if (!m.valid || m.phase != rc::MP_PRE || !clk::synced()) return false;
  return clk::nowUtc() + cfg::MATCH_PIN_S >= m.kickoffUtc;
}

bool matchCard(const ui::Data& d) {
  const rc::Match& m = d.match;
  if (matchLive(d) || matchSoon(d)) return true;
  if (!m.valid || m.phase != rc::MP_PRE || !clk::synced()) return false;
  int32_t days = rc::localDayNum(m.kickoffUtc) - clk::todayDayNum();
  return days >= 0 && days < static_cast<int32_t>(cfg::MATCH_CARD_DAYS);
}

bool printerRecentlyDone(const ui::Data& d) {
  const PrinterState& p = d.printer;
  if (!p.connected || p.phase != rc::K2_DONE || !p.st.startUtc || !clk::synced()) return false;
  uint32_t end = p.st.startUtc + p.st.jobS;
  uint32_t now = clk::nowUtc();
  return now >= end && now - end < cfg::PRINTER_DONE_CARD_S;
}

bool cardAvailable(Card c, const ui::Data& d) {
  switch (c) {
    case CARD_PRINTER: return ui::printerActive() || printerRecentlyDone(d);
    case CARD_MATCH: return matchCard(d);
    case CARD_WEATHER: return d.fc.valid;
    case CARD_WIND: return d.fc.valid && d.fc.now.wind10 != rc::NAU16;
    case CARD_RAIN: return d.fc.valid && d.fc.hourCount >= 6;
    case CARD_SUN: return d.daily.valid && d.daily.days[0].sunrise;
    case CARD_SEA: return d.sea.valid;
    case CARD_AIR: return d.air.valid;
    case CARD_EVENT: {
      if (!d.events.count || !clk::synced()) return false;
      int days = static_cast<int>(d.events.items[0].dayNum - clk::todayDayNum());
      uint8_t tier = rc::countdownTier(days);
      if (tier == rc::TIER_FREQUENT || tier == rc::TIER_TODAY) return true;
      return tier == rc::TIER_RARE && (s_rotation % 3) == 0;
    }
    case CARD_QUAKE: return d.quake.valid && d.quake.latest.timeUtc != 0;
    case CARD_OFFLINE: return !d.net.wifi || (d.net.lastFullOkUtc && clk::nowUtc() - d.net.lastFullOkUtc > 3600);
    case CARD_SD: return !sdcard::mounted();
    default: return false;
  }
}

size_t partEnd(const char* text, const char* part) {
  const char* p = strstr(text, part);
  return p ? static_cast<size_t>(p - text) : strlen(text);
}

void fillCard(Card c, const ui::Data& d, char* icon, size_t icap, char* text, size_t tcap, uint32_t& color,
              size_t& fitFrom, size_t& fitTo) {
  fitFrom = 0;
  fitTo = SIZE_MAX;
  color = theme::C_NEON_CYAN;
  char a[16], b[16];
  switch (c) {
    case CARD_WEATHER: {
      bool feels = d.fc.now.feels10 != rc::NA16;
      int16_t t10 = feels ? d.fc.now.feels10 : d.fc.now.temp10;
      if (t10 == rc::NA16) snprintf(a, sizeof(a), "--°");
      else rc::formatTemp(a, sizeof(a), t10);
      snprintf(icon, icap, RS_THERMO);
      if (d.fc.now.rh == rc::NA8) snprintf(text, tcap, "%s %s", feels ? rc::L("Hissedilen", "Feels like") : rc::L("Sıcaklık", "Temperature"), a);
      else snprintf(text, tcap, rc::L("%s %s · Nem %%%u", "%s %s · Humidity %u%%"), feels ? rc::L("Hissedilen", "Feels like") : rc::L("Sıcaklık", "Temperature"), a, static_cast<unsigned>(d.fc.now.rh));
      color = theme::C_NEON_ORANGE;
      break;
    }
    case CARD_WIND: {
      uint16_t dir = d.fc.now.windDir == rc::NAU16 ? 0 : d.fc.now.windDir;
      snprintf(icon, icap, RS_WIND);
      snprintf(text, tcap, rc::L("%s (%s) %u km/sa · hamle %u", "%s (%s) %u km/h · gusts %u"), rc::windNameTr(dir), rc::windAbbrTr(dir),
               static_cast<unsigned>((d.fc.now.wind10 + 5) / 10),
               static_cast<unsigned>(d.fc.now.gust10 == rc::NAU16 ? 0 : (d.fc.now.gust10 + 5) / 10));
      color = theme::C_NEON_CYAN;
      break;
    }
    case CARD_RAIN: {
      uint8_t maxPop = 0;
      for (int i = 0; i < 6 && i < d.fc.hourCount; i++) {
        if (d.fc.hours[i].pop != rc::NA8 && d.fc.hours[i].pop > maxPop) maxPop = d.fc.hours[i].pop;
      }
      snprintf(icon, icap, RS_UMBRELLA);
      color = theme::C_NEON_BLUE;
      if (maxPop >= 50) {
        snprintf(text, tcap, rc::L("Yağış %%%u · Şemsiye al!", "Rain %u%% · Take an umbrella!"), static_cast<unsigned>(maxPop));
        color = theme::C_NEON_YELLOW;
      } else if (maxPop >= 20) {
        snprintf(text, tcap, rc::L("6 saatte yağış ihtimali %%%u", "Rain chance in 6 h: %u%%"), static_cast<unsigned>(maxPop));
      } else {
        snprintf(text, tcap, "%s", rc::L("6 saat yağış beklenmiyor", "No rain expected for 6 h"));
      }
      break;
    }
    case CARD_SUN: {
      rc::LocalTime rise = rc::toLocal(d.daily.days[0].sunrise);
      rc::LocalTime set = rc::toLocal(d.daily.days[0].sunset);
      rc::formatHm(a, sizeof(a), rise.hour, rise.minute);
      rc::formatHm(b, sizeof(b), set.hour, set.minute);
      char dur[24];
      rc::durationTr(d.daily.days[0].sunset - d.daily.days[0].sunrise, dur, sizeof(dur));
      snprintf(icon, icap, RS_SUN);
      snprintf(text, tcap, "\xE2\x86\x91%s  \xE2\x86\x93%s · %s", a, b, dur);
      color = theme::C_NEON_YELLOW;
      break;
    }
    case CARD_SEA: {
      rc::formatDecimalTr(a, sizeof(a), d.sea.sst10);
      snprintf(icon, icap, RS_WATER);
      snprintf(text, tcap, rc::L("Deniz suyu %s° · %s", "Sea water %s° · %s"), a, d.settings.city);
      fitFrom = partEnd(text, " · ") + 4;
      color = theme::C_NEON_CYAN;
      break;
    }
    case CARD_AIR: {
      snprintf(icon, icap, RS_LEAF);
      snprintf(text, tcap, rc::L("Hava kalitesi %s · AQI %u", "Air quality %s · AQI %u"), rc::aqiLevelTr(d.air.eaqi), static_cast<unsigned>(d.air.eaqi));
      color = rc::aqiLevelIndex(d.air.eaqi) >= 3 ? theme::C_NEON_ORANGE : theme::C_NEON_GREEN;
      break;
    }
    case CARD_EVENT: {
      const rc::EventOcc& e = d.events.items[0];
      char t[96];
      rc::eventCountdownText(e, clk::todayDayNum(), t, sizeof(t));
      snprintf(icon, icap, e.kind >= rc::EV_BIRTHDAY ? RS_CAKE : RS_CALENDAR);
      snprintf(text, tcap, "%s", t);
      const char* sep = strstr(text, " · ");
      if (sep) {
        fitTo = static_cast<size_t>(sep - text);
      } else if (strncmp(text, rc::L("Bugün: ", "Today: "), strlen(rc::L("Bugün: ", "Today: "))) &&
                 strncmp(text, rc::L("Yarın: ", "Tomorrow: "), strlen(rc::L("Yarın: ", "Tomorrow: ")))) {
        const char* last = strrchr(text, ' ');
        const char* num = last;
        while (num && num > text && num[-1] != ' ') num--;
        if (num && num > text) fitTo = static_cast<size_t>(num - 1 - text);
      }
      color = theme::C_NEON_PINK;
      break;
    }
    case CARD_MATCH: {
      const rc::Match& m = d.match;
      char t[32];
      snprintf(icon, icap, RS_FUTBOL);
      color = theme::team();
      bool homeOurs = m.usHome == 1;
      if (m.phase == rc::MP_PRE) {
        match_ui::kickoffText(m, t, sizeof(t));
        snprintf(text, tcap, "%s · %s \xE2\x80\x93 %s", t, homeOurs ? m.home.abbr : m.home.name,
                 homeOurs ? m.away.name : m.away.abbr);
        if (homeOurs) {
          fitFrom = partEnd(text, " \xE2\x80\x93 ") + 5;
        } else {
          fitFrom = partEnd(text, " · ") + 4;
          fitTo = partEnd(text, " \xE2\x80\x93 ");
        }
      } else {
        match_ui::scoreText(m, t, sizeof(t));
        if (m.phase == rc::MP_POST) snprintf(text, tcap, rc::L("Maç bitti · %s", "Full time · %s"), t);
        else snprintf(text, tcap, "%s · %s", t, m.clock);
      }
      break;
    }
    case CARD_PRINTER: {
      const PrinterState& p = d.printer;
      char dur[24];
      char name[40];
      char mat[12];
      snprintf(icon, icap, RS_CUBE);
      color = theme::C_NEON_CYAN;
      if (printerProblem(d)) {
        if (rc::k2Runout(p.st)) snprintf(text, tcap, "%s", rc::L("Yazıcı: filament bitti, baskı bekliyor", "Printer: out of filament, print paused"));
        else snprintf(text, tcap, rc::L("Yazıcı hatası · kod %ld", "Printer error · code %ld"), static_cast<long>(p.st.errCode));
        color = theme::C_NEON_RED;
      } else if (p.phase == rc::K2_DONE) {
        rc::k2JobName(p.st.file, name, sizeof(name), mat, sizeof(mat));
        snprintf(text, tcap, rc::L("Baskı bitti: %s", "Print done: %s"), name[0] ? name : rc::L("yazıcı", "printer"));
        fitFrom = partEnd(text, ": ") + 2;
        color = theme::C_NEON_GREEN;
      } else if (p.phase == rc::K2_PAUSED) {
        snprintf(text, tcap, rc::L("Baskı duraklatıldı · %%%u", "Print paused · %u%%"), static_cast<unsigned>(p.st.progress));
        color = theme::C_NEON_YELLOW;
      } else if (p.st.leftS) {
        rc::durationTr(p.st.leftS, dur, sizeof(dur));
        snprintf(text, tcap, rc::L("Baskı %%%u · %s kaldı", "Print %u%% · %s left"), static_cast<unsigned>(p.st.progress), dur);
      } else {
        snprintf(text, tcap, rc::L("Baskı %%%u · %s", "Print %u%% · %s"), static_cast<unsigned>(p.st.progress), rc::k2PhaseTr(static_cast<rc::K2Phase>(p.phase)));
      }
      break;
    }
    case CARD_QUAKE: {
      const rc::Quake& q = d.quake.latest;
      char ago[24];
      uint32_t now = clk::nowUtc();
      rc::agoTr(now > q.timeUtc ? now - q.timeUtc : 0, ago, sizeof(ago));
      rc::formatDecimalTr(a, sizeof(a), q.mag10);
      snprintf(icon, icap, RS_WARNING);
      const char* comma = strstr(q.place, ", ");
      const char* place = comma ? comma + 2 : q.place;
      snprintf(text, tcap, "M%s %s · %s", a, place, ago);
      const char* paren = strstr(place, " (");
      if (paren && du::width(theme::fontM(), text) > CARD_TEXT_W)
        snprintf(text, tcap, "M%s %.*s · %s", a, static_cast<int>(paren - place), place, ago);
      fitFrom = partEnd(text, " ") + 1;
      fitTo = partEnd(text, " · ");
      color = q.mag10 >= cfg::QUAKE_ALERT_MAG10 ? theme::C_NEON_RED : theme::C_NEON_ORANGE;
      break;
    }
    case CARD_OFFLINE: {
      snprintf(icon, icap, RS_WIFI);
      if (d.fc.fetchedUtc) {
        rc::LocalTime lt = rc::toLocal(d.fc.fetchedUtc);
        rc::formatHm(a, sizeof(a), lt.hour, lt.minute);
        snprintf(text, tcap, rc::L("Bağlantı yok · son veri %s", "Offline · last data %s"), a);
      } else {
        snprintf(text, tcap, "%s", rc::L("İnternet bağlantısı bekleniyor", "Waiting for internet"));
      }
      color = theme::C_NEON_RED;
      break;
    }
    case CARD_SD: {
      snprintf(icon, icap, RS_SD);
      snprintf(text, tcap, "%s", rc::L("SD kart yok · kişisel günler kaydedilmez", "No SD card · personal days won't be saved"));
      color = theme::C_NEON_ORANGE;
      break;
    }
    default:
      icon[0] = 0;
      text[0] = 0;
      break;
  }
}

void showCard(bool advance) {
  if (!s_card) return;
  const ui::Data& d = ui::data();
  Card pick = CARD_COUNT;
  if (ui::quakeAlertActive() && (s_rotation & 1) == 0) {
    pick = CARD_QUAKE;
  } else if (printerProblem(d) && (s_rotation & 1) == 0) {
    pick = CARD_PRINTER;
  } else if (matchLive(d) || matchPinned(d) || (matchSoon(d) && (s_rotation & 1) == 0)) {
    pick = CARD_MATCH;
  } else {
    for (int i = 0; i < CARD_COUNT; i++) {
      uint8_t idx = static_cast<uint8_t>((s_cardIdx + (advance ? 1 : 0) + i) % CARD_COUNT);
      if (cardAvailable(static_cast<Card>(idx), d)) {
        pick = static_cast<Card>(idx);
        break;
      }
    }
  }
  if (pick == CARD_COUNT) {
    setText(s_cardIcon, RS_CLOCK);
    setText(s_cardText, rc::L("Veriler yükleniyor…", "Loading data…"));
    return;
  }
  if (advance && pick <= s_cardIdx) s_rotation++;
  s_cardIdx = pick;
  char icon[8], text[160], fitted[160];
  uint32_t color;
  size_t fitFrom, fitTo;
  fillCard(pick, d, icon, sizeof(icon), text, sizeof(text), color, fitFrom, fitTo);
  setText(s_cardIcon, icon);
  setText(s_cardText, du::fit(theme::fontM(), text, CARD_TEXT_W, fitted, sizeof(fitted), fitFrom, fitTo));
  lv_obj_set_style_text_color(s_cardIcon, C(color), 0);
  bool alert = (pick == CARD_QUAKE && ui::quakeAlertActive()) || (pick == CARD_PRINTER && printerProblem(d));
  bool live = pick == CARD_MATCH && matchLive(d);
  lv_obj_set_style_bg_color(s_card, C(alert ? 0x3A0A10 : live ? theme::teamBg() : theme::C_BG), 0);
  lv_obj_set_style_border_color(s_card, C(color), 0);
  if (pick == CARD_MATCH) {
    lv_obj_add_flag(s_cardIcon, LV_OBJ_FLAG_HIDDEN);
    if (live) lv_obj_add_flag(s_cardText, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_remove_flag(s_cardText, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_cardLive, LV_OBJ_FLAG_HIDDEN);
    lv_obj_invalidate(s_cardLive);
  } else {
    lv_obj_remove_flag(s_cardIcon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(s_cardText, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(s_cardLive, LV_OBJ_FLAG_HIDDEN);
  }
  if (advance) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, s_cardText);
    lv_anim_set_values(&a, S(12), 0);
    lv_anim_set_duration(&a, 260);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&a, [](void* o, int32_t v) { lv_obj_set_style_translate_y(static_cast<lv_obj_t*>(o), v, 0); });
    lv_anim_start(&a);
  }
}

void cardTimerCb(lv_timer_t* t) {
  (void)t;
  if (ui::night() && !ui::nightAwake()) return;
  showCard(true);
}

uint32_t aqiColor(uint16_t eaqi) {
  static const uint32_t kColors[6] = {theme::C_NEON_GREEN, theme::C_NEON_LIME, theme::C_NEON_YELLOW,
                                      theme::C_NEON_ORANGE, theme::C_NEON_RED, theme::C_NEON_PURPLE};
  return kColors[rc::aqiLevelIndex(eaqi)];
}

void drawClock(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const lv_font_t* f = theme::fontNum();
  int32_t y = c.y1 + (lv_area_get_height(&c) - lv_font_get_line_height(f)) / 2;
  if (!clk::synced()) {
    du::text(layer, f, C(theme::C_TEXT3), LV_TEXT_ALIGN_LEFT, c.x1, y, c.x2, "--:--");
    return;
  }
  rc::LocalTime lt = clk::local();
  char hh[4], mm[4];
  snprintf(hh, sizeof(hh), "%02u", static_cast<unsigned>(lt.hour));
  snprintf(mm, sizeof(mm), "%02u", static_cast<unsigned>(lt.minute));
  int32_t x = c.x1;
  int32_t wh = textW(f, hh);
  int32_t wc = textW(f, ":");
  du::text(layer, f, C(theme::C_NEON_GREEN), LV_TEXT_ALIGN_LEFT, x, y, x + wh + S(2), hh);
  x += wh;
  s_colonArea = {x, y, x + wc - 1, y + lv_font_get_line_height(f) - 1};
  if (s_colon) du::text(layer, f, C(theme::C_NEON_CYAN), LV_TEXT_ALIGN_LEFT, x, y, x + wc + S(2), ":");
  x += wc;
  du::text(layer, f, C(theme::C_WHITE), LV_TEXT_ALIGN_LEFT, x, y, c.x2, mm);
}

void drawWeather(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const ui::Data& d = ui::data();
  const lv_font_t* fs = theme::fontS();
  const lv_font_t* fh = WIDE ? theme::fontM() : fs;
  const lv_font_t* ft = theme::fontTemp();
  int32_t x = c.x1 + WX_PAD;
  int32_t y = c.y1 + NP_SHIFT;
  int32_t right = c.x1 + (COL_RIGHT - WX_X);
  if (clk::synced()) {
    rc::LocalTime lt = clk::local();
    char g[48];
    snprintf(g, sizeof(g), "%s%s%s", rc::greetingTr(lt.hour), d.settings.name[0] ? " " : "", d.settings.name);
    if (textW(fs, g) > right - x) snprintf(g, sizeof(g), "%s", rc::greetingTr(lt.hour));
    du::text(layer, fs, C(theme::C_NEON_CYAN), LV_TEXT_ALIGN_LEFT, x, y, right, g);
  }
  bool ok = d.fc.valid;
  uint8_t icon = ok ? rc::wmoIcon(d.fc.now.code, d.fc.now.isDay) : rc::IC_OVERCAST;
  char temp[24];
  lv_color_t tc = C(theme::C_TEXT3);
  if (ok && d.fc.now.temp10 != rc::NA16) {
    rc::formatTemp(temp, sizeof(temp), d.fc.now.temp10);
    tc = theme::tempColor(d.fc.now.temp10);
  } else {
    snprintf(temp, sizeof(temp), "--°");
  }
  char th[16] = "";
  char tl[16] = "";
  if (d.daily.valid && d.daily.days[0].tmax10 != rc::NA16 && d.daily.days[0].tmin10 != rc::NA16) {
    char hi[12], lo[12];
    rc::formatTemp(hi, sizeof(hi), d.daily.days[0].tmax10);
    rc::formatTemp(lo, sizeof(lo), d.daily.days[0].tmin10);
    snprintf(th, sizeof(th), "\xE2\x86\x91%s", hi);
    snprintf(tl, sizeof(tl), "\xE2\x86\x93%s", lo);
  }
  int32_t px = icons::pixels(WX_ICON);
  int32_t ix = x - px * 10 / 64;
  int32_t iy = y + WX_ICON_DY;
  if (icon == rc::IC_CLEAR_DAY || icon == rc::IC_CLEAR_NIGHT) {
    du::image(layer, icons::get(static_cast<rc::IconId>(icon), WX_ICON_CLEAR), ix + px * 9 / 64,
              iy - WX_ICON_LIFT + px * 8 / 64);
  } else {
    du::image(layer, icons::get(static_cast<rc::IconId>(icon), WX_ICON), ix, iy - WX_ICON_LIFT);
  }
  int32_t tx = ix + px + WX_TEMP_GAP;
  du::text(layer, ft, tc, LV_TEXT_ALIGN_LEFT, tx, iy + WX_TEMP_DY, right + S(4), temp);
  if (th[0]) {
    du::text(layer, fh, C(theme::C_NEON_ORANGE), LV_TEXT_ALIGN_LEFT, tx, iy + WX_HILO_DY, right + S(4), th);
    du::text(layer, fh, C(theme::C_NEON_CYAN), LV_TEXT_ALIGN_LEFT, tx + textW(fh, th) + S(6), iy + WX_HILO_DY,
             right + S(4), tl);
  }
}

struct DateText {
  const char* day;
  char date[24];
  int32_t dayW;
  int32_t dateW;
  int32_t w;
};

DateText dateText() {
  rc::LocalTime lt = clk::local();
  const lv_font_t* f = theme::fontL();
  DateText t;
  snprintf(t.date, sizeof(t.date), "%u %s", static_cast<unsigned>(lt.day), rc::monthNameTr(lt.month));
  t.dateW = textW(f, t.date);
  const int32_t room = DATE_W - S(2);
  t.day = rc::dayNameTr(lt.wday);
  t.dayW = textW(f, t.day);
  if (t.dayW + DAY_GAP + t.dateW > room) {
    t.day = rc::dayShortTr(lt.wday);
    t.dayW = textW(f, t.day);
  }
  if (t.dayW + DAY_GAP + t.dateW > room) {
    snprintf(t.date, sizeof(t.date), "%u %s", static_cast<unsigned>(lt.day), rc::monthShortTr(lt.month));
    t.dateW = textW(f, t.date);
  }
  if (t.dayW + DAY_GAP + t.dateW > room) {
    t.day = "";
    t.dayW = 0;
  }
  t.w = t.day[0] ? t.dayW + DAY_GAP + t.dateW : t.dateW;
  return t;
}

int32_t clockW() {
  const lv_font_t* f = theme::fontNum();
  return 2 * textW(f, "00") + textW(f, ":");
}

void drawDate(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  if (!clk::synced()) {
    const char* wait = rc::L("Saat eşitleniyor", "Syncing clock");
    const lv_font_t* f = textW(theme::fontL(), wait) <= c.x2 - c.x1 - S(2) ? theme::fontL() : theme::fontM();
    int32_t y = c.y1 + (lv_area_get_height(&c) - lv_font_get_line_height(f)) / 2;
    du::text(layer, f, C(theme::C_TEXT3), LV_TEXT_ALIGN_LEFT, c.x1 + S(2), y, c.x2, wait);
    return;
  }
  DateText t = dateText();
  const lv_font_t* f = theme::fontL();
  int32_t lo = c.x1 + S(2);
  int32_t x = LV_CLAMP(lo, CLOCK_X + (clockW() - t.w) / 2, LV_MAX(lo, c.x2 + 1 - t.w));
  if (t.day[0]) {
    du::text(layer, f, C(theme::C_NEON_YELLOW), LV_TEXT_ALIGN_LEFT, x, c.y1, c.x2, t.day);
    x += t.dayW + DAY_GAP;
  }
  du::text(layer, f, C(theme::C_NEON_GREEN), LV_TEXT_ALIGN_LEFT, x, c.y1, c.x2, t.date);
}

int32_t npFrame() {
  int32_t period = RECORD_PERIOD_DEG * 10;
  return (s_npAngle % period) * RECORD_FRAMES / period;
}

int32_t npTextX() { return NP_X + NP_DISC + NP_TEXT_GAP; }

int32_t npTextDy() { return (NP_DISC - lv_font_get_line_height(theme::fontS())) / 2; }

bool npFits() { return s_npTitleW <= NP_RIGHT - npTextX() + 1; }

void npMeasure(const char* title) {
  const lv_font_t* f = theme::fontS();
  const char* shown = theme::printable(f, title, s_npTitle, sizeof(s_npTitle), rc::L("Bilinmeyen ad", "Unknown title"));
  if (shown != s_npTitle) snprintf(s_npTitle, sizeof(s_npTitle), "%s", shown);
  const char* s = s_npTitle;
  const int32_t cap = static_cast<int32_t>(sizeof(s_npPos)) / static_cast<int32_t>(sizeof(s_npPos[0])) - 1;
  uint32_t i = 0;
  int32_t x = 0;
  s_npChars = 0;
  while (s[i] && s_npChars < cap) {
    s_npByte[s_npChars] = static_cast<uint8_t>(i);
    s_npPos[s_npChars] = static_cast<int16_t>(x);
    uint32_t letter = lv_text_encoded_next(s, &i);
    x += lv_font_get_glyph_width(f, letter, lv_text_encoded_next(&s[i], nullptr));
    s_npChars++;
  }
  s_npByte[s_npChars] = static_cast<uint8_t>(i);
  s_npPos[s_npChars] = static_cast<int16_t>(x);
  s_npTitleW = x;
}

void npDrawSpan(lv_layer_t* layer, const lv_font_t* f, lv_color_t col, int32_t x0, int32_t ty) {
  const char* title = s_npTitle;
  int32_t lo = 0;
  while (lo < s_npChars && x0 + s_npPos[lo + 1] <= npTextX()) lo++;
  int32_t hi = lo;
  while (hi < s_npChars && x0 + s_npPos[hi] <= NP_RIGHT) hi++;
  if (hi <= lo) return;
  char buf[sizeof(media::State::title)];
  size_t n = static_cast<size_t>(s_npByte[hi] - s_npByte[lo]);
  memcpy(buf, title + s_npByte[lo], n);
  buf[n] = 0;
  du::text(layer, f, col, LV_TEXT_ALIGN_LEFT, x0 + s_npPos[lo], ty, x0 + s_npPos[hi], buf);
}

void drawNowPlaying(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  if (!s_npShown) {
    du::image(layer, RECORD_IMAGES[0], NP_X, c.y1, LV_OPA_40);
    du::text(layer, theme::fontS(), C(theme::C_TEXT3), LV_TEXT_ALIGN_LEFT, npTextX(), c.y1 + npTextDy(), NP_RIGHT,
             rc::L("Çalan yok", "Nothing playing"));
    return;
  }
  du::image(layer, RECORD_IMAGES[npFrame()], NP_X, c.y1);
  const char* title = s_npTitle;
  if (!title[0]) return;
  const lv_font_t* fm = theme::fontS();
  int32_t tx = npTextX();
  int32_t ty = c.y1 + npTextDy();
  int32_t ty2 = ty + lv_font_get_line_height(fm) - 1;
  lv_color_t col = C(s_npPlaying ? theme::C_WHITE : theme::C_TEXT2);
  du::Clip clip(layer, tx, ty, NP_RIGHT, ty2);
  if (clip.empty()) return;
  if (npFits()) {
    du::text(layer, fm, col, LV_TEXT_ALIGN_LEFT, tx, ty, NP_RIGHT, title);
    return;
  }
  int32_t x = tx - s_npOff;
  npDrawSpan(layer, fm, col, x, ty);
  x += s_npTitleW + NP_LOOP_GAP;
  if (x <= NP_RIGHT) npDrawSpan(layer, fm, col, x, ty);
  lv_color_t bg = C(theme::C_BG);
  du::hfade(layer, NP_RIGHT - NP_FADE + 1, ty, NP_RIGHT, ty2, bg, LV_OPA_TRANSP, LV_OPA_COVER);
  if (s_npOff > 0) du::hfade(layer, tx, ty, tx + NP_FADE - 1, ty2, bg, LV_OPA_COVER, LV_OPA_TRANSP);
}

void bar(lv_layer_t* layer, int32_t x1, int32_t y, int32_t x2, int32_t pct, lv_color_t a, lv_color_t b) {
  du::box(layer, x1, y, x2, y + S(7), C(theme::C_BAR_BG), LV_OPA_COVER, S(4));
  if (pct <= 0) return;
  if (pct > 100) pct = 100;
  int32_t w = (x2 - x1) * pct / 100;
  if (w < S(8)) w = S(8);
  du::hgrad(layer, x1, y, x1 + w, y + S(7), a, b, S(4));
}

void drawBars(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const ui::Data& d = ui::data();
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fs = theme::fontS();
  int32_t iy = c.y1 + S(4);
  int32_t by = c.y1 + S(9);
  int32_t ty = c.y1 + S(5);
  int32_t extra = lv_area_get_width(&c) - S(308);
  char buf[16];

  int32_t x = c.x1 + S(2);
  du::text(layer, fm, C(theme::C_NEON_ORANGE), LV_TEXT_ALIGN_LEFT, x, iy, x + S(18), RS_THERMO);
  int32_t pct = 0;
  lv_color_t lo = C(theme::C_NEON_BLUE);
  lv_color_t hi = C(theme::C_NEON_ORANGE);
  bool tv = d.fc.valid && d.fc.now.temp10 != rc::NA16;
  if (tv && d.daily.valid && d.daily.days[0].tmax10 > d.daily.days[0].tmin10 && d.daily.days[0].tmin10 != rc::NA16) {
    int32_t mn = d.daily.days[0].tmin10;
    int32_t mx = d.daily.days[0].tmax10;
    int32_t t = LV_CLAMP(mn, d.fc.now.temp10, mx);
    pct = (t - mn) * 100 / (mx - mn);
    if (pct < 6) pct = 6;
    lo = theme::tempColor(static_cast<int16_t>(mn));
    hi = theme::tempColor(d.fc.now.temp10);
  }
  bar(layer, x + S(20), by, x + S(70), pct, lo, hi);
  if (tv) rc::formatTemp(buf, sizeof(buf), d.fc.now.temp10);
  else snprintf(buf, sizeof(buf), "--");
  du::text(layer, fs, C(theme::C_WHITE), LV_TEXT_ALIGN_LEFT, x + S(75), ty, x + S(104), buf);

  x = c.x1 + S(108) + extra / 2;
  du::text(layer, fm, C(theme::C_NEON_CYAN), LV_TEXT_ALIGN_LEFT, x, iy, x + S(16), RS_DROP);
  bool hv = d.fc.valid && d.fc.now.rh != rc::NA8;
  bar(layer, x + S(18), by, x + S(66), hv ? d.fc.now.rh : 0, C(theme::C_NEON_BLUE), C(theme::C_NEON_CYAN));
  if (hv) snprintf(buf, sizeof(buf), "%%%u", static_cast<unsigned>(d.fc.now.rh));
  else snprintf(buf, sizeof(buf), "--");
  du::text(layer, fs, C(theme::C_WHITE), LV_TEXT_ALIGN_LEFT, x + S(71), ty, x + S(100), buf);

  x = c.x1 + S(212) + extra;
  bool av = d.air.valid && d.air.eaqi != rc::NAU16;
  lv_color_t ac = C(av ? aqiColor(d.air.eaqi) : theme::C_TEXT3);
  du::text(layer, fm, ac, LV_TEXT_ALIGN_LEFT, x, iy, x + S(16), RS_LEAF);
  int32_t ap = av ? (d.air.eaqi > 100 ? 100 : d.air.eaqi) : 0;
  if (av && ap < 8) ap = 8;
  bar(layer, x + S(18), by, x + S(62), ap, C(theme::C_NEON_GREEN), ac);
  if (av) snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(d.air.eaqi));
  else snprintf(buf, sizeof(buf), "--");
  du::text(layer, fs, ac, LV_TEXT_ALIGN_LEFT, x + S(67), ty, c.x2, buf);
}

void updateCity() {
  if (!s_city) return;
  char buf[48], fitted[48];
  snprintf(buf, sizeof(buf), RS_PIN " %s", ui::data().settings.city);
  setText(s_city, du::fit(theme::fontS(), buf, CITY_W, fitted, sizeof(fitted)));
}

void updateLight() {
  if (!s_bulb) return;
  const ui::Data& d = ui::data();
  const BulbState& b = d.bulb;
  bool paired = d.settings.bulbMac[0] != 0;
  bool on = paired && b.online && b.pilot.valid && b.pilot.on;
  uint32_t border = 0x3A4452;
  lv_color_t icon = C(theme::C_TEXT2);
  lv_color_t bg = C(theme::C_BG);
  if (!paired) {
    icon = C(theme::C_TEXT3);
  } else if (!b.online) {
    border = theme::C_NEON_RED;
    icon = C(theme::C_TEXT3);
  } else if (on) {
    rc::Rgb c = rc::pilotColor(b.pilot);
    icon = lv_color_mix(lv_color_make(c.r, c.g, c.b), C(theme::C_NEON_YELLOW), 128);
    border = theme::C_NEON_YELLOW;
    bg = C(0x2A2200);
  }
  lv_obj_set_style_border_color(s_bulb, C(border), 0);
  lv_obj_set_style_bg_color(s_bulb, bg, 0);
  lv_obj_set_style_text_color(s_bulbIcon, icon, 0);
}

bool musicPlaying() {
  const media::State& m = media::state();
  return m.link && m.active && m.playing;
}

void updateMusic() {
  bool on = musicPlaying();
  uint32_t accent = media::state().accent;
  rumy::setMusic(s_face, on, C(theme::C_NEON_PINK), C(accent ? accent : theme::C_NEON_YELLOW));
  uint32_t period = 0;
  uint32_t beat = 0;
  if (on && media::tempo(period, beat)) rumy::setTempo(s_face, period, beat);
  else rumy::setTempo(s_face, 0, 0);
}

uint32_t textHash(const char* s) {
  uint32_t h = 2166136261u;
  while (*s) h = (h ^ static_cast<uint8_t>(*s++)) * 16777619u;
  return h;
}

void npRestart() {
  s_npOff = 0;
  s_npScrolling = false;
  s_npPhaseAt = lv_tick_get();
}

void npDiscArea(lv_area_t& a) {
  lv_area_t c;
  lv_obj_get_coords(s_np, &c);
  a = {NP_X, c.y1, NP_X + NP_DISC - 1, c.y1 + NP_DISC - 1};
}

void npTextArea(lv_area_t& a) {
  lv_area_t c;
  lv_obj_get_coords(s_np, &c);
  int32_t ty = c.y1 + npTextDy();
  a = {npTextX(), ty, NP_RIGHT, ty + lv_font_get_line_height(theme::fontS()) - 1};
}

void npTick(lv_timer_t* t) {
  (void)t;
  if (!s_np) return;
  uint32_t now = lv_tick_get();
  uint32_t dt = now - s_npLastTick;
  s_npLastTick = now;
  if (dt > 100) dt = 100;
  int32_t frame = npFrame();
  s_npAngle = (s_npAngle + static_cast<int32_t>(dt) * NP_DECIDEG_PER_MS) % 3600;
  lv_area_t a;
  if (npFrame() != frame) {
    npDiscArea(a);
    lv_obj_invalidate_area(s_np, &a);
  }
  if (npFits()) return;
  if (!s_npScrolling) {
    if (now - s_npPhaseAt < NP_HOLD_MS) return;
    s_npScrolling = true;
    s_npPhaseAt = now;
  }
  int32_t off = static_cast<int32_t>((now - s_npPhaseAt) * static_cast<uint32_t>(NP_SPEED) / 1000U);
  if (off >= s_npTitleW + NP_LOOP_GAP) {
    off = 0;
    s_npScrolling = false;
    s_npPhaseAt = now;
  }
  if (off == s_npOff) return;
  s_npOff = off;
  npTextArea(a);
  lv_obj_invalidate_area(s_np, &a);
}

void npTimerSync() {
  if (!s_npTimer) return;
  bool run = s_visible && s_npShown && s_npPlaying;
  if (run == s_npRunning) return;
  s_npRunning = run;
  if (run) {
    s_npLastTick = lv_tick_get();
    npRestart();
    lv_timer_resume(s_npTimer);
  } else {
    lv_timer_pause(s_npTimer);
  }
}

void updateNowPlaying() {
  if (!s_np) return;
  const media::State& m = media::state();
  bool playing = m.link && m.active && m.playing;
  if (playing) s_npPausedAt = 0;
  else if (!s_npPausedAt) s_npPausedAt = lv_tick_get() | 1u;
  bool shown = m.link && m.active && m.title[0] &&
               (playing || lv_tick_get() - s_npPausedAt < cfg::MEDIA_PAUSED_HOME_MS);
  bool dirty = false;
  uint32_t h = textHash(m.title);
  if (h != s_npTitleHash) {
    s_npTitleHash = h;
    npMeasure(m.title);
    npRestart();
    dirty = true;
  }
  if (playing != s_npPlaying) {
    s_npPlaying = playing;
    npRestart();
    dirty = true;
  }
  if (shown != s_npShown) {
    s_npShown = shown;
    lv_obj_invalidate(s_np);
    npRestart();
  }
  if (dirty) lv_obj_invalidate(s_np);
  npTimerSync();
}

void npClick(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_MEDIA, -1);
}

void drawAgents(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const rc::Agents& a = agents::state();
  uint8_t top = rc::agentsTop(a);
  int32_t cy = c.y1 + AG_H / 2;
  agents_ui::drawBeacon(layer, c.x1 + AG_CX, cy, AG_R, top, lv_tick_get());
  uint8_t n = rc::agentsTopCount(a);
  if (n < 2) return;
  char buf[8];
  snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(n));
  const lv_font_t* f = theme::fontS();
  du::text(layer, f, C(agents_ui::color(top)), LV_TEXT_ALIGN_LEFT, c.x1 + AG_CX + AG_R + S(6),
           cy - lv_font_get_line_height(f) / 2, c.x2, buf);
}

void agTick(lv_timer_t* t) {
  (void)t;
  if (!s_ag) return;
  lv_area_t c;
  lv_obj_get_coords(s_ag, &c);
  int32_t g = AG_R + S(5);
  int32_t cy = c.y1 + AG_H / 2;
  lv_area_t a = {c.x1 + AG_CX - g, cy - g, c.x1 + AG_CX + g, cy + g};
  lv_obj_invalidate_area(s_ag, &a);
}

void agTimerSync() {
  if (!s_agTimer) return;
  bool run = s_visible && agents_ui::animated(s_agTop);
  if (run == s_agRunning) return;
  s_agRunning = run;
  if (run) lv_timer_resume(s_agTimer);
  else lv_timer_pause(s_agTimer);
}

void updateAgents() {
  if (!s_ag) return;
  s_agTop = rc::agentsTop(agents::state());
  if (s_agTop == rc::AG_NONE) lv_obj_add_flag(s_ag, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_remove_flag(s_ag, LV_OBJ_FLAG_HIDDEN);
  lv_obj_invalidate(s_ag);
  agTimerSync();
}

void agClick(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_AGENTS, -1);
}

bool idleSleep() {
  uint8_t m = ui::data().settings.eyeSleepMin;
  if (!m || !s_visible) return false;
  return lv_display_get_inactive_time(nullptr) >= static_cast<uint32_t>(m) * 60000UL;
}

uint32_t s_lookSeq = 0;
uint32_t s_lookBase = 0;
uint8_t s_forceKind = 0xFF;
uint32_t s_forceUntil = 0;

bool forcedWeather(rc::WxKind& k) {
  if (s_forceKind == 0xFF) return false;
  if (static_cast<int32_t>(s_forceUntil - lv_tick_get()) <= 0) {
    s_forceKind = 0xFF;
    return false;
  }
  k = static_cast<rc::WxKind>(s_forceKind);
  return true;
}

rumy::Mood pickMood(uint8_t& acc, rumy::Effect& fx) {
  const ui::Data& d = ui::data();
  acc = rc::ACC_NONE;
  fx = rumy::FX_NONE;
  if (ui::night() && !ui::nightAwake()) return rumy::ASLEEP;
  if (ui::quakeSurprise()) return rumy::SURPRISED;
  if (musicPlaying()) {
    s_idleAsleep = false;
    return rumy::MUSIC;
  }
  uint8_t react;
  if (match_ui::reaction(react) || agents_ui::reaction(react)) {
    s_idleAsleep = false;
    return static_cast<rumy::Mood>(react);
  }
  if (idleSleep()) {
    s_idleAsleep = true;
    return rumy::ASLEEP;
  }
  if (s_idleAsleep) {
    s_idleAsleep = false;
    s_wakeUntil = lv_tick_get() + WAKE_MS;
  }
  if (static_cast<int32_t>(s_wakeUntil - lv_tick_get()) > 0) return rumy::SLEEPY;
  if (!d.net.wifi) return rumy::SAD;
  if (clk::synced() && d.events.todayCount > 0 && d.events.items[0].dayNum == clk::todayDayNum()) {
    const rc::EventOcc& e = d.events.items[0];
    acc = e.accessory;
    return e.kind == rc::EV_COMMEMORATION ? rumy::CALM : rumy::CELEBRATE;
  }
  rc::WxKind k = d.fc.valid ? rc::wxKind(d.fc.now) : rc::WXK_NONE;
  bool forced = forcedWeather(k);
  if (!forced && !rc::wxKindWet(k) && ui::rainSoon()) {
    acc = rc::ACC_UMBRELLA;
    return rumy::LOOKUP;
  }
  rumy::Look look = rumy::weatherLook(k);
  acc = look.acc;
  fx = look.fx;
  return look.mood;
}

void updateMood() {
  if (!s_face) return;
  uint8_t acc;
  rumy::Effect fx;
  rumy::Mood m = pickMood(acc, fx);
  uint32_t autoSig = static_cast<uint32_t>(m) | (static_cast<uint32_t>(acc) << 8) | (static_cast<uint32_t>(fx) << 16);
  look::Pick req;
  uint32_t seq;
  bool mine = look::request(req, seq);
  if (mine && seq != s_lookSeq) {
    s_lookSeq = seq;
    s_lookBase = autoSig;
  } else if (mine && autoSig != s_lookBase) {
    look::drop(seq);
    mine = false;
  }
  if (mine) {
    m = static_cast<rumy::Mood>(req.mood);
    acc = req.acc;
    fx = static_cast<rumy::Effect>(req.fx);
  }
  look::publish({static_cast<uint8_t>(m), acc, static_cast<uint8_t>(fx), static_cast<uint8_t>(mine ? 1 : 0)});
  rumy::setMood(s_face, m);
  rumy::setAccessory(s_face, acc);
  rumy::setEffect(s_face, fx);
  const BulbState& b = ui::data().bulb;
  lv_color_t eye = C(theme::C_EYE);
  if (b.online && b.pilot.valid && b.pilot.on && m != rumy::ASLEEP) {
    rc::Rgb c = rc::pilotColor(b.pilot);
    eye = lv_color_mix(lv_color_make(c.r, c.g, c.b), eye, 40);
  }
  rumy::setColor(s_face, eye);
  updateMusic();
  rumy::setAnimated(s_face, s_visible);
}

void weatherClick(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_WEATHER, 1);
}

void lightClick(lv_event_t* e) {
  (void)e;
  if (!ui::data().settings.bulbMac[0]) {
    ui::go(ui::SCR_PAIR, 1);
    return;
  }
  wiz::togglePower();
}

void lightLong(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_LIGHT, 1);
}

void drawLive(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const rc::Match& m = ui::data().match;
  const lv_font_t* f = theme::fontM();
  int32_t y = c.y1 + (lv_area_get_height(&c) - lv_font_get_line_height(f)) / 2;
  match_ui::drawLeague(layer, m.league, c.x1 + S(8), c.y1, lv_area_get_height(&c), C(theme::team()));
  if (!matchLive(ui::data())) return;
  bool homeOurs = m.usHome == 1;
  char score[16];
  snprintf(score, sizeof(score), "%u \xE2\x80\x93 %u", static_cast<unsigned>(m.homeScore),
           static_cast<unsigned>(m.awayScore));
  int32_t x = c.x1 + S(30);
  du::text(layer, f, C(homeOurs ? theme::team() : theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x, y, c.x2, m.home.abbr);
  x += textW(f, m.home.abbr) + S(8);
  du::text(layer, f, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, x, y, c.x2, score);
  x += textW(f, score) + S(8);
  du::text(layer, f, C(homeOurs ? theme::C_TEXT2 : theme::team()), LV_TEXT_ALIGN_LEFT, x, y, c.x2, m.away.abbr);
  int32_t rx = c.x2 - S(8);
  if (m.phase == rc::MP_HALF) {
    du::text(layer, f, C(theme::C_TEXT2), LV_TEXT_ALIGN_RIGHT, c.x1, y, rx, rc::L("İY", "HT"));
    return;
  }
  du::text(layer, f, C(theme::C_TEXT), LV_TEXT_ALIGN_RIGHT, c.x1, y, rx, m.clock);
  int32_t dx = rx - textW(f, m.clock) - S(14);
  int32_t dy = c.y1 + lv_area_get_height(&c) / 2;
  du::box(layer, dx, dy - S(4), dx + S(7), dy + S(3), C(theme::C_NEON_RED), LV_OPA_COVER, LV_RADIUS_CIRCLE);
}

void cardClick(lv_event_t* e) {
  (void)e;
  if (s_cardIdx == CARD_MATCH && cardAvailable(CARD_MATCH, ui::data())) {
    ui::go(ui::SCR_MATCH, -1);
    return;
  }
  if (s_cardIdx == CARD_PRINTER && cardAvailable(CARD_PRINTER, ui::data())) {
    ui::go(ui::SCR_PRINTER, -1);
    return;
  }
  showCard(true);
}

void deleted(lv_event_t* e) {
  (void)e;
  if (s_cardTimer) lv_timer_delete(s_cardTimer);
  if (s_npTimer) lv_timer_delete(s_npTimer);
  if (s_agTimer) lv_timer_delete(s_agTimer);
  s_cardTimer = nullptr;
  s_npTimer = nullptr;
  s_agTimer = nullptr;
  s_ag = nullptr;
  s_scr = s_face = s_city = s_bulb = s_bulbIcon = s_clock = s_wx = s_date = s_bars = nullptr;
  s_card = s_cardIcon = s_cardText = s_cardLive = s_np = nullptr;
}

lv_obj_t* drawObj(int32_t x, int32_t y, int32_t w, int32_t h, lv_event_cb_t cb) {
  lv_obj_t* o = theme::makeBox(s_scr);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(o, cb, LV_EVENT_DRAW_MAIN, nullptr);
  return o;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  ui::attachGestures(s_scr);
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);

  s_face = rumy::create(s_scr, theme::SW, FACE_H, FACE_EYE);
  lv_obj_set_pos(s_face, 0, 0);

  s_city = theme::makeLabel(s_scr, theme::fontS(), theme::C_NEON_GREEN);
  lv_obj_set_pos(s_city, S(8), S(8));
  lv_obj_set_width(s_city, CITY_W);
  theme::oneLine(s_city);

  s_bulb = theme::makeBox(s_scr);
  lv_obj_set_size(s_bulb, S(42), S(42));
  lv_obj_set_pos(s_bulb, R(272), S(4));
  lv_obj_set_style_radius(s_bulb, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(s_bulb, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(s_bulb, C(theme::C_BG), 0);
  lv_obj_set_style_border_width(s_bulb, S(2), 0);
  lv_obj_add_style(s_bulb, theme::stylePressed(), LV_STATE_PRESSED);
  lv_obj_add_flag(s_bulb, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_ext_click_area(s_bulb, S(6));
  lv_obj_add_event_cb(s_bulb, lightClick, LV_EVENT_SHORT_CLICKED, nullptr);
  lv_obj_add_event_cb(s_bulb, lightLong, LV_EVENT_LONG_PRESSED, nullptr);
  s_bulbIcon = theme::makeLabel(s_bulb, theme::fontL(), theme::C_TEXT2);
  lv_label_set_text_static(s_bulbIcon, RS_BULB);
  lv_obj_center(s_bulbIcon);

  s_clock = drawObj(CLOCK_X, CLOCK_Y, CLOCK_W, CLOCK_H, drawClock);
  s_wx = drawObj(WX_X, COL_Y, theme::SW - WX_X, WX_H, drawWeather);
  lv_obj_add_flag(s_wx, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(s_wx, weatherClick, LV_EVENT_CLICKED, nullptr);
  s_date = drawObj(ROW_X, DATE_Y, DATE_W, DATE_H, drawDate);
  s_np = drawObj(NP_X, COL_Y, NP_RIGHT - NP_X + 1, NP_DISC, drawNowPlaying);
  lv_obj_add_flag(s_np, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(s_np, npClick, LV_EVENT_CLICKED, nullptr);
  s_npShown = s_npPlaying = s_npRunning = false;
  s_npTitleHash = 0;
  s_npTimer = lv_timer_create(npTick, NP_FRAME_MS, nullptr);
  lv_timer_pause(s_npTimer);
  s_bars = drawObj(ROW_X, BARS_Y, ROW_W, BARS_H, drawBars);
  s_ag = drawObj(AG_X, AG_Y, AG_W, AG_H, drawAgents);
  lv_obj_add_flag(s_ag, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_ext_click_area(s_ag, S(6));
  lv_obj_add_event_cb(s_ag, agClick, LV_EVENT_CLICKED, nullptr);
  s_agRunning = false;
  s_agTimer = lv_timer_create(agTick, cfg::AGENT_FRAME_MS, nullptr);
  lv_timer_pause(s_agTimer);

  s_card = theme::makeBox(s_scr);
  lv_obj_set_pos(s_card, ROW_X, CARD_Y);
  lv_obj_set_size(s_card, ROW_W, CARD_H);
  lv_obj_set_style_radius(s_card, S(8), 0);
  lv_obj_set_style_bg_opa(s_card, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(s_card, C(theme::C_BG), 0);
  lv_obj_set_style_border_width(s_card, 1, 0);
  lv_obj_set_style_border_color(s_card, C(theme::C_NEON_CYAN), 0);
  lv_obj_add_flag(s_card, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(s_card, cardClick, LV_EVENT_CLICKED, nullptr);
  s_cardIcon = theme::makeLabel(s_card, theme::fontM(), theme::C_NEON_CYAN);
  lv_obj_set_pos(s_cardIcon, S(8), S(3));
  s_cardText = theme::makeLabel(s_card, theme::fontM(), theme::C_WHITE);
  lv_obj_set_pos(s_cardText, S(30), S(3));
  s_cardLive = lv_obj_create(s_card);
  lv_obj_remove_style_all(s_cardLive);
  lv_obj_set_size(s_cardLive, lv_pct(100), lv_pct(100));
  lv_obj_remove_flag(s_cardLive, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_flag(s_cardLive, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_event_cb(s_cardLive, drawLive, LV_EVENT_DRAW_MAIN, nullptr);
  lv_obj_set_width(s_cardText, CARD_TEXT_W);
  theme::oneLine(s_cardText);

  s_cardTimer = lv_timer_create(cardTimerCb, static_cast<uint32_t>(ui::data().settings.cardSeconds) * 1000U, nullptr);
  updateCity();
  updateLight();
  showCard(false);
  updateMood();
  updateNowPlaying();
  updateAgents();
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr) return;
  if (mask & (ui::M_MINUTE | ui::M_SETTINGS | ui::M_NET)) {
    lv_obj_invalidate(s_clock);
    lv_obj_invalidate(s_date);
  }
  if ((mask & ui::M_SECOND) && mask != ui::M_ALL && clk::synced()) {
    s_colon = !s_colon;
    if (s_colonArea.x2 >= s_colonArea.x1) lv_obj_invalidate_area(s_clock, &s_colonArea);
  }
  if (mask & (ui::M_FORECAST | ui::M_DAILY | ui::M_MINUTE | ui::M_SETTINGS | ui::M_NET)) lv_obj_invalidate(s_wx);
  if (mask & (ui::M_FORECAST | ui::M_DAILY | ui::M_AIR)) lv_obj_invalidate(s_bars);
  if (mask & ui::M_SETTINGS) updateCity();
  if (mask & (ui::M_BULB | ui::M_SETTINGS)) updateLight();
  if (mask & ui::M_SETTINGS) {
    if (s_cardTimer) lv_timer_set_period(s_cardTimer, static_cast<uint32_t>(ui::data().settings.cardSeconds) * 1000U);
  }
  if (mask & (ui::M_QUAKE | ui::M_NET | ui::M_EVENTS | ui::M_FORECAST | ui::M_AIR | ui::M_MARINE | ui::M_PRINTER |
              ui::M_MATCH)) {
    if (mask != ui::M_ALL) showCard(false);
  }
  if (mask & (ui::M_FORECAST | ui::M_BULB | ui::M_NET | ui::M_EVENTS | ui::M_NIGHT | ui::M_QUAKE | ui::M_MINUTE |
              ui::M_SETTINGS | ui::M_SECOND | ui::M_MEDIA))
    updateMood();
  if (mask & (ui::M_MEDIA | ui::M_SECOND | ui::M_MINUTE | ui::M_SETTINGS)) updateNowPlaying();
  if (mask & ui::M_AGENTS) updateAgents();
}

void setVisible(bool visible) {
  s_visible = visible;
  updateMood();
  npTimerSync();
  agTimerSync();
}

void onTouch() {
  if (s_idleAsleep) updateMood();
}

void glance() {
  if (!s_face || !s_visible) return;
  lv_area_t c;
  lv_obj_get_coords(s_ag, &c);
  rumy::poke(s_face, c.x1 + AG_CX, c.y1 + AG_H / 2);
}

void forceWeather(uint8_t kind, uint32_t ms) {
  s_forceKind = kind < rc::WXK_COUNT ? kind : 0xFF;
  s_forceUntil = lv_tick_get() + ms;
  updateMood();
}

}
