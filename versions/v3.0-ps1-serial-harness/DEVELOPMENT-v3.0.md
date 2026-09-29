# PlayBridge V3.0 development plan

V3.0 starts from the validated, frozen V2.1 PCB and evaluates a dedicated
PS1 serial-port donor harness. The requested physical path is ESP32 GPIO17/TX,
GPIO16/RX and GND through a keyed USB-A-style connector marked `NOT USB`.

## Fixed baseline

- Keep J1 and J2 at their validated lower-edge positions and orientations.
- Keep J3 USB-C, ESP32/antenna geometry, RESET, BOOT, STATUS, PWR, and the
  TP2 GND / TP3 TX / TP4 RX SERVICE / DEBUG row functional and accessible.
- Preserve the V2.1 board outline and unrelated placement/copper unless a
  measured mechanical constraint makes a scoped change necessary.

## Architecture gate: J2 or J4

1. Resolve why the physically present, all-NC J2 cannot serve the PS1 serial
   harness, because that was its historical reserved purpose.
2. Add J4 only if the teammate architecture or mechanical requirements prove a
   distinct third connector is necessary.
3. For either connector, use the approved J1/J2 family and exact 3D
   registration, mouth outward, with unambiguous `NOT USB` silkscreen.
4. Assign only TX, RX and GND provisionally; reserve all other contacts until
   the teammate provides the exact harness pinout and electrical specification.
5. Keep UART0 SERVICE/DEBUG on GPIO1/GPIO3 separate from the proposed PS1 link
   on GPIO17/GPIO16. If sharing is later required, select and prove series
   isolation, a mux/switch, or another UART before routing.
6. Check enclosure, cable-overmold and strain-relief clearance for the selected
   connector layout.
7. Require DRC 0 errors, ERC 0/0, parity 0, zero unconnected items and populated
   render review before any manufacturing outputs are regenerated.

Ordering remains blocked until the teammate firmware, protocol, exact logic
levels, directionality, baud/latency targets, harness wiring and supported PS1
revisions are recorded and the V3.0 architecture is explicitly approved.
