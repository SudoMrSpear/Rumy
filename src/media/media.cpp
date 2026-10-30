#include "media.h"

#include <Arduino.h>
#include <esp_partition.h>
#include <esp_rom_crc.h>
#include <stdlib.h>
#include <string.h>

#include "afad.h"
#include "app/config.h"
#include "bridge.h"
#include "camera.h"
#include "diag/diag.h"
#include "fwupdate.h"
#include "agents/agents.h"
#include "match/fast.h"
#include "net/relay.h"
#include "media_proto.h"
#include "settings/settings.h"
#include "text_tr.h"

namespace media {

namespace {

constexpr size_t HDR = rc::ART_HDR;
constexpr int MAX_SLOTS = 24;
constexpr uint32_t SECTOR = 4096;
constexpr uint32_t NEED_RETRY_MS = 5000;
constexpr uint32_t SEEK_SLACK_MS = 1500;

struct Rx {
  bool on;
  int slot;
  uint32_t hash;
  uint16_t w;
  uint16_t h;
  uint32_t len;
  uint32_t got;
  uint32_t crc;
  uint32_t crcCalc;
  uint32_t erased;
  uint32_t lastByte;
  int replaces;
};

const esp_partition_t* s_part = nullptr;
const uint8_t* s_map = nullptr;
spi_flash_mmap_handle_t s_mapHandle = 0;
int s_slots = 0;
uint32_t s_slotHash[MAX_SLOTS];
uint32_t s_slotSeq[MAX_SLOTS];
uint16_t s_slotW[MAX_SLOTS];
uint16_t s_slotH[MAX_SLOTS];
uint32_t s_slotCrc[MAX_SLOTS];
uint32_t s_seq = 0;

State s_state;
uint32_t s_ver = 1;
uint32_t s_posMs = 0;
uint32_t s_posAt = 0;
uint32_t s_lastMsg = 0;
lv_image_dsc_t s_img;
int s_artSlot = -1;
uint32_t s_wantHash = 0;
uint32_t s_needHash = 0;
uint32_t s_needAt = 0;
uint32_t s_artOk = 0;
uint32_t s_artErr = 0;
bool s_quiet = false;
uint32_t s_quietAt = 0;
uint32_t s_quietStart = 0;
uint32_t s_dropped = 0;
uint32_t s_tempoPeriod = 0;
uint32_t s_tempoBeat = 0;
uint32_t s_tempoAt = 0;
uint16_t s_volPending = 0;
uint32_t s_volSentAt = 0;
Playlists s_pl;
lv_image_dsc_t s_sheetImg[rc::PL_SHEETS];
uint8_t s_plExpect = 0;
bool s_plOpen = false;
uint32_t s_sheetNeedHash = 0;
uint32_t s_sheetNeedAt = 0;
uint32_t s_photoHash = 0;
uint32_t s_photoNone = 0;
uint32_t s_photoAt = 0;
uint8_t s_photoTries = 0;
char s_photoName[48];
bool s_futSent = false;
uint32_t s_futVer = 0;
uint32_t s_futAt = 0;
lv_image_dsc_t s_photoImg;

char s_line[cfg::MEDIA_LINE];
size_t s_lineLen = 0;
bool s_lineOver = false;
Rx s_rx;

uint32_t slotOff(int slot) { return static_cast<uint32_t>(slot) * cfg::MEDIA_SLOT_BYTES; }

void copyField(char* dst, size_t cap, const char* src) { rc::utf8Copy(dst, cap, src, strlen(src)); }

int findSlot(uint32_t hash) {
  for (int i = 0; i < s_slots; i++)
    if (s_slotHash[i] == hash) return i;
  return -1;
}

bool isSheet(uint32_t hash) {
  if (!hash) return false;
  for (int k = 0; k < rc::PL_SHEETS; k++)
    if (s_pl.sheet[k] == hash) return true;
  return false;
}

int victimSlot() {
  int best = -1;
  for (int i = 0; i < s_slots; i++) {
    if (i == s_artSlot || isSheet(s_slotHash[i]) || (s_photoHash && s_slotHash[i] == s_photoHash)) continue;
    if (best < 0 || s_slotSeq[i] < s_slotSeq[best]) best = i;
  }
  return best;
}

void showSlot(int slot) {
  if (slot == s_artSlot) return;
  s_artSlot = slot;
  if (slot >= 0) {
    memset(&s_img, 0, sizeof(s_img));
    s_img.header.magic = LV_IMAGE_HEADER_MAGIC;
    s_img.header.cf = LV_COLOR_FORMAT_RGB565;
    s_img.header.w = s_slotW[slot];
    s_img.header.h = s_slotH[slot];
    s_img.header.stride = static_cast<uint32_t>(s_slotW[slot]) * 2U;
    s_img.data = s_map + slotOff(slot) + HDR;
    s_img.data_size = static_cast<uint32_t>(s_slotW[slot]) * s_slotH[slot] * 2U;
  }
  s_ver++;
}

void wantArt(uint32_t hash) {
  s_wantHash = hash;
  if (!hash) {
    showSlot(-1);
    return;
  }
  int slot = findSlot(hash);
  if (slot >= 0) {
    showSlot(slot);
    return;
  }
  showSlot(-1);
  if (!s_part || s_rx.on) return;
  if (s_needHash != hash || millis() - s_needAt >= NEED_RETRY_MS) {
    s_needHash = hash;
    s_needAt = millis();
    bridge::printf("@need %08lx\n", static_cast<unsigned long>(hash));
  }
}

void scanSlots() {
  s_slots = static_cast<int>(s_part->size / cfg::MEDIA_SLOT_BYTES);
  if (s_slots > MAX_SLOTS) s_slots = MAX_SLOTS;
  for (int i = 0; i < s_slots; i++) {
    rc::ArtSlotHdr h;
    s_slotHash[i] = 0;
    s_slotSeq[i] = 0;
    if (esp_partition_read(s_part, slotOff(i), &h, sizeof(h)) != ESP_OK) continue;
    if (!rc::artSlotValid(h, static_cast<uint16_t>(cfg::MEDIA_ART), cfg::MEDIA_SLOT_BYTES,
                          static_cast<uint16_t>(cfg::PHOTO_PX)))
      continue;
    s_slotHash[i] = h.hash;
    s_slotSeq[i] = h.seq;
    s_slotW[i] = h.w;
    s_slotH[i] = h.h;
    s_slotCrc[i] = h.crc;
    if (h.seq > s_seq) s_seq = h.seq;
  }
}

void abortRx(const char* why) {
  s_rx.on = false;
  s_artErr++;
  bridge::printf("@art err %s\n", why);
  discardUntilQuiet();
  diag::note("medya", "kapak alinamadi: %s", why);
}

void finishRx() {
  s_rx.on = false;
  if (s_rx.crcCalc != s_rx.crc) {
    s_artErr++;
    bridge::println("@art err crc");
    discardUntilQuiet();
    return;
  }
  rc::ArtSlotHdr h;
  h.magic = rc::ART_MAGIC;
  h.hash = s_rx.hash;
  h.seq = ++s_seq;
  h.w = s_rx.w;
  h.h = s_rx.h;
  h.crc = s_rx.crc;
  h.size = s_rx.len;
  if (esp_partition_write(s_part, slotOff(s_rx.slot), &h, sizeof(h)) != ESP_OK) {
    abortRx("baslik");
    return;
  }
  s_slotHash[s_rx.slot] = h.hash;
  s_slotSeq[s_rx.slot] = h.seq;
  s_slotW[s_rx.slot] = h.w;
  s_slotH[s_rx.slot] = h.h;
  s_slotCrc[s_rx.slot] = h.crc;
  int old = s_rx.replaces;
  if (old >= 0 && old != s_rx.slot && s_slotHash[old] == h.hash) {
    s_slotHash[old] = 0;
    s_slotSeq[old] = 0;
    esp_partition_erase_range(s_part, slotOff(old), SECTOR);
  }
  s_artOk++;
  bridge::printf("@art ok %08lx\n", static_cast<unsigned long>(h.hash));
  if (h.hash == s_wantHash) showSlot(s_rx.slot);
  if (isSheet(h.hash)) s_ver++;
}

bool eraseNext() {
  uint32_t upto = s_rx.got + cfg::MEDIA_CHUNK;
  if (upto > s_rx.len) upto = s_rx.len;
  uint32_t end = slotOff(s_rx.slot) + HDR + upto;
  while (s_rx.erased < end) {
    if (esp_partition_erase_range(s_part, s_rx.erased, SECTOR) != ESP_OK) {
      abortRx("silme");
      return false;
    }
    s_rx.erased += SECTOR;
  }
  s_rx.lastByte = millis();
  return true;
}

void startRx(const char* args) {
  if (!s_part || !s_map) {
    bridge::println("@art err bolum");
    return;
  }
  rc::ArtOffer o;
  if (!rc::parseArtOffer(args, cfg::MEDIA_SLOT_BYTES, o)) {
    bridge::println("@art err boyut");
    return;
  }
  uint32_t hash = o.hash;
  int have = findSlot(hash);
  if (have >= 0 && s_slotCrc[have] == o.crc) {
    bridge::printf("@art ok %08lx\n", static_cast<unsigned long>(hash));
    if (hash == s_wantHash) showSlot(have);
    return;
  }
  int slot = victimSlot();
  if (slot < 0) {
    bridge::println("@art err yer");
    return;
  }
  s_slotHash[slot] = 0;
  s_slotSeq[slot] = 0;
  memset(&s_rx, 0, sizeof(s_rx));
  s_rx.on = true;
  s_rx.slot = slot;
  s_rx.hash = hash;
  s_rx.w = o.w;
  s_rx.h = o.h;
  s_rx.len = o.len;
  s_rx.crc = o.crc;
  s_rx.replaces = have;
  s_rx.erased = slotOff(slot);
  if (!eraseNext()) return;
  bridge::printf("@go %08lx\n", static_cast<unsigned long>(hash));
}

void nowPlaying(char* p) {
  rc::NowPlaying np;
  rc::parseNowPlaying(p, np);
  bool changed = !s_state.link || np.active != s_state.active || np.playing != s_state.playing ||
                 np.durMs != s_state.durMs || np.accent != s_state.accent || np.track != s_state.track ||
                 strcmp(np.app, s_state.app) != 0;
  if (rc::seekJumped(np.posMs, positionMs(), SEEK_SLACK_MS)) changed = true;

  s_state.link = true;
  s_state.active = np.active;
  s_state.playing = np.playing;
  s_state.durMs = np.durMs;
  s_state.accent = np.accent;
  s_state.track = np.track;
  copyField(s_state.app, sizeof(s_state.app), np.app);
  copyField(s_state.title, sizeof(s_state.title), np.title);
  copyField(s_state.artist, sizeof(s_state.artist), np.artist);
  copyField(s_state.album, sizeof(s_state.album), np.album);
  s_posMs = np.posMs;
  s_posAt = millis();
  s_lastMsg = s_posAt;
  if (changed) s_ver++;
  if (np.artHash != s_wantHash || (np.artHash && s_artSlot < 0)) wantArt(np.artHash);
}

uint8_t sheetIndex(uint32_t hash) {
  if (!hash) return 0xFF;
  for (int k = 0; k < rc::PL_SHEETS; k++)
    if (s_pl.sheet[k] == hash) return static_cast<uint8_t>(k);
  for (int k = 0; k < rc::PL_SHEETS; k++) {
    if (s_pl.sheet[k]) continue;
    s_pl.sheet[k] = hash;
    return static_cast<uint8_t>(k);
  }
  return 0xFF;
}

void plHead(char* p) {
  rc::PlaylistHead h;
  if (!rc::parsePlaylistHead(p, h)) return;
  if (h.status != rc::PLS_READY || h.ver != s_pl.ver) {
    s_pl.n = 0;
    memset(s_pl.sheet, 0, sizeof(s_pl.sheet));
  }
  s_pl.ver = h.ver;
  s_pl.status = h.status;
  s_pl.shuffle = h.shuffle;
  s_plExpect = h.n;
  copyField(s_pl.msg, sizeof(s_pl.msg), h.msg);
  s_ver++;
}

void plItem(char* p) {
  rc::PlaylistLine l;
  if (!rc::parsePlaylistLine(p, l) || l.index > s_pl.n || l.index >= s_plExpect) return;
  PlaylistItem& it = s_pl.item[l.index];
  copyField(it.name, sizeof(it.name), l.name);
  it.total = l.total;
  it.cell = l.cell;
  it.sheet = sheetIndex(l.sheet);
  if (l.index == s_pl.n) s_pl.n++;
  s_ver++;
}

void plNow(const char* p) {
  int i = atoi(p);
  s_pl.playing = static_cast<int8_t>(i >= 0 && i < rc::PL_MAX ? i : -1);
  s_ver++;
}

void plReply(char* p) {
  rc::PlaylistReply r;
  if (!rc::parsePlaylistReply(p, r) || r.index < 0 || r.index != s_pl.pick) return;
  if (r.result == rc::PLR_OK) {
    s_pl.playing = s_pl.pick;
    s_pl.pick = -1;
    s_pl.pickResult = rc::PLR_NONE;
    s_pl.started++;
  } else {
    s_pl.pickResult = r.result;
    s_pl.pickAt = millis();
    copyField(s_pl.pickMsg, sizeof(s_pl.pickMsg), r.msg);
  }
  s_ver++;
}

void needSheets() {
  if (!s_plOpen || !s_part || s_rx.on) return;
  if (s_wantHash && s_artSlot < 0 && millis() - s_needAt < NEED_RETRY_MS) return;
  for (int k = 0; k < rc::PL_SHEETS; k++) {
    uint32_t h = s_pl.sheet[k];
    if (!h || findSlot(h) >= 0) continue;
    if (s_sheetNeedHash == h && millis() - s_sheetNeedAt < NEED_RETRY_MS) return;
    s_sheetNeedHash = h;
    s_sheetNeedAt = millis();
    bridge::printf("@need %08lx\n", static_cast<unsigned long>(h));
    return;
  }
}

void photoLoop() {
  if (!s_photoHash || s_photoNone == s_photoHash || s_photoTries >= cfg::PHOTO_TRIES) return;
  if (!s_part || s_rx.on || !s_state.link || (findSlot(s_photoHash) >= 0 && s_photoTries)) return;
  if (s_photoTries && millis() - s_photoAt < cfg::PHOTO_RETRY_MS) return;
  s_photoTries++;
  s_photoAt = millis();
  bridge::printf("@foto %08lx %d %s\n", static_cast<unsigned long>(s_photoHash), static_cast<int>(cfg::PHOTO_PX),
                 s_photoName);
}

void plLoop() {
  if (s_pl.pick >= 0 && s_pl.pickResult == rc::PLR_STARTING && millis() - s_pl.pickAt > cfg::MEDIA_PL_START_MS) {
    s_pl.pickResult = rc::PLR_FAIL;
    copyField(s_pl.pickMsg, sizeof(s_pl.pickMsg), rc::L("Bilgisayardan yanıt gelmedi", "No reply from the computer"));
    s_ver++;
  }
  needSheets();
}

void footballLoop() {
  if (!s_state.link) {
    s_futSent = false;
    return;
  }
  uint32_t ver = settings::version();
  if (s_futSent && ver == s_futVer && millis() - s_futAt < cfg::MEDIA_FOOTBALL_MS) return;
  Settings st = settings::get();
  char line[48];
  if (!rc::footballLine(line, sizeof(line), st.football != 0, st.teamId, st.teamC1, st.teamC2)) return;
  s_futSent = true;
  s_futVer = ver;
  s_futAt = millis();
  bridge::write(reinterpret_cast<const uint8_t*>(line), strlen(line));
}

void tempoLine(const char* p) {
  rc::TempoMsg t;
  if (!rc::parseTempo(p, t)) {
    s_tempoPeriod = 0;
    return;
  }
  s_tempoPeriod = t.periodMs;
  s_tempoBeat = millis() + t.nextMs;
  s_tempoAt = millis();
}

}

void begin() {
  memset(&s_state, 0, sizeof(s_state));
  s_state.vol = -1;
  memset(&s_pl, 0, sizeof(s_pl));
  s_pl.playing = -1;
  s_pl.pick = -1;
  memset(&s_rx, 0, sizeof(s_rx));
  s_part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, static_cast<esp_partition_subtype_t>(0x40), "art");
  if (!s_part) {
    diag::note("medya", "kapak bolumu yok, kapaklar gosterilmeyecek");
    return;
  }
  const void* p = nullptr;
  if (esp_partition_mmap(s_part, 0, s_part->size, SPI_FLASH_MMAP_DATA, &p, &s_mapHandle) != ESP_OK) {
    diag::note("medya", "kapak bolumu eslenemedi");
    s_part = nullptr;
    return;
  }
  s_map = static_cast<const uint8_t*>(p);
  scanSlots();
  int used = 0;
  for (int i = 0; i < s_slots; i++)
    if (s_slotHash[i]) used++;
  diag::note("medya", "kapak onbellegi %d/%d yuva", used, s_slots);
}

