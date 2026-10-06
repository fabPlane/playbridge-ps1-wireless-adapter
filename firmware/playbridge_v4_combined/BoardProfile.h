#pragma once
#include <stdint.h>

// Select explicitly: 1 = console-tested Node32 breadboard, 2 = V4 PCB map.
// Profile 2 is compile-checked, not physical-PCBA qualified.
#ifndef PLAYBRIDGE_PROFILE
#define PLAYBRIDGE_PROFILE 1
#endif
#if PLAYBRIDGE_PROFILE == 1
static constexpr uint8_t CMD = 34, CLK = 32, ATT = 21;
static constexpr uint8_t DATA_DRIVE = 33, ACK_DRIVE = 27, DATA_SENSE = 19;
static constexpr bool HAS_DATA_READBACK = true;
static constexpr const char *PROFILE_NAME = "bench-cmd34-data19";
#elif PLAYBRIDGE_PROFILE == 2
static constexpr uint8_t CMD = 19, CLK = 32, ATT = 21;
static constexpr uint8_t DATA_DRIVE = 33, ACK_DRIVE = 27;
// Placeholder never configured/read; GPIO34 on PCB is ACK sense, NOT DATA.
static constexpr uint8_t DATA_SENSE = 34;
static constexpr bool HAS_DATA_READBACK = false;
static constexpr const char *PROFILE_NAME = "v4-pcb-unqualified";
#else
#error Select PLAYBRIDGE_PROFILE=1 (bench) or 2 (V4 PCB)
#endif
static constexpr int UART_RX = 16, UART_TX = 17;
static_assert(CMD != UART_RX && CMD != UART_TX && DATA_DRIVE != UART_TX,
              "Controller and serial pins must not overlap");
