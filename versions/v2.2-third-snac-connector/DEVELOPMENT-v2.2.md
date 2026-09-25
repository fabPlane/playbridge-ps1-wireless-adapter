# PlayBridge V2.2 development plan

V2.2 starts from the validated V2.1 PCB and adds a third SNAC-style
donor-harness connector as J4.

## Fixed baseline

- Keep J1 and J2 at their validated lower-edge positions and orientations.
- Keep J3 USB-C, ESP32/antenna geometry, RESET, BOOT, STATUS, PWR, and the
  TP2 GND / TP3 TX / TP4 RX SERVICE / DEBUG row functional and accessible.
- Preserve the V2.1 board outline and unrelated placement/copper unless a
  measured mechanical constraint makes a scoped change necessary.

## J4 gates

1. Use the same approved nine-contact USB-A connector family and exact 3D
   registration as J1/J2, with its mouth facing outward.
2. Mark J4 and `NOT USB` clearly on F.Silkscreen.
3. Decide and document the J4 signal map, port-selection behavior, power and
   backfeed protection before routing. Never connect native console buses in
   parallel by assumption.
4. Check enclosure and cable/strain-relief clearance with all three connector
   bodies populated.
5. Require DRC 0 errors, ERC 0/0, parity 0, zero unconnected items, populated
   render review, and fresh version-local manufacturing/order outputs.

Ordering remains blocked until the V2.2 electrical architecture and final
mechanical render are explicitly approved.
