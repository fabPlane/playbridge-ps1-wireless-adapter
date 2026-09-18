# Manufacturing notes

- Product PCB solder mask: red.
- PS1 bare-male-pin fit-test coupon solder mask: red.
- J1 uses provisional 1.00 mm drills and 1.70 mm pads for nine straight male pins on continuous 4.00 mm pitch.
- The coupon starting profile is 0.64 × 0.64 mm square and gold plated, projecting 16.00 mm beyond the PCB-edge datum. The inner 8.00 mm is for printed-nose capture/alignment/support; only the outer 8.00 mm is the console insertion section.
- A nine-pin printed guide has passed an initial simultaneous insertion fit in a spare original PS1 port. Exact pin MPN, tail/retention geometry, manufacturer-recommended finished drill/tolerance, gold thickness and assembly method remain TBD.
- J1 pin 5 is `PS_3V5_CONSOLE`, bench-measured at 3.58 V relative to pin 4/GND with the console powered and ESP32 disconnected. Leave it isolated from all PCB power rails to prevent backfeed. Pin 8 is the only NC.
- Never substitute a 2.54 mm header or alter the measured 4.00 mm pitch.
- No populated PlayBridge PCB is approved until a connector-only coupon passes unpowered fit, retention, continuity, isolation and repeated-cycle testing. The printed keyed shell must carry mating loads.
