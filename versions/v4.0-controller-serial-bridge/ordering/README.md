# V4.0 ordering package

For the October 2026 JLCPCB assembly corrections, see
[`jlc-2026-10-26/README.md`](jlc-2026-10-26/README.md). That package contains
an order-specific BOM and records placement corrections made in the JLC
viewer. Its corrected CPL is still pending reconciliation; the original
`assembly-cpl.csv` does not reproduce those viewer edits.

The current plan is to order bare PCBs and assemble them in-house on the
LumenPnP.

- Upload `gerbers-jlcpcb.zip` for the bare-board order.
- `assembly-bom.csv` and `assembly-cpl.csv` contain the 77 fitted references
  for in-house assembly preparation.
- `lcsc-shopping-list.csv` is the canonical one-board grouped parts list.
  Distributor MOQ quantities may be larger than the design quantity.
- `order-metadata.json` records board and output metadata.

The complete canonical outputs remain under `../exports/`. Confirm board
preview, dimensions, layer count, drill files, and quantity before purchase.
Do not release assembled hardware to production until `../reports/bench-test.md`
has recorded passing results.
