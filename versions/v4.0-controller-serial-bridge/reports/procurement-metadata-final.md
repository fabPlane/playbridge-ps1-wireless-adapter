# NewPlayBridge2 procurement metadata update

Date: 2026-09-29

## Applied substitutions

| References | Final manufacturer / MPN | LCSC | Quantity |
| --- | --- | --- | ---: |
| L1 | Magnetsyc CPN4020H4R7MT | C52741363 | 1 |
| Q4, Q5 | onsemi MMBT3904LT1G | C81464 | 2 |
| U2 | WCH CH340C | C84681 | 1 |
| R5, R7, R9, R10, R11, R14, R15, R41, R42 | 0603WAF1001T5E | C21190 | 9 |

The red power LED D8 remains MPN 19-217/GHC-YR1S2/3T, LCSC C2286.
No C22548, C98363, C20526, or C7464026 identifiers remain in the canonical
source, schematic, PCB metadata, engineering BOM, assembly BOM, or shopping
list.

## L1 engineering review

The electrical inductance remains 4.7 uH +/-20% and the existing 4 x 4 mm
footprint remains unchanged. CPN4020H4R7MT is rated for 2.5 A DC current,
4.7 A saturation current, and approximately 123 milliohm DCR.

The 4.7 A saturation rating comfortably exceeds the AP63203 full-load peak
inductor-current range. The 2.5 A DC rating is acceptable for this board only
while sustained load remains below approximately 1.85 A. It provides 25%
headroom over a 2.0 A load, while the AP63203 datasheet recommends at least
35% DC-current-rating headroom (2.7 A for a sustained 2.0 A load). The 123
milliohm DCR is also above the datasheet's preferred sub-100-milliohm value
for highest efficiency.

Therefore L1 is accepted for the expected sub-2 A board load, subject to the
existing production HOLD and current/temperature verification during bench
qualification. It must not be represented as qualified for continuous 2.0 A
operation without thermal testing or a higher-current inductor.

## No-functional-change guard

- ERC: 0 errors, 0 warnings.
- DRC: 0 violations, 0 unconnected pads, 0 schematic parity issues.
- Connected-net parity: 53/53 named nets, 0 node mismatches.
- Metadata-normalized source, schematic, and PCB signatures exactly match
  their pre-update signatures.
- Q4/Q5 remain SOT-23 with pins 1=B, 2=E, 3=C and retain their DTR/RTS nets.
- CPL, engineering positions, fabrication ZIP, every Gerber/drill/map, and
  exported connectivity/netlist files are byte-for-byte unchanged.
- Engineering BOM: 80 references.
- Assembly BOM: 77 fitted references.
- Shopping list: 29 purchasing groups totaling 77 fitted components.

Production remains on HOLD pending physical bench tests.
