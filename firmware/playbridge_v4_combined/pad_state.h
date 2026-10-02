#pragma once
#include <stdint.h>

// A full-state lease prevents dropped HTTP releases leaving a button held.
struct PadState {
  static constexpr uint32_t LEASE_MS = 800;
  uint16_t buttons = 0xFFFF;
  uint32_t updated = 0;
  void set(uint16_t value, uint32_t now) { buttons = value; updated = now; }
  uint16_t get(uint32_t now) const {
    return uint32_t(now - updated) < LEASE_MS ? buttons : 0xFFFF;
  }
};
