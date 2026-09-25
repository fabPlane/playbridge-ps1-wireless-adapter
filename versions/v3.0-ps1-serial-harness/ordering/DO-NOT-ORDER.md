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
  `53c5e7433b1f62957e516f66e7e664dc6d699ae5377469fee0ea4a07bdd9e771`
- `jlcpcb-bom.csv` SHA-256:
  `940b544198f858c39223112ffdff71d2602239547ab33d38eca0a42286edc9f0`
- `jlcpcb-cpl.csv` SHA-256:
  `731d9a0e6846cbff34ae818d6703acabe2dd118965fbf10fea5762f6c6c0a346`
- Generic `exports/fabrication.zip` SHA-256:
  `6df04be37730c71ab8cdb788056cc6929ac6e8ec569c8a4d1aaf6fa4ee44f648`

The only remaining hold is external assembler checkout review. Before ordering:

- Confirm live JLCPCB/LCSC availability for Molex 48393-0003 / C3197928 (two
  required per board).
- Confirm availability or an approved substitute for L1, MYX
  MCNR4030AF-4R7M / C49449313.
- Verify J2, R30 and R31 in the uploaded BOM/CPL and inspect their rotations and
  placement previews.
- Approve all assembler substitutions and confirm project-local/custom
  footprints are accepted.

Once the upload previews and sourcing checks pass, this ordering hold can be
removed for the tested hardware configuration. It is not a remaining firmware,
protocol, J2-routing, DRC/ERC, or file-generation blocker.
