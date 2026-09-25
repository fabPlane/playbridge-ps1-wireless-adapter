# PlayBridge V3.0 — PS1 serial harness evaluation

Status: **J2 implementation and bench test passed; manufacturing package refresh required before ordering**.

This directory is the authoritative V3.0 development project derived from the
validated, frozen V2.1 release. V3.0 evaluates a third keyed donor-harness
connector, J4, for a PS1 serial-port link while preserving the V2.1 status
LEDs and connector-free UART0 service row.

## V3.0 design intent

- Evaluate whether the currently all-NC J2 can safely serve the PS1 serial
  harness before adding another connector. Do not populate both J2 and J4 for
  the same purpose without a documented mechanical/electrical reason.
- If J4 remains necessary, use one outward-facing USB-A connector body matching
  J1/J2 and identify it clearly as J4 and `NOT USB`.
- Preserve J1, J2, J3, the SERVICE / DEBUG row, RESET, BOOT, STATUS, PWR,
  antenna clearance, and all V2.1 functionality.
- Confirm the PS1 serial-harness pinout, logic levels, direction, protocol,
  throughput/latency, supported console revisions, and isolation/backfeed
  policy before committing copper. Do not silently parallel console buses.
- Keep the intended path explicit: PC/browser -> Wi-Fi -> ESP32 -> GPIO17/TX,
  GPIO16/RX and GND -> keyed NOT-USB harness -> PS1 serial port.
- The user/team confirmed the teammate firmware/protocol and electrical bench
  PoC on 2026-09-25. The preserved photos document the powered test hardware;
  functional operation is a user-reported bench result rather than something
  independently visible in the still images. Do not generalize that result to
  untested console revisions, cables, protocol modes, or full handshaking.

## V2.1 baseline retained

- D1 is the green firmware `STATUS` LED: Everlight `19-217/GHC-YR1S2/3T` / LCSC `C72043`, driven by ESP32 module pin 26 on `STATUS_LED` through R13, 1 kΩ UNI-ROYAL `0603WAF1001T5E` / `C21190`.
- D6 is the red always-on 3V3 `PWR` LED: KENTO `KT-0603R` / `C2286`, through R29, 1 kΩ UNI-ROYAL `0603WAF1001T5E` / `C21190`.
- The physical J4 header was removed. The lower-right SERVICE/DEBUG row contains DNP plated holes: TP2 GND at (43.500, 54.000) mm, TP3 UART_TX at (46.700, 54.000) mm, and TP4 UART_RX at (49.900, 54.000) mm. Pitch is 3.200 mm; each pad is 2.0 mm with a 1.0 mm finished plated hole.
- J1, J2, J3, and SERVICE / DEBUG are explicitly identified on F.Silkscreen. No J4 reference is fabricated in V2.1; V3.0 must first decide whether J2 can serve the serial harness or whether a distinct J4 is justified.
- TP1 remains the existing 3V3 test point. UART signals are 3.3 V logic only.
- RESET, BOOT, J1/J2/J3, USB orientations, antenna keepout, outline, and unrelated V2.0 placement/copper remain unchanged.

## Final verification and outputs

Verified 2026-09-25: DRC 0 errors, 0 unconnected, and only three approved historical `track_dangling` warnings; ERC 0 errors/0 warnings; schematic/PCB parity 0; release gate PASS.

- Accepted verification renders: `renders/service-debug-full-top.png` and `renders/service-debug-close.png`.
- Generic outputs: `exports/gerbers/`, `exports/gerbers/board.drl`, `exports/bom.csv`, `exports/positions.csv`, `exports/assembly-bom.csv`, `exports/assembly-cpl.csv`, `exports/netlist.net`, and `exports/fabrication.zip`.
- JLCPCB preparation: `ordering/jlcpcb-bom.csv`, `ordering/jlcpcb-cpl.csv`, and `ordering/gerbers-jlcpcb.zip`.
- Detailed evidence: `reports/V2.1-AUDIT.md`.
- V3.0 PoC evidence: `reports/evidence/README.md` and the two preserved setup
  photographs.
- V3.0 release result and package hashes: `reports/final-release-gate.md`.

**ORDER CHECKOUT HOLD:** fresh V3.0 manufacturing files are generated from the
verified board. Before purchase, upload them to the assembler and complete the
BOM/CPL rotation, substitution, and live-stock review described in
`ordering/DO-NOT-ORDER.md`. The user-reported J2 bench test has passed.

The fabrication exports and validation reports have been regenerated for V2.1.

V2 replaces the PCB-mounted PlayStation pin array with a removable donor-plug harness.

The main PlayBridge board will expose removable multi-contact connectors. A console-specific adapter will use a genuine donor controller cable and molded PlayStation male plug soldered to a matching breakout board. Swapping the adapter cable will make it possible to support other console systems without manufacturing their proprietary plug contacts directly on the main PCB.

