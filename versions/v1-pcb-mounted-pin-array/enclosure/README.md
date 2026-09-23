# PlayBridge direct bare-male-pin architecture

## Corrected signal path

**Eight bare PCB-mounted male contacts in a nine-position pattern → PS1 console female controller port → PCB traces → protection/interface → ESP32.**

The two user photographs under `references/user-photos/` show nine recessed console socket openings behind three keyed plastic housing divisions. Direct physical measurement establishes that the nine electrical contact centers remain on one continuous uniform pitch; the plastic divisions do not introduce wider contact gaps.

The product has no donor plug, cable, internal wires, or female socket contacts. Eight provisional straight male contacts project from the console-facing PCB edge; position 8 is intentionally empty, matching the reference controller plug. A future printed keyed shell must align the contacts, reproduce the plastic 3+3+3 divisions, set the insertion datum, prevent rotation, and transfer insertion/removal/rocking loads into the enclosure rather than the contacts or solder joints.

## Fixed architecture

- CONSOLE SIDE: eight bare male contacts projecting outward in positions 1–7 and 9; position 8 is empty. The three visible groups are mechanical shell divisions only.
- USER SIDE: USB-C on the exact opposite edge, opening outward.
- ESP32-WROOM-32 retained with full antenna keepout.
- Maximum assembled envelope: 42 × 42 mm; PCB must fit the measured internal cavity.
- Manufacturing solder mask: **red**.

Position 8 is the only true console NC and has no metal contact on the reference controller plug. Position 5 is physically present and is `PS_3V5_CONSOLE`: it measured 3.58 V relative to pin 4/GND with the console powered and the ESP32 disconnected. The self-powered MVP intentionally leaves pin 5 electrically isolated to prevent backfeed; it must never be tied directly to board 3V3 or USB-derived power.

## Gate status

| Item | Status |
| --- | --- |
| Electrical gender | Eight bare male contacts in a nine-position pattern, mating with the console's female port |
| Nine positions and plastic 3+3+3 divisions | Proven by user photo |
| Electrical center pitch/span | Measured and spacing-fit-checked with nine temporary test pins: continuous 4.00 mm; 32.00 mm first-to-last. Production omits position 8. |
| Pin projection | 16.00 mm beyond PCB edge: inner 8.00 mm capture/support plus outer 8.00 mm console insertion; coupon confirmation required |
| Console contact/opening size | Approximately 1.00 mm; exact mating profile/tolerance still required |
| Exact male pin MPN, production drill and assembly process | Not selected |
| Production footprint | Not created |
| Fit-test status | Nine-position guide fit passed; production uses eight contacts. MPN, hole tolerance, retention and assembled-process validation remain open. |
| Full PCB placement/routing | Frozen |

See `male-pin-feasibility.md` and `../fit-test-coupon/`.

## Provisional male-pin-array update

J1 is a provisional nine-position pattern populated with eight bare male contacts. Looking directly at the male mating face, positions run left-to-right as `1 2 3 | 4 5 6 | 7 8 9`, with position 8 empty. In PCB top view the order is mirrored. Contact-center positions are `0, 4, 8, 12, 16, 20, 24, 28, 32 mm`: continuous uniform 4.00 mm pitch and a 32.00 mm first-to-last span. The separators shown by `|` describe only the future keyed shell divisions.

Each provisional installed contact is 0.64 × 0.64 mm square and gold plated, projecting 16.00 mm beyond the PCB-edge datum. The inner 8.00 mm is reserved for keyed-shell capture, alignment and lateral support; only the outermost 8.00 mm enters the console. The current PCB footprint/model still needs position 8 removed before release. The 1.00 mm drills and 1.70 mm pads are **MECHANICAL TBD / NOT PRODUCTION RELEASED**. A coupon must validate the selected pin MPN, mating fit, both datums, retention, production drill and assembly method before any populated-board order.
