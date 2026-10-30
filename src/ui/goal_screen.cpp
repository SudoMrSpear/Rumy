#include "goal_screen.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "draw_util.h"
#include "match/photo.h"
#include "match_art.h"
#include "match_ui.h"
#include "src/core/lv_obj_private.h"
#include "rumy/rumy.h"
#include "theme.h"
#include "text_tr.h"

namespace goal_screen {

namespace {

using theme::CX;
using theme::S;
using theme::SH;
using theme::SW;

constexpr uint32_t FRAME_MS = 30;
constexpr uint32_t WIPE_MS = 340;
constexpr uint32_t EXIT_MS = 300;
constexpr int32_t BAND_W = S(64);
constexpr int32_t BAND_SLANT = S(56);
constexpr int32_t BAND_PAD = S(24);
constexpr int32_t EDGE_W = S(3);
constexpr int32_t STRIPE_STEP = S(24);
constexpr int32_t STRIPE_RUN = S(120);
constexpr int32_t STRIPE_W = S(10);
constexpr int32_t ROW_H = S(16);
constexpr int ROWS = (SH + ROW_H - 1) / ROW_H;
constexpr int LETTERS = 6;
constexpr int32_t DROP_H = S(110);
constexpr int32_t POP_H = S(14);
constexpr int32_t TOP_Y = S(4);
constexpr uint32_t MOVE_MS = 450;
constexpr uint32_t PANEL_DELAY_MS = 150;
constexpr uint32_t PANEL_MS = 480;
constexpr int32_t WAVE_A = S(4);
constexpr uint32_t WAVE_MS = 900;
constexpr uint32_t WAVE_IN_MS = 250;
constexpr uint32_t SHINE_FIRST_MS = 250;
constexpr uint32_t SHINE_EVERY_MS = 1800;
constexpr uint32_t SHINE_MS = 520;
constexpr int32_t SHINE_W = S(22);
constexpr int32_t PHOTO_X = S(16);
constexpr int32_t PHOTO_Y = S(58);
constexpr int32_t PHOTO_D = cfg::PHOTO_PX;
constexpr int32_t RING_W = S(3);
constexpr int32_t INFO_X = S(114);
constexpr int32_t INFO_Y = S(60);
constexpr int32_t INFO_H = S(84);
constexpr uint32_t BOX_MS = 340;
constexpr uint32_t ROLL_MS = 420;
constexpr int CONFETTI = 30;
constexpr uint32_t CONFETTI_MS = 3800;
constexpr uint32_t CONFETTI_SPREAD_MS = 900;
constexpr int SPARKS = 18;
constexpr uint32_t SPARK_MS = 1100;
constexpr uint32_t FLASH_MS = 90;
constexpr int BURSTS = 5;

constexpr int32_t NET_X1 = CX(86);
constexpr int32_t NET_X2 = CX(234);
constexpr int32_t NET_Y1 = S(34);
constexpr int32_t NET_Y2 = S(116);
constexpr int32_t NET_GAP = S(11);
constexpr int32_t NET_STEP = S(6);
constexpr int32_t POST_W = S(4);
constexpr int32_t GROUND_X1 = CX(40);
constexpr int32_t GROUND_X2 = CX(280);
constexpr int32_t HIT_X = CX(180);
constexpr int32_t HIT_Y = S(68);
constexpr float BULGE = S(9);
constexpr float BULGE_R = S(30);
constexpr uint32_t SHOT_T = 400;
constexpr uint32_t HIT_T = 860;
constexpr uint32_t SINK_MS = 520;
constexpr uint32_t BULGE_MS = 620;
constexpr uint32_t NET_END = 1500;

constexpr int32_t PANEL_X1 = CX(56);
constexpr int32_t PANEL_X2 = CX(264);
constexpr int32_t PANEL_Y1 = S(4);
constexpr int32_t PANEL_Y2 = S(84);
constexpr int32_t FACE_EYE = S(48);
constexpr int32_t BOX_X1 = CX(40);
constexpr int32_t BOX_X2 = CX(279);

struct Layout {
  int32_t heroY;
  int32_t boxY;
  int32_t boxH;
  int32_t footY;
};

constexpr Layout kStd = {S(54), S(156), S(44), S(210)};
constexpr Layout kRumy = {S(94), S(156), S(44), S(210)};

struct Plan {
  uint32_t letters;
  uint32_t step;
  uint32_t dur;
  bool drop;
  uint32_t box;
  uint32_t roll;
  uint32_t confetti;
  uint32_t phase2;
};

constexpr Plan kPlans[V_COUNT] = {
    {360, 70, 560, true, 1100, 1480, 380, 3400},
    {NET_END, 55, 300, false, 1700, 2080, NET_END, 3700},
    {820, 45, 320, false, 1200, 1580, 0, 3400},
    {480, 55, 320, false, 860, 1240, 0, 3400},
};

struct Burst {
  int32_t x;
  int32_t y;
  uint32_t launch;
  uint32_t at;
  uint32_t color;
};

constexpr uint32_t COL_TEAM = 0x1000000;
constexpr uint32_t COL_SKY = 0x2000000;

constexpr Burst kBursts[BURSTS] = {
    {CX(70), S(72), 340, 720, COL_TEAM},   {CX(250), S(58), 520, 900, theme::C_WHITE},
    {CX(160), S(40), 820, 1200, COL_TEAM}, {CX(108), S(50), 1500, 1860, COL_SKY},
    {CX(222), S(84), 1700, 2060, COL_TEAM},
};

uint32_t burstColor(uint32_t c) { return c == COL_TEAM ? theme::team() : c == COL_SKY ? theme::teamSky() : c; }

const char* const kLetters[LETTERS] = {"G", "O", "O", "O", "L", "!"};
const char* const kNames[V_COUNT] = {"harf", "ag", "fisek", "rumy"};

lv_obj_t* s_obj = nullptr;
lv_timer_t* s_timer = nullptr;
lv_obj_t* s_face = nullptr;
lv_obj_t* s_hidden = nullptr;
uint32_t s_start = 0;
uint32_t s_now = 0;
uint32_t s_exitAt = 0;
uint32_t s_seed = 0;
uint8_t s_var = V_LETTERS;
const Layout* s_lay = &kStd;
int32_t s_letterX[LETTERS + 1];
char s_name[rc::MATCH_PLAYER];
char s_first[40];
char s_last[40];
char s_detail[40];
char s_initials[12];
const lv_image_dsc_t* s_photo = nullptr;
const void* s_photoData = nullptr;
char s_homeAbbr[8];
char s_awayAbbr[8];
char s_homeNum[6];
char s_awayNum[6];
char s_oldNum[6];
char s_foot[72];
const lv_image_dsc_t* s_logo = nullptr;
bool s_usHome = true;
bool s_roll = false;
int32_t s_cellH1 = 0;
int32_t s_cellA1 = 0;
int32_t s_cellHW = 0;
int32_t s_cellAW = 0;
int32_t s_sepX = 0;
int32_t s_dirtyX1[ROWS];
int32_t s_dirtyX2[ROWS];

lv_color_t C(uint32_t hex) { return theme::col(hex); }

int32_t I(float v) { return static_cast<int32_t>(lroundf(v)); }

float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }

float prog(uint32_t t, uint32_t start, uint32_t dur) {
  if (t <= start || !dur) return t > start ? 1.0f : 0.0f;
  return clamp01(static_cast<float>(t - start) / static_cast<float>(dur));
}

float easeOut(float p) {
  float q = 1 - p;
  return 1 - q * q * q;
}

float easeInOut(float p) { return 0.5f - 0.5f * cosf(p * 3.14159265f); }

float easeOutBack(float p) {
  const float c1 = 1.70158f;
  float q = p - 1;
  return 1 + (c1 + 1) * q * q * q + c1 * q * q;
}

float bounce(float p) {
  const float n = 7.5625f;
  const float d = 2.75f;
  if (p < 1 / d) return n * p * p;
  if (p < 2 / d) {
    p -= 1.5f / d;
    return n * p * p + 0.75f;
  }
  if (p < 2.5f / d) {
    p -= 2.25f / d;
    return n * p * p + 0.9375f;
  }
  p -= 2.625f / d;
  return n * p * p + 0.984375f;
}

uint32_t hash(uint32_t x) {
  x ^= x >> 16;
  x *= 0x7FEB352Du;
  x ^= x >> 15;
  x *= 0x846CA68Bu;
  x ^= x >> 16;
  return x;
}

uint32_t umod(uint32_t v, int32_t m) { return v % static_cast<uint32_t>(m); }

bool inClip(lv_layer_t* layer, const lv_area_t& a) {
  const lv_area_t& c = layer->_clip_area;
  return !(a.x2 < c.x1 || a.x1 > c.x2 || a.y2 < c.y1 || a.y1 > c.y2);
}

void dirtyReset() {
  for (int r = 0; r < ROWS; r++) {
    s_dirtyX1[r] = INT32_MAX;
    s_dirtyX2[r] = INT32_MIN;
  }
}

void mark(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > SW - 1) x2 = SW - 1;
  if (y2 > SH - 1) y2 = SH - 1;
  if (x1 > x2 || y1 > y2) return;
  for (int r = y1 / ROW_H; r <= y2 / ROW_H && r < ROWS; r++) {
    if (x1 < s_dirtyX1[r]) s_dirtyX1[r] = x1;
    if (x2 > s_dirtyX2[r]) s_dirtyX2[r] = x2;
  }
}

