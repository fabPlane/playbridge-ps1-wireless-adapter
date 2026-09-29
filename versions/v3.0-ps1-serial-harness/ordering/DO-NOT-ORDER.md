# V3.0 ORDER REVIEW COMPLETE — not yet purchased

Date: 2026-09-25

The V3.0 J2 serial implementation is complete. On 2026-09-25 the user confirmed
that the teammate firmware/protocol and electrical PoC passed on the tested
hardware; the two supplied setup photographs are preserved under
`reports/evidence/`. KiCad verification also passes with zero unconnected pads,
zero ERC findings, zero schematic-parity findings, and only the three inherited
locally approved UART0 test-stub warnings.

Fresh V3.0 manufacturing and assembly files have now been generated from the
verified V3.0 board. The package includes J2, R30 and R31. On 2026-09-25 the
files were uploaded to a live JLCPCB Standard PCBA quote for five boards; all
34 BOM rows were confirmed and the placement preview was reviewed. The quote
reached $157.05 before shipping and tax. Save to Cart was not clicked and no
order was placed.

The live quote required one catalog substitution:

- R1, R3, R5, R13, R16, R29, R30 and R31 changed from unavailable UNI-ROYAL
  0603WAF1001T5E / C21190 to Yageo RC0603FR-071KL / C22548. Both are 0603,
  1 kOhm, +/-1%, 100 mW, 75 V resistors; land pattern, routing and placement
  are unchanged.
- J1/J2 were reselected as the exact original C7501856, and SW1/SW2 were
  reselected as the exact original C2837195. JLCPCB's initial zero-quantity
  matches were stale; these were not substitutions.

Current package hashes:

- `gerbers-jlcpcb.zip` SHA-256:
  `a88b31894732a681873b0c5fa4dddc82e82b0fdcf7c66dfae1b98b0ab3e725d8`
- `jlcpcb-bom.csv` SHA-256:
  `dc14498df3f4fc5ce8dd69c2211daf1fb5ab602a2a89adb5fe740414ea47b6fa`
- `jlcpcb-cpl.csv` SHA-256:
  `686bd018bfcfe1f596070ecc0887670ec0c4ec81c907151657b2d36fe336b2dd`
- Generic `exports/fabrication.zip` SHA-256:
  `92a6d5d03cca62bb854a6787f041eaa93b8f6ecc017e9756aa2eaffc077d354a`

The previous external assembler checkout hold is complete for the reviewed
2026-09-25 quote. Before a future purchase, recheck time-sensitive stock and
pricing, confirm the same catalog selections remain active, and perform JLCPCB's
final production-file/placement confirmation. No firmware, protocol, J2-routing,
DRC/ERC, sourcing, or file-generation blocker remains for the tested hardware
configuration.
