# V3.0 J2 serial implementation gate

- Date: 2026-09-25
- PCB DRC errors: 0
- PCB unconnected pads: 0
- Approved historical warnings: 3 `track_dangling` UART0 test stubs
- ERC errors/warnings: 0/0
- Schematic/PCB parity findings: 0
- J2 serial pins: GPIO17/UART2 TX, GPIO16/UART2 RX, GND
- Bench result: PASS, confirmed by the user/team for the tested PoC; supporting
  setup photos are preserved under `reports/evidence/`
- Design gate: PASS
- Manufacturing package gate: PASS — fresh V3.0 Gerbers, drill, BOM, CPL,
  netlist and fabrication archives were regenerated and include J2/R30/R31
- Archive integrity: PASS — both ZIP archives pass `zip -T`
- Assembler checkout gate: PENDING — live stock, substitutions and uploaded
  BOM/CPL rotation/placement previews require final human review
- Order state: HOLD until the assembler checkout gate passes

## J1/J2 sourcing replacement

- Replaced unavailable/pre-order Molex C3197928 with ready-for-assembly Hong
  Cheng HC-USB3.0-L168-ZP / JLCPCB C7501856.
- All nine signal-pad centers are unchanged; no routed copper moved.
- Official C7501856 shell slots, courtyard, footprint and WRL/STEP model are
  included locally.
- New top, perspective, lower-edge and bottom renders confirm correct seating
  and outward-facing connector mouths.

## Release hashes

- PCB: `9f35f14a72d0283f639ca8e53e08e3d4ad675e4c4d4937ba2a34dda01c05f976`
- Schematic: `73d8a4e9e3b117d3dc982e4980389b215d62b1fa0540ba7ee33bbf3e6a9babcf`
- Generic fabrication ZIP: `f9774fabc9b61e5b25378fa0577895634e06d6d3ad8792c2b2a0efb07ab0fc4d`
- JLCPCB Gerber ZIP: `79cee6ab2a2817c448c97eeb5dafd0ae774262c85803f4c59c31d65bda27bab0`
- JLCPCB BOM: `843957971777767e52d6aeeaf623a05dd8f5f9a323449939d0142b77e3f60fef`
- JLCPCB CPL: `686bd018bfcfe1f596070ecc0887670ec0c4ec81c907151657b2d36fe336b2dd`