void mark(const lv_area_t& a) { mark(a.x1, a.y1, a.x2, a.y2); }

void dirtyFlush() {
  for (int r = 0; r < ROWS; r++) {
    if (s_dirtyX1[r] > s_dirtyX2[r]) continue;
    int32_t y2 = r * ROW_H + ROW_H - 1;
    lv_area_t a = {s_dirtyX1[r], r * ROW_H, s_dirtyX2[r], y2 < SH - 1 ? y2 : SH - 1};
    lv_obj_invalidate_area(s_obj, &a);
  }
}

int32_t bandX(float p) {
  const int32_t from = -BAND_SLANT - BAND_W;
  const int32_t to = SW + BAND_W / 2;
  return from + I(static_cast<float>(to - from) * easeInOut(p));
}

void markBand(int32_t a, int32_t b) {
  int32_t lo = a < b ? a : b;
  int32_t hi = a < b ? b : a;
  mark(lo - BAND_W / 2 - S(4), 0, hi + BAND_SLANT + BAND_W / 2 + S(4), SH - 1);
}

uint32_t lettersDone() {
  const Plan& p = kPlans[s_var];
  return p.letters + p.step * (LETTERS - 1) + p.dur;
}

int32_t baseY(uint32_t t) {
  const Plan& p = kPlans[s_var];
  if (t <= p.phase2) return s_lay->heroY;
  return s_lay->heroY + I(static_cast<float>(TOP_Y - s_lay->heroY) * easeInOut(prog(t, p.phase2, MOVE_MS)));
}

int32_t wave(int i, uint32_t t) {
  const Plan& p = kPlans[s_var];
  uint32_t from = lettersDone();
  if (t <= from || t >= p.phase2 + MOVE_MS) return 0;
  float amp = static_cast<float>(WAVE_A) * prog(t, from, WAVE_IN_MS) * (1 - prog(t, p.phase2, MOVE_MS));
  float ph = static_cast<float>(t - from) / WAVE_MS * 6.2831853f - static_cast<float>(i) * 0.9f;
  return I(-amp * sinf(ph));
}

bool shine(uint32_t t, int32_t& x) {
  uint32_t from = lettersDone() + SHINE_FIRST_MS;
  if (t < from) return false;
  uint32_t k = (t - from) % SHINE_EVERY_MS;
  if (k >= SHINE_MS) return false;
  int32_t a = s_letterX[0] - SHINE_W;
  int32_t b = s_letterX[LETTERS] + SHINE_W;
  x = a + I(static_cast<float>(b - a) * easeInOut(static_cast<float>(k) / SHINE_MS));
  return true;
}

bool letter(int i, uint32_t t, int32_t& y, lv_opa_t& opa, uint32_t& color) {
  const Plan& p = kPlans[s_var];
  uint32_t st = p.letters + p.step * static_cast<uint32_t>(i);
  if (t < st) return false;
  float q = prog(t, st, p.dur);
  y = baseY(t) + wave(i, t);
  if (p.drop) {
    y -= I(DROP_H * (1 - bounce(q)));
    opa = LV_OPA_COVER;
    color = theme::team();
  } else {
    y += I(POP_H * (1 - easeOutBack(q)));
    opa = static_cast<lv_opa_t>(I(255 * clamp01(q * 4)));
    color = q < 0.3f ? theme::C_WHITE : theme::team();
  }
  return true;
}