## Architecture

- Keep the ESP32, signal protection and protocol-generation circuitry on the main board.
- Replace the console-facing PCB pin array with a removable cable interface.
- Use the genuine molded PlayStation donor plug for console alignment, contact geometry and insertion loads.
- Map every donor wire by continuity testing; never rely on wire color.
- Treat the observed missing pin 8 as a donor-plug property that must be verified electrically.
- Define connector pin allocation, power/backfeed protection, shield policy and cable requirements before designing the V2 PCB.
- Add other console adapters only after the PS1 harness is validated.

## Safety warning

The concept currently uses USB 3 Type-A connector hardware because it provides enough inexpensive contacts and readily available breakout boards. The signals would be native console signals—not USB. Any implementation must be clearly marked **NOT USB**, protected against accidental connection to computers/chargers, and reviewed before hardware is powered.

## References

- [`concepts/`](concepts/) contains the supplied system architecture, breakout-board example and illustrative product layout.
- [`references/ps1-donor-plug/`](references/ps1-donor-plug/) contains photographs of the actual donor plug, including the empty pin-8 position.

## V2.0.0 baseline checkpoint

This directory began as a self-contained copy of the V2.0.0 release snapshot. It includes the
KiCad project, FabDesk netlist/configuration, local symbol/footprint/3D-model
libraries, verification reports, manufacturing package, release tools,
references, and accepted renders.

The baseline details below remain useful for comparison, but V2.1 must pass a
fresh release gate after modification.

- Board: 56.295 × 62.000 mm, four layers, 1.20 mm thick.
- J1: Molex 48393-0003 / JLCPCB C3197928 used as a nine-contact **non-USB** PS1 donor-harness interface.
- J2: the same connector reserved for a future rear-serial harness; its contacts remain intentionally unassigned until that interface is characterized.
- J1/J2 use `Molex_48393-0003_TraceParts_exact_visual.wrl`, converted from the exact TraceParts catalog preview for 48393-0003. Registration is documented in `PlayBridge.3dshapes/README_MOLEX_48393-0003_TRACEPARTS.md`.
- J1 is at (4.525, 50.525) mm, J2 at (22.525, 50.525) mm, both at 0°, with their mating mouths facing outward through the lower edge. F1 is at (12.500, 38.000) mm. J3 remains at (51.025, 9.625) mm, 90°, facing outward through the right edge.
- ERC, DRC, schematic parity, and connectivity currently pass with zero errors, warnings, or unconnected items. Ordering remains subject to the sourcing and PCBA-completeness checks in `ordering/`.

See [`RELEASE-v2.0.0.md`](RELEASE-v2.0.0.md) for the release gate,
manufacturing-package status, 3D-model provenance, and remaining checkout hold.

## Snapshot layout

- `board.kicad_*`, `circuit.netlist.json`, and `fabdesk.json` — complete V2 design.
- `PlayBridge.pretty/`, `PlayBridge.3dshapes/`, and the symbol libraries — project-local libraries required to reopen the design.
- `ordering/` — Gerber archive, BOM, CPL, sourcing audit, and checkout hold.
- `reports/` — release build, DRC, ERC, statistics, and gate reports.
- `tools/` — scripts used for model inspection and order-file generation.
- `renders/` — accepted V2.0.0 renders only.

## USB-A mechanical fit

J1 and J2 use the official Molex 48393-0003 through-hole pattern: nine 0.70 mm
signal holes plus four polarized shell-retention slots. The 13.80 × 14.25 mm
connector body fits the footprint and requires no PCB cutout. With the current
placement, its mating lip projects 0.275 mm beyond the straight board edge so
the cable remains accessible, while every signal pin and retention leg remains
inside the board outline. The V2 PCB thickness is 1.20 mm, matching the PCB
thickness stated on Molex drawing `483930003_sd.pdf`.

## 3D-model validation

The datasheet-derived model contains separate solids for all nine signal tails
and four shell-retention posts. Their centerlines match the corresponding
footprint holes/slots with a reported XY delta of 0.000 mm. The model uses zero
offset and zero rotation in the footprint frame; its WRL scale is
`0.3937007874` on all axes. The nominal tails/posts extend 1.10 mm below the
top seating datum on the 1.20 mm PCB.

The production footprints retain the official Molex entry geometry. The
TraceParts visual model is used for populated-board inspection, while the
datasheet-derived section proof remains available for dimensional audit.
See `PlayBridge.3dshapes/README_MOLEX_48393-0003_TRACEPARTS.md` and
`PlayBridge.3dshapes/Molex_48393-0003_DATASHEET_DERIVED_README.md`.
