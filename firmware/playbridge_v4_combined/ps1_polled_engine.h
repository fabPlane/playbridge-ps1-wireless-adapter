#pragma once
#include <Arduino.h>
#include <esp_cpu.h>
#include <esp_rom_sys.h>
#include <soc/gpio_struct.h>
#include "BoardProfile.h"

// Same pin map as the successful receive-only polling capture.
// Temporary diagnostic input ONLY: DATA node -> 1 kohm -> GPIO19.
// Do not reconnect the old ACK-sense jumper to GPIO34 (CMD).
static uint32_t cyclesPerUs = 240;
static portMUX_TYPE busMux = portMUX_INITIALIZER_UNLOCKED;
static uint32_t transactions = 0, addresses = 0, commands = 0, fullReplies = 0;
static uint32_t aborted = 0, ackPulses = 0;
static uint8_t lastRx[5] = {0}, lastBytes = 0;
static uint8_t lastData[5] = {0};
static uint32_t dataByteMismatches = 0;
static uint32_t lastByteEndUs = 0, lastAbortUs = 0;
static uint8_t lastAbortReason = 0, lastAbortBit = 0;
static bool senseEnabled = false;

static inline __attribute__((always_inline)) uint8_t IRAM_ATTR levels() {
  const uint32_t low = GPIO.in;
  const uint32_t high = GPIO.in1.val;
  if constexpr (CMD == 34) {
    return (((low >> ATT) & 1U) << 2) | ((high & 1U) << 1) | ((high >> 2) & 1U);
  } else {
    return (((low >> ATT) & 1U) << 2) | ((high & 1U) << 1) | ((low >> CMD) & 1U);
  }
}

// NPN inversion: GPIO HIGH pulls the corresponding PS1 line LOW.
static inline __attribute__((always_inline)) void IRAM_ATTR dataBit(bool high) {
  if (high) GPIO.out1_w1tc.val = 1U << (DATA_DRIVE - 32);
  else GPIO.out1_w1ts.val = 1U << (DATA_DRIVE - 32);
}
static inline __attribute__((always_inline)) void IRAM_ATTR ack(bool pullLow) {
  if (pullLow) GPIO.out_w1ts = 1U << ACK_DRIVE;
  else GPIO.out_w1tc = 1U << ACK_DRIVE;
}

static inline __attribute__((always_inline)) bool IRAM_ATTR waitClock(bool high, uint32_t origin, uint8_t &sample) {
  while (true) {
    sample = levels();
    if (sample & 4U) { lastAbortReason = 1; return false; }
    if ((bool)(sample & 2U) == high) return true;
    if ((uint32_t)(esp_cpu_get_cycle_count() - origin) >= cyclesPerUs * 2000U) {
      lastAbortReason = 2;
      return false;
    }
  }
}

// noinline is required: IRAM_ATTR alone allowed the compiler to inline the
// entire critical transaction into flash-resident loop() in the v1 binary.
static void IRAM_ATTR __attribute__((noinline)) exchange(uint16_t buttons, uint32_t origin) {
  const uint8_t reply[5] = {0xFF, 0x41, 0x5A, (uint8_t)buttons, (uint8_t)(buttons >> 8)};
  uint8_t sample = 0;
  lastBytes = 0;
  lastByteEndUs = lastAbortUs = 0;
  lastAbortReason = lastAbortBit = 0;
  for (uint8_t i = 0; i < 5; ++i) lastRx[i] = lastData[i] = 0;
  ++transactions;
  for (uint8_t i = 0; i < 5; ++i) {
    uint8_t received = 0;
    uint8_t observedData = 0;
    for (uint8_t bit = 0; bit < 8; ++bit) {
      const bool bitHigh = (reply[i] >> bit) & 1U;
      lastAbortBit = bit;
      if (!waitClock(false, origin, sample)) goto failed;
      dataBit(bitHigh);
      if (!waitClock(true, origin, sample)) goto failed;
      // Read the physical output, not the GPIO33 base-drive register.
      const uint32_t senseRegister = DATA_SENSE < 32 ? uint32_t(GPIO.in) : uint32_t(GPIO.in1.val);
      if (senseEnabled && ((senseRegister >> (DATA_SENSE % 32)) & 1U)) observedData |= 1U << bit;
      if (sample & 1U) received |= 1U << bit;
    }
    lastRx[i] = received;
    lastData[i] = observedData;
    if (senseEnabled && observedData != reply[i]) ++dataByteMismatches;
    lastBytes = i + 1;
    lastByteEndUs = (esp_cpu_get_cycle_count() - origin) / cyclesPerUs;
    if (i == 0) {
      if (received != 0x01) { lastAbortReason = 3; return; }
      ++addresses;
    }
    if (i == 1) {
      if (received != 0x42) { lastAbortReason = 4; return; }
      ++commands;
    }
    if (i < 4) {
      esp_rom_delay_us(14);
      if (levels() & 4U) { lastAbortReason = 5; goto failed; }
      ack(true);
      esp_rom_delay_us(2);
      ack(false);
      ++ackPulses;
    } else ++fullReplies;
  }
  return;
failed:
  lastAbortUs = (esp_cpu_get_cycle_count() - origin) / cyclesPerUs;
  ++aborted;
}

static void IRAM_ATTR __attribute__((noinline)) pollBus(uint16_t buttons) {
  portENTER_CRITICAL(&busMux);
  const uint32_t waitStart = esp_cpu_get_cycle_count();
  uint8_t previous = levels();
  // Start only on an observed ATT falling edge, never halfway through a frame.
  while ((uint32_t)(esp_cpu_get_cycle_count() - waitStart) < cyclesPerUs * 2000U) {
    const uint8_t current = levels();
    if ((previous & 4U) && !(current & 4U)) {
      exchange(buttons, esp_cpu_get_cycle_count());
      break;
    }
    previous = current;
  }
  // Includes every timeout, invalid address/command, and successful exit.
  dataBit(true);
  ack(false);
  portEXIT_CRITICAL(&busMux);
}
