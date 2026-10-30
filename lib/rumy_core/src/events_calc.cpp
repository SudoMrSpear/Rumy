#include "events_calc.h"

#include <stdio.h>
#include <string.h>

#include "afad.h"
#include "text_tr.h"
#include "timeutil.h"

namespace rc {

namespace {

struct FixedEvent {
  uint8_t month;
  uint8_t day;
  uint8_t kind;
  uint8_t accessory;
  uint16_t since;
  const char* name;
  const char* dative;
  const char* greeting;
};

const FixedEvent kFixed[] = {
    {1, 1, EV_NEWYEAR, ACC_PARTY, 0, "Yılbaşı", "Yılbaşına", "Mutlu yıllar!"},
    {4, 23, EV_NATIONAL, ACC_FLAG, 1920, "Ulusal Egemenlik ve Çocuk Bayramı", "23 Nisan'a",
     "23 Nisan Ulusal Egemenlik ve Çocuk Bayramı kutlu olsun!"},
    {5, 1, EV_LABOUR, ACC_NONE, 0, "Emek ve Dayanışma Günü", "1 Mayıs'a",
     "1 Mayıs Emek ve Dayanışma Günü kutlu olsun!"},
    {5, 19, EV_NATIONAL, ACC_FLAG, 1919, "Atatürk'ü Anma, Gençlik ve Spor Bayramı", "19 Mayıs'a",
     "19 Mayıs Atatürk'ü Anma, Gençlik ve Spor Bayramı kutlu olsun!"},
    {7, 15, EV_COMMEMORATION, ACC_FLAG, 2016, "Demokrasi ve Millî Birlik Günü", "15 Temmuz'a",
     "15 Temmuz şehitlerimizi rahmetle anıyoruz."},
    {8, 30, EV_NATIONAL, ACC_FLAG, 1922, "Zafer Bayramı", "Zafer Bayramı'na", "30 Ağustos Zafer Bayramı kutlu olsun!"},
    {10, 29, EV_NATIONAL, ACC_FLAG, 1923, "Cumhuriyet Bayramı", "Cumhuriyet Bayramı'na",
     "Cumhuriyet Bayramı kutlu olsun!"},
    {11, 10, EV_COMMEMORATION, ACC_FLAG, 1938, "Atatürk'ü Anma Günü", "10 Kasım'a",
     "Ulu Önder Atatürk'ü saygı ve özlemle anıyoruz."},
};

struct FixedEn {
  const char* name;
  const char* greeting;
};

const FixedEn kFixedEn[] = {
    {"New Year's Day", "Happy New Year!"},
    {"National Sovereignty and Children's Day", "Happy National Sovereignty and Children's Day!"},
    {"Labour and Solidarity Day", "Happy Labour Day!"},
    {"Atatürk Commemoration, Youth and Sports Day", "Happy Youth and Sports Day!"},
    {"Democracy and National Unity Day", "Remembering the martyrs of July 15."},
    {"Victory Day", "Happy Victory Day!"},
    {"Republic Day", "Happy Republic Day!"},
    {"Atatürk Memorial Day", "Remembering Atatürk with respect and longing."},
};

const char* fixedName(const FixedEvent& f) { return langEn() ? kFixedEn[&f - kFixed].name : f.name; }
const char* fixedGreeting(const FixedEvent& f) { return langEn() ? kFixedEn[&f - kFixed].greeting : f.greeting; }

const char* const kRamazanName = "Ramazan Bayramı";
const char* const kRamazanDative = "Ramazan Bayramı'na";
const char* const kRamazanGreeting = "Ramazan Bayramınız mübarek olsun!";
const char* const kKurbanName = "Kurban Bayramı";
const char* const kKurbanDative = "Kurban Bayramı'na";
const char* const kKurbanGreeting = "Kurban Bayramınız mübarek olsun!";

void copyText(char* dst, size_t cap, const char* src) { utf8Copy(dst, cap, src, strlen(src)); }

void insertSorted(EventOcc* out, int& n, int cap, const EventOcc& e) {
  int pos = n;
  while (pos > 0 && out[pos - 1].dayNum > e.dayNum) pos--;
  if (pos >= cap) return;
  int last = n < cap ? n : cap - 1;
  for (int i = last; i > pos; i--) out[i] = out[i - 1];
  out[pos] = e;
  if (n < cap) n++;
}

bool inWindow(int32_t day, int32_t today, int horizon) { return day >= today && day - today <= horizon; }

EventOcc makeOcc(int year, int month, int day, uint8_t kind, uint8_t acc, uint16_t ordinal, const char* name,
                 const char* dative) {
  EventOcc e{};
  e.dayNum = daysFromCivil(year, month, day);
  e.year = static_cast<uint16_t>(year);
  e.month = static_cast<uint8_t>(month);
  e.day = static_cast<uint8_t>(day);
  e.kind = kind;
  e.accessory = acc;
  e.ordinal = ordinal;
  copyText(e.name, sizeof(e.name), name);
  copyText(e.dative, sizeof(e.dative), dative ? dative : "");
  return e;
}

}

int collectEvents(const Religious* rel, int relCount, const PersonalEvent* personal, int personalCount,
                  int32_t today, int horizonDays, EventOcc* out, int cap) {
  int n = 0;
  CivilDate cd = civilFromDays(today);
  for (int y = cd.year; y <= cd.year + 1; y++) {
    for (const FixedEvent& f : kFixed) {
      int32_t d = daysFromCivil(y, f.month, f.day);
      if (!inWindow(d, today, horizonDays)) continue;
      uint16_t ord = f.since ? static_cast<uint16_t>(y - f.since) : 0;
      insertSorted(out, n, cap, makeOcc(y, f.month, f.day, f.kind, f.accessory, ord, fixedName(f), L(f.dative, "")));
    }
    for (int i = 0; i < relCount; i++) {
      const Religious& r = rel[i];
      if (!r.valid || r.year != y) continue;
      int32_t dr = daysFromCivil(y, r.ramazanMonth, r.ramazanDay);
      if (inWindow(dr, today, horizonDays)) {
        insertSorted(out, n, cap,
                     makeOcc(y, r.ramazanMonth, r.ramazanDay, EV_RELIGIOUS, ACC_NONE, 0, L(kRamazanName, "Eid al-Fitr"),
                             L(kRamazanDative, "")));
      }
      int32_t dk = daysFromCivil(y, r.kurbanMonth, r.kurbanDay);
      if (inWindow(dk, today, horizonDays)) {
        insertSorted(out, n, cap,
                     makeOcc(y, r.kurbanMonth, r.kurbanDay, EV_RELIGIOUS, ACC_NONE, 0, L(kKurbanName, "Eid al-Adha"),
                             L(kKurbanDative, "")));
      }
    }
    for (int i = 0; i < personalCount; i++) {
      const PersonalEvent& p = personal[i];
      if (p.month < 1 || p.month > 12 || p.day < 1) continue;
      int day = p.day;
      if (p.month == 2 && day == 29 && !isLeap(y)) day = 28;
      if (day > daysInMonth(y, p.month)) continue;
      int32_t d = daysFromCivil(y, p.month, day);
      if (!inWindow(d, today, horizonDays)) continue;
      uint16_t ord = (p.year > 0 && y > p.year) ? static_cast<uint16_t>(y - p.year) : 0;
      uint8_t acc = p.kind == EV_BIRTHDAY ? ACC_PARTY : ACC_NONE;
      insertSorted(out, n, cap, makeOcc(y, p.month, day, p.kind, acc, ord, p.name, nullptr));
    }
  }
  return n;
}

void buildSnapshot(const Religious* rel, int relCount, const PersonalEvent* personal, int personalCount, int32_t today,
                   EventsSnapshot& out) {
  memset(&out, 0, sizeof(out));
  out.computedDay = today;
  int n = collectEvents(rel, relCount, personal, personalCount, today, EVENT_HORIZON_DAYS, out.items, MAX_UPCOMING);
  out.count = static_cast<uint8_t>(n);
  for (int i = 0; i < n; i++) {
    if (out.items[i].dayNum == today) out.todayCount++;
  }
}

uint8_t countdownTier(int days) {
  if (days < 0) return TIER_NONE;
  if (days == 0) return TIER_TODAY;
  if (days <= 7) return TIER_FREQUENT;
  if (days <= EVENT_HORIZON_DAYS) return TIER_RARE;
  return TIER_NONE;
}

size_t eventCountdownText(const EventOcc& e, int32_t today, char* out, size_t cap) {
  int days = static_cast<int>(e.dayNum - today);
  int n;
  if (days == 0) {
    n = snprintf(out, cap, rc::L("Bugün: %s", "Today: %s"), e.name);
  } else if (days == 1) {
    n = snprintf(out, cap, rc::L("Yarın: %s", "Tomorrow: %s"), e.name);
  } else if (e.dative[0]) {
    n = snprintf(out, cap, "%s %d gün", e.dative, days);
  } else {
    n = snprintf(out, cap, rc::L("%s · %d gün", "%s · %d days"), e.name, days);
  }
  if (n < 0) {
    if (cap) out[0] = 0;
    return 0;
  }
  return static_cast<size_t>(n) < cap ? static_cast<size_t>(n) : cap - 1;
}

size_t eventGreeting(const EventOcc& e, char* out, size_t cap) {
  const char* fixed = nullptr;
  if (e.kind == EV_RELIGIOUS) {
    fixed = strcmp(e.name, L(kRamazanName, "Eid al-Fitr")) == 0 ? L(kRamazanGreeting, "Eid Mubarak!")
                                                                 : L(kKurbanGreeting, "Eid Mubarak!");
  } else if (e.kind <= EV_LABOUR) {
    for (const FixedEvent& f : kFixed) {
      if (f.month == e.month && f.day == e.day) fixed = fixedGreeting(f);
    }
  }
  int n;
  if (fixed) n = snprintf(out, cap, "%s", fixed);
  else if (e.kind == EV_BIRTHDAY) n = snprintf(out, cap, rc::L("Bugün %s!", "Today is %s!"), e.name);
  else n = snprintf(out, cap, rc::L("Bugün: %s", "Today: %s"), e.name);
  if (n < 0) {
    if (cap) out[0] = 0;
    return 0;
  }
  return static_cast<size_t>(n) < cap ? static_cast<size_t>(n) : cap - 1;
}

size_t eventSubtitle(const EventOcc& e, char* out, size_t cap) {
  const char* month = monthNameTr(e.month);
  int n;
  if (e.ordinal > 0 && e.kind == EV_BIRTHDAY) {
    n = snprintf(out, cap, rc::L("%u %s · %u. yaş", "%u %s · age %u"), static_cast<unsigned>(e.day), month, static_cast<unsigned>(e.ordinal));
  } else if (e.ordinal > 0 && e.kind == EV_COMMEMORATION) {
    n = snprintf(out, cap, rc::L("%u %s · %u. yıl dönümü", "%u %s · %u years"), static_cast<unsigned>(e.day), month,
                 static_cast<unsigned>(e.ordinal));
  } else if (e.ordinal > 0) {
    n = snprintf(out, cap, rc::L("%u %s · %u. yıl", "%u %s · year %u"), static_cast<unsigned>(e.day), month, static_cast<unsigned>(e.ordinal));
  } else {
    n = snprintf(out, cap, "%u %s", static_cast<unsigned>(e.day), month);
  }
  if (n < 0) {
    if (cap) out[0] = 0;
    return 0;
  }
  return static_cast<size_t>(n) < cap ? static_cast<size_t>(n) : cap - 1;
}

uint8_t personalKindFromText(const char* s) {
  if (!s) return EV_OTHER;
  if (strcmp(s, "birthday") == 0) return EV_BIRTHDAY;
  if (strcmp(s, "anniversary") == 0) return EV_ANNIVERSARY;
  return EV_OTHER;
}

const char* personalKindText(uint8_t kind) {
  if (kind == EV_BIRTHDAY) return "birthday";
  if (kind == EV_ANNIVERSARY) return "anniversary";
  return "other";
}

bool personalParse(ByteSource& src, JsonArena& arena, PersonalEvent* out, int cap, int& count) {
  count = 0;
  arena.reset();
  JsonDocument filter(&arena);
  filter["name"] = true;
  filter["day"] = true;
  filter["month"] = true;
  filter["year"] = true;
  filter["type"] = true;
  size_t base = arena.mark();
  JsonDocument doc(&arena);
  if (!expectChar(src, '[')) {
    filter.clear();
    arena.reset();
    return false;
  }
  bool ok = true;
  int c = skipSpaces(src);
  if (c == ']') {
    src.read();
  } else {
    for (;;) {
      doc.clear();
      arena.rewind(base);
      if (deserializeJson(doc, src, DeserializationOption::Filter(filter))) {
        ok = false;
        break;
      }
      int m = doc["month"] | 0;
      int d = doc["day"] | 0;
      int y = doc["year"] | 0;
      const char* name = doc["name"] | "";
      if (count < cap && m >= 1 && m <= 12 && d >= 1 && d <= 31 && name[0] && (y == 0 || (y > 1800 && y < 2200))) {
        PersonalEvent& p = out[count++];
        memset(&p, 0, sizeof(p));
        p.month = static_cast<uint8_t>(m);
        p.day = static_cast<uint8_t>(d);
        p.year = static_cast<uint16_t>(y);
        p.kind = personalKindFromText(doc["type"] | "other");
        copyText(p.name, sizeof(p.name), name);
      }
      c = skipSpaces(src);
      if (c == ',') {
        src.read();
        continue;
      }
      if (c == ']') {
        src.read();
        break;
      }
      ok = false;
      break;
    }
  }
  doc.clear();
  filter.clear();
  arena.reset();
  return ok;
}

size_t jsonEscape(const char* in, char* out, size_t cap) {
  size_t k = 0;
  if (cap == 0) return 0;
  for (const char* p = in; *p; ++p) {
    unsigned char ch = static_cast<unsigned char>(*p);
    char esc = 0;
    if (ch == '"') esc = '"';
    else if (ch == '\\') esc = '\\';
    else if (ch == '\n') esc = 'n';
    else if (ch == '\r') esc = 'r';
    else if (ch == '\t') esc = 't';
    if (esc) {
      if (k + 2 >= cap) break;
      out[k++] = '\\';
      out[k++] = esc;
    } else if (ch < 0x20) {
      continue;
    } else {
      if (k + 1 >= cap) break;
      out[k++] = static_cast<char>(ch);
    }
  }
  out[k] = 0;
  return k;
}

size_t personalSerialize(const PersonalEvent* items, int count, char* out, size_t cap) {
  if (cap < 3) return 0;
  size_t k = 0;
  out[k++] = '[';
  for (int i = 0; i < count; i++) {
    char name[96];
    jsonEscape(items[i].name, name, sizeof(name));
    int n = snprintf(out + k, cap - k, "%s{\"name\":\"%s\",\"day\":%u,\"month\":%u,\"year\":%u,\"type\":\"%s\"}",
                     i ? "," : "", name, static_cast<unsigned>(items[i].day), static_cast<unsigned>(items[i].month),
                     static_cast<unsigned>(items[i].year), personalKindText(items[i].kind));
    if (n < 0 || k + static_cast<size_t>(n) >= cap - 1) return 0;
    k += static_cast<size_t>(n);
  }
  out[k++] = ']';
  out[k] = 0;
  return k;
}

}
