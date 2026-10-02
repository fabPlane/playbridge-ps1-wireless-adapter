# Combined firmware verification — 2026-10-02

Status: native tests and both ESP32 builds pass. **Not flashed and not tested
with simultaneous controller and J2 traffic.** The user's working
controller-only firmware remains the hardware baseline.

## Executed checks

- Native controller protocol and button lease tests: PASS for both profiles.
- Actual transport core under AddressSanitizer and UndefinedBehaviorSanitizer:
  PASS. Covers binary forwarding, partial writes, queue backpressure, independent
  stall deadlines, disconnect/reconnect cleanup, UART/socket faults, single
  ownership, malformed controls, all supported baud requests, bounded drain,
  clock rollover, duplicate/lost-ACK controls, and failed baud changes.
- Embedded browser script with mocked DOM/network: PASS for press/release,
  chords, heartbeat, cancel/blur, serialized updates, neutral, network errors
  and page exit.
- Arduino CLI 1.5.1, Arduino-ESP32 3.3.11 (ESP-IDF 5.5.5), Node32s: PASS for
  default bench profile and explicitly selected V4 PCB profile.
- ELF symbol inspection: both profiles place controller `exchange` and
  `pollBus` in `.iram0.text`.
- Git whitespace/error check: PASS.

| Compiled profile | Program bytes / 1,310,720 | Static RAM bytes / 327,680 |
| --- | ---: | ---: |
| 1 — working bench pin map | 942,392 | 65,760 |
| 2 — V4 PCB pin map | 942,384 | 65,760 |

Locally generated application binary SHA-256 values:

- Bench: `d9149f65b6262d916cbd0fd903daa90e7038ccbdd131227623361f3f64dca870`
- PCB: `56e2dd3263a751f37990032d135bd0d4e3dc5f2119894aacd165f6cf3ee2311a`

These identify this verification run, not a reproducible-build guarantee.
Binaries and build caches are not committed. A subsequent source comment
correction only clarifies that the serial task now owns GPIO16/17.

## Limits

Native tests mock physical GPIO/UART/network behavior. The ESP32 adapter was
compiled, not exercised on real UART hardware. The UART driver reset and
nonblocking drain implementation were reviewed against the installed API and
[ESP-IDF 5.5.5 UART source](https://github.com/espressif/esp-idf/blob/v5.5.5/components/esp_driver_uart/src/uart.c).
This does not establish real transfer integrity or throughput.

Static RAM figures exclude runtime task stacks, UART rings, TCP/Wi-Fi buffers,
and other heap allocations. Monitor heap/stack and UART error counters during
hardware qualification. There is no hardware flow control.

The earlier user confirmation that every web-controller button works applies
to the preserved controller-only sketch, not this combined image. External
PS1 patches, PC asset server and game assets are not included. Follow the
README acceptance checklist before claiming combined operation or flashing
an assembled V4 PCB.
