# Combined firmware verification — 2026-10-02

Status: native tests and both ESP32 builds pass. **Bench-profile combined image
flashed; controller movement and real read-only serial transfers verified.**
Simultaneous gameplay/rapid input with serial traffic and the assembled PCB
remain unqualified. The controller-only firmware remains the rollback baseline.

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

## Subsequent bench checks — 2026-10-02

The bench binary identified above was flashed and its upload hash verified.
The user confirmed movement in the web-controller UI on this combined image.
This does not extend the earlier every-button confirmation to a new simultaneous
controller/serial stress test.

### Harness and electrical changes

- UART2 remains TX GPIO17 / RX GPIO16, 115200 8N1, no inversion/flow control.
- Continuity and readable Unirom startup output established ground and the
  console-TX receive path. Final bench mapping: yellow console RX via **220 Ω**
  to GPIO17; orange console TX via **1 kΩ** to GPIO16; green to common GND.
  Colours identify this particular verified harness only, not a standard.
- A **10 kΩ** pull-up connects the orange/harness-side end of its 1 kΩ resistor
  to verified ESP32 3.3 V. It removed UART noise with the console disconnected.
  The orange junction is in A30/B30/C30 on this particular breadboard; the
  opposite F30–J30 group is separate across the centre gap. Row coordinates
  must not be generalized to another layout.
- The yellow TX resistor was originally 1 kΩ. Changing it to 220 Ω was followed
  by command echoes and later successful dumps. A voltage-divider explanation
  was a hypothesis, not an oscilloscope-confirmed cause.
- With the pull-up and console disconnected: 4 UART bytes sent, 0 received,
  no UART errors. Reconnecting the console restored the PING echo. No wire
  swapping or PCB-value change is justified by the echo alone.

### Transport and real-console evidence

1. With the PS1 disconnected, UART loopback returned 65,536 bytes exactly at
   115200 (8.077 s), then 131,072 bytes exactly at each higher supported rate:
   230400 (9.798 s), 518400 (3.966 s), 691200 (2.276 s), 1036800 (1.996 s),
   2073600 (1.612 s). Hash comparisons passed; UART/socket errors were zero.
   These are loopback observations, not PS1 throughput qualifications.
2. Passive capture received readable Unirom 8.0.K startup text from console
   TX through ESP32 RX16 and Wi-Fi. Boot-time UART errors occurred; error-free
   transfer claims below refer to counter deltas during those tests, not to
   lifetime counters.
3. PING returned `PING` (including with a three-second receive window and a
   separately paced-byte test), not `PONG`. The user reported the same PING
   limitation in their working Windows setup. Therefore the old local PING UI
   verdict was not a reliable end-to-end acceptance gate for this console.
4. Two 64-byte RAM dumps succeeded at 115200: `0x80010000` (all zero) and
   `0x80000080` (40 nonzero bytes). Handshake: `DUMPOKV2OKAY`, with host `UPV2`.
   The latter had additive checksum **2992**, matching the console; each test
   had 16 host-to-UART bytes, 80 UART-to-host bytes and zero new UART/socket
   errors. Nonzero dump SHA-256:
   `9b9456c961739dcd4ddbec1957fb1c214aab95f6780c814dad58235e5c233a13`.
5. Physical memory-card **slot 1**: **131,072 bytes** read via Unirom to temporary
   PS1 RAM at `0x80080000`, then through rear serial → ESP32 → Wi-Fi → Mac →
   mounted exFAT SD card. Protocol V2 checksum **0x014ac447** matched; raw image
   began with `MC`. The saved file's SHA-256 matched its in-memory payload on
   read-back. No memory-card writes were requested. Transfer elapsed time for
   the successful continuation was **52.815 s**, including card read and host
   transfer; this is not a pure Wi-Fi throughput measurement.

The first card-download attempt followed the newer client's `MCDN → HLTD`
expectation and timed out without sending a card index or saving a file. The
older client source confirmed **`MCDN → OKAY`**. The bench session was continued
once by sending slot index 0, then receiving `MCRD` and performing the dump.
During that successful continuation, counter deltas were TX/TCP-to-UART **276**,
RX/UART-to-TCP **131,100**, UART errors **0**, socket errors **0**. These exclude
the initial MCDN negotiation. The repository tool uses the corrected complete
sequence; its refactored CLI is offline-tested, not claimed as a second physical
card backup run. No blind resume feature is shipped.

Personal memory-card contents, the backup image, its per-file manifest, private
host paths and network configuration are intentionally **not committed**.
Only non-content measurements and source/test tooling are included.

### Host-tool regression checks

`tests/test_memcard_backup.py` exercises fragmented V2/V3 handshakes and full
synthetic card transfers, slot indexing, 2048-byte MORE pacing, checksum
rejection, echo-only timeout, console error tokens, RAM buffer bounds, readiness
guards, exclusive file creation and file read-back. No hardware/network used.

## Limits

Native tests mock physical GPIO/UART/network behavior. The ESP32 adapter has
now been exercised on the bench as described above. The UART driver reset and
nonblocking drain implementation were reviewed against the installed API and
[ESP-IDF 5.5.5 UART source](https://github.com/espressif/esp-idf/blob/v5.5.5/components/esp_driver_uart/src/uart.c).
Source review alone does not establish physical behavior beyond the recorded tests.

Static RAM figures exclude runtime task stacks, UART rings, TCP/Wi-Fi buffers,
and other heap allocations. Monitor heap/stack and UART error counters during
hardware qualification. There is no hardware flow control.

The earlier every-button confirmation applies to the controller-only sketch.
Combined-build movement and a read-only serial backup are now verified, but
Mac-to-PS1 game loading, high-speed PS1 transfers, simultaneous gameplay,
write/restore operations and assembled V4 PCB electrical qualification are not.
External PS1 patches, PC asset server and game assets are not included. Follow
the README acceptance checklist before production claims or assembled-PCB use.
