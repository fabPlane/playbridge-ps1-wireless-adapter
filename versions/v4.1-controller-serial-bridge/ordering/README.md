# V4.1 ordering package

The current plan is to order bare PCBs and assemble them in-house on the
LumenPnP.

- Upload `NewPlayBridge2-v4.1-ANTS-Gerbers.zip` when replying to ANTS PCB.
  It is a vendor-named copy of the verified 13-file fabrication archive.
- `gerbers-jlcpcb.zip` remains the generic bare-board-order copy of the same
  fabrication data.
- Confirm the drill preview shows 190 routed vias with 0.40 mm drills and
  0.90 mm copper diameters before purchase.
- `assembly-bom.csv` and `assembly-cpl.csv` contain the 77 fitted references
  for in-house assembly preparation.
- `lcsc-shopping-list.csv` is the canonical one-board grouped parts list.
  Distributor MOQ quantities may be larger than the design quantity.
- `order-metadata.json` records board and output metadata.

The complete canonical outputs remain under `../exports/`. Confirm board
preview, dimensions, layer count, drill files, and quantity before purchase.
Do not release assembled hardware to production until `../reports/bench-test.md`
has recorded passing results.
