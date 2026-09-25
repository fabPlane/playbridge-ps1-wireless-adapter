# DO NOT ORDER — V3.0 manufacturing package not regenerated

Date: 2026-09-25

The V3.0 J2 serial implementation is complete and the user reports that the
bench test passed on the tested hardware. KiCad verification also passes with
zero unconnected pads, zero ERC findings, zero schematic-parity findings, and
only the three inherited locally approved UART0 test-stub warnings.

The remaining hold is not the J2 routing or tested UART function. The checked-in
ordering package still represents the inherited V2.1 board:

- `jlcpcb-bom.csv` still identifies J2 as the reserved/all-NC footprint and does
  not contain the new R30/R31 series resistors.
- `jlcpcb-cpl.csv` and `gerbers-jlcpcb.zip` predate the routed V3.0 J2 design.
- The BOM/CPL rotation preview and any assembler substitutions have not been
  reviewed against a freshly uploaded V3.0 package.

Before ordering, regenerate the Gerbers, drill files, BOM, CPL and fabrication
ZIP from the committed V3.0 board, then perform these checkout checks:

- Confirm live JLCPCB/LCSC availability for Molex 48393-0003 / C3197928 (two
  required per board).
- Confirm availability or an approved substitute for L1, MYX
  MCNR4030AF-4R7M / C49449313.
- Verify J2, R30 and R31 in the uploaded BOM/CPL and inspect their rotations and
  placement previews.
- Approve all assembler substitutions and confirm project-local/custom
  footprints are accepted.

Once the fresh package and checkout preview pass those checks, this ordering
hold can be removed for the tested hardware configuration.
