#include <stdio.h>
#include <string.h>

#include "app/config.h"
#include "draw_util.h"
#include "fonts/symbols.h"
#include "media/media.h"
#include "screens.h"
#include "theme.h"
#include "ui.h"
#include "text_tr.h"

namespace scr_playlist {

namespace {

using theme::S;

constexpr int32_t HEAD_H = S(38);
constexpr int32_t BACK_W = S(200);
constexpr int32_t MIX_W = S(88);
constexpr int32_t MIX_H = S(28);
constexpr int32_t MIX_X = theme::SW - S(7) - MIX_W;
constexpr int32_t MIX_Y = S(6);
constexpr int32_t VIEW_H = theme::SH - HEAD_H;
constexpr int32_t ROW_H = S(48);
constexpr int32_t ROW_INSET = S(4);
constexpr int32_t COVER = cfg::MEDIA_PL_COVER;
constexpr int32_t COVER_X = S(10);
constexpr int32_t NAME_X = S(60);
constexpr int32_t NAME_Y = S(4);
constexpr int32_t SUB_Y = S(26);
constexpr int32_t ICON_W = S(36);
constexpr int32_t SPIN_R = S(9);
constexpr uint32_t SPIN_MS = 60;
constexpr uint32_t C_SPOTIFY = 0x1ED760;

lv_obj_t* s_scr = nullptr;
lv_obj_t* s_back = nullptr;
lv_obj_t* s_mix = nullptr;
lv_obj_t* s_cont = nullptr;
lv_obj_t* s_list = nullptr;
lv_timer_t* s_timer = nullptr;
uint32_t s_started = 0;
bool s_scrolled = false;

lv_color_t C(uint32_t hex) { return theme::col(hex); }

int32_t textW(const lv_font_t* f, const char* s) {
  lv_point_t p;
  lv_text_get_size(&p, s, f, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_EXPAND);
  return p.x;
}

const char* fit(const lv_font_t* f, const char* s, int32_t w, char* buf, size_t n) {
  if (textW(f, s) <= w) return s;
  size_t len = strlen(s);
  if (len + 4 > n) len = n - 4;
  while (len > 0) {
    do {
      len--;
    } while (len > 0 && (static_cast<uint8_t>(s[len]) & 0xC0) == 0x80);
    size_t cut = len;
    while (cut > 0 && s[cut - 1] == ' ') cut--;
    snprintf(buf, n, "%.*s…", static_cast<int>(cut), s);
    if (textW(f, buf) <= w) return buf;
  }
  return "…";
}

void coords(lv_event_t* e, lv_area_t& c) { lv_obj_get_coords(lv_event_get_current_target_obj(e), &c); }

bool starting(const media::Playlists& p, int i) { return p.pick == i && p.pickResult == rc::PLR_STARTING; }

bool failed(const media::Playlists& p, int i) { return p.pick == i && p.pickResult == rc::PLR_FAIL; }

bool marked(const media::Playlists& p, int i) { return p.pick == i || (p.pick < 0 && p.playing == i); }

void drawBack(lv_event_t* e) {
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  coords(e, c);
  const lv_font_t* f = theme::fontM();
  int32_t y = c.y1 + (HEAD_H - lv_font_get_line_height(f)) / 2;
  du::text(layer, f, C(theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, c.x1 + S(10), y, c.x1 + S(30), RS_LEFT);
  du::text(layer, f, C(theme::C_TEXT), LV_TEXT_ALIGN_LEFT, c.x1 + S(32), y, c.x2, rc::L("Çalma listelerim", "My playlists"));
}

void drawMix(lv_event_t* e) {
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  coords(e, c);
  bool on = media::playlists().shuffle;
  int32_t r = MIX_H / 2;
  du::box(layer, c.x1, c.y1, c.x2, c.y2, C(on ? C_SPOTIFY : theme::C_BORDER), LV_OPA_COVER, r);
  du::box(layer, c.x1 + 1, c.y1 + 1, c.x2 - 1, c.y2 - 1, C(theme::C_SURFACE2), LV_OPA_COVER, r - 1);
  const lv_font_t* f = theme::fontS();
  int32_t y = c.y1 + (MIX_H - lv_font_get_line_height(f)) / 2;
  du::text(layer, f, C(on ? C_SPOTIFY : theme::C_TEXT3), LV_TEXT_ALIGN_LEFT, c.x1 + S(10), y, c.x1 + S(28), RS_RANDOM);
  du::text(layer, f, C(on ? theme::C_TEXT : theme::C_TEXT2), LV_TEXT_ALIGN_LEFT, c.x1 + S(29), y, c.x2, rc::L("Karışık", "Shuffle"));
}

void drawCover(lv_layer_t* layer, int i, int32_t x, int32_t y) {
  int32_t dx = 0;
  int32_t dy = 0;
  const lv_image_dsc_t* sheet = media::playlistSheet(i, dx, dy);
  if (sheet) {
    du::Clip clip(layer, x, y, x + COVER - 1, y + COVER - 1);
    if (!clip.empty()) du::image(layer, sheet, x - dx, y - dy);
    return;
  }
  du::box(layer, x, y, x + COVER - 1, y + COVER - 1, C(theme::C_SURFACE2), LV_OPA_COVER, S(4));
  const lv_font_t* f = theme::fontM();
  du::text(layer, f, C(theme::C_TEXT3), LV_TEXT_ALIGN_CENTER, x, y + (COVER - lv_font_get_line_height(f)) / 2,
           x + COVER - 1, RS_MUSIC);
}

void drawSpinner(lv_layer_t* layer, int32_t cx, int32_t cy) {
  lv_draw_arc_dsc_t a;
  lv_draw_arc_dsc_init(&a);
  a.color = C(C_SPOTIFY);
  a.width = S(3);
  a.center.x = cx;
  a.center.y = cy;
  a.radius = static_cast<uint16_t>(SPIN_R);
  int32_t start = static_cast<int32_t>(lv_tick_get() / 4 % 360);
  a.start_angle = start;
  a.end_angle = start + 270;
  a.rounded = 1;
  lv_draw_arc(layer, &a);
}

void drawRow(lv_layer_t* layer, const lv_area_t& c, const media::Playlists& p, int i, int32_t y) {
  const media::PlaylistItem& it = p.item[i];
  bool start = starting(p, i);
  bool fail = failed(p, i);
  bool playing = p.pick < 0 && p.playing == i;
  if (marked(p, i)) {
    int32_t x1 = c.x1 + ROW_INSET;
    int32_t x2 = c.x2 - ROW_INSET;
    int32_t y1 = y + S(1);
    int32_t y2 = y + ROW_H - S(2);
    if (p.pick == i) {
      du::box(layer, x1, y1, x2, y2, C(fail ? theme::C_NEON_RED : C_SPOTIFY), LV_OPA_COVER, S(8));
      du::box(layer, x1 + 1, y1 + 1, x2 - 1, y2 - 1, C(theme::C_SURFACE2), LV_OPA_COVER, S(8) - 1);
    } else {
      du::box(layer, x1, y1, x2, y2, C(theme::C_SURFACE2), LV_OPA_COVER, S(8));
    }
  }
  drawCover(layer, i, c.x1 + COVER_X, y + (ROW_H - COVER) / 2);
  int32_t right = c.x2 - (playing || start ? ICON_W : S(10));
  char buf[64];
  const char* sub = buf;
  uint32_t subColor = theme::C_TEXT2;
  if (start) {
    sub = rc::L("Bilgisayarda başlatılıyor…", "Starting on the computer…");
    subColor = C_SPOTIFY;
  } else if (fail) {
    sub = p.pickMsg[0] ? p.pickMsg : rc::L("Başlatılamadı", "Couldn't start");
    subColor = theme::C_NEON_RED;
  } else {
    snprintf(buf, sizeof(buf), rc::L("%u şarkı%s", "%u songs%s"), static_cast<unsigned>(it.total),
             playing ? (media::state().playing ? rc::L(" · çalıyor", " · playing") : rc::L(" · duraklatıldı", " · paused")) : "");
  }
  {
    du::Clip clip(layer, c.x1 + NAME_X, y, right, y + ROW_H - 1);
    if (!clip.empty()) {
      int32_t w = right - (c.x1 + NAME_X) + 1;
      char fitted[64];
      char clean[sizeof(it.name)];
      const char* name = theme::printable(theme::fontM(), it.name, clean, sizeof(clean), rc::L("Bilinmeyen ad", "Unknown title"));
      du::text(layer, theme::fontM(), C(playing ? C_SPOTIFY : theme::C_TEXT), LV_TEXT_ALIGN_LEFT, c.x1 + NAME_X,
               y + NAME_Y, right, fit(theme::fontM(), name, w, fitted, sizeof(fitted)));
      du::text(layer, theme::fontS(), C(subColor), LV_TEXT_ALIGN_LEFT, c.x1 + NAME_X, y + SUB_Y, right,
               fit(theme::fontS(), sub, w, fitted, sizeof(fitted)));
    }
  }
  if (playing) {
    const lv_font_t* f = theme::fontM();
    du::text(layer, f, C(C_SPOTIFY), LV_TEXT_ALIGN_RIGHT, c.x1, y + (ROW_H - lv_font_get_line_height(f)) / 2,
             c.x2 - S(14), RS_VOL_UP);
  }
  if (start) drawSpinner(layer, c.x2 - S(24), y + ROW_H / 2);
  if (!marked(p, i) && i + 1 < p.n && !marked(p, i + 1))
    du::box(layer, c.x1 + NAME_X, y + ROW_H - 1, c.x2 - S(8), y + ROW_H - 1, C(theme::C_SURFACE2), LV_OPA_COVER, 0);
}

void drawEmpty(lv_layer_t* layer, const lv_area_t& c, const media::Playlists& p) {
  const lv_font_t* fs = theme::fontS();
  const lv_font_t* fm = theme::fontM();
  const lv_font_t* fl = theme::fontL();
  int32_t mid = c.y1 + VIEW_H / 2;
  if (p.status == rc::PLS_LOGIN) {
    int32_t y = c.y1 + S(18);
    du::text(layer, fl, C(C_SPOTIFY), LV_TEXT_ALIGN_CENTER, c.x1, y, c.x2, RS_SPOTIFY);
    y += lv_font_get_line_height(fl) + S(10);
    du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_CENTER, c.x1, y, c.x2, rc::L("Spotify hesabı bağlı değil", "Spotify account not linked"));
    y += lv_font_get_line_height(fm) + S(8);
    du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, y, c.x2, rc::L("Bilgisayarda bir kez giriş yapılınca", "After you sign in once on the computer,"));
    y += lv_font_get_line_height(fs) + S(2);
    du::text(layer, fs, C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, y, c.x2, rc::L("listelerin burada görünür.", "your playlists appear here."));
    y += lv_font_get_line_height(fs) + S(12);
    int32_t bh = S(30);
    du::box(layer, c.x1 + S(40), y, c.x2 - S(40), y + bh, C(theme::C_BORDER), LV_OPA_COVER, S(10));
    du::box(layer, c.x1 + S(40) + 1, y + 1, c.x2 - S(40) - 1, y + bh - 1, C(theme::C_SURFACE2), LV_OPA_COVER, S(10) - 1);
    du::text(layer, fs, C(theme::C_TEXT), LV_TEXT_ALIGN_CENTER, c.x1, y + (bh - lv_font_get_line_height(fs)) / 2,
             c.x2, "spotify_bridge.py --spotify-giris");
    return;
  }
  if (p.status == rc::PLS_FAIL) {
    int32_t y = mid - lv_font_get_line_height(fl) - S(4);
    du::text(layer, fl, C(theme::C_NEON_RED), LV_TEXT_ALIGN_CENTER, c.x1, y, c.x2, RS_WARNING);
    du::text(layer, fm, C(theme::C_TEXT), LV_TEXT_ALIGN_CENTER, c.x1, mid + S(4), c.x2,
             p.msg[0] ? p.msg : rc::L("Listeler alınamadı", "Couldn't load playlists"));
    return;
  }
  const char* txt = rc::L("Listeler yükleniyor…", "Loading playlists…");
  if (!media::state().link) txt = rc::L("Bilgisayar bağlı değil", "Computer not connected");
  else if (p.status == rc::PLS_READY) txt = rc::L("Spotify'da çalma listesi yok", "No playlists in Spotify");
  du::text(layer, fm, C(theme::C_TEXT2), LV_TEXT_ALIGN_CENTER, c.x1, mid - lv_font_get_line_height(fm), c.x2, txt);
}

void drawList(lv_event_t* e) {
  lv_layer_t* layer = lv_event_get_layer(e);
  lv_area_t c;
  coords(e, c);
  const media::Playlists& p = media::playlists();
  if (!p.n) {
    drawEmpty(layer, c, p);
    return;
  }
  for (int i = 0; i < p.n; i++) {
    int32_t y = c.y1 + i * ROW_H;
    if (du::visible(layer, y, y + ROW_H)) drawRow(layer, c, p, i, y);
  }
}

void layoutList() {
  const media::Playlists& p = media::playlists();
  int32_t h = p.n ? p.n * ROW_H + S(4) : VIEW_H;
  if (lv_obj_get_height(s_list) != h) lv_obj_set_height(s_list, h);
  if (!s_scrolled && p.n && p.playing >= 0) {
    s_scrolled = true;
    lv_obj_update_layout(s_cont);
    lv_obj_scroll_to_y(s_cont, LV_MAX(0, (p.playing - 1) * ROW_H), LV_ANIM_OFF);
  }
}

void backClick(lv_event_t* e) {
  (void)e;
  ui::go(ui::SCR_MEDIA, -1);
}

void mixClick(lv_event_t* e) {
  (void)e;
  media::setShuffle(!media::playlists().shuffle);
}

void listClick(lv_event_t* e) {
  lv_area_t c;
  coords(e, c);
  lv_point_t pt;
  lv_indev_get_point(lv_indev_active(), &pt);
  if (pt.y < c.y1) return;
  int i = static_cast<int>((pt.y - c.y1) / ROW_H);
  if (i >= media::playlists().n) return;
  media::playPlaylist(i);
}

void spinTick(lv_timer_t* t) {
  (void)t;
  const media::Playlists& p = media::playlists();
  if (!s_list || p.pick < 0 || p.pickResult != rc::PLR_STARTING) return;
  lv_area_t c;
  lv_obj_get_coords(s_list, &c);
  int32_t cy = c.y1 + p.pick * ROW_H + ROW_H / 2;
  lv_area_t a = {c.x2 - S(24) - SPIN_R - S(3), cy - SPIN_R - S(3), c.x2 - S(24) + SPIN_R + S(3), cy + SPIN_R + S(3)};
  lv_obj_invalidate_area(s_list, &a);
}

lv_obj_t* drawObj(lv_obj_t* parent, int32_t x, int32_t y, int32_t w, int32_t h, lv_event_cb_t draw,
                  lv_event_cb_t click) {
  lv_obj_t* o = theme::makeBox(parent);
  lv_obj_set_pos(o, x, y);
  lv_obj_set_size(o, w, h);
  lv_obj_add_event_cb(o, draw, LV_EVENT_DRAW_MAIN, nullptr);
  if (click) {
    lv_obj_add_flag(o, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(o, click, LV_EVENT_CLICKED, nullptr);
  } else {
    lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
  }
  return o;
}

void deleted(lv_event_t* e) {
  (void)e;
  media::playlistsOpen(false);
  if (s_timer) lv_timer_delete(s_timer);
  s_timer = nullptr;
  s_scr = s_back = s_mix = s_cont = s_list = nullptr;
}

}

lv_obj_t* create() {
  s_scr = theme::makeScreen();
  ui::attachGestures(s_scr);
  lv_obj_add_event_cb(s_scr, deleted, LV_EVENT_DELETE, nullptr);
  s_back = drawObj(s_scr, 0, 0, BACK_W, HEAD_H, drawBack, backClick);
  s_mix = drawObj(s_scr, MIX_X, MIX_Y, MIX_W, MIX_H, drawMix, mixClick);
  lv_obj_set_ext_click_area(s_mix, S(6));
  s_cont = theme::makeBox(s_scr);
  lv_obj_set_pos(s_cont, 0, HEAD_H);
  lv_obj_set_size(s_cont, theme::SW, VIEW_H);
  lv_obj_add_flag(s_cont, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(s_cont, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(s_cont, LV_SCROLLBAR_MODE_OFF);
  s_list = drawObj(s_cont, 0, 0, theme::SW, VIEW_H, drawList, listClick);
  s_started = media::playlists().started;
  s_scrolled = false;
  media::playlistsOpen(true);
  s_timer = lv_timer_create(spinTick, SPIN_MS, nullptr);
  layoutList();
  return s_scr;
}

void update(uint32_t mask) {
  if (!s_scr || !(mask & ui::M_MEDIA)) return;
  const media::Playlists& p = media::playlists();
  if (p.started != s_started) {
    s_started = p.started;
    ui::go(ui::SCR_MEDIA, -1);
    return;
  }
  layoutList();
  lv_obj_invalidate(s_mix);
  lv_obj_invalidate(s_list);
}

}