void loop() {
  uint32_t now = millis();
  if (s_rx.on && now - s_rx.lastByte > cfg::MEDIA_RX_TIMEOUT_MS) abortRx("zaman asimi");
  if (s_state.link && now - s_lastMsg > cfg::MEDIA_LINK_MS) {
    s_state.link = false;
    s_state.active = false;
    s_state.playing = false;
    s_state.vol = -1;
    s_volPending = 0;
    s_pl.status = rc::PLS_NONE;
    s_pl.pick = -1;
    s_pl.pickResult = rc::PLR_NONE;
    s_ver++;
  }
  plLoop();
  photoLoop();
  footballLoop();
}

void volLine(const char* p) {
  int v = -1;
  if (*p >= '0' && *p <= '9') v = LV_MIN(atoi(p), 100);
  if (s_volPending && millis() - s_volSentAt < cfg::MEDIA_VOL_SETTLE_MS && --s_volPending) return;
  s_volPending = 0;
  if (v == s_state.vol) return;
  s_state.vol = static_cast<int8_t>(v);
  s_ver++;
}

void lineBegin() {
  s_lineLen = 0;
  s_lineOver = false;
}

void lineChar(char c) {
  if (s_lineLen + 1 < sizeof(s_line)) s_line[s_lineLen++] = c;
  else s_lineOver = true;
}

