#include <string.h>
#include <unity.h>

#include "json_arena.h"
#include "text_tr.h"
#include "wind_tr.h"
#include "wmo_tr.h"

void setUp() {}
void tearDown() {}

static void test_date_and_greeting() {
  char buf[48];
  rc::formatDateTr(buf, sizeof(buf), 5, 25, 9);
  TEST_ASSERT_EQUAL_STRING("Cuma, 25 Eylül", buf);
  rc::formatDateTr(buf, sizeof(buf), 3, 1, 2);
  TEST_ASSERT_EQUAL_STRING("Çarşamba, 1 Şubat", buf);
  TEST_ASSERT_EQUAL_STRING("Ağu", rc::monthShortTr(8));
  TEST_ASSERT_EQUAL_STRING("Oca", rc::monthShortTr(1));
  TEST_ASSERT_EQUAL_STRING("Ara", rc::monthShortTr(12));
  TEST_ASSERT_EQUAL_STRING("", rc::monthShortTr(0));
  TEST_ASSERT_EQUAL_STRING("", rc::monthShortTr(13));
  TEST_ASSERT_EQUAL_STRING("Cmt", rc::dayShortTr(6));
  TEST_ASSERT_EQUAL_STRING("Günaydın", rc::greetingTr(7));
  TEST_ASSERT_EQUAL_STRING("İyi günler", rc::greetingTr(13));
  TEST_ASSERT_EQUAL_STRING("İyi akşamlar", rc::greetingTr(19));
  TEST_ASSERT_EQUAL_STRING("İyi geceler", rc::greetingTr(23));
  TEST_ASSERT_EQUAL_STRING("İyi geceler", rc::greetingTr(3));
}

static void test_temp_formatting() {
  char buf[16];
  rc::formatTemp(buf, sizeof(buf), 217);
  TEST_ASSERT_EQUAL_STRING("22°", buf);
  rc::formatTemp(buf, sizeof(buf), -35);
  TEST_ASSERT_EQUAL_STRING("-4°", buf);
  rc::formatTemp(buf, sizeof(buf), -4);
  TEST_ASSERT_EQUAL_STRING("0°", buf);
  rc::formatTemp(buf, sizeof(buf), rc::NA16);
  TEST_ASSERT_EQUAL_STRING("--°", buf);
  rc::formatDecimalTr(buf, sizeof(buf), 217);
  TEST_ASSERT_EQUAL_STRING("21,7", buf);
  rc::formatDecimalTr(buf, sizeof(buf), -5);
  TEST_ASSERT_EQUAL_STRING("-0,5", buf);
}

static rc::WxMinutely mk(uint16_t a, uint16_t b, uint16_t c, uint16_t d) {
  rc::WxMinutely m{};
  m.count = 4;
  m.precip100[0] = a;
  m.precip100[1] = b;
  m.precip100[2] = c;
  m.precip100[3] = d;
  return m;
}

