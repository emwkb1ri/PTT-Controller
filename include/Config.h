#pragma once

#include <stdint.h>

namespace Config {
constexpr uint8_t buttonPin = 1;
constexpr uint8_t modePin = 2;
constexpr uint8_t pairingResetPin = 4;
constexpr uint32_t debounceMs = 25;
constexpr uint32_t keyHoldMs = 10;
constexpr char deviceName[] = "XIAO-AltT-Key";
}