void lineEnd() {
  s_line[s_lineLen] = 0;
  if (s_lineOver) return;
  if (!strncmp(s_line, "np\t", 3)) nowPlaying(s_line + 3);
  else if (!strncmp(s_line, "art ", 4)) startRx(s_line + 4);
  else if (!strncmp(s_line, "tempo ", 6)) tempoLine(s_line + 6);
  else if (!strncmp(s_line, "vol ", 4)) volLine(s_line + 4);
  else if (!strncmp(s_line, "pls ", 4)) plHead(s_line + 4);
  else if (!strncmp(s_line, "pl\t", 3)) plItem(s_line + 3);
  else if (!strncmp(s_line, "plnow ", 6)) plNow(s_line + 6);
  else if (!strncmp(s_line, "plst ", 5)) plReply(s_line + 5);
  else if (!strncmp(s_line, "cam ", 4)) cam::start(s_line + 4);
  else if (!strncmp(s_line, "foto yok ", 9)) s_photoNone = static_cast<uint32_t>(strtoul(s_line + 9, nullptr, 16));
  else if (!strncmp(s_line, "gol ", 4) || !strncmp(s_line, "skor ", 5)) fast::line(s_line);
  else if (!strncmp(s_line, "ajanlar ", 8)) agents::head(s_line + 8);
  else if (!strncmp(s_line, "ajan\t", 5)) agents::row(s_line + 5);
  else if (!strncmp(s_line, "guncelle ", 9)) fwupdate::offer(s_line + 9);
  else if (!strncmp(s_line, "deprem ", 7) || !strncmp(s_line, "bayram ", 7)) relay::line(s_line);
  else if (!strcmp(s_line, "ping")) bridge::println("@pong");
}

