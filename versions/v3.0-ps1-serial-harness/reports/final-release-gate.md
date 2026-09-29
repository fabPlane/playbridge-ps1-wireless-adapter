# V3.0 combined board and firmware release gate

- Date: 2026-09-25
- PCB DRC errors: 0
- PCB unconnected pads: 0
- Approved historical warnings: 3 `track_dangling` UART0 test stubs
- ERC errors/warnings: 0/0
- Schematic/PCB parity findings: 0
- J2 serial pins: GPIO17/UART2 TX, GPIO16/UART2 RX, GND
- Connector-zone refill: PASS — no shell-slot/copper clearance violations
- Firmware target: `esp32:esp32:node32s`, Arduino-ESP32 3.3.11
- Combined firmware compile: PASS
- Combined firmware host tests: PASS, 6 tests
- Hardware flash and simultaneous console test: PENDING
- Manufacturing package: PASS — regenerated Gerbers, drill, BOM, CPL,
  netlist and archives include J2/R30/R31 and the J2 `NOT USB` silkscreen
- Archive integrity: PASS — both ZIP archives pass `zip -T`
- Assembler checkout: PENDING — stock, substitutions and BOM/CPL
  rotation/placement previews require human review
- Order state: HOLD until physical validation and assembler checkout pass

## J1/J2 sourcing replacement

- Replaced unavailable/pre-order Molex C3197928 with Hong Cheng
  HC-USB3.0-L168-ZP / JLCPCB C7501856.
- All nine signal-pad centers are unchanged; no routed signal copper moved.
- Official C7501856 shell slots, courtyard, footprint and WRL/STEP model are
  included locally.
- Copper zones were refilled around the new mechanical shell slots; the final
  DRC has zero errors.
- J2 is visibly marked `NOT USB` on fabricated front silkscreen.

## Physical acceptance still required

The prior three-signal proof of concept passed on the documented test setup, but
it does not prove this PCB revision or the combined workload. Before release:

1. Measure J2 contact 4 / ESP32 RX in all power states, including ESP32 off.
2. Scope RX/TX at every supported baud and select/document R30/R31.
3. Pass controller-only and bridge-only tests on an assembled V3.0 board.
4. Pass simultaneous controller input and large bridge transfers without stale
   input, corruption, UART errors, or resets.
5. Complete the assembler preview, stock and substitution review.

## Release hashes

- PCB: `021bbbd606dcc45a21979e1d17c41adfee77bec10d3eb67a69f065ad12b3d648`
- Schematic: `73d8a4e9e3b117d3dc982e4980389b215d62b1fa0540ba7ee33bbf3e6a9babcf`
- Generic fabrication ZIP: `d06c3571e46c5b2771d93d95e1269a83d357a5a69792ccdeed2d44e32c0637ab`
- JLCPCB Gerber ZIP: `08e8110f0dd3d0aeac8df0ad29650505a54588c4303ddc4c9b5df4a56702d868`
- JLCPCB BOM: `843957971777767e52d6aeeaf623a05dd8f5f9a323449939d0142b77e3f60fef`
- JLCPCB CPL: `686bd018bfcfe1f596070ecc0887670ec0c4ec81c907151657b2d36fe336b2dd`
