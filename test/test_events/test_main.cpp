#include <string.h>
#include <unity.h>

#include "events_calc.h"
#include "nager.h"
#include "text_tr.h"
#include "timeutil.h"

static uint8_t g_mem[8192];
static rc::JsonArena g_arena(g_mem, sizeof(g_mem));

void setUp() {}
void tearDown() {}

static int32_t day(int y, int m, int d) { return rc::daysFromCivil(y, m, d); }

static void test_civil_roundtrip() {
  TEST_ASSERT_EQUAL_INT32(0, day(1970, 1, 1));
  TEST_ASSERT_EQUAL_INT32(20721, day(2026, 9, 25));
  TEST_ASSERT_EQUAL_INT32(20722, rc::localDayNum(1790370000u));
  rc::CivilDate c = rc::civilFromDays(day(2028, 2, 29));
  TEST_ASSERT_EQUAL_INT16(2028, c.year);
  TEST_ASSERT_EQUAL_UINT8(2, c.month);
  TEST_ASSERT_EQUAL_UINT8(29, c.day);
  TEST_ASSERT_EQUAL_UINT8(5, rc::weekdayFromDays(day(2026, 9, 25)));
  TEST_ASSERT_EQUAL_UINT8(4, rc::weekdayFromDays(0));
}

static void test_republic_day_countdown() {
  int32_t today = day(2026, 9, 25);
  rc::EventOcc items[rc::MAX_UPCOMING];
  int n = rc::collectEvents(nullptr, 0, nullptr, 0, today, 40, items, rc::MAX_UPCOMING);
  TEST_ASSERT_EQUAL_INT(1, n);
  TEST_ASSERT_EQUAL_STRING("Cumhuriyet Bayramı", items[0].name);
  TEST_ASSERT_EQUAL_UINT16(103, items[0].ordinal);
  char buf[96];
  rc::eventCountdownText(items[0], today, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Cumhuriyet Bayramı'na 34 gün", buf);
  rc::eventSubtitle(items[0], buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("29 Ekim · 103. yıl", buf);
}

static void test_english_events() {
  rc::setLang(rc::LANG_EN);
  int32_t today = day(2026, 9, 25);
  rc::EventOcc items[rc::MAX_UPCOMING];
  int n = rc::collectEvents(nullptr, 0, nullptr, 0, today, 40, items, rc::MAX_UPCOMING);
  TEST_ASSERT_EQUAL_INT(1, n);
  TEST_ASSERT_EQUAL_STRING("Republic Day", items[0].name);
  char buf[96];
  rc::eventCountdownText(items[0], today, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Republic Day · 34 days", buf);
  rc::eventCountdownText(items[0], day(2026, 10, 29), buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Today: Republic Day", buf);
  rc::eventGreeting(items[0], buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Happy Republic Day!", buf);
  rc::eventSubtitle(items[0], buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("29 October · year 103", buf);
  rc::setLang(rc::LANG_TR);
}

static void test_tiers() {
  TEST_ASSERT_EQUAL_UINT8(rc::TIER_NONE, rc::countdownTier(31));
  TEST_ASSERT_EQUAL_UINT8(rc::TIER_RARE, rc::countdownTier(30));
  TEST_ASSERT_EQUAL_UINT8(rc::TIER_RARE, rc::countdownTier(8));
  TEST_ASSERT_EQUAL_UINT8(rc::TIER_FREQUENT, rc::countdownTier(7));
  TEST_ASSERT_EQUAL_UINT8(rc::TIER_FREQUENT, rc::countdownTier(1));
  TEST_ASSERT_EQUAL_UINT8(rc::TIER_TODAY, rc::countdownTier(0));
  TEST_ASSERT_EQUAL_UINT8(rc::TIER_NONE, rc::countdownTier(-1));
}

static void test_today_and_tomorrow_text() {
  int32_t today = day(2026, 10, 29);
  rc::EventOcc items[rc::MAX_UPCOMING];
  int n = rc::collectEvents(nullptr, 0, nullptr, 0, today, 30, items, rc::MAX_UPCOMING);
  TEST_ASSERT_TRUE(n >= 1);
  char buf[96];
  rc::eventCountdownText(items[0], today, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Bugün: Cumhuriyet Bayramı", buf);
  rc::eventGreeting(items[0], buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Cumhuriyet Bayramı kutlu olsun!", buf);
  rc::eventCountdownText(items[0], today - 1, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Yarın: Cumhuriyet Bayramı", buf);
  TEST_ASSERT_EQUAL_UINT8(rc::ACC_FLAG, items[0].accessory);
}

static void test_commemoration_is_calm() {
  int32_t today = day(2026, 11, 10);
  rc::EventOcc items[rc::MAX_UPCOMING];
  int n = rc::collectEvents(nullptr, 0, nullptr, 0, today, 0, items, rc::MAX_UPCOMING);
  TEST_ASSERT_EQUAL_INT(1, n);
  TEST_ASSERT_EQUAL_UINT8(rc::EV_COMMEMORATION, items[0].kind);
  char buf[96];
  rc::eventSubtitle(items[0], buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("10 Kasım · 88. yıl dönümü", buf);
}

static void test_religious_and_year_wrap() {
  rc::Religious rel[2];
  rc::religiousTable(2027, rel[0]);
  rc::religiousTable(2026, rel[1]);
  int32_t today = day(2027, 2, 20);
  rc::EventOcc items[rc::MAX_UPCOMING];
  int n = rc::collectEvents(rel, 2, nullptr, 0, today, 30, items, rc::MAX_UPCOMING);
  TEST_ASSERT_EQUAL_INT(1, n);
  TEST_ASSERT_EQUAL_STRING("Ramazan Bayramı", items[0].name);
  TEST_ASSERT_EQUAL_INT32(day(2027, 3, 9), items[0].dayNum);
  int32_t dec = day(2026, 12, 20);
  n = rc::collectEvents(rel, 2, nullptr, 0, dec, 30, items, rc::MAX_UPCOMING);
  TEST_ASSERT_EQUAL_INT(1, n);
  TEST_ASSERT_EQUAL_STRING("Yılbaşı", items[0].name);
  TEST_ASSERT_EQUAL_UINT16(2027, items[0].year);
}

static void test_personal_parse_and_birthday() {
  const char* j =
      "[{\"name\":\"Elif'in doğum günü\",\"day\":14,\"month\":10,\"year\":1990,\"type\":\"birthday\"},"
      "{\"name\":\"\",\"day\":1,\"month\":1},"
      "{\"name\":\"Kuruluş yıl dönümü\",\"day\":29,\"month\":2,\"type\":\"anniversary\"},"
      "{\"name\":\"Bozuk\",\"day\":40,\"month\":13}]";
  rc::MemSource src(j, strlen(j));
  rc::PersonalEvent p[rc::MAX_PERSONAL];
  int count = 0;
  TEST_ASSERT_TRUE(rc::personalParse(src, g_arena, p, rc::MAX_PERSONAL, count));
  TEST_ASSERT_EQUAL_INT(2, count);
  TEST_ASSERT_EQUAL_UINT8(rc::EV_BIRTHDAY, p[0].kind);
  int32_t today = day(2026, 10, 10);
  rc::EventOcc items[rc::MAX_UPCOMING];
  int n = rc::collectEvents(nullptr, 0, p, count, today, 7, items, rc::MAX_UPCOMING);
  TEST_ASSERT_EQUAL_INT(1, n);
  TEST_ASSERT_EQUAL_UINT16(36, items[0].ordinal);
  char buf[96];
  rc::eventCountdownText(items[0], today, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Elif'in doğum günü · 4 gün", buf);
  TEST_ASSERT_EQUAL_UINT8(rc::ACC_PARTY, items[0].accessory);
  n = rc::collectEvents(nullptr, 0, p, count, day(2027, 2, 27), 3, items, rc::MAX_UPCOMING);
  TEST_ASSERT_EQUAL_INT(1, n);
  TEST_ASSERT_EQUAL_UINT8(28, items[0].day);
}

static void test_personal_serialize_roundtrip() {
  rc::PersonalEvent p[2] = {};
  p[0].month = 3;
  p[0].day = 14;
  p[0].year = 0;
  p[0].kind = rc::EV_OTHER;
  strcpy(p[0].name, "Ali'nin \"sürpriz\" günü");
  p[1].month = 12;
  p[1].day = 31;
  p[1].kind = rc::EV_BIRTHDAY;
  strcpy(p[1].name, "Deniz");
  char buf[512];
  size_t n = rc::personalSerialize(p, 2, buf, sizeof(buf));
  TEST_ASSERT_TRUE(n > 0);
  rc::MemSource src(buf, n);
  rc::PersonalEvent back[4];
  int count = 0;
  TEST_ASSERT_TRUE(rc::personalParse(src, g_arena, back, 4, count));
  TEST_ASSERT_EQUAL_INT(2, count);
  TEST_ASSERT_EQUAL_STRING("Ali'nin \"sürpriz\" günü", back[0].name);
  TEST_ASSERT_EQUAL_UINT8(rc::EV_BIRTHDAY, back[1].kind);
  char tiny[8];
  TEST_ASSERT_EQUAL_UINT32(0, rc::personalSerialize(p, 2, tiny, sizeof(tiny)));
}

static void test_snapshot_limits_and_today_count() {
  rc::PersonalEvent p[10] = {};
  for (int i = 0; i < 10; i++) {
    p[i].month = 5;
    p[i].day = static_cast<uint8_t>(10 + i);
    p[i].kind = rc::EV_OTHER;
    strcpy(p[i].name, "Etkinlik");
  }
  rc::EventsSnapshot s;
  rc::buildSnapshot(nullptr, 0, p, 10, day(2026, 5, 10), s);
  TEST_ASSERT_EQUAL_UINT8(rc::MAX_UPCOMING, s.count);
  TEST_ASSERT_EQUAL_UINT8(1, s.todayCount);
  TEST_ASSERT_EQUAL_INT32(day(2026, 5, 10), s.items[0].dayNum);
  for (int i = 1; i < s.count; i++) TEST_ASSERT_TRUE(s.items[i - 1].dayNum <= s.items[i].dayNum);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_civil_roundtrip);
  RUN_TEST(test_republic_day_countdown);
  RUN_TEST(test_tiers);
  RUN_TEST(test_english_events);
  RUN_TEST(test_today_and_tomorrow_text);
  RUN_TEST(test_commemoration_is_calm);
  RUN_TEST(test_religious_and_year_wrap);
  RUN_TEST(test_personal_parse_and_birthday);
  RUN_TEST(test_personal_serialize_roundtrip);
  RUN_TEST(test_snapshot_limits_and_today_count);
  return UNITY_END();
}
