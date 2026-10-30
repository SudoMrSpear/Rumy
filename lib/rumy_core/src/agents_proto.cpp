#include "agents_proto.h"
#include "text_tr.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace rc {

namespace {

constexpr int ROW_FIELDS = 8;
constexpr unsigned long COUNT_MAX = 99;

char* nextField(char*& p) {
  if (!p) return nullptr;
  char* out = p;
  char* tab = strchr(p, '\t');
  if (tab) {
    *tab = 0;
    p = tab + 1;
  } else {
    p = nullptr;
  }
  return out;
}

bool number(const char*& p, unsigned long& out, int base) {
  while (*p == ' ') p++;
  char* end = nullptr;
  out = strtoul(p, &end, base);
  if (end == p) return false;
  p = end;
  return true;
}

uint8_t small(unsigned long v) { return static_cast<uint8_t>(v > COUNT_MAX ? COUNT_MAX : v); }

uint8_t statusOf(const char* s) {
  if (!strcmp(s, "c")) return AG_WORK;
  if (!strcmp(s, "b")) return AG_WAIT;
  if (!strcmp(s, "i")) return AG_IDLE;
  return AG_NONE;
}

void copyText(char* dst, size_t cap, const char* src) {
  size_t n = strlen(src);
  if (n >= cap) {
    n = cap - 1;
    while (n > 0 && (static_cast<uint8_t>(src[n]) & 0xC0) == 0x80) n--;
  }
  memcpy(dst, src, n);
  dst[n] = 0;
}

const AgentRow* find(const Agents& a, uint32_t key) {
  for (int i = 0; i < a.n; i++)
    if (a.rows[i].key == key) return &a.rows[i];
  return nullptr;
}

}

bool parseAgentsHead(const char* args, AgentsHead& out) {
  unsigned long ver, n, work, wait, idle;
  const char* p = args;
  if (!number(p, ver, 16) || !number(p, n, 10) || !number(p, work, 10) || !number(p, wait, 10) ||
      !number(p, idle, 10))
    return false;
  out.ver = static_cast<uint32_t>(ver);
  out.n = static_cast<uint8_t>(n > static_cast<unsigned long>(AGENT_ROWS) ? AGENT_ROWS : n);
  out.work = small(work);
  out.wait = small(wait);
  out.idle = small(idle);
  return true;
}

bool parseAgentRow(char* fields, uint8_t& index, AgentRow& out) {
  char* f[ROW_FIELDS];
  char* p = fields;
  for (int i = 0; i < ROW_FIELDS; i++) {
    f[i] = nextField(p);
    if (!f[i]) return false;
  }
  char* end = nullptr;
  unsigned long idx = strtoul(f[0], &end, 10);
  if (end == f[0] || idx >= static_cast<unsigned long>(AGENT_ROWS)) return false;
  unsigned long key = strtoul(f[1], &end, 16);
  if (end == f[1]) return false;
  uint8_t st = statusOf(f[2]);
  if (st == AG_NONE) return false;
  unsigned long since = strtoul(f[3], &end, 10);
  if (end == f[3]) return false;
  unsigned long work = strtoul(f[4], &end, 10);
  if (end == f[4]) return false;
  memset(&out, 0, sizeof(out));
  index = static_cast<uint8_t>(idx);
  out.key = static_cast<uint32_t>(key);
  out.status = st;
  out.sinceS = static_cast<uint32_t>(since);
  out.workS = static_cast<uint32_t>(work);
  out.agent = strcmp(f[5], "codex") ? AK_CLAUDE : AK_CODEX;
  copyText(out.project, sizeof(out.project), f[6]);
  copyText(out.detail, sizeof(out.detail), f[7]);
  return true;
}

void agentsClear(Agents& a) { memset(&a, 0, sizeof(a)); }

uint8_t agentsTop(const Agents& a) {
  if (!a.valid) return AG_NONE;
  if (a.wait) return AG_WAIT;
  if (a.work) return AG_WORK;
  if (a.idle) return AG_IDLE;
  return AG_NONE;
}

uint8_t agentsTopCount(const Agents& a) {
  switch (agentsTop(a)) {
    case AG_WAIT: return a.wait;
    case AG_WORK: return a.work;
    case AG_IDLE: return a.idle;
    default: return 0;
  }
}

int agentsNotes(const Agents& before, const Agents& after, uint32_t doneMinS, AgentNote* out, int cap) {
  if (!before.valid || !after.valid) return 0;
  int k = 0;
  for (int i = 0; i < after.n && k < cap; i++) {
    const AgentRow& r = after.rows[i];
    const AgentRow* old = find(before, r.key);
    uint8_t was = old ? old->status : static_cast<uint8_t>(AG_NONE);
    uint8_t kind = AN_NONE;
    if (r.status == AG_WAIT && was != AG_WAIT) kind = AN_WAIT;
    else if (r.status == AG_IDLE && was == AG_WORK && r.workS >= doneMinS) kind = AN_DONE;
    if (kind == AN_NONE) continue;
    out[k].kind = kind;
    out[k].row = r;
    k++;
  }
  return k;
}

const char* agentName(uint8_t agent) { return agent == AK_CODEX ? "Codex" : "Claude"; }

void agentSpanTr(uint32_t s, char* out, size_t cap) {
  if (s < 60) {
    snprintf(out, cap, rc::L("%lu sn", "%lu s"), static_cast<unsigned long>(s));
    return;
  }
  uint32_t m = s / 60;
  if (m < 60) {
    snprintf(out, cap, rc::L("%lu dk", "%lu min"), static_cast<unsigned long>(m));
    return;
  }
  uint32_t h = m / 60;
  m %= 60;
  if (h >= 24 || !m) snprintf(out, cap, rc::L("%lu sa", "%lu h"), static_cast<unsigned long>(h));
  else snprintf(out, cap, rc::L("%lu sa %lu dk", "%lu h %lu min"), static_cast<unsigned long>(h), static_cast<unsigned long>(m));
}

}
