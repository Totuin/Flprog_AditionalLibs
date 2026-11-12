#pragma once
#include <stdint.h>
uint32_t millis();
struct MockSerial {
  void begin(unsigned long) {}
  template <typename T> void print(const T&) {}
  template <typename T> void println(const T&) {}
};
extern MockSerial Serial;
inline void delay(uint32_t) {}
