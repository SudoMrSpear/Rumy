#include <stdio.h>
#include <string.h>

#include "app/config.h"
#include "clock/clock.h"
#include "draw_util.h"
#include "fonts/symbols.h"
#include "icons.h"
#include "screens.h"
#include "text_tr.h"
#include "theme.h"
#include "timeutil.h"
#include "ui.h"
#include "wind_tr.h"
#include "wmo_tr.h"

namespace scr_weather {

namespace {

using theme::FW;
using theme::R;
using theme::S;

constexpr int32_t CARD_W = FW(308);
constexpr int32_t HOUR_W = S(46);
constexpr int32_t HOUR_H = S(88);
constexpr int32_t DAY_ROW = S(30);
constexpr int32_t TILE_GAP = S(6);
constexpr int32_t TILE_W = (CARD_W - TILE_GAP) / 2;
constexpr int32_t TILE_H = S(64);
constexpr int TILE_COUNT = 10;
constexpr int MAX_ITEMS = rc::HOURS + 4;

enum ItemKind : uint8_t {
  IT_NOW = 0,
  IT_HOUR,
  IT_SUNRISE,
  IT_SUNSET,
};

struct HourItem {
  uint32_t t;
  int16_t temp10;
  uint8_t icon;
  uint8_t pop;
  uint8_t kind;
  uint8_t reserved[3];
};

struct Tile {
  char head[28];
  char value[20];
  char sub[36];
  uint32_t subColor;
};

struct State {
  HourItem items[MAX_ITEMS];
  Tile tiles[TILE_COUNT];
  uint8_t count;
  uint8_t iconId;
  uint8_t sky;
  uint8_t rainVisible;
};

State* s_st = nullptr;
lv_obj_t* s_scr = nullptr;
lv_obj_t* s_city = nullptr;
lv_obj_t* s_icon = nullptr;
lv_obj_t* s_temp = nullptr;
lv_obj_t* s_cond = nullptr;
lv_obj_t* s_hilo = nullptr;
lv_obj_t* s_rainCard = nullptr;
lv_obj_t* s_rainText = nullptr;
lv_obj_t* s_rainChart = nullptr;
lv_obj_t* s_hourScroll = nullptr;
lv_obj_t* s_hourStrip = nullptr;
lv_obj_t* s_days = nullptr;
lv_obj_t* s_grid = nullptr;
lv_obj_t* s_footer = nullptr;

using theme::setText;

lv_color_t C(uint32_t hex) { return theme::col(hex); }

bool rainWorthShowing(const rc::Forecast& f) {
  if (!f.valid) return false;
  for (int i = 0; i < f.m15.count && i < rc::MINUTELY; i++) {
    if (f.m15.precip100[i] != rc::NAU16 && f.m15.precip100[i] >= 10) return true;
  }
  for (int i = 0; i < 2 && i < f.hourCount; i++) {
    if (f.hours[i].pop != rc::NA8 && f.hours[i].pop >= 40) return true;
  }
  return false;
}

void buildItems() {
  State& st = *s_st;
  st.count = 0;
  const ui::Data& d = ui::data();
  const rc::Forecast& f = d.fc;
  if (!f.valid || f.hourCount == 0) return;
  uint32_t nowT = clk::synced() ? clk::nowUtc() : f.now.time;
  HourItem& first = st.items[st.count++];
  memset(&first, 0, sizeof(first));
  first.t = f.hourStart;
  first.temp10 = f.now.temp10 != rc::NA16 ? f.now.temp10 : f.hours[0].temp10;
  first.icon = rc::wmoIcon(f.now.code != rc::NA8 ? f.now.code : f.hours[0].code,
                           f.now.code != rc::NA8 ? f.now.isDay : f.hours[0].isDay);
  first.pop = f.hours[0].pop;
  first.kind = IT_NOW;

  uint32_t sun[2 * rc::DAYS];
  uint8_t sunKind[2 * rc::DAYS];
  int sunN = 0;
  if (d.daily.valid) {
    for (int i = 0; i < d.daily.count && i < rc::DAYS; i++) {
      if (d.daily.days[i].sunrise) {
        sun[sunN] = d.daily.days[i].sunrise;
        sunKind[sunN++] = IT_SUNRISE;
      }
      if (d.daily.days[i].sunset) {
        sun[sunN] = d.daily.days[i].sunset;
        sunKind[sunN++] = IT_SUNSET;
      }
    }
  }

  for (int h = 0; h < f.hourCount && st.count < MAX_ITEMS; h++) {
    uint32_t t = f.hourStart + static_cast<uint32_t>(h) * 3600U;
    if (h > 0) {
      HourItem& it = st.items[st.count++];
      memset(&it, 0, sizeof(it));
      it.t = t;
      it.temp10 = f.hours[h].temp10;
      it.icon = rc::wmoIcon(f.hours[h].code, f.hours[h].isDay);
      it.pop = f.hours[h].pop;
      it.kind = IT_HOUR;
    }
    for (int k = 0; k < sunN && st.count < MAX_ITEMS; k++) {
      if (sun[k] >= t && sun[k] < t + 3600U && sun[k] > nowT) {
        HourItem& it = st.items[st.count++];
        memset(&it, 0, sizeof(it));
        it.t = sun[k];
        it.kind = sunKind[k];
        it.icon = sunKind[k] == IT_SUNRISE ? rc::IC_SUNRISE : rc::IC_SUNSET;
        it.pop = rc::NA8;
        it.temp10 = rc::NA16;
      }
    }
  }

  if (s_hourStrip) {
    int32_t w = st.count ? st.count * HOUR_W : CARD_W - S(12);
    lv_obj_set_width(s_hourStrip, w);
    lv_obj_invalidate(s_hourStrip);
  }
}

void setTile(Tile& t, const char* sym, const char* title) {
  snprintf(t.head, sizeof(t.head), "%s  %s", sym, title);
  snprintf(t.value, sizeof(t.value), "--");
  snprintf(t.sub, sizeof(t.sub), "%s", rc::L("Veri yok", "No data"));
  t.subColor = theme::C_TEXT2;
}

void buildTiles() {
  const ui::Data& d = ui::data();
  const rc::WxNow& n = d.fc.now;
  bool fv = d.fc.valid;
  Tile* t = s_st->tiles;
  char a[12];
  char b[12];

  setTile(t[0], RS_THERMO, rc::L("Hissedilen", "Feels like"));
  if (fv && n.feels10 != rc::NA16) {
    rc::formatTemp(t[0].value, sizeof(t[0].value), n.feels10);
    int diff = n.temp10 != rc::NA16 ? rc::roundTenths(n.feels10) - rc::roundTenths(n.temp10) : 0;
    snprintf(t[0].sub, sizeof(t[0].sub), "%s",
             diff <= -2 ? rc::L("Olduğundan serin", "Cooler than actual") : (diff >= 2 ? rc::L("Olduğundan sıcak", "Warmer than actual") : rc::L("Gerçeğe yakın", "Close to actual")));
  }

  setTile(t[1], RS_DROP, rc::L("Nem", "Humidity"));
  if (fv && n.rh != rc::NA8) {
    snprintf(t[1].value, sizeof(t[1].value), "%%%u", static_cast<unsigned>(n.rh));
    snprintf(t[1].sub, sizeof(t[1].sub), "%s", n.rh >= 75 ? rc::L("Nemli", "Humid") : (n.rh <= 30 ? rc::L("Kuru", "Dry") : rc::L("Rahat", "Comfortable")));
  }

  setTile(t[2], RS_WIND, rc::L("Rüzgar", "Wind"));
  if (fv && n.wind10 != rc::NAU16) {
    uint16_t dir = n.windDir == rc::NAU16 ? 0 : n.windDir;
    snprintf(t[2].value, sizeof(t[2].value), rc::L("%u km/sa", "%u km/h"), static_cast<unsigned>((n.wind10 + 5) / 10));
    if (n.gust10 != rc::NAU16) {
      snprintf(t[2].sub, sizeof(t[2].sub), rc::L("%s · hamle %u", "%s · gusts %u"), rc::windNameTr(dir), static_cast<unsigned>((n.gust10 + 5) / 10));
    } else {
      snprintf(t[2].sub, sizeof(t[2].sub), "%s (%s)", rc::windNameTr(dir), rc::windAbbrTr(dir));
    }
  }

  setTile(t[3], RS_GAUGE, rc::L("Basınç", "Pressure"));
  if (fv && n.press10 != rc::NAU16) {
    snprintf(t[3].value, sizeof(t[3].value), "%u hPa", static_cast<unsigned>((n.press10 + 5) / 10));
    uint16_t p = static_cast<uint16_t>((n.press10 + 5) / 10);
    snprintf(t[3].sub, sizeof(t[3].sub), "%s", p < 1005 ? rc::L("Alçak basınç", "Low pressure") : (p > 1022 ? rc::L("Yüksek basınç", "High pressure") : "Normal"));
  }

  setTile(t[4], RS_SUN, rc::L("UV indeksi", "UV index"));
  uint8_t uv = fv ? n.uv10 : rc::NA8;
  if (uv == rc::NA8 && d.daily.valid) uv = d.daily.days[0].uvMax10;
  if (uv != rc::NA8) {
    snprintf(t[4].value, sizeof(t[4].value), "%u", static_cast<unsigned>((uv + 5) / 10));
    snprintf(t[4].sub, sizeof(t[4].sub), "%s", rc::uvLevelTr(uv));
    if (uv >= 60) t[4].subColor = theme::C_WARN;
  }

  setTile(t[5], RS_EYE, rc::L("Görüş", "Visibility"));
  if (fv && n.visDam != rc::NAU16) {
    if (n.visDam >= 1000) {
      snprintf(t[5].value, sizeof(t[5].value), "%u km", static_cast<unsigned>((n.visDam + 50) / 100));
    } else if (n.visDam < 100) {
      snprintf(t[5].value, sizeof(t[5].value), "%u m", static_cast<unsigned>(n.visDam) * 10U);
    } else {
      rc::formatDecimalTr(a, sizeof(a), (n.visDam + 5) / 10);
      snprintf(t[5].value, sizeof(t[5].value), "%s km", a);
    }
    snprintf(t[5].sub, sizeof(t[5].sub), "%s", n.visDam >= 1000 ? rc::L("Açık", "Clear") : (n.visDam >= 400 ? rc::L("Orta", "Moderate") : rc::L("Düşük", "Poor")));
  }

  setTile(t[6], RS_SUN, rc::L("Gün doğumu", "Sunrise"));
  if (d.daily.valid && d.daily.days[0].sunrise) {
    rc::LocalTime r = rc::toLocal(d.daily.days[0].sunrise);
    rc::LocalTime s = rc::toLocal(d.daily.days[0].sunset);
    rc::formatHm(t[6].value, sizeof(t[6].value), r.hour, r.minute);
    rc::formatHm(b, sizeof(b), s.hour, s.minute);
    snprintf(t[6].sub, sizeof(t[6].sub), rc::L("Gün batımı %s", "Sunset %s"), b);
  }

  setTile(t[7], RS_UMBRELLA, rc::L("Yağış", "Precipitation"));
  if (fv && n.precip100 != rc::NAU16) {
    rc::formatDecimalTr(a, sizeof(a), (n.precip100 + 5) / 10);
    snprintf(t[7].value, sizeof(t[7].value), "%s mm", a);
    if (d.daily.valid && d.daily.days[0].popMax != rc::NA8) {
      snprintf(t[7].sub, sizeof(t[7].sub), rc::L("Bugün ihtimal %%%u", "Today's chance %u%%"), static_cast<unsigned>(d.daily.days[0].popMax));
    } else {
      snprintf(t[7].sub, sizeof(t[7].sub), "%s", rc::L("Son 15 dakika", "Last 15 minutes"));
    }
  }

  setTile(t[8], RS_LEAF, rc::L("Hava kalitesi", "Air quality"));
  if (d.air.valid && d.air.eaqi != rc::NAU16) {
    snprintf(t[8].value, sizeof(t[8].value), "%u", static_cast<unsigned>(d.air.eaqi));
    snprintf(t[8].sub, sizeof(t[8].sub), "%s", rc::aqiLevelTr(d.air.eaqi));
    if (rc::aqiLevelIndex(d.air.eaqi) >= 3) t[8].subColor = theme::C_WARN;
  }

  setTile(t[9], RS_WATER, rc::L("Deniz suyu", "Sea water"));
  if (d.sea.valid && d.sea.sst10 != rc::NA16) {
    rc::formatDecimalTr(a, sizeof(a), d.sea.sst10);
    snprintf(t[9].value, sizeof(t[9].value), "%s°", a);
    snprintf(t[9].sub, sizeof(t[9].sub), "%s", d.sea.sst10 >= 200 ? rc::L("Yüzmeye uygun", "Good for swimming") : rc::L("Serin", "Cool"));
  }
  if (s_grid) lv_obj_invalidate(s_grid);
}

void updateHeader() {
  const ui::Data& d = ui::data();
  char buf[64];
  snprintf(buf, sizeof(buf), RS_PIN "  %s", d.settings.city);
  setText(s_city, buf);
  if (!d.fc.valid) {
    setText(s_temp, "--°");
    setText(s_cond, rc::L("Veri bekleniyor", "Waiting for data"));
    setText(s_hilo, "");
  } else {
    rc::formatTemp(buf, sizeof(buf), d.fc.now.temp10);
    setText(s_temp, buf);
    setText(s_cond, rc::wmoTextTr(d.fc.now.code));
    char hi[12], lo[12], fl[12];
    if (d.daily.valid) {
      rc::formatTemp(hi, sizeof(hi), d.daily.days[0].tmax10);
      rc::formatTemp(lo, sizeof(lo), d.daily.days[0].tmin10);
      rc::formatTemp(fl, sizeof(fl), d.fc.now.feels10);
      snprintf(buf, sizeof(buf), rc::L("\xE2\x86\x91%s  \xE2\x86\x93%s  ·  Hissedilen %s", "\xE2\x86\x91%s  \xE2\x86\x93%s  ·  Feels like %s"), hi, lo, fl);
    } else {
      rc::formatTemp(fl, sizeof(fl), d.fc.now.feels10);
      snprintf(buf, sizeof(buf), rc::L("Hissedilen %s", "Feels like %s"), fl);
    }
    setText(s_hilo, buf);
  }
  uint8_t icon = d.fc.valid ? rc::wmoIcon(d.fc.now.code, d.fc.now.isDay) : rc::IC_OVERCAST;
  if (icon != s_st->iconId) {
    s_st->iconId = icon;
    lv_image_set_src(s_icon, icons::get(static_cast<rc::IconId>(icon), icons::S64));
  }
  uint8_t sky = d.fc.valid ? rc::wmoSky(d.fc.now.code, d.fc.now.isDay) : rc::SKY_CLOUDY_DAY;
  if (sky != s_st->sky) {
    s_st->sky = sky;
    theme::SkyGradient g = theme::sky(static_cast<rc::SkyClass>(sky));
    lv_obj_set_style_bg_color(s_scr, C(g.top), 0);
    lv_obj_set_style_bg_grad_color(s_scr, C(g.bottom), 0);
  }
}

void updateRain() {
  const ui::Data& d = ui::data();
  bool show = rainWorthShowing(d.fc);
  if (show != (s_st->rainVisible != 0)) {
    s_st->rainVisible = show ? 1 : 0;
    if (show) lv_obj_remove_flag(s_rainCard, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(s_rainCard, LV_OBJ_FLAG_HIDDEN);
  }
  if (!show) return;
  char buf[64];
  rc::rainSummaryTr(d.fc.m15, rc::wmoIsSnow(d.fc.now.code), buf, sizeof(buf));
  setText(s_rainText, buf);
  lv_obj_invalidate(s_rainChart);
}

void updateFooter() {
  const ui::Data& d = ui::data();
  char buf[80];
  uint32_t t = d.fc.fetchedUtc;
  if (!t) {
    setText(s_footer, rc::L("Son güncelleme: henüz yok · Open-Meteo", "Last update: none yet · Open-Meteo"));
    return;
  }
  rc::LocalTime lt = rc::toLocal(t);
  char hm[8];
  rc::formatHm(hm, sizeof(hm), lt.hour, lt.minute);
  bool stale = clk::synced() && clk::nowUtc() > t && clk::nowUtc() - t > 3600;
  snprintf(buf, sizeof(buf), rc::L("Son güncelleme %s%s · Open-Meteo", "Last update %s%s · Open-Meteo"), hm, stale ? rc::L(" (eski)", " (stale)") : "");
  setText(s_footer, buf);
  lv_obj_set_style_text_color(s_footer, C(stale ? theme::C_WARN : theme::C_TEXT), 0);
}

void drawRain(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const rc::WxMinutely& m = ui::data().fc.m15;
  static const char* const kLbl[rc::MINUTELY] = {rc::L("Şimdi", "Now"), rc::L("+15 dk", "+15 min"), rc::L("+30 dk", "+30 min"), rc::L("+45 dk", "+45 min")};
  int32_t w = lv_area_get_width(&c);
  int32_t slot = w / rc::MINUTELY;
  int32_t base = c.y2 - S(16);
  int32_t top = c.y1 + S(2);
  int32_t maxH = base - top;
  du::box(layer, c.x1, base, c.x2, base, C(theme::C_TEXT), LV_OPA_20, 0);
  for (int i = 0; i < rc::MINUTELY; i++) {
    int32_t x = c.x1 + i * slot;
    uint16_t v = (i < m.count) ? m.precip100[i] : rc::NAU16;
    if (v != rc::NAU16 && v > 0) {
      int32_t capped = v > 150 ? 150 : v;
      int32_t h = static_cast<int32_t>(lv_sqrt32(static_cast<uint32_t>(capped * 10000 / 150)) * maxH / 100);
      if (h < S(4)) h = S(4);
      du::box(layer, x + S(10), base - h, x + slot - S(10), base - 1, C(theme::C_BLUE), v >= 10 ? LV_OPA_COVER : LV_OPA_50,
              S(4));
    }
    du::text(layer, theme::fontS(), C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, x, base + S(2), x + slot - 1, kLbl[i]);
  }
}

void drawItem(lv_layer_t* layer, const HourItem& it, int32_t x, int32_t y) {
  char buf[16];
  rc::LocalTime lt = rc::toLocal(it.t);
  if (it.kind == IT_NOW) snprintf(buf, sizeof(buf), "%s", rc::L("Şimdi", "Now"));
  else if (it.kind == IT_HOUR) snprintf(buf, sizeof(buf), "%02u", static_cast<unsigned>(lt.hour));
  else rc::formatHm(buf, sizeof(buf), lt.hour, lt.minute);
  du::text(layer, theme::fontS(), C(it.kind == IT_NOW ? theme::C_TEXT : theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, x,
           y + S(4), x + HOUR_W - 1, buf);
  du::image(layer, icons::get(static_cast<rc::IconId>(it.icon), icons::S28), x + (HOUR_W - icons::pixels(icons::S28)) / 2,
            y + S(22));
  if (it.kind == IT_NOW || it.kind == IT_HOUR) {
    if (it.pop != rc::NA8 && it.pop >= 20) {
      snprintf(buf, sizeof(buf), "%%%u", static_cast<unsigned>(it.pop));
      du::text(layer, theme::fontS(), C(theme::C_BLUE), LV_TEXT_ALIGN_CENTER, x, y + S(51), x + HOUR_W - 1, buf);
    }
    rc::formatTemp(buf, sizeof(buf), it.temp10);
    du::text(layer, theme::fontM(), C(theme::C_TEXT), LV_TEXT_ALIGN_CENTER, x, y + S(66), x + HOUR_W - 1, buf);
  } else {
    du::text(layer, theme::fontS(), C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, x, y + S(68), x + HOUR_W - 1,
             it.kind == IT_SUNRISE ? rc::L("Doğum", "Rise") : rc::L("Batım", "Set"));
  }
}

void drawStrip(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  if (!s_st || s_st->count == 0) {
    du::text(layer, theme::fontM(), C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, c.y1 + S(34), c.x2,
             rc::L("Saatlik veri bekleniyor", "Waiting for hourly data"));
    return;
  }
  int32_t sx = lv_obj_get_scroll_x(s_hourScroll);
  int32_t vw = lv_obj_get_width(s_hourScroll);
  int first = static_cast<int>(sx / HOUR_W) - 1;
  int last = static_cast<int>((sx + vw) / HOUR_W) + 1;
  if (first < 0) first = 0;
  if (last >= s_st->count) last = s_st->count - 1;
  for (int i = first; i <= last; i++) drawItem(layer, s_st->items[i], c.x1 + i * HOUR_W, c.y1);
}

void drawDays(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  const ui::Data& d = ui::data();
  if (!d.daily.valid || d.daily.count == 0) {
    du::text(layer, theme::fontM(), C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, c.y1 + S(40), c.x2,
             rc::L("Günlük veri bekleniyor", "Waiting for daily data"));
    return;
  }
  int16_t lo = 32767, hi = -32767;
  for (int i = 0; i < d.daily.count && i < rc::DAYS; i++) {
    if (d.daily.days[i].tmin10 != rc::NA16 && d.daily.days[i].tmin10 < lo) lo = d.daily.days[i].tmin10;
    if (d.daily.days[i].tmax10 != rc::NA16 && d.daily.days[i].tmax10 > hi) hi = d.daily.days[i].tmax10;
  }
  int32_t today = clk::synced() ? clk::todayDayNum() : rc::localDayNum(d.daily.days[0].date);
  int32_t bx1 = c.x1 + S(170);
  int32_t bx2 = c.x2 - S(57);
  char buf[16];
  for (int i = 0; i < d.daily.count && i < rc::DAYS; i++) {
    const rc::WxDay& w = d.daily.days[i];
    int32_t y = c.y1 + i * DAY_ROW;
    if (!du::visible(layer, y, y + DAY_ROW)) continue;
    if (i > 0) du::box(layer, c.x1 + S(10), y, c.x2 - S(10), y, C(theme::C_TEXT), LV_OPA_10, 0);
    int32_t dn = rc::localDayNum(w.date);
    const char* name = dn == today ? rc::L("Bugün", "Today") : rc::dayShortTr(rc::weekdayFromDays(dn));
    du::text(layer, theme::fontM(), C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, c.x1 + S(12), y + S(6), c.x1 + S(66), name);
    du::image(layer, icons::get(rc::wmoIcon(w.code, true), icons::S24), c.x1 + S(68), y + S(3));
    if (w.popMax != rc::NA8 && w.popMax >= 20) {
      snprintf(buf, sizeof(buf), "%%%u", static_cast<unsigned>(w.popMax));
      du::text(layer, theme::fontS(), C(theme::C_BLUE), LV_TEXT_ALIGN_LEFT, c.x1 + S(96), y + S(8), c.x1 + S(130), buf);
    }
    rc::formatTemp(buf, sizeof(buf), w.tmin10);
    du::text(layer, theme::fontM(), C(theme::C_TEXT2), LV_TEXT_ALIGN_RIGHT, c.x1 + S(126), y + S(6), c.x1 + S(164), buf);
    rc::formatTemp(buf, sizeof(buf), w.tmax10);
    du::text(layer, theme::fontM(), C(theme::C_TEXT), LV_TEXT_ALIGN_RIGHT, bx2 + S(4), y + S(6), c.x2 - S(12), buf);
    du::box(layer, bx1, y + S(13), bx2, y + S(18), lv_color_black(), LV_OPA_30, S(3));
    if (hi > lo && w.tmin10 != rc::NA16 && w.tmax10 != rc::NA16) {
      int32_t span = hi - lo;
      int32_t x1 = bx1 + (w.tmin10 - lo) * (bx2 - bx1) / span;
      int32_t x2 = bx1 + (w.tmax10 - lo) * (bx2 - bx1) / span;
      if (x2 - x1 < S(6)) x2 = x1 + S(6);
      if (x2 > bx2) {
        x1 -= x2 - bx2;
        x2 = bx2;
      }
      du::hgrad(layer, x1, y + S(13), x2, y + S(18), theme::tempColor(w.tmin10), theme::tempColor(w.tmax10), S(3));
      if (dn == today && d.fc.valid && d.fc.now.temp10 != rc::NA16) {
        int32_t t = d.fc.now.temp10;
        if (t < lo) t = lo;
        if (t > hi) t = hi;
        int32_t cx = bx1 + (t - lo) * (bx2 - bx1) / span;
        du::box(layer, cx - S(5), y + S(11), cx + S(5), y + S(21), lv_color_black(), LV_OPA_60, LV_RADIUS_CIRCLE);
        du::box(layer, cx - S(3), y + S(13), cx + S(3), y + S(19), C(theme::C_TEXT), LV_OPA_COVER, LV_RADIUS_CIRCLE);
      }
    }
  }
}

void drawGrid(lv_event_t* e) {
  lv_obj_t* obj = lv_event_get_current_target_obj(e);
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  lv_obj_get_coords(obj, &c);
  if (!s_st) return;
  for (int i = 0; i < TILE_COUNT; i++) {
    const Tile& t = s_st->tiles[i];
    int32_t x = c.x1 + (i % 2) * (TILE_W + TILE_GAP);
    int32_t y = c.y1 + (i / 2) * (TILE_H + TILE_GAP);
    if (!du::visible(layer, y, y + TILE_H)) continue;
    du::box(layer, x, y, x + TILE_W - 1, y + TILE_H - 1, lv_color_black(), LV_OPA_30, theme::RADIUS_M);
    du::Clip clip(layer, x + S(2), y, x + TILE_W - S(4), y + TILE_H - 1);
    if (clip.empty()) continue;
    du::text(layer, theme::fontS(), C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x + S(10), y + S(6), x + TILE_W - S(8), t.head);
    du::text(layer, theme::fontL(), C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, x + S(10), y + S(21), x + TILE_W - S(8), t.value);
    du::text(layer, theme::fontS(), C(t.subColor), LV_TEXT_ALIGN_LEFT, x + S(10), y + S(46), x + TILE_W - S(8), t.sub);
  }
}

void stripScrolled(lv_event_t* e) {
  (void)e;
  if (s_hourStrip) lv_obj_invalidate(s_hourStrip);
}

lv_obj_t* card(lv_obj_t* parent, const char* title, int32_t h) {
  lv_obj_t* c = theme::makeBox(parent);
  lv_obj_add_style(c, theme::styleCardGlass(), 0);
  lv_obj_set_size(c, CARD_W, h);
  lv_obj_t* l = theme::makeLabel(c, theme::fontS(), theme::C_TEXT2);
  lv_label_set_text_static(l, title);
  lv_obj_set_pos(l, S(12), S(7));
  return c;
}

lv_obj_t* drawObj(lv_obj_t* parent, int32_t x, int32_t y, int32_t w, int32_t h, lv_event_cb_t cb) {
  lv_obj_t* o = theme::makeBox(parent);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(o, cb, LV_EVENT_DRAW_MAIN, nullptr);
  return o;
}

void deleted(lv_event_t* e) {
  (void)e;
  if (s_st) lv_free(s_st);
  s_st = nullptr;
  s_scr = s_city = s_icon = s_temp = s_cond = s_hilo = nullptr;
  s_rainCard = s_rainText = s_rainChart = s_hourScroll = s_hourStrip = s_days = s_grid = s_footer = nullptr;
}

void backClick(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_HOME, -1);
}

}

lv_obj_t* create() {
  s_st = static_cast<State*>(lv_malloc_zeroed(sizeof(State)));
  if (!s_st) return nullptr;
  s_st->iconId = 0xFF;
  s_st->sky = 0xFF;
  s_st->rainVisible = 1;

  s_scr = theme::makeScreen();
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  ui::attachGestures(s_scr);
  lv_obj_set_style_bg_grad_dir(s_scr, LV_GRAD_DIR_VER, 0);
  lv_obj_add_flag(s_scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(s_scr, LV_DIR_VER);
  theme::styleScrollbar(s_scr);
  lv_obj_set_flex_flow(s_scr, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(s_scr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_top(s_scr, 0, 0);
  lv_obj_set_style_pad_bottom(s_scr, S(10), 0);
  lv_obj_set_style_pad_row(s_scr, S(6), 0);

  lv_obj_t* head = theme::makeBox(s_scr);
  lv_obj_set_size(head, theme::SW, S(120));
  s_city = theme::makeLabel(head, theme::fontM(), theme::C_TEXT);
  lv_obj_set_pos(s_city, S(12), S(8));
  lv_obj_set_width(s_city, FW(250));
  theme::oneLine(s_city);
  lv_obj_t* back = theme::makeButton(head, RS_HOME, theme::fontM());
  lv_obj_set_size(back, S(40), S(32));
  lv_obj_set_pos(back, R(274), S(4));
  lv_obj_set_style_bg_opa(back, LV_OPA_30, 0);
  lv_obj_set_style_bg_color(back, lv_color_black(), 0);
  lv_obj_set_style_border_width(back, 0, 0);
  lv_obj_add_event_cb(back, backClick, LV_EVENT_CLICKED, nullptr);
  s_temp = theme::makeLabel(head, theme::fontNum(), theme::C_TEXT);
  lv_obj_set_pos(s_temp, S(12), S(34));
  s_icon = lv_image_create(head);
  lv_obj_set_size(s_icon, S(64), S(64));
  lv_obj_set_pos(s_icon, R(238), S(36));
  s_cond = theme::makeLabel(head, theme::fontM(), theme::C_TEXT);
  lv_obj_set_pos(s_cond, S(14), S(80));
  lv_obj_set_width(s_cond, FW(220));
  theme::oneLine(s_cond);
  s_hilo = theme::makeLabel(head, theme::fontS(), theme::C_TEXT2);
  lv_obj_set_pos(s_hilo, S(14), S(101));
  lv_obj_set_width(s_hilo, FW(296));
  theme::oneLine(s_hilo);

  s_rainCard = card(s_scr, rc::L(RS_UMBRELLA "  Sonraki 1 saat", RS_UMBRELLA "  Next hour"), S(104));
  s_rainText = theme::makeLabel(s_rainCard, theme::fontM(), theme::C_TEXT);
  lv_obj_set_pos(s_rainText, S(12), S(26));
  lv_obj_set_width(s_rainText, CARD_W - S(24));
  theme::oneLine(s_rainText);
  s_rainChart = drawObj(s_rainCard, S(8), S(48), CARD_W - S(16), S(52), drawRain);

  lv_obj_t* hours = card(s_scr, rc::L(RS_CLOCK "  Saatlik tahmin", RS_CLOCK "  Hourly forecast"), HOUR_H + S(30));
  s_hourScroll = theme::makeBox(hours);
  lv_obj_set_pos(s_hourScroll, S(6), S(26));
  lv_obj_set_size(s_hourScroll, CARD_W - S(12), HOUR_H);
  lv_obj_add_flag(s_hourScroll, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(s_hourScroll, LV_DIR_HOR);
  lv_obj_set_scrollbar_mode(s_hourScroll, LV_SCROLLBAR_MODE_OFF);
  lv_obj_remove_flag(s_hourScroll, LV_OBJ_FLAG_GESTURE_BUBBLE);
  lv_obj_remove_flag(s_hourScroll, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
  lv_obj_add_event_cb(s_hourScroll, stripScrolled, LV_EVENT_SCROLL, nullptr);
  s_hourStrip = drawObj(s_hourScroll, 0, 0, CARD_W - S(12), HOUR_H, drawStrip);

  lv_obj_t* days = card(s_scr, rc::L(RS_CALENDAR "  7 günlük tahmin", RS_CALENDAR "  7-day forecast"), S(30) + DAY_ROW * rc::DAYS);
  s_days = drawObj(days, 0, S(26), CARD_W, DAY_ROW * rc::DAYS, drawDays);

  s_grid = drawObj(s_scr, 0, 0, CARD_W, 5 * TILE_H + 4 * TILE_GAP, drawGrid);

  s_footer = theme::makeLabel(s_scr, theme::fontS(), theme::C_TEXT);
  lv_obj_set_style_text_opa(s_footer, LV_OPA_80, 0);
  lv_obj_set_width(s_footer, CARD_W);
  lv_obj_set_style_text_align(s_footer, LV_TEXT_ALIGN_CENTER, 0);
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr || !s_st) return;
  if (mask & (ui::M_FORECAST | ui::M_DAILY | ui::M_SETTINGS)) updateHeader();
  if (mask & ui::M_FORECAST) updateRain();
  if (mask & (ui::M_FORECAST | ui::M_DAILY)) {
    buildItems();
    if (s_days) lv_obj_invalidate(s_days);
  }
  if (mask & (ui::M_FORECAST | ui::M_DAILY | ui::M_AIR | ui::M_MARINE)) buildTiles();
  if (mask & (ui::M_FORECAST | ui::M_MINUTE | ui::M_NET)) updateFooter();
  if ((mask & ui::M_MINUTE) && mask != ui::M_ALL) {
    buildItems();
    if (s_days) lv_obj_invalidate(s_days);
  }
}

bool benchStrip() {
  if (!s_hourScroll) return false;
  lv_obj_scroll_by(s_hourScroll, -HOUR_W * 4, 0, LV_ANIM_ON);
  return true;
}

void enduranceStep() {
  if (!s_scr) return;
  int32_t bottom = lv_obj_get_scroll_bottom(s_scr);
  if (bottom <= 0) lv_obj_scroll_to_y(s_scr, 0, LV_ANIM_ON);
  else lv_obj_scroll_by(s_scr, 0, -S(80), LV_ANIM_ON);
  if (s_hourScroll) {
    int32_t right = lv_obj_get_scroll_right(s_hourScroll);
    if (right <= 0) lv_obj_scroll_to_x(s_hourScroll, 0, LV_ANIM_OFF);
    else lv_obj_scroll_by(s_hourScroll, -HOUR_W * 3, 0, LV_ANIM_ON);
  }
}

}
