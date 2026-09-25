# DO NOT ORDER — V3.0 physical validation required

Date: 2026-09-25

The V3.0 PCB, manufacturing files, and combined controller/serial firmware pass
the automated checks documented in `reports/final-release-gate.md`. The latest
Hong Cheng C7501856 connector footprint is included, its shell-slot clearances
were corrected by refilling the copper zones, and J2 now carries a fabricated
`NOT USB` warning.

These automated results do not establish electrical safety or simultaneous
real-console operation. Keep this order hold in place until all steps below are
recorded against a built V3.0 board:

- Confirm live JLCPCB/LCSC availability for two Hong Cheng
  HC-USB3.0-L168-ZP / C7501856 connectors per board and for L1 (MYX
  MCNR4030AF-4R7M / C49449313), or approve documented substitutes.
- Verify J2, R30 and R31 in the uploaded BOM/CPL and inspect connector and
  component rotations/placements in the assembler preview.
- Confirm the physical J2 `NOT USB` silkscreen is present and legible.
- Measure J2 contact 4 / ESP32 RX voltage in every console and adapter power
  state, including with the ESP32 unpowered. Do not proceed if it exceeds the
  ESP32 input limits or back-powers the board.
- Scope J2 RX/TX at every supported baud rate and select/document R30/R31 values
  that give safe levels and adequate signal integrity.
- Flash the combined firmware and pass controller-only, bridge-only, and
  simultaneous controller-plus-large-transfer console tests without stale
  input, UART errors, resets, or data corruption.
- Approve all assembler substitutions and confirm the project-local footprints
  and mechanical slots are accepted.

Current release artifacts:

- `gerbers-jlcpcb.zip` SHA-256:
  `08e8110f0dd3d0aeac8df0ad29650505a54588c4303ddc4c9b5df4a56702d868`
- `jlcpcb-bom.csv` SHA-256:
  `843957971777767e52d6aeeaf623a05dd8f5f9a323449939d0142b77e3f60fef`
- `jlcpcb-cpl.csv` SHA-256:
  `686bd018bfcfe1f596070ecc0887670ec0c4ec81c907151657b2d36fe336b2dd`
- Generic `exports/fabrication.zip` SHA-256:
  `d06c3571e46c5b2771d93d95e1269a83d357a5a69792ccdeed2d44e32c0637ab`

Remove this hold only after both physical validation and assembler checkout
pass. The earlier three-signal proof of concept is supporting evidence, not a
substitute for testing this manufactured V3.0 board and combined firmware.
