#include <cassert>
#include <array>
#include <algorithm>
#include "../pad_state.h"
#include "../ps1_polled_engine.h"

// Deterministic ideal-electrical host. This tests the actual responder source,
// not transistor/cable behavior or ESP32 instruction-level timing.
static uint32_t nowCycles;
static bool dataLow, ackLow, stuckClock;
static unsigned sampled, ackCount;
static std::array<uint8_t, 5> request, observed;
static uint32_t attEnd;
static constexpr uint32_t FIRST = 140 * 240;
static void advance(uint32_t cycles) {
  const uint32_t end = nowCycles + cycles;
  while (sampled < 40 && !stuckClock) {
    const unsigned byte = sampled / 8, bit = sampled % 8;
    const uint32_t edge = FIRST + byte * 60 * 240 + (bit * 4 + 2) * 240;
    if (edge > end || edge >= attEnd) break;
    if (!dataLow) observed[byte] |= 1U << bit;
    ++sampled;
  }
  nowCycles = end;
}
uint32_t mockCycles() { advance(12); return nowCycles; }
uint32_t mockLow() {
  advance(12);
  return ((nowCycles < 100 * 240 || nowCycles >= attEnd) ? 1U << 21 : 0)
         | (CMD == 19 ? ((mockHigh() & 4U) ? 1U << 19 : 0) : (dataLow ? 0 : 1U << 19));
}
uint32_t mockHigh() {
  advance(12);
  if (stuckClock) return 0;
  if (nowCycles < FIRST) return 5;
  const uint32_t elapsed = nowCycles - FIRST;
  const unsigned byte = elapsed / (60 * 240), phase = elapsed % (60 * 240);
  if (byte >= 5 || phase >= 32 * 240) return 5;
  const unsigned bit = phase / (4 * 240);
  const bool clock = phase % (4 * 240) >= 2 * 240;
  return (clock ? 1 : 0) | (((request[byte] >> bit) & 1) ? 4 : 0);
}
void mockWrite(int kind, uint32_t) {
  if (kind == 1) dataLow = true;
  if (kind == 2) dataLow = false;
  if (kind == 3) { if (!ackLow) ++ackCount; ackLow = true; }
  if (kind == 4) ackLow = false;
}
void mockDelay(uint32_t us) { advance(us * 240); }
static void reset() {
  nowCycles = 0; dataLow = ackLow = stuckClock = false;
  sampled = ackCount = 0; observed.fill(0); request = {1, 0x42, 0, 0, 0};
  attEnd = 450 * 240;
  transactions = addresses = commands = fullReplies = aborted = ackPulses = dataByteMismatches = 0;
  senseEnabled = HAS_DATA_READBACK;
}
int main() {
  PadState pad;
  assert(pad.get(0) == 0xFFFF);
  pad.set(0xFFBF, 100);
  assert(pad.get(899) == 0xFFBF && pad.get(900) == 0xFFFF);
  pad.set(0xFFEF, 800);
  assert(pad.get(1500) == 0xFFEF);
  pad.set(0xFFFF, 1501);
  assert(pad.get(1502) == 0xFFFF);
  pad.set(0xFFBF, UINT32_MAX-100);
  assert(pad.get(100) == 0xFFBF && pad.get(700) == 0xFFFF);
  for (uint16_t buttons : {0xFFFF, 0xFFBF, 0xFFEF}) {
    reset(); pollBus(buttons);
    assert(fullReplies == 1 && addresses == 1 && commands == 1);
    assert(ackCount == 4 && ackPulses == 4 && aborted == 0);
    assert((observed == std::array<uint8_t, 5>{0xFF, 0x41, 0x5A, (uint8_t)buttons, (uint8_t)(buttons >> 8)}));
    assert(dataByteMismatches == 0 && !dataLow && !ackLow);
  }
  reset(); request[0] = 0x81; pollBus(0xFFFF);
  assert(addresses == 0 && ackCount == 0 && lastAbortReason == 3 && !dataLow && !ackLow);
  reset(); request[1] = 0x43; pollBus(0xFFFF);
  assert(commands == 0 && ackCount == 1 && lastAbortReason == 4 && !dataLow && !ackLow);
  reset(); attEnd = 150 * 240; pollBus(0xFFFF);
  assert(aborted == 1 && lastAbortReason == 1 && ackCount == 0 && !dataLow && !ackLow);
  reset(); stuckClock = true; attEnd = 10000 * 240; pollBus(0xFFFF);
  assert(aborted == 1 && lastAbortReason == 2 && nowCycles < 2200 * 240 && !dataLow && !ackLow);
  std::puts("PASS: button lease/refresh/release/millis rollover; neutral/down/up bytes, physical readback, 4 ACKs/no final ACK, address/command rejection, ATT abort, timeout, output release.");
}