bool receiving() { return s_rx.on; }

void discardUntilQuiet() {
  s_quiet = true;
  s_quietAt = millis();
  s_quietStart = s_quietAt;
}

bool discarding() {
  if (!s_quiet) return false;
  uint32_t now = millis();
  if (now - s_quietAt >= cfg::MEDIA_QUIET_MS || now - s_quietStart >= cfg::MEDIA_QUIET_MAX_MS) s_quiet = false;
  return s_quiet;
}

void discard() {
  uint8_t sink[64];
  int avail = bridge::available();
  if (avail <= 0) return;
  size_t n = bridge::read(sink, static_cast<size_t>(avail) < sizeof(sink) ? static_cast<size_t>(avail) : sizeof(sink));
  s_dropped += static_cast<uint32_t>(n);
  s_quietAt = millis();
}

bool receive() {
  if (!s_rx.on) return false;
  uint32_t need = cfg::MEDIA_CHUNK - s_rx.got % cfg::MEDIA_CHUNK;
  if (need > s_rx.len - s_rx.got) need = s_rx.len - s_rx.got;
  if (static_cast<uint32_t>(bridge::available()) < need) return false;
  uint8_t buf[256];
  while (need) {
    size_t n = bridge::read(buf, need < sizeof(buf) ? need : sizeof(buf));
    if (!n) {
      abortRx("okuma");
      return true;
    }
    if (esp_partition_write(s_part, slotOff(s_rx.slot) + HDR + s_rx.got, buf, n) != ESP_OK) {
      abortRx("yazma");
      return true;
    }
    s_rx.crcCalc = esp_rom_crc32_le(s_rx.crcCalc, buf, n);
    s_rx.got += n;
    need -= n;
  }
  s_rx.lastByte = millis();
  if (s_rx.got >= s_rx.len) {
    finishRx();
    return true;
  }
  if (!eraseNext()) return true;
  bridge::printf("@ok %lu\n", static_cast<unsigned long>(s_rx.got));
  return true;
}

