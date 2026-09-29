# PlayBridge V4.0.0 digital design checkpoint

Date: 2026-09-29

V4.0.0 records the digitally verified two-layer controller/serial bridge and
its manufacturing outputs for bench testing. The canonical board is 68 × 60 mm
and provides a PS1 controller harness on J1, PS1 UART2 serial harness on J2,
USB-C programming/power, protected PS1 power, and TPS2116 source multiplexing.

The digital verification gate passes: ERC, DRC, schematic/PCB parity, connectivity,
BOM/CPL consistency, 3D model coverage for fitted parts, fabrication archive
integrity, and the SHA-256 manifest all pass. Procurement metadata includes
the final LCSC substitutions and consolidates all nine 1 kΩ 0603 resistors on
C21190.

This is not a production qualification. Production remains on hold until the
physical measurements in `reports/bench-test.md` pass and are recorded.
The Magnetsyc CPN4020H4R7MT inductor is accepted for the expected sub-2 A load
but must not be claimed for sustained 2.0 A operation without thermal testing.

Order files are under `exports/` and convenient copies are under `ordering/`.