lv_area_t letterArea(int i, int32_t y) {
  int32_t lh = lv_font_get_line_height(theme::fontHero());
  return {s_letterX[i] - S(3), y - S(2), s_letterX[i + 1] + S(3), y + lh + S(2)};
}

lv_area_t wordArea(uint32_t a, uint32_t b) {
  int32_t ya = baseY(a);
  int32_t yb = baseY(b);
  int32_t lh = lv_font_get_line_height(theme::fontHero());
  return {s_letterX[0] - S(4), (ya < yb ? ya : yb) - WAVE_A - S(3), s_letterX[LETTERS] + S(4),
          (ya < yb ? yb : ya) + lh + WAVE_A + S(3)};
}

int32_t panelShift(uint32_t t) {
  uint32_t at = kPlans[s_var].phase2 + PANEL_DELAY_MS;
  return I(static_cast<float>(SW) * 0.85f * (1 - easeOutBack(prog(t, at, PANEL_MS))));
}

lv_area_t panelArea() { return {0, PHOTO_Y - RING_W - S(2), SW - 1, INFO_Y + INFO_H + S(2)}; }

lv_area_t boxArea() { return {BOX_X1 - S(16), s_lay->boxY - S(2), BOX_X2 + S(16), s_lay->boxY + s_lay->boxH + S(2)}; }

lv_area_t rollArea() {
  int32_t x1 = s_usHome ? s_cellH1 : s_cellA1;
  int32_t w = s_usHome ? s_cellHW : s_cellAW;
  return {x1 - S(3), s_lay->boxY, x1 + w + S(3), s_lay->boxY + s_lay->boxH};
}

bool confetti(int i, uint32_t t, lv_area_t& a, uint32_t& color) {
  const Plan& p = kPlans[s_var];
  if (!p.confetti) return false;
  uint32_t h = hash(s_seed + static_cast<uint32_t>(i) * 0x9E3779B1u);
  uint32_t delay = p.confetti + h % CONFETTI_SPREAD_MS;
  if (t < delay) return false;
  float dt = static_cast<float>(t - delay) / 1000.0f;
  float vy = static_cast<float>(S(56) + static_cast<int32_t>(umod(h >> 10, S(60))));
  float y = static_cast<float>(-S(8)) + vy * dt + static_cast<float>(S(30)) * dt * dt;
  if (y > SH + S(8)) return false;
  float sway = static_cast<float>(S(4) + static_cast<int32_t>(umod(h >> 16, S(10))));
  float ph = static_cast<float>((h >> 20) % 360) * 0.0174533f;
  float x = static_cast<float>(umod(h >> 4, SW)) + sway * sinf(ph + dt * 5.5f);
  bool flip = (((t - delay) / 110 + static_cast<uint32_t>(i)) & 1) != 0;
  int32_t hw = flip ? S(3) : S(1) + 1;
  int32_t hh = flip ? S(1) + 1 : S(3);
  a = {I(x) - hw, I(y) - hh, I(x) + hw, I(y) + hh};
  const uint32_t kCol[4] = {theme::team(), theme::C_WHITE, theme::teamSky(), theme::teamPale()};
  color = kCol[(h >> 28) & 3];
  return true;
}

bool rocket(int b, uint32_t t, lv_area_t& a) {
  const Burst& k = kBursts[b];
  if (t < k.launch || t >= k.at) return false;
  float q = easeOut(prog(t, k.launch, k.at - k.launch));
  int32_t y = SH + I(static_cast<float>(k.y - SH) * q);
  a = {k.x - S(2), y - S(2), k.x + S(2), y + S(16)};
  return true;
}

bool flash(int b, uint32_t t, lv_area_t& a) {
  const Burst& k = kBursts[b];
  if (t < k.at || t >= k.at + FLASH_MS) return false;
  a = {k.x - S(8), k.y - S(8), k.x + S(8), k.y + S(8)};
  return true;
}

struct Spark {
  int32_t x[3];
  int32_t y[3];
  uint32_t color;
  lv_opa_t opa;
};

bool spark(int b, int j, uint32_t t, Spark& s) {
  const Burst& k = kBursts[b];
  if (t < k.at || t >= k.at + SPARK_MS) return false;
  uint32_t h = hash(s_seed ^ (static_cast<uint32_t>(b) * 131u + static_cast<uint32_t>(j) * 7919u));
  float dt = static_cast<float>(t - k.at) / 1000.0f;
  float life = static_cast<float>(SPARK_MS) / 1000.0f;
  float ang = (static_cast<float>(j) * 360.0f / SPARKS + static_cast<float>(h % 16) - 8.0f) * 0.0174533f;
  float spd = static_cast<float>(S(92) + static_cast<int32_t>(umod(h >> 8, S(56))));
  float d = spd * dt * (1 - dt / (2 * life));
  float g = static_cast<float>(S(52)) * dt * dt;
  float cx = cosf(ang);
  float cy = sinf(ang);
  static const float kTrail[3] = {1.0f, 0.8f, 0.6f};
  for (int i = 0; i < 3; i++) {
    s.x[i] = k.x + I(cx * d * kTrail[i]);
    s.y[i] = k.y + I(cy * d * kTrail[i] + g * (0.6f + 0.4f * kTrail[i]));
  }
  s.color = t - k.at < 160 ? theme::C_WHITE : ((h >> 24) % 3 == 0 ? theme::C_WHITE : burstColor(k.color));
  float fade = dt < life * 0.55f ? 1.0f : 1 - (dt - life * 0.55f) / (life * 0.45f);
  s.opa = static_cast<lv_opa_t>(I(255 * clamp01(fade)));
  return true;
}

lv_area_t sparkArea(const Spark& s) {
  lv_area_t a = {s.x[0], s.y[0], s.x[0], s.y[0]};
  for (int i = 1; i < 3; i++) {
    if (s.x[i] < a.x1) a.x1 = s.x[i];
    if (s.x[i] > a.x2) a.x2 = s.x[i];
    if (s.y[i] < a.y1) a.y1 = s.y[i];
    if (s.y[i] > a.y2) a.y2 = s.y[i];
  }
  int32_t r = S(2) + 1;
  return {a.x1 - r, a.y1 - r, a.x2 + r, a.y2 + r};
}

float bulgeAmp(uint32_t t) {
  if (t < HIT_T || t > HIT_T + BULGE_MS) return 0;
  float dt = static_cast<float>(t - HIT_T) / 1000.0f;
  return BULGE * expf(-dt * 6.0f) * cosf(dt * 26.0f);
}

