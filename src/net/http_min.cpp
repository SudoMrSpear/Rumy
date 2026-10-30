#include "http_min.h"

#include <Arduino.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app/config.h"
#include "text_tr.h"

namespace http {

namespace {

bool startsWithNoCase(const char* s, const char* prefix) {
  while (*prefix) {
    if (tolower(static_cast<unsigned char>(*s)) != tolower(static_cast<unsigned char>(*prefix))) return false;
    s++;
    prefix++;
  }
  return true;
}

const char* headerValue(const char* line, const char* name) {
  if (!startsWithNoCase(line, name)) return nullptr;
  const char* v = line + strlen(name);
  while (*v == ' ' || *v == '\t') v++;
  return v;
}

bool containsNoCase(const char* s, const char* needle) {
  for (; *s; s++) {
    if (startsWithNoCase(s, needle)) return true;
  }
  return false;
}

}

Response::Response(Client& client, uint32_t timeoutMs) : c_(client), timeoutMs_(timeoutMs) {}

bool Response::fill() {
  pos_ = 0;
  len_ = 0;
  for (;;) {
    int avail = c_.available();
    if (avail > 0) {
      size_t want = static_cast<size_t>(avail) < sizeof(buf_) ? static_cast<size_t>(avail) : sizeof(buf_);
      int n = c_.read(buf_, want);
      if (n > 0) {
        len_ = static_cast<size_t>(n);
        total_ += static_cast<uint32_t>(n);
        return true;
      }
    }
    if (!c_.connected()) return false;
    if (static_cast<int32_t>(millis() - deadline_) > 0) {
      timedOut_ = true;
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(4));
  }
}

int Response::raw() {
  if (pos_ >= len_ && !fill()) return -1;
  return buf_[pos_++];
}

bool Response::readLine(char* out, size_t cap) {
  size_t k = 0;
  for (;;) {
    int ch = raw();
    if (ch < 0) return false;
    if (ch == '\n') break;
    if (ch == '\r') continue;
    if (k + 1 < cap) out[k++] = static_cast<char>(ch);
  }
  out[k] = 0;
  return true;
}

bool Response::startChunk() {
  char line[24];
  if (!firstChunk_) {
    if (!readLine(line, sizeof(line))) return false;
  }
  firstChunk_ = false;
  if (!readLine(line, sizeof(line))) return false;
  char* end = nullptr;
  long size = strtol(line, &end, 16);
  if (end == line || size < 0) return false;
  chunkLeft_ = static_cast<int32_t>(size);
  return true;
}

int Response::next() {
  if (eof_) return -1;
  if (chunked_) {
    if (chunkLeft_ == 0) {
      if (!startChunk() || chunkLeft_ == 0) {
        eof_ = true;
        return -1;
      }
    }
    int ch = raw();
    if (ch < 0) {
      eof_ = true;
      return -1;
    }
    chunkLeft_--;
    return ch;
  }
  if (left_ == 0) {
    eof_ = true;
    return -1;
  }
  int ch = raw();
  if (ch < 0) {
    eof_ = true;
    return -1;
  }
  if (left_ > 0) left_--;
  return ch;
}

int Response::read() {
  if (peeked_ != -2) {
    int ch = peeked_;
    peeked_ = -2;
    return ch;
  }
  return next();
}

int Response::peek() {
  if (peeked_ == -2) peeked_ = next();
  return peeked_;
}

size_t Response::readBytes(char* buffer, size_t length) {
  size_t k = 0;
  while (k < length) {
    int ch = read();
    if (ch < 0) break;
    buffer[k++] = static_cast<char>(ch);
  }
  return k;
}

int Response::begin(const char* host, const char* path) {
  deadline_ = millis() + timeoutMs_;
  char req[800];
  int n = snprintf(req, sizeof(req),
                   "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: Rumy/%s (ESP32)\r\nAccept: application/json\r\n"
                   "Accept-Encoding: identity\r\nConnection: close\r\n\r\n",
                   path, host, RUMY_VERSION);
  if (n <= 0 || static_cast<size_t>(n) >= sizeof(req)) return E_BUILD;
  if (c_.write(reinterpret_cast<const uint8_t*>(req), static_cast<size_t>(n)) != static_cast<size_t>(n)) return E_WRITE;

  char line[160];
  if (!readLine(line, sizeof(line))) return timedOut_ ? E_TIMEOUT : E_PROTO;
  if (strncmp(line, "HTTP/1.", 7) != 0) return E_PROTO;
  const char* sp = strchr(line, ' ');
  if (!sp) return E_PROTO;
  int status = atoi(sp + 1);
  if (status < 100 || status > 599) return E_PROTO;
  for (int guard = 0; guard < 64; guard++) {
    if (!readLine(line, sizeof(line))) return timedOut_ ? E_TIMEOUT : E_PROTO;
    if (!line[0]) {
      if (contentLength_ >= 0) left_ = contentLength_;
      return status;
    }
    const char* v;
    if ((v = headerValue(line, "content-length:")) != nullptr) contentLength_ = static_cast<int32_t>(atol(v));
    else if ((v = headerValue(line, "transfer-encoding:")) != nullptr) chunked_ = containsNoCase(v, "chunked");
    else if ((v = headerValue(line, "content-encoding:")) != nullptr) {
      if (!startsWithNoCase(v, "identity")) return E_ENCODING;
    }
  }
  return E_PROTO;
}

const char* errText(int code) {
  switch (code) {
    case E_CONNECT: return rc::L("bağlanamadı", "connect failed");
    case E_TIMEOUT: return rc::L("zaman aşımı", "timeout");
    case E_PROTO: return "protokol";
    case E_WRITE: return rc::L("gönderilemedi", "send failed");
    case E_TLS_MEM: return rc::L("TLS için bellek yok", "no memory for TLS");
    case E_PARSE: return "JSON";
    case E_NO_WIFI: return rc::L("WiFi yok", "no WiFi");
    case E_BUILD: return "istek";
    case E_ENCODING: return rc::L("sıkıştırma", "compression");
    default: return code >= 200 ? "HTTP" : "?";
  }
}

}
