#pragma once

#include <stdint.h>

enum NightMode : uint8_t {
  NIGHT_OFF = 0,
  NIGHT_SUN = 1,
  NIGHT_HOURS = 2,
};

struct Settings {
  char name[24];
  char city[32];
  float lat;
  float lon;
  uint8_t province;
  uint8_t brightness;
  uint8_t nightMode;
  uint8_t nightBrightness;
  uint16_t nightStart;
  uint16_t nightEnd;
  uint16_t homeTimeoutS;
  uint8_t cardSeconds;
  uint8_t maintReboot;
  uint8_t touchRot;
  uint8_t flip;
  uint8_t wifiConfigured;
  uint8_t quakeMinMag10;
  uint8_t eyeSleepMin;
  uint32_t teamId;
  char bulbMac[13];
  char bulbName[24];
  char printerHost[40];
  uint8_t wizAgent;
  uint8_t wizGoal;
  uint8_t wizOnlyOn;
  uint8_t goalSl;
  uint8_t goalUcl;
  uint8_t football;
  uint32_t teamC1;
  uint32_t teamC2;
  uint8_t lang;
};

namespace settings {

void load();
Settings get();
void set(const Settings& s);
void setBulb(const char* mac, const char* name);
void setWifiConfigured(bool v);
void factoryReset();
uint32_t version();
bool sane(Settings& s);
uint32_t bootCount();
void bumpBootCount();
uint32_t sleepUntil();
void setSleepUntil(uint32_t utc);

}
