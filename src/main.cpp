#include <Arduino.h>
#include <lvgl.h>

#include "agents/agents.h"
#include "app/app.h"
#include "app/config.h"
#include "app/mem.h"
#include "app/portal.h"
#include "app/store.h"
#include "board/display.h"
#include "board/lgfx_cyd.h"
#include "board/sdcard.h"
#include "diag/diag.h"
#include "events/events.h"
#include "media/bridge.h"
#include "media/camera.h"
#include "media/fwupdate.h"
#include "media/media.h"
#include "net/net.h"
#include "net/relay.h"
#include "quake/quake.h"
#include "settings/settings.h"
#include "ui/ui.h"
#include "weather/weather.h"
#include "wiz/wiz.h"
#include "text_tr.h"

namespace {

constexpr int BOOT_STEPS = 9;
const char* s_stepName[BOOT_STEPS];
uint32_t s_stepFree[BOOT_STEPS];
int s_stepN = 0;

void step(const char* name) {
  if (s_stepN >= BOOT_STEPS) return;
  s_stepName[s_stepN] = name;
  s_stepFree[s_stepN] = mem::freeHeap();
  s_stepN++;
}

void reportSteps() {
  char buf[160];
  size_t k = 0;
  for (int i = 0; i < s_stepN && k < sizeof(buf); i++) {
    int n = snprintf(buf + k, sizeof(buf) - k, "%s%s %u", i ? " · " : "", s_stepName[i],
                     static_cast<unsigned>(s_stepFree[i]));
    if (n < 0) break;
    k += static_cast<size_t>(n);
  }
  diag::note("boot", "bos bellek: %s", buf);
}

void fatal(const char* what) {
  diag::note("boot", "HATA: %s", what);
  if (display::begin(settings::get().touchRot)) {
    display::setFlip(settings::get().flip);
    LgfxCyd& g = display::gfx();
    display::setBrightnessPct(60);
    g.setTextColor(0xFFFFu, 0x0000u);
    g.setTextSize(2);
    g.setCursor(12, 90);
    g.print(rc::L("Rumy baslatilamadi", "Rumy failed to start"));
    g.setTextSize(1);
    g.setCursor(12, 124);
    g.print(what);
    g.setCursor(12, 140);
    g.print(rc::L("10 sn sonra yeniden denenecek", "Retrying in 10 s"));
  }
  vTaskDelay(pdMS_TO_TICKS(10000));
  ESP.restart();
}

}

void setup() {
  Serial.setRxBufferSize(cfg::SERIAL_RX_BYTES);
  Serial.begin(cfg::SERIAL_BAUD);
  display::ledsOff();
  store::init();
  settings::load();
  settings::bumpBootCount();
  diag::begin();
  bridge::begin();

  Settings st = settings::get();
  if (!st.wifiConfigured) portal::run();

  step("basla");
  if (!mem::init()) fatal("bellek ayrilamadi");
  step("tampon");
  if (!display::begin(st.touchRot)) fatal("ekran surucusu");
  display::setFlip(st.flip);
  lv_init();
  display::startLvgl(mem::drawBuf1(), mem::drawBuf2(), cfg::DRAW_BUF_BYTES);
  step("ekran");

  if (!sdcard::begin()) diag::note("boot", "SD kart yok ya da okunamadi");
  step("sd");
  weather::loadCache();
  quake::loadCache();
  events::begin();
  step("veri");

  media::begin();
  ui::begin();
  step("arayuz");
  net::begin();
  step("ag");
  wiz::begin();
  step("wiz");
  mem::report(Serial);
  reportSteps();
  diag::note("boot", "hazir");
}

void loop() {
  ui::loop();
  diag::loop();
  media::loop();
  agents::loop();
  fwupdate::loop();
  relay::service();
  cam::loop();
  app::poll();
}
