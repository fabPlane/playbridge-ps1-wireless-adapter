# NewPlayBridge2 build notes

- Board: 68 x 60 mm, two copper layers.
- Status: digital order package passes; production remains on HOLD pending
  physical current-limited power, source-switching, manual programming, J1,
  and J2 tests.
- Manual RESET/BOOT is the accepted programming method. The retained DTR/RTS
  network is documented but is not bench-qualified.
- TP1/TP2/TP3 are plated service holes and are DNP. They remain in the
  engineering BOM/position file and are excluded from the assembly BOM/CPL.
- J1 and J2 are proprietary PlayStation harness ports and are not USB.
- See `firmware-handoff.md`, `order-metadata.json`, and the reports directory.
