# V4.0 ordering package

For the October 2026 JLCPCB assembly corrections, see
[`jlc-2026-10-26/README.md`](jlc-2026-10-26/README.md). That package contains
an order-specific BOM and records placement corrections made in the JLC
viewer. Its corrected CPL is still pending reconciliation; the original
`assembly-cpl.csv` does not reproduce those viewer edits.

The October 26 prototype order uses **JLCPCB full assembly of five V4.0
boards plus one separate top-side stencil**. Bare PCB production files were
approved on October 28. Final JLC placement coordinates and the Q4/Q5
engineering clarification are still missing from the recorded package.
Use the dated package above to understand that order; the following
canonical source exports also support in-house assembly preparation.

- Upload `gerbers-jlcpcb.zip` for the bare-board order.
- `assembly-bom.csv` and `assembly-cpl.csv` contain the 77 fitted references
  for in-house assembly preparation. The BOM LED metadata matches the JLC
  order, but the CPL does **not** include JLC viewer/engineering corrections.
- `lcsc-shopping-list.csv` is the canonical one-board grouped parts list.
  Distributor MOQ quantities may be larger than the design quantity.
- `order-metadata.json` records board and output metadata.

The complete canonical outputs remain under `../exports/`. Confirm board
preview, dimensions, layer count, drill files, and quantity before purchase.
Do not release assembled hardware to production until `../reports/bench-test.md`
has recorded passing results.
