# PlayBridge V3 combined controller and serial bridge

Status: experimental; build-validated only. Do not call this stable until an
assembled V3 board passes simultaneous controller and game-transfer tests.

This sketch keeps the existing `playbridge_wifi_test` source unchanged and
combines its PS1 controller-port responder with a transparent PS1 rear-serial
bridge. Both functions share one Wi-Fi manager but use separate console ports.

## Resource map

| Resource | Owner |
| --- | --- |
| SPI2; GPIO19/21/32/33/34 | PS1 digital controller responder |
| UART2; GPIO16 RX / GPIO17 TX | PS1 rear-serial bridge through V3 J2 |
| HTTP TCP 80 | Controller UI and `/bridge/status` |
| TCP 3333 | Raw bidirectional game-data stream |
| UDP 3334 | 20-byte PSB1 baud/abort control |
| UART0 | Bounded setup and diagnostics |

UART2 boots at 115200 baud. Accepted control rates are 115200, 230400, 518400,
691200, 1036800 and 2073600 baud. The raw TCP stream never carries logging or
control text.

## Build

Use the PlayBridge-validated Arduino-ESP32 core 3.3.11:

```sh
arduino-cli compile \
  --fqbn esp32:esp32:node32s \
  firmware/playbridge_combined_bridge
```

No upload is performed by the verification workflow.

The V3 design uses an ESP32-WROOM-32E-N4 module with 4 MB flash. The build uses
the `node32s` board definition and its 1,310,720-byte application partition; it
does not inherit the handoff's `esp32dev`, COM10, or 16 MB assumptions.

## Safety and limitations

- J2 is PS1 serial and **not USB**.
- Verify the J2 harness by continuity before applying power.
- Confirm console TX voltage is within the ESP32 input limit before fitting a
  direct R31 path. The PCB release hold remains active until this measurement.
- Controller input returns to neutral after five seconds without browser
  heartbeat traffic. The supplied page sends a heartbeat while long holds are
  active.
- Wi-Fi credential changes are rejected during a data transfer.
- Game patches, the PC asset server, and PS1 launcher are separate components.
- Compilation does not prove console timing, integrity, or game compatibility.

## Hardware acceptance sequence

1. Controller-only regression at the established SPI timing.
2. Bridge-only CRC-verified transfers at every supported baud.
3. Simultaneous rapid and held controller input during repeated large loads.
4. Network interruption, duplicate PSB1 abort, disconnect and reconnect tests.
5. Record corruption/retries, pad polls, maximum bridge scheduling gap, heap,
   UART errors and input latency against the standalone baselines.