const State& state() { return s_state; }

uint32_t version() { return s_ver; }

uint32_t positionMs() { return rc::predictPosMs(s_posMs, millis() - s_posAt, s_state.playing, s_state.durMs); }

const lv_image_dsc_t* art() { return s_artSlot >= 0 ? &s_img : nullptr; }

bool tempo(uint32_t& periodMs, uint32_t& beatMs) {
  if (!s_tempoPeriod || !s_state.playing || millis() - s_tempoAt > cfg::MEDIA_TEMPO_MS) return false;
  periodMs = s_tempoPeriod;
  beatMs = s_tempoBeat;
  return true;
}

void send(Cmd c) {
  static const char* const kNames[] = {"play", "next", "prev", "volup", "voldown"};
  if (c > CMD_VOL_DOWN) return;
  bridge::printf("@cmd %s\n", kNames[c]);
  if (c == CMD_PLAY_PAUSE && s_state.active) {
    s_posMs = positionMs();
    s_posAt = millis();
    s_state.playing = !s_state.playing;
    s_ver++;
  }
  if (c == CMD_VOL_UP || c == CMD_VOL_DOWN) {
    if (millis() - s_volSentAt >= cfg::MEDIA_VOL_SETTLE_MS) s_volPending = 0;
    if (s_volPending < UINT16_MAX) s_volPending++;
    s_volSentAt = millis();
  }
  if ((c == CMD_VOL_UP || c == CMD_VOL_DOWN) && s_state.vol >= 0) {
    int v = s_state.vol + (c == CMD_VOL_UP ? cfg::MEDIA_VOL_STEP : -cfg::MEDIA_VOL_STEP);
    v = (v + cfg::MEDIA_VOL_STEP / 2) / cfg::MEDIA_VOL_STEP * cfg::MEDIA_VOL_STEP;
    s_state.vol = static_cast<int8_t>(LV_CLAMP(0, v, 100));
    s_ver++;
  }
}

