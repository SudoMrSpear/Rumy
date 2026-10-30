#include "text_tr.h"

#include <stdio.h>
#include <stdlib.h>

namespace rc {

namespace {

const char* const kDays[7] = {"Pazar", "Pazartesi", "Salı", "Çarşamba", "Perşembe", "Cuma", "Cumartesi"};
const char* const kDaysShort[7] = {"Paz", "Pzt", "Sal", "Çar", "Per", "Cum", "Cmt"};
const char* const kMonths[12] = {"Ocak",   "Şubat", "Mart",  "Nisan",   "Mayıs", "Haziran",
                                 "Temmuz", "Ağustos", "Eylül", "Ekim", "Kasım", "Aralık"};
const char* const kMonthsShort[12] = {"Oca", "Şub", "Mar", "Nis", "May", "Haz", "Tem", "Ağu", "Eyl", "Eki", "Kas", "Ara"};
const char* const kDaysEn[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
const char* const kDaysShortEn[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char* const kMonthsEn[12] = {"January", "February", "March",     "April",   "May",      "June",
                                   "July",    "August",   "September", "October", "November", "December"};
const char* const kMonthsShortEn[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

uint8_t s_lang = LANG_TR;

size_t fin(int n, size_t cap) {
  if (n < 0) return 0;
  return static_cast<size_t>(n) < cap ? static_cast<size_t>(n) : (cap ? cap - 1 : 0);
}

}

void setLang(uint8_t lang) { s_lang = lang == LANG_EN ? LANG_EN : LANG_TR; }
uint8_t lang() { return s_lang; }

const char* dayNameTr(uint8_t wday) { return (langEn() ? kDaysEn : kDays)[wday % 7]; }
const char* dayShortTr(uint8_t wday) { return (langEn() ? kDaysShortEn : kDaysShort)[wday % 7]; }
const char* monthNameTr(uint8_t month) {
  return (month >= 1 && month <= 12) ? (langEn() ? kMonthsEn : kMonths)[month - 1] : "";
}
const char* monthShortTr(uint8_t month) {
  return (month >= 1 && month <= 12) ? (langEn() ? kMonthsShortEn : kMonthsShort)[month - 1] : "";
}

size_t formatDateTr(char* out, size_t cap, uint8_t wday, uint8_t mday, uint8_t month) {
  if (langEn())
    return fin(snprintf(out, cap, "%s, %s %u", dayNameTr(wday), monthNameTr(month), static_cast<unsigned>(mday)), cap);
  return fin(snprintf(out, cap, "%s, %u %s", dayNameTr(wday), static_cast<unsigned>(mday), monthNameTr(month)), cap);
}

const char* greetingTr(uint8_t hour) {
  if (hour >= 5 && hour < 11) return L("Günaydın", "Good morning");
  if (hour >= 11 && hour < 17) return L("İyi günler", "Good afternoon");
  if (hour >= 17 && hour < 22) return L("İyi akşamlar", "Good evening");
  return L("İyi geceler", "Good night");
}

int16_t roundTenths(int16_t v10) {
  if (v10 == NA16) return NA16;
  return static_cast<int16_t>(v10 >= 0 ? (v10 + 5) / 10 : -((-v10 + 5) / 10));
}

size_t formatTemp(char* out, size_t cap, int16_t temp10) {
  if (temp10 == NA16) return fin(snprintf(out, cap, "--°"), cap);
  return fin(snprintf(out, cap, "%d°", static_cast<int>(roundTenths(temp10))), cap);
}

size_t formatDecimalTr(char* out, size_t cap, int32_t value10) {
  int32_t a = value10 < 0 ? -value10 : value10;
  return fin(snprintf(out, cap, langEn() ? "%s%ld.%ld" : "%s%ld,%ld", value10 < 0 ? "-" : "", static_cast<long>(a / 10), static_cast<long>(a % 10)),
             cap);
}

size_t formatHm(char* out, size_t cap, uint8_t hour, uint8_t minute) {
  return fin(snprintf(out, cap, "%02u:%02u", static_cast<unsigned>(hour), static_cast<unsigned>(minute)), cap);
}

size_t rainSummaryTr(const WxMinutely& m, bool snow, char* out, size_t cap) {
  const char* what = snow ? L("Kar", "Snow") : L("Yağmur", "Rain");
  int n = m.count > MINUTELY ? MINUTELY : m.count;
  if (n == 0) return fin(snprintf(out, cap, "%s", L("Kısa vadeli yağış verisi yok", "No short-term rain data")), cap);
  bool wet[MINUTELY] = {};
  bool any = false;
  for (int i = 0; i < n; i++) {
    wet[i] = m.precip100[i] != NAU16 && m.precip100[i] >= 10;
    any = any || wet[i];
  }
  if (!any) return fin(snprintf(out, cap, "%s", L("Önümüzdeki 1 saat yağış beklenmiyor", "No rain expected in the next hour")), cap);
  if (wet[0]) {
    for (int i = 1; i < n; i++) {
      if (!wet[i]) return fin(snprintf(out, cap, L("%s %d dk içinde dinecek", "%s stopping in %d min"), what, i * 15), cap);
    }
    return fin(snprintf(out, cap, L("%s en az 1 saat sürecek", "%s for at least 1 hour"), what), cap);
  }
  for (int i = 1; i < n; i++) {
    if (wet[i]) return fin(snprintf(out, cap, L("%s %d dk içinde başlayacak", "%s starting in %d min"), what, i * 15), cap);
  }
  return fin(snprintf(out, cap, "%s", L("Önümüzdeki 1 saat yağış beklenmiyor", "No rain expected in the next hour")), cap);
}

const char* uvLevelTr(uint8_t uv10) {
  if (uv10 == NA8) return "-";
  int uv = (uv10 + 5) / 10;
  if (uv <= 2) return L("Düşük", "Low");
  if (uv <= 5) return L("Orta", "Moderate");
  if (uv <= 7) return L("Yüksek", "High");
  if (uv <= 10) return L("Çok yüksek", "Very high");
  return L("Aşırı", "Extreme");
}

uint8_t aqiLevelIndex(uint16_t eaqi) {
  if (eaqi == NAU16) return 0;
  if (eaqi <= 20) return 0;
  if (eaqi <= 40) return 1;
  if (eaqi <= 60) return 2;
  if (eaqi <= 80) return 3;
  if (eaqi <= 100) return 4;
  return 5;
}

const char* aqiLevelTr(uint16_t eaqi) {
  static const char* const kLevels[6] = {"İyi", "Makul", "Orta", "Kötü", "Çok kötü", "Aşırı kötü"};
  static const char* const kLevelsEn[6] = {"Good", "Fair", "Moderate", "Poor", "Very poor", "Extremely poor"};
  if (eaqi == NAU16) return "-";
  return (langEn() ? kLevelsEn : kLevels)[aqiLevelIndex(eaqi)];
}

size_t agoTr(uint32_t s, char* out, size_t cap) {
  if (s < 60) return fin(snprintf(out, cap, "%s", L("şimdi", "now")), cap);
  if (s < 3600) return fin(snprintf(out, cap, L("%lu dk önce", "%lu min ago"), static_cast<unsigned long>(s / 60)), cap);
  if (s < 86400) return fin(snprintf(out, cap, L("%lu sa önce", "%lu h ago"), static_cast<unsigned long>(s / 3600)), cap);
  return fin(snprintf(out, cap, L("%lu gün önce", "%lu d ago"), static_cast<unsigned long>(s / 86400)), cap);
}

size_t durationTr(uint32_t s, char* out, size_t cap) {
  uint32_t h = s / 3600;
  uint32_t m = (s % 3600) / 60;
  if (h == 0) return fin(snprintf(out, cap, L("%lu dk", "%lu min"), static_cast<unsigned long>(m)), cap);
  if (m == 0) return fin(snprintf(out, cap, L("%lu sa", "%lu h"), static_cast<unsigned long>(h)), cap);
  return fin(snprintf(out, cap, L("%lu sa %lu dk", "%lu h %lu min"), static_cast<unsigned long>(h), static_cast<unsigned long>(m)), cap);
}

}
