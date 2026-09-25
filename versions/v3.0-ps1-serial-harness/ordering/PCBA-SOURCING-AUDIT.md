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
- L1: MYX MCNR4030AF-4R7M / C49449313, 4.0 x 4.0 mm, 4.7 uH, 2.5 A rated, 3 A saturation. The inherited sourcing audit recorded low stock (40 units); this regeneration did not perform a live-stock query, so recheck immediately before ordering.
- R20: TA-I RMS06FT6980 / C912219, 698 ohm E96 substitute for the 700 ohm nominal. The TPS2113A current-limit setpoint difference is approximately +0.29%.
- J1/J2: Molex 48393-0003 / C3197928. The inherited sourcing audit recorded only 7 available units; this regeneration did not perform a live-stock query. With two populated per board, recheck stock at checkout. J2 carries the user/team bench-validated three-signal PS1 serial harness on contacts 1 (PS1_RXD), 4 (PS1_TXD) and 7 (GND); all other signal contacts remain NC.
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