void seek(uint32_t ms) {
  if (!s_state.link || !s_state.active || !s_state.durMs) return;
  if (ms > s_state.durMs) ms = s_state.durMs;
  bridge::printf("@cmd seek %lu\n", static_cast<unsigned long>(ms));
  s_posMs = ms;
  s_posAt = millis();
  s_ver++;
}

void report(Print& out) {
  int used = 0;
  for (int i = 0; i < s_slots; i++)
    if (s_slotHash[i]) used++;
  out.printf("[medya] kopru %s · %s · ses %d · kapak %d/%d yuva · alinan %lu · hata %lu · atilan %lu B\n",
             s_state.link ? "bagli" : "yok", s_state.active ? (s_state.playing ? "caliyor" : "duraklatildi") : "calmiyor",
             s_state.vol, used, s_slots, static_cast<unsigned long>(s_artOk), static_cast<unsigned long>(s_artErr),
             static_cast<unsigned long>(s_dropped));
  static const char* const kStatus[] = {"istenmedi", "yukleniyor", "hazir", "giris yok", "hata"};
  int sheets = 0;
  for (int k = 0; k < rc::PL_SHEETS; k++)
    if (s_pl.sheet[k] && findSlot(s_pl.sheet[k]) >= 0) sheets++;
  out.printf("[medya] listeler %s · %u liste · pafta %d · karisik %s · calan %d\n",
             kStatus[s_pl.status < 5 ? s_pl.status : 0], static_cast<unsigned>(s_pl.n), sheets,
             s_pl.shuffle ? "acik" : "kapali", static_cast<int>(s_pl.playing));
}

