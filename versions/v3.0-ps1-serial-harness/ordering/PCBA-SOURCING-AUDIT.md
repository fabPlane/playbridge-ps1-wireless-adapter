# JLCPCB PCBA sourcing audit — 2026-09-25

## Result

- Fitted references: 67
- Unique orderable line items: 34
- DNP/bare-board test pads: TP1, TP2, TP3, TP4
- Missing fitted references: none
- Duplicate reference assignments: none
- Board routing/placement changed by this sourcing pass: no

## Footprint-sensitive selections

- D2: GOODWORK SMAJ12A / C908766, DO-214AC (SMA). The earlier value label said SMBJ12A, but the PCB uses an SMA land pattern; the selected SMAJ12A preserves the 12 V unidirectional TVS function and fits the footprint family.
- F1: BHFUSE BSMD1206-075-30V / C976305, 1206, 750 mA hold, 1.5 A trip, 30 V.
- L1: Sunlord WPN4020H4R7MT / C98363, 4.0 x 4.0 x 2.0 mm, 4.7 uH +/-20%, 2.85 A rated, 4.9 A saturation, 108 mOhm. The official 1.9 mm pad gap and 1.1 x 3.7 mm lands exactly match the inherited routed footprint.
- R20: UNI-ROYAL 0603WAF6800T5E / C23228, 680 ohm +/-1%. Relative to the previous 698 ohm selection, the calculated TPS2113A current limit increases by approximately 2.65%; verify intended current-limit acceptability.
- C3: Samsung CL21A106KOQNNNE / C1713, 10 uF 16 V X5R +/-10%, 0805.
- C9: Samsung CL21A226MQQNNNE / C5674, 22 uF 6.3 V X5R +/-20%, 0805.
- D1: KENTO KT-0603YG / C2289, yellow-green 0603 STATUS LED; polarity and STATUS_LED wiring are unchanged.
- J1/J2: Hong Cheng HC-USB3.0-L168-ZP / C7501856. JLCPCB listed this as a ready-for-assembly in-stock part during the 2026-09-25 replacement pass; recheck live stock at checkout. Its nine electrical-pad coordinates exactly match the replaced connector, so routing is unchanged. J2 carries the user/team bench-validated three-signal PS1 serial harness on contacts 1 (PS1_RXD), 4 (PS1_TXD) and 7 (GND); all other signal contacts remain NC.
- D6: KENTO KT-0603R / C2286, red 0603 always-on 3V3 POWER indicator.
- R29: UNI-ROYAL 0603WAF1001T5E / C21190, 1 kΩ 0603 POWER-LED series resistor.

## Order files

- `jlcpcb-bom.csv`: grouped JLC/LCSC BOM; all fitted references assigned exactly once.
- `jlcpcb-cpl.csv`: top-side placement list derived from FabDesk's existing `exports/positions.csv`, excluding bare-board test pads.
- `gerbers-jlcpcb.zip`: flat manufacturing Gerber/drill archive copied from FabDesk's verified export set.

## Fresh release verification

- KiCad DRC: 0 errors, with only the three approved historical track_dangling warnings
- Unconnected items: 0
- Schematic-to-PCB parity issues: 0
- KiCad ERC: 0 violations
- Board and schematic hashes remained unchanged during the BOM/order-file pass.

Stock is time-sensitive; JLCPCB/LCSC availability must be rechecked by the order portal at upload time.
