#include "wind_tr.h"

#include "text_tr.h"

namespace rc {

namespace {

const char* const kNames[8] = {"Yıldız", "Poyraz", "Gündoğusu", "Keşişleme", "Kıble", "Lodos", "Günbatısı", "Karayel"};
const char* const kAbbr[8] = {"K", "KD", "D", "GD", "G", "GB", "B", "KB"};
const char* const kNamesEn[8] = {"North", "Northeast", "East", "Southeast", "South", "Southwest", "West", "Northwest"};
const char* const kAbbrEn[8] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};

}

uint8_t windSector8(uint16_t deg) {
  uint32_t d = deg % 360;
  return static_cast<uint8_t>(((d * 2 + 45) / 90) % 8);
}

const char* windNameTr(uint16_t deg) { return (langEn() ? kNamesEn : kNames)[windSector8(deg)]; }

const char* windAbbrTr(uint16_t deg) { return (langEn() ? kAbbrEn : kAbbr)[windSector8(deg)]; }

}
