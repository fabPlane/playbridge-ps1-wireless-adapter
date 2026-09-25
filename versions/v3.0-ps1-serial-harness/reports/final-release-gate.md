# V3.0 J2 serial implementation gate

- Date: 2026-09-25
- PCB DRC errors: 0
- PCB unconnected pads: 0
- Approved historical warnings: 3 `track_dangling` UART0 test stubs
- ERC errors/warnings: 0/0
- Schematic/PCB parity findings: 0
- J2 serial pins: GPIO17/UART2 TX, GPIO16/UART2 RX, GND
- Bench result: PASS, reported by the user for the tested hardware
- Design gate: PASS
- Manufacturing package gate: FAIL — checked-in Gerbers/BOM/CPL are inherited
  from V2.1 and must be regenerated from V3.0
- Order state: DO-NOT-ORDER until the fresh package and checkout preview pass
