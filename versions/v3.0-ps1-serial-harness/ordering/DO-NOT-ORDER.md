# DO NOT ORDER — V3.0 assembler checkout review required

Date: 2026-09-25

The V3.0 J2 serial implementation is complete. On 2026-09-25 the user confirmed
that the teammate firmware/protocol and electrical PoC passed on the tested
hardware; the two supplied setup photographs are preserved under
`reports/evidence/`. KiCad verification also passes with zero unconnected pads,
zero ERC findings, zero schematic-parity findings, and only the three inherited
locally approved UART0 test-stub warnings.

Fresh V3.0 manufacturing and assembly files have now been generated from the
verified V3.0 board. The package includes J2, R30 and R31:

- `gerbers-jlcpcb.zip` SHA-256:
  `79cee6ab2a2817c448c97eeb5dafd0ae774262c85803f4c59c31d65bda27bab0`
- `jlcpcb-bom.csv` SHA-256:
  `843957971777767e52d6aeeaf623a05dd8f5f9a323449939d0142b77e3f60fef`
- `jlcpcb-cpl.csv` SHA-256:
  `686bd018bfcfe1f596070ecc0887670ec0c4ec81c907151657b2d36fe336b2dd`
- Generic `exports/fabrication.zip` SHA-256:
  `f9774fabc9b61e5b25378fa0577895634e06d6d3ad8792c2b2a0efb07ab0fc4d`

The only remaining hold is external assembler checkout review. Before ordering:

- Confirm live JLCPCB/LCSC availability for Hong Cheng HC-USB3.0-L168-ZP /
  C7501856 (two required per board). It was listed as a ready-for-assembly,
  in-stock part during the 2026-09-25 replacement pass.
- Confirm availability or an approved substitute for L1, MYX
  MCNR4030AF-4R7M / C49449313.
- Verify J2, R30 and R31 in the uploaded BOM/CPL and inspect their rotations and
  placement previews.
- Approve all assembler substitutions and confirm project-local/custom
  footprints are accepted.

Once the upload previews and sourcing checks pass, this ordering hold can be
removed for the tested hardware configuration. It is not a remaining firmware,
protocol, J2-routing, DRC/ERC, or file-generation blocker.
