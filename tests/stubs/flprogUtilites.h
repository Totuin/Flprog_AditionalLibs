#pragma once
#include "Arduino.h"
namespace flprog { inline bool isTimer(uint32_t from, uint32_t timeout) {
  return (uint32_t)(millis() - from) >= timeout;
} }
