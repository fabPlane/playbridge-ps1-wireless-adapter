# PlayBridge V4 controller and J2 firmware

Experimental combined firmware for the PS1 controller port and rear serial
port. This preserves the console-tested polling controller and adds an opaque
TCP to UART2 bridge. The combined build is not yet console-tested or approved
for production. No hardware was flashed for this PR.

The unchanged, working controller-only source is preserved in
[`../playbridge_v4_controller`](../playbridge_v4_controller).
The pre-existing `../playbridge_wifi_test` sketch is also untouched.
The older handoff prompt's SPI pin mapping and claim that J2 is NC must not be
used as V4 wiring instructions.

## Select the physical pin profile

The default is the **actual working breadboard**, not the assembled PCB.

| Signal | Profile 1 bench default | Profile 2 V4 PCB |
| --- | --- | --- |
| CMD input | GPIO34 | GPIO19 |
| CLK input | GPIO32 | GPIO32 |
| ATT input | GPIO21 | GPIO21 |
| DATA drive | GPIO33 | GPIO33 |
| ACK drive | GPIO27 | GPIO27 |
| DATA diagnostic input | GPIO19 via 1 kΩ | Disabled |
| ACK diagnostic input | Not used | GPIO34 reserved, not read |
| J2 TX | GPIO17 via 1 kΩ to J2.1 | Same |
| J2 RX | J2.4 via 1 kΩ to GPIO16 | Same |
| J2 GND | J2.7 | Same |

J2.1 connects to **console RX**, J2.4 to **console TX**. These are J2 connector
numbers, not rear-PS1-port pin numbers. Continuity-map the real harness and
verify 3.3 V-compatible levels; never infer by wire colour. J2 carries no power,
is not USB, and all other J2 contacts remain open. Remove the old UART loopback
before attaching the peer. UART0 USB programming/debug is separate.

Bench controller values verified during debugging: DATA C50 pull-up **1 kΩ** to
3.3 V; E51 to F51 control resistor **1 kΩ**; C51 pull-down **100 kΩ** to GND.
Keep E50 to F50 **1 kΩ** and G50 to GPIO19 for diagnostic readback in profile 1.
GPIO HIGH turns on the external output stage and pulls the PS1 line LOW.
The bench NPN circuit is not evidence that V4 PCB component values or its
2N7002 stages are physically qualified. This PR changes no PCB, BOM, or wiring.

## Architecture

- Core 1 Arduino loop owns the IRAM polling controller. The 16-bit active-low
  button state is latched once per transaction. ACK delay is 14 µs, LOW for
  2 µs, and there is no final-byte ACK. No per-edge ISR, networking, allocation,
  or logging occurs inside the controller transaction.
- Core 0 web task owns Wi-Fi AP/STA and HTTP 80. Saved credentials are reused,
  Wi-Fi sleep is disabled, and runtime credential writes/OTA are not exposed.
  The current full-state `POST /state` interface is retained; the obsolete
  reference `/input` and `/button` interfaces are not imported.
- A separate core 0 task owns UART2, TCP 3333 and UDP 3334. It initializes the
  IDF UART driver on that core, which also allocates the UART ISR there.
  HTTP never accesses live UART or bridge sockets.
- Short synchronized snapshots provide `/status` (controller) and
  `/bridge/status` (serial counters, baud, fault, owner core, timing, heap,
  stack). Web handling remains active during transfers.
- Browser holds refresh every 200 ms. Button input expires after 800 ms
  without an accepted update; blur/cancel/neutral release immediately when
  the request arrives. HTTP success is not proof the console accepted a reply.

The bounded polling wait still disables interrupts on core 1 for up to 2 ms,
plus up to a 2 ms transaction timeout. Core separation reduces UART contention
but is not proof of performance. Measure simultaneous pad latency, serial
integrity and throughput on hardware.

## Serial protocol

UART starts at **115200 8N1**, with no hardware flow control. The existing
PC launcher coordinates the PS1-side baud switch; the bridge does not negotiate
it with the console. Supported rates: 115200, 230400, 518400, 691200, 1036800,
2073600. Support here means accepted configuration, not a measured speed claim.

TCP 3333 has one owner and carries only raw bytes, with no added framing or
logs. The PC asset server, patched PS1 launcher/game code, and checksums/retries
remain external. This is not direct SD access or universal game compatibility.
The only supplied host tool is the unchanged handoff `host/bridge_baud.py`.