bool ball(uint32_t t, int32_t& x, int32_t& y, int& frame) {
  if (s_var != V_NET || t < SHOT_T || t >= NET_END) return false;
  if (t < HIT_T) {
    float q = prog(t, SHOT_T, HIT_T - SHOT_T);
    float a = (1 - q) * (1 - q);
    float b = 2 * (1 - q) * q;
    float c = q * q;
    x = I(a * CX(150) + b * CX(118) + c * HIT_X);
    y = I(a * (SH + S(14)) + b * S(40) + c * HIT_Y);
    frame = static_cast<int>((t / 30) % BALL_FRAMES);
    return true;
  }
  float q = prog(t, HIT_T, SINK_MS);
  int32_t rest = NET_Y2 - BALL_PX / 2 - S(1);
  x = HIT_X + I(static_cast<float>(S(6)) * q);
  y = HIT_Y + I(static_cast<float>(rest - HIT_Y) * bounce(q));
  frame = static_cast<int>(((q < 1 ? t : HIT_T + SINK_MS) / 60) % BALL_FRAMES);
  return true;
}

lv_area_t ballArea(int32_t x, int32_t y) {
  return {x - BALL_PX / 2 - 1, y - BALL_PX / 2 - 1, x + BALL_PX / 2 + 1, y + BALL_PX / 2 + 1};
}

lv_area_t netArea() {
  return {GROUND_X1 - S(2), NET_Y1 - POST_W, GROUND_X2 + S(2), NET_Y2 + S(3)};
}

template <typename F>
void window(uint32_t prev, uint32_t now, uint32_t a, uint32_t b, F rect) {
  if (now < a || prev > b) return;
  if (prev >= a) rect(prev < b ? prev : b);
  rect(now < b ? now : b);
}

void markElements(uint32_t prev, uint32_t now) {
  const Plan& p = kPlans[s_var];
  for (int i = 0; i < LETTERS; i++) {
    uint32_t st = p.letters + p.step * static_cast<uint32_t>(i);
    window(prev, now, st, st + p.dur, [i](uint32_t t) {
      int32_t y;
      lv_opa_t opa;
      uint32_t col;
      if (letter(i, t, y, opa, col)) mark(letterArea(i, y));
    });
  }
  window(prev, now, lettersDone(), p.phase2 + MOVE_MS, [prev, now](uint32_t) { mark(wordArea(prev, now)); });
  int32_t sx;
  if (shine(prev, sx) || shine(now, sx)) mark(wordArea(prev, now));
  uint32_t pa = p.phase2 + PANEL_DELAY_MS;
  window(prev, now, pa, pa + PANEL_MS, [](uint32_t) { mark(panelArea()); });
  if (s_var == V_RUMY && prev < p.phase2 && now >= p.phase2) mark(PANEL_X1, PANEL_Y1, PANEL_X2, PANEL_Y2);
  if (now >= pa) {
    const lv_image_dsc_t* ph = photo::get(s_name);
    const void* data = ph ? ph->data : nullptr;
    if (data != s_photoData) {
      s_photo = ph;
      s_photoData = data;
      mark(panelArea());
    }
  }
  window(prev, now, p.box, p.box + BOX_MS, [](uint32_t) { mark(boxArea()); });
  if (s_roll) window(prev, now, p.roll, p.roll + ROLL_MS, [](uint32_t) { mark(rollArea()); });
  if (p.confetti) {
    window(prev, now, p.confetti, p.confetti + CONFETTI_MS, [](uint32_t t) {
      for (int i = 0; i < CONFETTI; i++) {
        lv_area_t a;
        uint32_t col;
        if (confetti(i, t, a, col)) mark(a);
      }
    });
  }
  if (s_var == V_NET) {
    window(prev, now, SHOT_T, NET_END, [](uint32_t t) {
      int32_t x, y;
      int f;
      if (ball(t, x, y, f)) mark(ballArea(x, y));
    });
    window(prev, now, HIT_T, HIT_T + BULGE_MS, [](uint32_t) { mark(netArea()); });
    if (prev < NET_END && now >= NET_END) mark(netArea());
  }
  if (s_var == V_FIREWORKS) {
    for (int b = 0; b < BURSTS; b++) {
      window(prev, now, kBursts[b].launch, kBursts[b].at + SPARK_MS, [b](uint32_t t) {
        lv_area_t a;
        Spark sp;
        if (rocket(b, t, a)) mark(a);
        if (flash(b, t, a)) mark(a);
        for (int j = 0; j < SPARKS; j++) {
          if (spark(b, j, t, sp)) mark(sparkArea(sp));
        }
      });
    }
  }
}

void drawStripes(lv_layer_t* layer) {
  const lv_area_t& c = layer->_clip_area;
  lv_draw_line_dsc_t ld;
  lv_draw_line_dsc_init(&ld);
  ld.color = C(theme::teamStripe());
  ld.width = STRIPE_W;
  for (int32_t i = -STRIPE_RUN; i < SW; i += STRIPE_STEP) {
    int32_t xa = i + STRIPE_RUN * c.y1 / (SH - 1);
    int32_t xb = i + STRIPE_RUN * c.y2 / (SH - 1);
    if (xb + STRIPE_W < c.x1 || xa - STRIPE_W > c.x2) continue;
    ld.p1.x = i;
    ld.p1.y = 0;
    ld.p2.x = i + STRIPE_RUN;
    ld.p2.y = SH - 1;
    lv_draw_line(layer, &ld);
  }
}

void drawBand(lv_layer_t* layer, int32_t bx) {
  lv_draw_line_dsc_t ld;
  lv_draw_line_dsc_init(&ld);
  int32_t dx = BAND_SLANT * BAND_PAD / SH;
  ld.color = C(theme::team());
  ld.width = BAND_W;
  ld.p1.x = bx - dx;
  ld.p1.y = -BAND_PAD;
  ld.p2.x = bx + BAND_SLANT + dx;
  ld.p2.y = SH + BAND_PAD;
  lv_draw_line(layer, &ld);
  int32_t off = BAND_W / 2 - EDGE_W / 2;
  ld.color = C(theme::C_WHITE);
  ld.width = EDGE_W;
  ld.p1.x = bx - dx + off;
  ld.p2.x = bx + BAND_SLANT + dx + off;
  lv_draw_line(layer, &ld);
}

