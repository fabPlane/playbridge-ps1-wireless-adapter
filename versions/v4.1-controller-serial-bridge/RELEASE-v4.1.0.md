# PlayBridge V4.1.0 fabrication revision

Date: 2026-10-01

V4.1.0 preserves the electrically verified V4.0 controller/serial bridge while
updating the PCB for the ANTS fabrication limits. Every routed through-via is
now 0.90 mm in diameter with a 0.40 mm drill, providing a 0.25 mm annular ring.
The affected ESP32, CH340C, and USB-C escape routing was adjusted without
changing the schematic, component placement, connector mechanics, firmware
GPIO map, or BOM.

The regenerated digital release gate passes:

- ERC: 0 violations.
- DRC: 0 violations and 0 unconnected items.
- Routed-via audit: 190/190 vias at 0.90/0.40 mm.
- Excellon audit: 190 routed `ViaDrill` hits at 0.40 mm.
- Schematic/PCB connectivity remains 53 named nets and 227 connected nodes.
- Fabrication and ordering ZIPs contain the complete 13-file Gerber/drill set
  and pass archive-integrity testing.
- Updated board SVG, GLB, top, bottom, and perspective renders are included.

This is a fabrication adjustment, not production qualification. Production
remains on hold until the physical measurements in `reports/bench-test.md`
pass and are recorded. The Magnetsyc CPN4020H4R7MT remains accepted for the
expected sub-2 A load but is not qualified for sustained 2.0 A operation
without thermal testing.

Order files are under `exports/`; convenient vendor-facing copies are under
`ordering/`.
