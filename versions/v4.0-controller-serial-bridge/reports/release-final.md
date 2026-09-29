# NewPlayBridge2 final digital verification report

Date: 2026-09-29

## Status

**Digital design/order package: PASS. Production: HOLD pending physical bench tests.**

No bench qualification is claimed. Manual BOOT/RESET is the accepted
programming path.

## Final ESP32 map

| Function | GPIO | U1 pad | Direction / path |
| --- | ---: | ---: | --- |
| PS1 CMD | 19 | 31 | output via R7 1 kOhm |
| PS1 CLK | 32 | 8 | output via R9 1 kOhm |
| PS1 ATT | 21 | 33 | output via R21/R22 and R10 1 kOhm |
| PS1 DATA drive | 33 | 9 | output via R8 100 Ohm to Q1 gate |
| PS1 ACK drive | 27 | 12 | output via R37 100 Ohm to Q2 gate |
| PS1 ACK sense | 34 | 6 | input via R11 |
| Status LED | 25 | 10 | output via R42 to D9 |
| UART2 TX | 17 | 28 | output via R14 1 kOhm to J2 pin 1 |
| UART2 RX | 16 | 27 | input via R15 1 kOhm from J2 pin 4 |
| UART0 TX | 1 | 35 | CH340/debug only |
| UART0 RX | 3 | 34 | CH340/debug only |

GPIO13/U1 pad 16 and GPIO5/U1 pad 29 are unconnected.

## Open-drain stages

- Q1 is a 2N7002 DATA driver: pin 1 gate=`DATA_GATE`, pin 2 source=`GND`,
  pin 3 drain=`J1_DATA`.
- Q2 is a 2N7002 ACK driver: pin 1 gate=`ACK_GATE`, pin 2 source=`GND`,
  pin 3 drain=`J1_ACK`.
- Gate resistors R8/R37 remain 100 Ohm. The proven external series parts
  R7/R9/R10/R14/R15 remain 1 kOhm, MPN `0603WAF1001T5E`, LCSC `C21190`.

## BOOT/RESET topology

- SW1 manually pulls `ESP_EN` low.
- SW2 manually pulls `ESP_BOOT`/GPIO0 low.
- Retained Q4 MMBT3904: base from DTR through R39, emitter=`CH340_RTS`,
  collector=`ESP_EN`.
- Retained Q5 MMBT3904: base from RTS through R40, emitter=`CH340_DTR`,
  collector=`ESP_BOOT`/GPIO0.
- Q4/Q5 pin semantics are 1=B, 2=E, 3=C. Automatic behavior is not claimed
  as bench-qualified; the manual switches are accepted.

## Connectors and mechanics

- J1: 1 NC, 2 CMD, 3 CLK, 4 GND, 5 ATT, 6 DATA, 7 ACK, 8 PS1 power, 9 NC.
  Both retention/shell pads are pad 4/GND.
- J2: 1 TX, 4 RX, 7 GND; all other signal pins NC. Both retention/shell pads
  are pad 7/GND.
- J1/J2 anchors remain `(14.0,48.5)` and `(39.2,48.5)` mm. Their exact local
  HC-USB3.0-L168-ZP models use zero translation/rotation and unit scale.
- All nine connector pins and both retention legs cross the 1.6 mm PCB and
  protrude approximately 1.200096 mm below it for soldering.

## Validation

- ERC: 0 errors, 0 warnings.
- DRC: 0 violations, 0 unconnected pads, 0 schematic-parity issues.
- Source/schematic/PCB: 80/80 references, 53/53 connected nets, 227/227
  connected nodes, 0 value/footprint or MPN/LCSC mismatches.
- 3D models: 77/80 footprints. The only omissions are intentionally
  unpopulated plated holes TP1/TP2/TP3; every fitted footprint is modeled.
- Engineering BOM/positions: 80/80 references.
- Assembly BOM/CPL: 77/77 references, exact reference-set match, no blank
  LCSC codes.
- Fabrication ZIP: 13 entries and passes archive integrity testing.
- GLB: valid binary glTF v2 container.

## Core hashes

- schematic: `a37339ff2e3cd012f82b29cc9e7a413b695dbe06d35fb2b2dbc70e65833c2766`
- PCB: `55547e15ad8372b8ee5ed35aa6e1b8c1242d4903766b599b63c3a6ae3d871c63`
- source netlist: `7104430e672968607f18d1a9db0ea59e10c2b096bb1d0f0d475dc129700e53ef`
- GLB: `8cc8613cc6905d297376728690fd6a402aa38cf638dc03a9b4a27a006541b13c`
- fabrication ZIP: `821d027b0d9d7d508d3c2c3af538216915e23e5e23f13232947d0c2e20bbf30e`
- assembly BOM: `7c8db1dd1eff2c2cde9a478bdd75c6df08e845f80563c4fcab68b336eed56539`
- CPL: `688b4a6d6405525f1ae15db4892079bc81b94ff4be9aa11bdee67e2d655c9458`

See `reports/bench-test.md`. Do not release to production until its required
physical measurements have been completed and recorded.
