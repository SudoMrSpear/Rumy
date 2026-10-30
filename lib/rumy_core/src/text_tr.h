#pragma once

#include <stddef.h>
#include <stdint.h>

#include "rc_types.h"

namespace rc {

enum Lang : uint8_t {
  LANG_TR = 0,
  LANG_EN = 1,
};

void setLang(uint8_t lang);
uint8_t lang();
inline bool langEn() { return lang() == LANG_EN; }
inline const char* L(const char* tr, const char* en) { return langEn() ? en : tr; }

const char* dayNameTr(uint8_t wday);
const char* dayShortTr(uint8_t wday);
const char* monthNameTr(uint8_t month);
const char* monthShortTr(uint8_t month);
size_t formatDateTr(char* out, size_t cap, uint8_t wday, uint8_t mday, uint8_t month);
const char* greetingTr(uint8_t hour);
size_t formatTemp(char* out, size_t cap, int16_t temp10);
size_t formatDecimalTr(char* out, size_t cap, int32_t value10);
size_t formatHm(char* out, size_t cap, uint8_t hour, uint8_t minute);
size_t rainSummaryTr(const WxMinutely& m, bool snow, char* out, size_t cap);
const char* uvLevelTr(uint8_t uv10);
const char* aqiLevelTr(uint16_t eaqi);
uint8_t aqiLevelIndex(uint16_t eaqi);
size_t agoTr(uint32_t seconds, char* out, size_t cap);
size_t durationTr(uint32_t seconds, char* out, size_t cap);
int16_t roundTenths(int16_t v10);

}
