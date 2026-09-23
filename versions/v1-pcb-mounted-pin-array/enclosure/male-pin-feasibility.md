# PS1 bare male PCB-pin feasibility — NOT FOR ORDER

## Confirmed concept

PlayBridge uses eight bare conductive male contacts in a nine-position pattern, mounted directly to the PCB and projecting horizontally toward the PS1 console's female controller port. Position 8 is intentionally empty. There is no donor plug, cable, internal wiring, or female socket array.

Measured geometry:

- centers: `0, 4, 8, 12, 16, 20, 24, 28, 32 mm`;
- uniform 4.00 mm pitch;
- 32.00 mm first-to-last span;
- approximately 1.00 mm console openings;
- provisional mating section: 0.64 × 0.64 mm square, gold plated;
- projection beyond the PCB-edge datum: 16.00 mm total; inner 8.00 mm capture/support section and outer 8.00 mm console insertion section.

The plastic 3+3+3 divisions are mechanical keying features only. They do not change electrical pitch. A future printed shell must reproduce those divisions, align the pins, establish insertion depth, prevent rotation, and carry insertion/removal loads.

## Data required before selecting a production pin

1. Purchasable manufacturer and exact MPN.
2. Mating-section tolerance around the provisional 0.64 × 0.64 mm profile.
3. Gold plating thickness and underplate on the mating section.
4. Pin material, current rating and mating-cycle suitability.
5. Total pin length, 16.00 mm PCB-edge projection, 8.00 mm capture datum, 8.00 mm insertion datum, PCB thickness allowance and rear retention length.
6. Tail/retention style: straight PTH stake, compliant section, swage feature or carrier strip.
7. Manufacturer-recommended finished-hole diameter and tolerance.
8. Pad diameter, annular ring and solder-mask opening.
9. Hand, selective-solder, press-fit or fixture-assisted assembly process.
10. Coplanarity/perpendicularity tolerance and assembly fixture requirements.
11. Console insertion/extraction force and printed-shell load path.

## Provisional footprint

`PROVISIONAL_PS1_MALE_PIN_ARRAY_P4.00_SQ0.64_MECH_TBD` currently uses provisional 1.00 mm finished drills and 1.70 mm pads. Before release, position 8 must be removed from the populated footprint/model. These dimensions are not released until an actual contact manufacturer specifies its tail and hole. Position 1 remains the orientation datum. Looking directly at the male mating face, positions are `1 2 3 | 4 5 6 | 7 8 9`; position 8 is empty. PCB top view is mirrored.

## Release gate

Build a connector-only coupon before ordering a populated PlayBridge board. The coupon must verify fit without console damage, contact continuity and isolation, the 16.00 mm projection with separate 8.00 mm capture and 8.00 mm insertion sections, pin retention, alignment, repeated cycles and shell load transfer. Do not substitute 2.54 mm pitch or change the measured 4.00 mm pitch.
