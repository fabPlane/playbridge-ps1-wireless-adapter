# PlayBridge V2.1 review candidate

Status: **DO NOT ORDER — pending final human review**.

V2.1 preserves the validated V2.0 design and adds two clearly identified indicators plus a connector-free 3.3 V UART service interface. D1/R13 is the firmware STATUS indicator on ESP32 module pin 26. D6/R29 is the always-on 3V3 POWER indicator. The former J4 header is absent.

Top silkscreen identifies J1, J2, J3, and the SERVICE / DEBUG row. V2.1 deliberately has no fabricated J4 reference; the planned third SNAC-style connector in V2.2 will use J4.

The SERVICE/DEBUG row is TP2 GND (43.500,54.000), TP3 UART_TX (46.700,54.000), and TP4 UART_RX (49.900,54.000), with 3.200 mm pitch and 2.0/1.0 mm plated pad/hole geometry. TP1 remains 3V3.

Final 2026-09-25 gates: DRC 0 errors, 0 unconnected, three approved historical `track_dangling` warnings; ERC 0/0; parity 0; release gate PASS. Manufacturing and ordering files were regenerated from PCB SHA-256 `1a1a1da305ea273c94f223b624208c1a9615784997cdfd1591985f96933a2df3`.
