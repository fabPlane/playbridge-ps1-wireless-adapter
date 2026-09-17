# PlayBridge direct bare-male-pin architecture

## Corrected signal path

**Nine bare PCB-mounted male pins → PS1 console female controller port → PCB traces → protection/interface → ESP32.**

The two user photographs under `references/user-photos/` show nine recessed console socket openings behind three keyed plastic housing divisions. Direct physical measurement establishes that the nine electrical contact centers remain on one continuous uniform pitch; the plastic divisions do not introduce wider contact gaps.

The product has no donor plug, cable, internal wires, or female socket contacts. Nine provisional straight male pins project from the console-facing PCB edge. A future printed keyed shell must align the pins, reproduce the plastic 3+3+3 divisions, set the insertion datum, prevent rotation, and transfer insertion/removal/rocking loads into the enclosure rather than the pins or solder joints.

## Fixed architecture

- CONSOLE SIDE: nine bare male conductive pins projecting outward, positions 1–9 in verified mating-view order; the three visible groups are mechanical shell divisions only.
- USER SIDE: USB-C on the exact opposite edge, opening outward.
- ESP32-WROOM-32 retained with full antenna keepout.
- Maximum assembled envelope: 42 × 42 mm; PCB must fit the measured internal cavity.
- Manufacturing solder mask: **red**.

Position 8 is physically present and electrically NC. Position 5 remains electrically TBD until the PS1 pinout and power path are verified from authoritative documentation and bench measurement.

## Gate status

| Item | Status |
| --- | --- |
| Electrical gender | Nine bare male pins mating with the console's female port contacts |
| Nine positions and plastic 3+3+3 divisions | Proven by user photo |
| Electrical center pitch/span | Measured: continuous 4.00 mm; 32.00 mm first-to-last |
| Pin projection | 16.00 mm beyond PCB edge: inner 8.00 mm capture/support plus outer 8.00 mm console insertion; coupon confirmation required |
| Console contact/opening size | Approximately 1.00 mm; exact mating profile/tolerance still required |
| Exact male pin MPN, production drill and assembly process | Not selected |
| Production footprint | Not created |
| Fit-test coupon | Parameterized specification only; NOT FOR ORDER |
| Full PCB placement/routing | Frozen |

See `male-pin-feasibility.md` and `../fit-test-coupon/`.

## Provisional male-pin-array update

J1 is a provisional nine-position bare male-pin array. Looking at the adapter's console-facing mating side, electrical positions run left-to-right as `9 8 7 | 6 5 4 | 3 2 1`. Contact-center positions are `0, 4, 8, 12, 16, 20, 24, 28, 32 mm`: continuous uniform 4.00 mm pitch and a 32.00 mm first-to-last span. The separators shown by `|` describe only the future keyed shell divisions.

The provisional contact is 0.64 × 0.64 mm square and gold plated, projecting 16.00 mm beyond the PCB-edge datum. The inner 8.00 mm is reserved for keyed-shell capture, alignment and lateral support; only the outermost 8.00 mm enters the console. The current 1.00 mm drills and 1.70 mm pads are **MECHANICAL TBD / NOT PRODUCTION RELEASED**. A coupon must validate the selected pin MPN, mating fit, both datums, retention, production drill and assembly method before any populated-board order.