void netLine(lv_layer_t* layer, lv_draw_line_dsc_t& ld, int32_t x1, int32_t y1, int32_t x2, int32_t y2, float amp) {
  bool vertical = x1 == x2;
  int32_t perp = vertical ? abs(x1 - HIT_X) : abs(y1 - HIT_Y);
  if (amp == 0 || static_cast<float>(perp) > BULGE_R * 2.5f) {
    ld.p1.x = x1;
    ld.p1.y = y1;
    ld.p2.x = x2;
    ld.p2.y = y2;
    lv_draw_line(layer, &ld);
    return;
  }
  int32_t len = vertical ? y2 - y1 : x2 - x1;
  int n = static_cast<int>(len / NET_STEP) + 1;
  int32_t px = 0, py = 0;
  for (int k = 0; k <= n; k++) {
    int32_t x = vertical ? x1 : x1 + len * k / n;
    int32_t y = vertical ? y1 + len * k / n : y1;
    float dx = static_cast<float>(HIT_X - x);
    float dy = static_cast<float>(HIT_Y - y);
    float d2 = dx * dx + dy * dy;
    float d = sqrtf(d2);
    float f = amp * expf(-d2 / (BULGE_R * BULGE_R));
    int32_t ox = d < 1 ? x : x + I(dx / d * f);
    int32_t oy = d < 1 ? y : y + I(dy / d * f);
    if (k) {
      ld.p1.x = px;
      ld.p1.y = py;
      ld.p2.x = ox;
      ld.p2.y = oy;
      lv_draw_line(layer, &ld);
    }
    px = ox;
    py = oy;
  }
}

void drawNet(lv_layer_t* layer, uint32_t t) {
  if (!inClip(layer, netArea())) return;
  float amp = bulgeAmp(t);
  lv_draw_line_dsc_t ld;
  lv_draw_line_dsc_init(&ld);
  ld.color = C(0xC8D4EE);
  ld.opa = LV_OPA_50;
  ld.width = 1;
  for (int32_t x = NET_X1 + NET_GAP; x < NET_X2; x += NET_GAP) netLine(layer, ld, x, NET_Y1, x, NET_Y2, amp);
  for (int32_t y = NET_Y1 + NET_GAP; y < NET_Y2; y += NET_GAP) netLine(layer, ld, NET_X1, y, NET_X2, y, amp);
  du::box(layer, GROUND_X1, NET_Y2, GROUND_X2, NET_Y2 + S(1), C(theme::C_WHITE), LV_OPA_40, 0);
  lv_color_t w = C(theme::C_WHITE);
  du::box(layer, NET_X1 - POST_W / 2, NET_Y1 - POST_W / 2, NET_X1 + POST_W / 2, NET_Y2, w, LV_OPA_COVER, 0);
  du::box(layer, NET_X2 - POST_W / 2, NET_Y1 - POST_W / 2, NET_X2 + POST_W / 2, NET_Y2, w, LV_OPA_COVER, 0);
  du::box(layer, NET_X1 - POST_W / 2, NET_Y1 - POST_W / 2, NET_X2 + POST_W / 2, NET_Y1 + POST_W / 2, w, LV_OPA_COVER,
          0);
}

void drawFireworks(lv_layer_t* layer, uint32_t t) {
  for (int b = 0; b < BURSTS; b++) {
    const Burst& k = kBursts[b];
    if (t < k.launch || t >= k.at + SPARK_MS) continue;
    lv_area_t a;
    if (rocket(b, t, a) && inClip(layer, a)) {
      int32_t y = a.y1 + S(2);
      du::box(layer, k.x - S(1), y + S(10), k.x + S(1), y + S(12), C(burstColor(k.color)), LV_OPA_40, 0);
      du::box(layer, k.x - S(1), y + S(5), k.x + S(1), y + S(7), C(burstColor(k.color)), LV_OPA_70, 0);
      du::box(layer, k.x - S(2), y - S(2), k.x + S(2), y + S(2), C(theme::C_WHITE), LV_OPA_COVER, S(2));
    }
    if (flash(b, t, a) && inClip(layer, a)) {
      lv_opa_t o = static_cast<lv_opa_t>(I(255 * (1 - prog(t, k.at, FLASH_MS))));
      du::box(layer, a.x1, a.y1, a.x2, a.y2, C(theme::C_WHITE), o, S(8));
    }
    for (int j = 0; j < SPARKS; j++) {
      Spark sp;
      if (!spark(b, j, t, sp) || !sp.opa || !inClip(layer, sparkArea(sp))) continue;
      lv_color_t col = C(sp.color);
      static const int32_t kR[3] = {S(2), S(1), S(1)};
      static const uint8_t kO[3] = {255, 150, 70};
      for (int i = 2; i >= 0; i--) {
        lv_opa_t o = static_cast<lv_opa_t>(sp.opa * kO[i] / 255);
        du::box(layer, sp.x[i] - kR[i], sp.y[i] - kR[i], sp.x[i] + kR[i], sp.y[i] + kR[i], col, o, kR[i]);
      }
    }
  }
}

void drawConfetti(lv_layer_t* layer, uint32_t t) {
  const Plan& p = kPlans[s_var];
  if (!p.confetti || t < p.confetti || t > p.confetti + CONFETTI_MS) return;
  for (int i = 0; i < CONFETTI; i++) {
    lv_area_t a;
    uint32_t col;
    if (confetti(i, t, a, col) && inClip(layer, a)) du::box(layer, a.x1, a.y1, a.x2, a.y2, C(col), LV_OPA_COVER, 0);
  }
}