const Playlists& playlists() { return s_pl; }

void playlistsOpen(bool open) {
  s_plOpen = open;
  if (!open) return;
  if (s_pl.pickResult != rc::PLR_STARTING) {
    s_pl.pick = -1;
    s_pl.pickResult = rc::PLR_NONE;
  }
  if (!s_state.link) return;
  if (s_pl.status == rc::PLS_NONE) s_pl.status = rc::PLS_LOADING;
  bridge::println("@cmd pls");
  s_ver++;
}

void playPlaylist(int index) {
  if (!s_state.link || index < 0 || index >= s_pl.n) return;
  if (s_pl.pick >= 0 && s_pl.pickResult == rc::PLR_STARTING) return;
  s_pl.pick = static_cast<int8_t>(index);
  s_pl.pickResult = rc::PLR_STARTING;
  s_pl.pickAt = millis();
  s_pl.pickMsg[0] = 0;
  bridge::printf("@cmd plplay %lx %d\n", static_cast<unsigned long>(s_pl.ver), index);
  s_ver++;
}

void setShuffle(bool on) {
  s_pl.shuffle = on;
  bridge::printf("@cmd plmix %d\n", on ? 1 : 0);
  s_ver++;
}

const lv_image_dsc_t* playlistSheet(int index, int32_t& dx, int32_t& dy) {
  if (index < 0 || index >= s_pl.n || !s_map) return nullptr;
  const PlaylistItem& it = s_pl.item[index];
  if (it.sheet >= rc::PL_SHEETS) return nullptr;
  int slot = findSlot(s_pl.sheet[it.sheet]);
  if (slot < 0 || s_slotW[slot] != cfg::MEDIA_ART || s_slotH[slot] != cfg::MEDIA_ART) return nullptr;
  lv_image_dsc_t& d = s_sheetImg[it.sheet];
  memset(&d, 0, sizeof(d));
  d.header.magic = LV_IMAGE_HEADER_MAGIC;
  d.header.cf = LV_COLOR_FORMAT_RGB565;
  d.header.w = cfg::MEDIA_ART;
  d.header.h = cfg::MEDIA_ART;
  d.header.stride = static_cast<uint32_t>(cfg::MEDIA_ART) * 2U;
  d.data = s_map + slotOff(slot) + HDR;
  d.data_size = static_cast<uint32_t>(cfg::MEDIA_ART) * cfg::MEDIA_ART * 2U;
  dx = (it.cell % 3) * cfg::MEDIA_PL_CELL;
  dy = (it.cell / 3) * cfg::MEDIA_PL_CELL;
  return &d;
}

void wantPhoto(uint32_t hash, const char* name) {
  if (!hash || hash == s_photoHash) return;
  s_photoHash = hash;
  s_photoTries = 0;
  s_photoAt = 0;
  copyField(s_photoName, sizeof(s_photoName), name ? name : "");
  photoLoop();
}

const lv_image_dsc_t* photo(uint32_t hash) {
  if (!hash || !s_map) return nullptr;
  int slot = findSlot(hash);
  if (slot < 0 || s_slotW[slot] != cfg::PHOTO_PX || s_slotH[slot] != cfg::PHOTO_PX) return nullptr;
  memset(&s_photoImg, 0, sizeof(s_photoImg));
  s_photoImg.header.magic = LV_IMAGE_HEADER_MAGIC;
  s_photoImg.header.cf = LV_COLOR_FORMAT_RGB565;
  s_photoImg.header.w = s_slotW[slot];
  s_photoImg.header.h = s_slotH[slot];
  s_photoImg.header.stride = static_cast<uint32_t>(s_slotW[slot]) * 2U;
  s_photoImg.data = s_map + slotOff(slot) + HDR;
  s_photoImg.data_size = static_cast<uint32_t>(s_slotW[slot]) * s_slotH[slot] * 2U;
  return &s_photoImg;
}

}
