# PlayBridge V2.1 development record

V2.1 is derived from frozen V2.0 and preserves its USB connectors, switches, ESP32 placement/antenna keepout, outline, and unrelated routing.

Final scoped changes:

- D1/R13: green firmware STATUS LED on ESP32 module pin 26, `STATUS_LED`, 1 kΩ series resistance.
- D6/R29: red always-on 3V3 POWER LED, 1 kΩ series resistance.
- J4 removed.
- J1/J2/J3 and SERVICE / DEBUG added as explicit F.Silkscreen interface labels. J4 is reserved for the planned third SNAC-style connector in V2.2.
- TP1 retained as 3V3.
- TP2 GND (43.500,54.000), TP3 UART_TX (46.700,54.000), TP4 UART_RX (49.900,54.000); DNP 2.0/1.0 mm plated holes at 3.2 mm pitch.

Final board SHA-256: `1a1a1da305ea273c94f223b624208c1a9615784997cdfd1591985f96933a2df3`.

Final checks dated 2026-09-25: DRC 0 errors/0 unconnected with only three accepted historical dangling-track warnings; ERC 0/0; parity 0; release gate PASS.

See `reports/V2.1-AUDIT.md` for measurements and `ordering/DO-NOT-ORDER.md` for the remaining human review hold.