void drawScore(lv_layer_t* layer, uint32_t t, int32_t by1, int32_t by2, lv_opa_t opa) {
  const Plan& p = kPlans[s_var];
  const lv_font_t* ft = theme::fontTemp();
  int32_t sy = by1 + (by2 - by1 + 1 - lv_font_get_line_height(ft)) / 2;
  lv_color_t ours = C(theme::team());
  lv_color_t theirs = C(theme::C_TEXT);
  du::text(layer, ft, theirs, LV_TEXT_ALIGN_LEFT, s_sepX, sy, s_sepX + S(60), " - ", opa);
  int32_t cx1 = s_usHome ? s_cellH1 : s_cellA1;
  int32_t cw = s_usHome ? s_cellHW : s_cellAW;
  int32_t ox1 = s_usHome ? s_cellA1 : s_cellH1;
  int32_t ow = s_usHome ? s_cellAW : s_cellHW;
  du::text(layer, ft, theirs, LV_TEXT_ALIGN_CENTER, ox1, sy, ox1 + ow - 1, s_usHome ? s_awayNum : s_homeNum, opa);
  const char* now = s_usHome ? s_homeNum : s_awayNum;
  if (!s_roll || t >= p.roll + ROLL_MS) {
    du::text(layer, ft, ours, LV_TEXT_ALIGN_CENTER, cx1, sy, cx1 + cw - 1, now, opa);
    return;
  }
  if (t < p.roll) {
    du::text(layer, ft, ours, LV_TEXT_ALIGN_CENTER, cx1, sy, cx1 + cw - 1, s_oldNum, opa);
    return;
  }
  float q = prog(t, p.roll, ROLL_MS);
  int32_t span = by2 - by1;
  du::Clip clip(layer, cx1 - S(3), by1 + S(3), cx1 + cw + S(3), by2 - S(3));
  if (clip.empty()) return;
  du::text(layer, ft, ours, LV_TEXT_ALIGN_CENTER, cx1, sy - I(static_cast<float>(span) * easeOut(q)), cx1 + cw - 1,
           s_oldNum, opa);
  du::text(layer, ft, ours, LV_TEXT_ALIGN_CENTER, cx1, sy + I(static_cast<float>(span) * (1 - easeOutBack(q))),
           cx1 + cw - 1, now, opa);
}

void drawBox(lv_layer_t* layer, uint32_t t) {
  const Plan& p = kPlans[s_var];
  if (t < p.box) return;
  lv_area_t area = boxArea();
  if (!inClip(layer, area)) return;
  float q = prog(t, p.box, BOX_MS);
  int32_t full = (BOX_X2 - BOX_X1) / 2;
  int32_t half = I(static_cast<float>(full) * easeOutBack(q));
  int32_t r = S(14);
  if (half < r) half = r;
  int32_t mid = (BOX_X1 + BOX_X2) / 2;
  int32_t bx1 = mid - half;
  int32_t bx2 = mid + half;
  int32_t by1 = s_lay->boxY;
  int32_t by2 = s_lay->boxY + s_lay->boxH;
  du::box(layer, bx1, by1, bx2, by2, C(theme::team()), LV_OPA_COVER, r);
  du::box(layer, bx1 + S(2), by1 + S(2), bx2 - S(2), by2 - S(2), C(theme::teamDeep()), LV_OPA_COVER, r - S(2));
  float shown = clamp01((q - 0.45f) / 0.55f);
  if (shown <= 0) return;
  lv_opa_t opa = static_cast<lv_opa_t>(I(255 * shown));
  du::Clip clip(layer, bx1 + S(2), by1 + S(2), bx2 - S(2), by2 - S(2));
  if (clip.empty()) return;
  int32_t total = s_cellHW + (s_cellA1 - s_sepX) + s_cellAW;
  int32_t room = (BOX_X2 - BOX_X1 + 1 - total) / 2 - S(18) - S(8);
  const lv_font_t* fl = theme::fontL();
  if (du::width(fl, s_homeAbbr) > room || du::width(fl, s_awayAbbr) > room) fl = theme::fontM();
  int32_t ly = by1 + (by2 - by1 + 1 - lv_font_get_line_height(fl)) / 2;
  du::text(layer, fl, C(s_usHome ? theme::team() : theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, BOX_X1 + S(18), ly, BOX_X2,
           s_homeAbbr, opa);
  du::text(layer, fl, C(s_usHome ? theme::C_TEXT2 : theme::team()), LV_TEXT_ALIGN_RIGHT, BOX_X1, ly, BOX_X2 - S(18),
           s_awayAbbr, opa);
  drawScore(layer, t, by1, by2, opa);
}

void drawFoot(lv_layer_t* layer) {
  const lv_font_t* fs = theme::fontS();
  int32_t lh = lv_font_get_line_height(fs);
  int32_t rowH = s_logo ? LEAGUE_PX : lh;
  if (!du::visible(layer, s_lay->footY, s_lay->footY + rowH)) return;
  int32_t gap = s_logo ? LEAGUE_PX + S(6) : 0;
  int32_t w = gap + du::width(fs, s_foot);
  int32_t x = SW / 2 - w / 2;
  if (s_logo) du::image(layer, s_logo, x, s_lay->footY);
  du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x + gap, s_lay->footY + (rowH - lh) / 2, SW - 1, s_foot);
}

void drawLetters(lv_layer_t* layer, uint32_t t) {
  const lv_font_t* fh = theme::fontHero();
  int32_t sx = 0;
  bool lit = shine(t, sx);
  for (int i = 0; i < LETTERS; i++) {
    int32_t y;
    lv_opa_t opa;
    uint32_t col;
    if (!letter(i, t, y, opa, col)) continue;
    if (!inClip(layer, letterArea(i, y))) continue;
    du::text(layer, fh, C(col), LV_TEXT_ALIGN_LEFT, s_letterX[i], y, s_letterX[i + 1] + S(8), kLetters[i], opa);
    if (!lit || s_letterX[i + 1] < sx - SHINE_W || s_letterX[i] > sx + SHINE_W) continue;
    du::Clip band(layer, sx - SHINE_W / 2, 0, sx + SHINE_W / 2, SH - 1);
    if (!band.empty()) {
      du::text(layer, fh, C(theme::C_WHITE), LV_TEXT_ALIGN_LEFT, s_letterX[i], y, s_letterX[i + 1] + S(8), kLetters[i],
               LV_OPA_80);
    }
  }
}

