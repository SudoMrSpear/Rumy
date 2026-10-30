#include "wmo_tr.h"

#include "text_tr.h"

namespace rc {

const char* wmoTextTr(uint8_t code) {
  switch (code) {
    case 0: return L("Açık", "Clear");
    case 1: return L("Çoğunlukla açık", "Mostly clear");
    case 2: return L("Parçalı bulutlu", "Partly cloudy");
    case 3: return L("Kapalı", "Overcast");
    case 45: return L("Sisli", "Fog");
    case 48: return L("Kırağılı sis", "Rime fog");
    case 51: return L("Hafif çisenti", "Light drizzle");
    case 53: return L("Çisenti", "Drizzle");
    case 55: return L("Yoğun çisenti", "Heavy drizzle");
    case 56: return L("Hafif donan çisenti", "Light freezing drizzle");
    case 57: return L("Donan çisenti", "Freezing drizzle");
    case 61: return L("Hafif yağmur", "Light rain");
    case 63: return L("Yağmurlu", "Rain");
    case 65: return L("Şiddetli yağmur", "Heavy rain");
    case 66: return L("Hafif donan yağmur", "Light freezing rain");
    case 67: return L("Donan yağmur", "Freezing rain");
    case 71: return L("Hafif kar", "Light snow");
    case 73: return L("Karlı", "Snow");
    case 75: return L("Yoğun kar", "Heavy snow");
    case 77: return L("Kar taneleri", "Snow grains");
    case 80: return L("Hafif sağanak", "Light showers");
    case 81: return L("Sağanak", "Showers");
    case 82: return L("Şiddetli sağanak", "Heavy showers");
    case 85: return L("Hafif kar sağanağı", "Light snow showers");
    case 86: return L("Kar sağanağı", "Snow showers");
    case 95: return L("Gök gürültülü fırtına", "Thunderstorm");
    case 96: return L("Dolulu fırtına", "Thunderstorm, hail");
    case 99: return L("Şiddetli dolulu fırtına", "Severe hailstorm");
    default: return L("Bilinmiyor", "Unknown");
  }
}

IconId wmoIcon(uint8_t code, bool isDay) {
  switch (code) {
    case 0: return isDay ? IC_CLEAR_DAY : IC_CLEAR_NIGHT;
    case 1:
    case 2: return isDay ? IC_PARTLY_DAY : IC_PARTLY_NIGHT;
    case 3: return IC_OVERCAST;
    case 45:
    case 48: return IC_FOG;
    case 51:
    case 53:
    case 55: return IC_DRIZZLE;
    case 56:
    case 57:
    case 66:
    case 67: return IC_SLEET;
    case 61:
    case 63:
    case 65: return IC_RAIN;
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86: return IC_SNOW;
    case 80:
    case 81:
    case 82: return isDay ? IC_SHOWERS_DAY : IC_SHOWERS_NIGHT;
    case 95: return IC_THUNDER;
    case 96:
    case 99: return IC_HAIL;
    default: return IC_OVERCAST;
  }
}

SkyClass wmoSky(uint8_t code, bool isDay) {
  if (code == 0 || code == 1) return isDay ? SKY_CLEAR_DAY : SKY_CLEAR_NIGHT;
  if (code == 2 || code == 3) return isDay ? SKY_CLOUDY_DAY : SKY_CLOUDY_NIGHT;
  if (code == 45 || code == 48) return SKY_FOG;
  if (code >= 95) return SKY_STORM;
  if (wmoIsSnow(code)) return SKY_SNOW;
  if (wmoIsWet(code)) return SKY_RAIN;
  return isDay ? SKY_CLOUDY_DAY : SKY_CLOUDY_NIGHT;
}

bool wmoIsWet(uint8_t code) { return (code >= 51 && code <= 67) || (code >= 71 && code <= 86) || code >= 95; }

bool wmoIsSnow(uint8_t code) { return (code >= 71 && code <= 77) || code == 85 || code == 86; }

const char* iconFileName(IconId id) {
  static const char* const kNames[IC_COUNT] = {
      "clear-day",   "clear-night", "partly-cloudy-day", "partly-cloudy-night", "overcast",   "fog",
      "drizzle",     "rain",        "sleet",             "snow",                "showers-day", "showers-night",
      "thunderstorm", "hail",       "sunrise",           "sunset",
  };
  if (id >= IC_COUNT) return "overcast";
  return kNames[id];
}

}
