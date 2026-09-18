# PS1 contact fit-test coupon — NOT FOR ORDER

This coupon precedes the PlayBridge PCB. It contains only:

- nine provisional straight 0.64 × 0.64 mm square test pins for spacing validation; the production connector uses only eight contacts and leaves position 8 empty;
- labels 1 through 9 in PS1 mating-view order;
- nine contact centers on continuous uniform 4.00 mm pitch, with plastic-division reference marks kept mechanically separate;
- PCB-edge datum;
- contact-field centerline;
- approximately 8.0 mm mating projection beyond the console-facing datum;
- shell/nose mechanical reference marks;
- red solder mask.

No active circuitry, donor plug, cable, internal wires, or female socket array is included. The coupon uses the measured 4.00 mm pitch and must not substitute an ordinary 2.54 mm header.

The measured position centers are fixed at `0,4,8,12,16,20,24,28,32 mm`. A printed guide carrying all nine 0.64 mm square test pins was inserted together into a spare original PS1 controller port, so the pitch/span and insertion concept have passed an initial physical fit check. This test does not define the production population: the controller plug has no metal contact at position 8. The outer 8.0 mm is the console mating section; the inner 8.0 mm is reserved for nose capture and support. Exact pin MPN, tail geometry, production drill/tolerance, retention, plating thickness and assembly method remain TBD. Pin 5 is `PS_3V5_CONSOLE` and measured 3.58 V to pin 4/GND; it remains isolated on the PlayBridge MVP. Pin 8 is NC and physically omitted.

Acceptance:

- all eight production contacts enter their matching console contacts without binding, socket damage or permanent deflection, while the empty position 8 remains correctly aligned;
- keyed shell prevents wrong insertion and rotation;
- engagement reaches the measured datum without bottoming;
- pin retention survives repeated insertion;
- the production shell—not pin tails or solder joints—carries insertion force;
- continuity passes for installed positions 1–7 and 9; position 8 remains absent and isolated.