void drawPanel(lv_layer_t* layer, uint32_t t) {
  uint32_t at = kPlans[s_var].phase2 + PANEL_DELAY_MS;
  if (t < at || !inClip(layer, panelArea())) return;
  int32_t dx = panelShift(t);
  int32_t r = PHOTO_D / 2;
  int32_t cx = PHOTO_X + dx + r;
  int32_t cy = PHOTO_Y + r;
  du::box(layer, cx - r - RING_W, cy - r - RING_W, cx + r + RING_W, cy + r + RING_W, C(theme::team()), LV_OPA_COVER,
          LV_RADIUS_CIRCLE);
  du::box(layer, cx - r, cy - r, cx + r, cy + r, C(theme::teamDeep()), LV_OPA_COVER, LV_RADIUS_CIRCLE);
  if (s_photo) {
    lv_draw_image_dsc_t d;
    lv_draw_image_dsc_init(&d);
    d.src = s_photo;
    d.clip_radius = LV_RADIUS_CIRCLE;
    int32_t w = s_photo->header.w;
    int32_t h = s_photo->header.h;
    lv_area_t a = {cx - w / 2, cy - h / 2, cx - w / 2 + w - 1, cy - h / 2 + h - 1};
    du::Clip clip(layer, cx - r, cy - r, cx + r, cy + r);
    if (!clip.empty()) lv_draw_image(layer, &d, &a);
  } else {
    const lv_font_t* fl = theme::fontL();
    du::text(layer, fl, C(theme::team()), LV_TEXT_ALIGN_CENTER, cx - r, cy - lv_font_get_line_height(fl) / 2, cx + r,
             s_initials);
  }
  int32_t x = INFO_X + dx;
  int32_t x2 = SW - S(8) + dx;
  du::text(layer, theme::fontM(), C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, x, INFO_Y + S(4), x2, s_first);
  du::text(layer, theme::fontL(), C(theme::C_WHITE), LV_TEXT_ALIGN_LEFT, x, INFO_Y + S(24), x2, s_last);
  du::text(layer, theme::fontM(), C(theme::team()), LV_TEXT_ALIGN_LEFT, x, INFO_Y + S(58), x2, s_detail);
}

void drawContent(lv_layer_t* layer, uint32_t t) {
  drawLetters(layer, t);
  drawPanel(layer, t);
  drawBox(layer, t);
  drawFoot(layer);
}

void drawScene(lv_layer_t* layer, uint32_t t) {
  du::box(layer, 0, 0, SW - 1, SH - 1, C(theme::teamBg()), LV_OPA_COVER, 0);
  drawStripes(layer);
  if (s_var == V_NET && t < NET_END) drawNet(layer, t);
  if (s_var == V_FIREWORKS) drawFireworks(layer, t);
  if (s_var == V_RUMY && t < kPlans[s_var].phase2) {
    du::box(layer, PANEL_X1, PANEL_Y1, PANEL_X2, PANEL_Y2, C(theme::team()), LV_OPA_COVER, S(16));
    du::box(layer, PANEL_X1 + S(2), PANEL_Y1 + S(2), PANEL_X2 - S(2), PANEL_Y2 - S(2), C(theme::teamDeep()), LV_OPA_COVER,
            S(14));
  }
  drawContent(layer, t);
  int32_t bx, by;
  int f;
  if (ball(t, bx, by, f) && inClip(layer, ballArea(bx, by))) {
    du::image(layer, BALL_IMAGES[f], bx - BALL_PX / 2, by - BALL_PX / 2);
  }
  drawConfetti(layer, t);
}

void drawCb(lv_event_t* ev) {
  lv_layer_t* layer = lv_event_get_layer(ev);
  uint32_t t = s_exitAt ? s_exitAt : s_now;
  int32_t x1 = 0;
  int32_t x2 = SW - 1;
  bool band = false;
  int32_t bx = 0;
  if (s_exitAt) {
    bx = bandX(prog(s_now - s_exitAt, 0, EXIT_MS));
    x1 = bx + BAND_SLANT - BAND_W / 2;
    band = true;
  } else if (s_now < WIPE_MS) {
    bx = bandX(prog(s_now, 0, WIPE_MS));
    x2 = bx + BAND_W / 2;
    band = true;
  }
  {
    du::Clip clip(layer, x1, 0, x2, SH - 1);
    if (!clip.empty()) drawScene(layer, t);
  }
  if (band) drawBand(layer, bx);
}

void hiddenGone(lv_event_t* e) {
  (void)e;
  s_hidden = nullptr;
}

void hideScreen() {
  lv_obj_t* scr = lv_screen_active();
  if (!scr || s_hidden) return;
  if (scr->flags & LV_OBJ_FLAG_HIDDEN) return;
  s_hidden = scr;
  lv_obj_add_event_cb(scr, hiddenGone, LV_EVENT_DELETE, nullptr);
  scr->flags = static_cast<lv_obj_flag_t>(scr->flags | LV_OBJ_FLAG_HIDDEN);
}

void showScreen() {
  if (!s_hidden) return;
  lv_obj_remove_event_cb(s_hidden, hiddenGone);
  s_hidden->flags = static_cast<lv_obj_flag_t>(s_hidden->flags & ~LV_OBJ_FLAG_HIDDEN);
  lv_obj_invalidate(s_hidden);
  s_hidden = nullptr;
}

void makeFace() {
  int32_t in = S(4);
  s_face = rumy::create(s_obj, PANEL_X2 - PANEL_X1 - 2 * in, PANEL_Y2 - PANEL_Y1 - 2 * in, FACE_EYE);
  lv_obj_set_pos(s_face, PANEL_X1 + in, PANEL_Y1 + in);
  rumy::setBackground(s_face, C(theme::teamDeep()));
  lv_obj_remove_flag(s_face, LV_OBJ_FLAG_CLICKABLE);
  rumy::setColor(s_face, C(theme::team()));
  rumy::setMood(s_face, rumy::CELEBRATE);
  rumy::setAnimated(s_face, true);
}

void finish() {
  if (s_timer) lv_timer_delete(s_timer);
  s_timer = nullptr;
  showScreen();
  if (s_obj) lv_obj_delete(s_obj);
  s_obj = nullptr;
  s_face = nullptr;
  s_exitAt = 0;
}

void tickCb(lv_timer_t* timer) {
  (void)timer;
  if (!s_obj) return;
  uint32_t prev = s_now;
  uint32_t now = lv_tick_elaps(s_start);
  if (now <= prev) return;
  s_now = now;
  dirtyReset();
  if (s_exitAt) {
    uint32_t xp = prev > s_exitAt ? prev - s_exitAt : 0;
    uint32_t xn = now - s_exitAt;
    markBand(bandX(prog(xp, 0, EXIT_MS)), bandX(prog(xn, 0, EXIT_MS)));
    if (xn >= EXIT_MS + FRAME_MS) {
      finish();
      return;
    }
  } else {
    if (prev < WIPE_MS) markBand(bandX(prog(prev, 0, WIPE_MS)), bandX(prog(now, 0, WIPE_MS)));
    if (prev < WIPE_MS && now >= WIPE_MS) {
      hideScreen();
      if (s_var == V_RUMY) makeFace();
    }
    uint32_t p2 = kPlans[s_var].phase2;
    if (s_face && prev < p2 && now >= p2) {
      lv_obj_delete(s_face);
      s_face = nullptr;
    }
    markElements(prev, now);
  }
  dirtyFlush();
}