static void test_rain_summary() {
  char buf[64];
  rc::rainSummaryTr(mk(0, 0, 0, 0), false, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Önümüzdeki 1 saat yağış beklenmiyor", buf);
  rc::rainSummaryTr(mk(0, 0, 40, 60), false, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Yağmur 30 dk içinde başlayacak", buf);
  rc::rainSummaryTr(mk(20, 30, 0, 0), false, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Yağmur 30 dk içinde dinecek", buf);
  rc::rainSummaryTr(mk(20, 30, 50, 10), true, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Kar en az 1 saat sürecek", buf);
  rc::rainSummaryTr(mk(5, 0, 0, 0), false, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Önümüzdeki 1 saat yağış beklenmiyor", buf);
}

static void test_levels() {
  TEST_ASSERT_EQUAL_STRING("Düşük", rc::uvLevelTr(20));
  TEST_ASSERT_EQUAL_STRING("Orta", rc::uvLevelTr(53));
  TEST_ASSERT_EQUAL_STRING("Yüksek", rc::uvLevelTr(70));
  TEST_ASSERT_EQUAL_STRING("Aşırı", rc::uvLevelTr(115));
  TEST_ASSERT_EQUAL_STRING("İyi", rc::aqiLevelTr(26 - 10));
  TEST_ASSERT_EQUAL_STRING("Makul", rc::aqiLevelTr(26));
  TEST_ASSERT_EQUAL_STRING("Kötü", rc::aqiLevelTr(75));
  TEST_ASSERT_EQUAL_STRING("Aşırı kötü", rc::aqiLevelTr(140));
}

static void test_ago_and_duration() {
  char buf[32];
  rc::agoTr(30, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("şimdi", buf);
  rc::agoTr(600, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("10 dk önce", buf);
  rc::agoTr(7300, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("2 sa önce", buf);
  rc::agoTr(200000, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("2 gün önce", buf);
  rc::durationTr(11 * 3600 + 58 * 60, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("11 sa 58 dk", buf);
}

static void test_wmo_mapping() {
  TEST_ASSERT_EQUAL_STRING("Parçalı bulutlu", rc::wmoTextTr(2));
  TEST_ASSERT_EQUAL_UINT8(rc::IC_CLEAR_NIGHT, rc::wmoIcon(0, false));
  TEST_ASSERT_EQUAL_UINT8(rc::IC_SHOWERS_DAY, rc::wmoIcon(81, true));
  TEST_ASSERT_EQUAL_UINT8(rc::IC_SNOW, rc::wmoIcon(85, true));
  TEST_ASSERT_EQUAL_UINT8(rc::SKY_STORM, rc::wmoSky(95, true));
  TEST_ASSERT_EQUAL_UINT8(rc::SKY_CLEAR_NIGHT, rc::wmoSky(1, false));
  TEST_ASSERT_TRUE(rc::wmoIsWet(61));
  TEST_ASSERT_FALSE(rc::wmoIsWet(3));
  TEST_ASSERT_EQUAL_STRING("sunset", rc::iconFileName(rc::IC_SUNSET));
}

static void test_arena_behaviour() {
  static uint8_t mem[256];
  rc::JsonArena a(mem, sizeof(mem));
  void* p = a.allocate(10);
  TEST_ASSERT_NOT_NULL(p);
  size_t afterFirst = a.used();
  void* q = a.reallocate(p, 40);
  TEST_ASSERT_EQUAL_PTR(p, q);
  TEST_ASSERT_TRUE(a.used() > afterFirst);
  void* r = a.allocate(16);
  memset(r, 0xAB, 16);
  void* q2 = a.reallocate(q, 80);
  TEST_ASSERT_NOT_NULL(q2);
  TEST_ASSERT_TRUE(q2 != q);
  a.deallocate(q2);
  TEST_ASSERT_NULL(a.allocate(1000));
  TEST_ASSERT_EQUAL_UINT32(1, a.failures());
  size_t m = a.mark();
  a.allocate(8);
  a.rewind(m);
  TEST_ASSERT_EQUAL_UINT32(m, a.used());
  a.reset();
  TEST_ASSERT_EQUAL_UINT32(0, a.used());
  TEST_ASSERT_TRUE(a.peak() > 0);
}

static void test_english() {
  rc::setLang(rc::LANG_EN);
  char buf[64];
  rc::formatDateTr(buf, sizeof(buf), 5, 25, 9);
  TEST_ASSERT_EQUAL_STRING("Friday, September 25", buf);
  TEST_ASSERT_EQUAL_STRING("Sat", rc::dayShortTr(6));
  TEST_ASSERT_EQUAL_STRING("Aug", rc::monthShortTr(8));
  TEST_ASSERT_EQUAL_STRING("Good morning", rc::greetingTr(7));
  TEST_ASSERT_EQUAL_STRING("Good night", rc::greetingTr(23));
  rc::formatDecimalTr(buf, sizeof(buf), 217);
  TEST_ASSERT_EQUAL_STRING("21.7", buf);
  rc::rainSummaryTr(mk(0, 0, 40, 60), false, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("Rain starting in 30 min", buf);
  rc::rainSummaryTr(mk(0, 0, 0, 0), false, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("No rain expected in the next hour", buf);
  TEST_ASSERT_EQUAL_STRING("Very high", rc::uvLevelTr(85));
  TEST_ASSERT_EQUAL_STRING("Good", rc::aqiLevelTr(10));
  rc::agoTr(300, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("5 min ago", buf);
  rc::durationTr(3900, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("1 h 5 min", buf);
  TEST_ASSERT_EQUAL_STRING("Partly cloudy", rc::wmoTextTr(2));
  TEST_ASSERT_EQUAL_STRING("Northeast", rc::windNameTr(45));
  TEST_ASSERT_EQUAL_STRING("SW", rc::windAbbrTr(225));
  rc::setLang(rc::LANG_TR);
  TEST_ASSERT_EQUAL_STRING("Günaydın", rc::greetingTr(7));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_date_and_greeting);
  RUN_TEST(test_temp_formatting);
  RUN_TEST(test_rain_summary);
  RUN_TEST(test_levels);
  RUN_TEST(test_ago_and_duration);
  RUN_TEST(test_wmo_mapping);
  RUN_TEST(test_arena_behaviour);
  RUN_TEST(test_english);
  return UNITY_END();
}
