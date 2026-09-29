# PlayBridge V4.0 — controller + serial bridge

Status: **digital verification PASS; production HOLD pending physical bench tests**.

V4.0 is the self-contained FabDesk/KiCad project for the 68 × 60 mm,
two-layer PlayBridge board. It combines the PS1 controller interface on J1,
the PS1 serial interface on J2, USB-C programming/power, protected PS1 power,
and automatic source selection.

## Interfaces

- J1: pin 1 NC, 2 CMD, 3 CLK, 4 GND, 5 ATT, 6 DATA, 7 ACK, 8 PS1 power,
  and 9 NC. Both shell/retention pads are GND through pad 4.
- J2: pin 1 TX, 4 RX, and 7 GND; all other signal pins are NC. Both
  shell/retention pads are GND through pad 7.
- J1 and J2 use USB-A connector hardware only as keyed PS1 harness ports.
  They are **NOT USB**.
- The SERVICE / DEBUG holes are GND, UART0 TX, and UART0 RX at 3.2 mm pitch.

The exact firmware GPIO map is in `exports/firmware-handoff.md` and
`exports/firmware-pin-map.h`.

## V3.0 and V4.0 comparison

| Area | V3.0 | V4.0 | Compatibility / action |
| --- | --- | --- | --- |
| J1 harness pins | 2 CMD, 3 CLK, 4 GND, 5 ATT, 6 DATA, 7 ACK, 8 PS1 power; 1/9 NC | Same signal allocation | Harness pin allocation retained |
| J2 harness pins | 1 ESP32 TX, 4 ESP32 RX, 7 GND; other signal pins NC | Same signal allocation | Harness pin allocation retained |
| J1/J2 shells | Mechanically anchored and electrically isolated | J1 shells tied to pad 4/GND; J2 shells tied to pad 7/GND | Grounding policy changed |
| Controller GPIOs | CMD 32, CLK 33, ATT 34, DATA drive 19, ACK drive 21 | CMD 19, CLK 32, ATT 21, DATA drive 33, ACK drive 27, ACK sense 34 | Controller firmware pin map must change |
| DATA/ACK stages | 2N7002 open-drain drivers | 2N7002 open-drain drivers | Electrical drive method retained; V4 adds separate ACK sensing |
| J2 UART | GPIO17 TX / GPIO16 RX | GPIO17 TX / GPIO16 RX | Firmware-compatible |
| Status LED | GPIO4 | GPIO25 | Firmware pin must change |
| LEDs | Always-on power LED plus firmware status LED | Always-on power LED plus firmware status LED | Function retained |
| Power mux | TPS2113A | TPS2116 | Power architecture revised; bench-test source switching/backfeed |
| Programming | CH340C, manual BOOT/RESET, DTR/RTS transistor network | CH340C, manual BOOT/RESET, DTR/RTS transistor network | Manual method retained; V4 automatic behavior is not bench-qualified |
| PCB | 56.295 × 62.000 mm, four layers, 1.20 mm thick | 68 × 60 mm, two layers, 1.60 mm thick | Dimensions and stackup changed |
| J1/J2 connector | Molex 48393-0003 / C3197928 | Hong Cheng HC-USB3.0-L168-ZP / C7501856 | Connector model and PCB footprint changed |
| Validation | J1 baseline and J2 serial PoC have recorded bench evidence | Digital checks pass; physical bench plan is not yet executed | Complete V4 bench plan before production approval |

## Electrical and mechanical verification evidence

- ERC: 0 errors and 0 warnings.
- DRC: 0 violations, 0 unconnected pads, and 0 schematic-parity issues.
- Connectivity: 53/53 named nets and 227/227 connected nodes match.
- Board: 68 × 60 mm, two copper layers.
- J1/J2 signal pins and retention legs cross the 1.6 mm PCB and protrude
  about 1.2 mm below it for soldering.
- Every fitted footprint has a 3D model; TP1/TP2/TP3 are intentionally DNP.
- Engineering BOM: 80 references. Assembly BOM/CPL: 77 fitted references,
  with an exact reference-set match and no blank LCSC codes.
- The fabrication ZIP contains the complete Gerber and drill set and passes
  archive integrity testing.
- `exports/manifest-sha256.txt` verifies the canonical project, manufacturing
  outputs, accepted renders, and final reports.

## Directory layout

- `board.kicad_*`, `circuit.netlist.json`, and `fabdesk.json` — canonical project.
- `NewPlayBridge.pretty/`, `NewPlayBridge.kicad_sym`, and
  `PlayBridge.3dshapes/` — project-local libraries and models.
- `exports/` — canonical Gerbers, fabrication ZIP, BOM, CPL, positions,
  shopping list, firmware handoff, interchange files, and checksum manifest.
- `ordering/` — convenient vendor-facing copies for bare-board ordering and
  in-house assembly preparation.
- `reports/` — final electrical, mechanical, procurement, and verification evidence.
- `renders/` — generated populated-board, connector, and GLB renders.

Open this version directory itself in FabDesk; do not open the repository root.

## Bench-test hold

No assembled V4 board has been bench-qualified. Complete and record the plan
in `reports/bench-test.md` before production approval. In particular, verify
current-limited USB and PS1 power, TPS2116 source switching and backfeed,
manual programming, J1 controller waveforms, J2 UART operation, and L1
temperature/current margin. Manual BOOT/RESET is the accepted programming path.

See `RELEASE-v4.0.0.md` for the digitally verified design-checkpoint statement.