void fitTo(const lv_font_t* f, const char* in, char* out, size_t cap) {
  char buf[48];
  const char* r = du::fit(f, in, SW - INFO_X - S(10), buf, sizeof(buf));
  snprintf(out, cap, "%s", r);
}

size_t utf8Len(const char* p) {
  uint8_t c = static_cast<uint8_t>(*p);
  return c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
}

void splitName(const char* name, const char* team) {
  const char* sp = strrchr(name, ' ');
  char first[40];
  if (sp) {
    snprintf(first, sizeof(first), "%.*s", static_cast<int>(sp - name), name);
    fitTo(theme::fontM(), first, s_first, sizeof(s_first));
    fitTo(theme::fontL(), sp + 1, s_last, sizeof(s_last));
  } else {
    s_first[0] = 0;
    fitTo(theme::fontL(), name[0] ? name : (team[0] ? team : rc::L("Gol", "Goal")), s_last, sizeof(s_last));
  }
  size_t n = 0;
  if (sp && name[0]) {
    size_t k = utf8Len(name);
    memcpy(s_initials, name, k);
    n = k;
  }
  const char* last = sp ? sp + 1 : name;
  if (last[0]) {
    size_t k = utf8Len(last);
    memcpy(s_initials + n, last, k);
    n += k;
  }
  s_initials[n] = 0;
}

void prepare(const rc::Match& m, const rc::MatchEvent& e) {
  s_usHome = m.usHome == 1;
  snprintf(s_homeAbbr, sizeof(s_homeAbbr), "%s", m.home.abbr);
  snprintf(s_awayAbbr, sizeof(s_awayAbbr), "%s", m.away.abbr);
  snprintf(s_homeNum, sizeof(s_homeNum), "%u", static_cast<unsigned>(e.home));
  snprintf(s_awayNum, sizeof(s_awayNum), "%u", static_cast<unsigned>(e.away));
  unsigned ours = s_usHome ? e.home : e.away;
  s_roll = ours > 0;
  snprintf(s_oldNum, sizeof(s_oldNum), "%u", ours > 0 ? ours - 1 : 0);
  snprintf(s_name, sizeof(s_name), "%s", e.player);
  s_photo = nullptr;
  s_photoData = nullptr;
  splitName(e.player, s_usHome ? m.home.name : m.away.name);
  if (e.kind == rc::MK_OWN_GOAL) snprintf(s_detail, sizeof(s_detail), rc::L("%s · Kendi kalesine", "%s · Own goal"), e.minute);
  else if (e.kind == rc::MK_PEN_GOAL) snprintf(s_detail, sizeof(s_detail), rc::L("%s · Penaltı", "%s · Penalty"), e.minute);
  else snprintf(s_detail, sizeof(s_detail), rc::L("%s · Gol", "%s · Goal"), e.minute);
  const char* fitted = nullptr;
  s_logo = match_ui::leagueLogo(m.league);
  char foot[72];
  snprintf(foot, sizeof(foot), rc::L("%s · dokununca kapanır", "%s · tap to close"), rc::matchLeagueTr(m.league));
  int32_t room = SW - S(16) - (s_logo ? LEAGUE_PX + S(6) : 0);
  fitted = du::fit(theme::fontS(), foot, room, s_foot, sizeof(s_foot));
  if (fitted != s_foot) snprintf(s_foot, sizeof(s_foot), "%s", fitted);
  const lv_font_t* fg = theme::fontHero();
  int32_t total = du::width(fg, "GOOOL!");
  int32_t x0 = SW / 2 - total / 2;
  char prefix[8];
  s_letterX[0] = x0;
  for (int i = 1; i <= LETTERS; i++) {
    memcpy(prefix, "GOOOL!", static_cast<size_t>(i));
    prefix[i] = 0;
    s_letterX[i] = x0 + du::width(fg, prefix);
  }
  const lv_font_t* ft = theme::fontTemp();
  int32_t wh = du::width(ft, s_homeNum);
  int32_t wa = du::width(ft, s_awayNum);
  int32_t wo = du::width(ft, s_oldNum);
  if (s_usHome && wo > wh) wh = wo;
  if (!s_usHome && wo > wa) wa = wo;
  int32_t ws = du::width(ft, " - ");
  int32_t x = SW / 2 - (wh + ws + wa) / 2;
  s_cellH1 = x;
  s_cellHW = wh;
  s_sepX = x + wh;
  s_cellA1 = x + wh + ws;
  s_cellAW = wa;
}

}

void open(const rc::Match& m, const rc::MatchEvent& e, uint8_t variant, lv_event_cb_t clicked) {
  kill();
  s_var = variant < V_COUNT ? variant : static_cast<uint8_t>(V_LETTERS);
  s_lay = s_var == V_RUMY ? &kRumy : &kStd;
  s_seed = hash(e.id ^ (static_cast<uint32_t>(e.seq) << 16) ^ lv_tick_get());
  prepare(m, e);
  photo::request(e.player);
  s_obj = lv_obj_create(lv_layer_top());
  lv_obj_remove_style_all(s_obj);
  lv_obj_set_pos(s_obj, 0, 0);
  lv_obj_set_size(s_obj, SW, SH);
  lv_obj_remove_flag(s_obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(s_obj, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_add_event_cb(s_obj, drawCb, LV_EVENT_DRAW_MAIN, nullptr);
  if (clicked) lv_obj_add_event_cb(s_obj, clicked, LV_EVENT_CLICKED, nullptr);
  s_start = lv_tick_get();
  s_now = 0;
  s_exitAt = 0;
  s_timer = lv_timer_create(tickCb, FRAME_MS, nullptr);
}

void leave() {
  if (!s_obj || s_exitAt) return;
  uint32_t now = lv_tick_elaps(s_start);
  s_exitAt = now ? now : 1;
  s_now = s_exitAt;
  if (s_face) {
    lv_obj_delete(s_face);
    s_face = nullptr;
  }
  showScreen();
}

void kill() {
  if (!s_obj) return;
  finish();
}

bool active() { return s_obj != nullptr; }

bool leaving() { return s_obj && s_exitAt; }

const char* variantName(uint8_t variant) { return variant < V_COUNT ? kNames[variant] : "?"; }

}