UDP 3334 accepts exactly 20 bytes:

| Bytes | Meaning |
| --- | --- |
| 0 to 3 | ASCII PSB1 |
| 4 to 7 | Host nonce, little-endian |
| 8 to 11 | Baud, little-endian |
| 12 to 15 | Flags: 0 normal, 1 abort session |
| 16 to 19 | Standard CRC32 of bytes 0 to 15, little-endian |

Success echoes the exact request to its originating IP/port. Invalid packets,
wrong active-owner IP, busy queues, failed baud changes, or drain timeout get
no success echo. During normal handoff, both forwarding queues and new input
must be empty. TX drain is polled with zero ticks and a 150 ms deadline; new
input cancels that handoff. The launcher retries on no acknowledgement.

Successful control effects are cached **before** sending the echo: eight
requests for ten seconds, keyed by sender IP/port and exact frame. A retry
does not abort a new session. An older request whose baud has since changed
gets no misleading success echo. This is bounded duplicate suppression, not
authentication or indefinite replay protection; use a trusted LAN.

Abort/disconnect/error discards application queues and reinstalls UART2 to
discard driver RX/TX rings and reset FIFOs. Bytes already transmitted cannot be
retracted: abort does not reset the PS1 or restore Unirom. The launcher must
resynchronize. If UART reset fails, the bridge stays faulted until reboot.
New clients are accepted only after UART cleanup. UART bytes without a client
are discarded, so connect the PC session before requesting a console response.
TCP close is a session abort, not a supported half-close request/response mode.

Queue sizes: 16,384 UART RX; 4,096 UART TX; 16,384 UART-to-TCP pending;
1,024 TCP-to-UART pending. Partial writes retain offsets. Retryable socket
errors do not discard data. Each direction has its own ten-second progress
deadline; traffic in the other direction cannot mask a stalled queue.

## Build and test

Use Arduino CLI with installed Arduino-ESP32 **3.3.11**, Node32s, standard
4 MB board profile; do not copy the handoff's COM10 or 16 MB flash settings.
Commands from repository root:

```sh
sh firmware/playbridge_v4_combined/tests/run.sh

arduino-cli compile --fqbn esp32:esp32:node32s \
  --build-path /tmp/playbridge-combined-bench-build \
  firmware/playbridge_v4_combined

arduino-cli compile --fqbn esp32:esp32:node32s \
  --build-property compiler.cpp.extra_flags=-DPLAYBRIDGE_PROFILE=2 \
  --build-path /tmp/playbridge-combined-pcb-build \
  firmware/playbridge_v4_combined
```

Native tests need a C++17 compiler with ASan/UBSan and Node.js. They execute the
actual transport core and actual polled engine, not independent protocol
reimplementations. The browser script is exercised with a mocked DOM/network.
No test command opens UART, sends LAN traffic, or flashes hardware.

Keep the controller-only rollback source and existing verified binary before
any separately authorized upload. Change wires only with USB unplugged and PS1
off; physically disconnect both console harnesses before flashing. Do not
flash the handoff's compile-only harness.

## Evidence and remaining acceptance

See [TEST-RESULTS.md](TEST-RESULTS.md) and [provenance.json](provenance.json).
The user confirmed every controller web button works on the bench before this
merge. The preceding diagnostic reported 1,624 complete replies with zero
sampled DATA mismatches. That is controller-only evidence.

Before calling this combined firmware stable:

1. Recheck controller-only and known serial-only baselines.
2. Transfer a small known Unirom payload at 115200 with the existing launcher.
3. Run repeated large end-to-end CRC-verified transfers at every supported rate.
4. Repeat with rapid and held web inputs, checking pad poll failures/latency,
   UART errors, transfer retries, throughput, heap and stack headroom.
5. Exercise cable/network interruptions, baud changes, duplicate controls and
   reconnects; confirm no old bytes cross into a new session.
6. Observe simultaneous gameplay. Keep save-write tests separate from read-only
   integrity tests. Record exact harness, hardware/profile and firmware hash.

## Source references

The user-supplied handoff provides `bridge-txqueue-1`, its extraction starter,
PSB1 contract, and host helper. Hashes are recorded in provenance.json; no
game data, flash dumps, private credentials, or save files are included.
ACK timing follows the existing PlayBridge/BlueRetro PSX reference (BlueRetro,
Apache-2.0) and PSX-SPX documentation. The polling implementation replaces the
old SPI/per-edge controller path; existing repository attribution is preserved.
