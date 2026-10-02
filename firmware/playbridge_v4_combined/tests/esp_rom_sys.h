#pragma once
#include "Arduino.h"
inline void esp_rom_delay_us(uint32_t us) { mockDelay(us); }
