# PlayBridge V4.1 — ANTS-compatible controller + serial bridge

Status: **digital verification PASS; production HOLD pending physical bench tests**.

V4.1 is the self-contained FabDesk/KiCad project for the 68 × 60 mm,
two-layer PlayBridge board. It combines the PS1 controller interface on J1,
the PS1 serial interface on J2, USB-C programming/power, protected PS1 power,
and automatic source selection.

## V4.1 fabrication adjustment

- The schematic, component placement, connector mechanics, firmware GPIO map,
  and BOM are unchanged from V4.0.
- All 190 routed through-vias are 0.90 mm diameter with 0.40 mm drills and a
  0.25 mm minimum annular ring.
- Local routing around the ESP32, CH340C, and USB-C fanout was adjusted for the
  enlarged vias. The USB-C VBUS escape uses a documented 0.15 mm neckdown.
- Generated drill data identifies all routed vias as 0.40 mm `ViaDrill` hits.
- ERC reports 0 violations; DRC reports 0 violations and 0 unconnected items.

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

## V3.0 and V4.1 comparison

| Area | V3.0 | V4.1 | Compatibility / action |
| --- | --- | --- | --- |
| J1/J2 | Hong Cheng C7501856; same signal pinout | Same | Harness-compatible |
| Shells | Isolated | J1→GND pad 4; J2→GND pad 7 | Grounding changed |
| Placement | J1 `(8.025,50.825)`; J2 `(26.025,50.825)` mm | J1 `(14,48.5)`; J2 `(39.2,48.5)` mm | Not mechanically drop-in |
| Controller GPIO | CMD32, CLK33, ATT34, DATA19, ACK21 | CMD19, CLK32, ATT21, DATA33, ACK27/sense34 | Firmware change |
| DATA/ACK | 2N7002 open-drain; no ACK sense | Same drivers plus ACK sense | Firmware change |
| J2 UART | TX17 / RX16 | TX17 / RX16 | Compatible |
| Power | AP63203 + AP2112K + TPS2113A | AP63203 + AP2112K + TPS2116 | Mux changed |
| Protection | USBLC6, SMBJ12A, BAT54 clamps | SMAJ TVS, TPD2EUSB30A, ACK ESD | Prototype-test required |
| LEDs | Red power; yellow-green status GPIO4 | Green power; blue status GPIO25 | GPIO/color change |
| Programming | CH340C; manual BOOT/RESET; DTR/RTS | Same | Auto mode unqualified |
| PCB | 56.295 × 62 mm; 4-layer; 1.2 mm | 68 × 60 mm; 2-layer; 1.6 mm | Size/stackup changed |
| Validation | J1/J2 PoC tested | Digital checks pass; bench pending | First prototype only |

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

See `RELEASE-v4.1.0.md` for the digitally verified fabrication-revision statement.
