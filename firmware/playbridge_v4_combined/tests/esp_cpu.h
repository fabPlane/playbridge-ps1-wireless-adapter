#pragma once
#include "Arduino.h"
inline uint32_t esp_cpu_get_cycle_count() { return mockCycles(); }
