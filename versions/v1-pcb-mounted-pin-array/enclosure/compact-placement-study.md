# PlayBridge compact placement study

## Reference mesh measurements

Measurements were extracted directly from every triangle vertex in the binary STL meshes under `enclosure/reference/`. Coordinate extrema establish the external envelope; repeated planar vertex coordinates establish the opposing inner wall faces.

| Mesh | External bounds (mm) | Size (mm) | Dominant inner wall faces (mm) | Wall-to-wall cavity (mm) |
| --- | --- | --- | --- | --- |
| BlueRetro2_PS1_bottom_Ali.stl | X -26.000…16.000; Y -64.000…-22.000; Z 0…6.497 | 42.000 × 42.000 × 6.497 | X -24.504…14.506; Y -60.497…-24.603 | 39.010 × 35.894 |
| BlueRetro2_PS1_top_Ali.stl | X -26.000…16.000; Y -60.000…-18.000; Z 0…6.098 | 42.000 × 42.000 × 6.098 | X -24.505…14.505; Y -56.497…-20.606 | 39.010 × 35.891 |

The nominal common wall-to-wall cavity is therefore about **39.01 × 35.89 mm**, not 42 × 42 mm. A rectangular PCB must be smaller still. With a provisional 0.25 mm assembly gap per side, the pre-intrusion ceiling is **38.51 × 35.39 mm**. This is not yet an approved PCB outline: corner posts, shell-registration features, the PS1 plug shoulder capture, button actuators, and the USB-C opening intrude into this rectangle and require either measured corner cutouts or local PCB keepouts. Their irregular mesh surfaces were not reduced to invented rectangular dimensions.

## MVP size-reduction options

| Block/item | MVP status | Current implementation | Compact action | Count reduction | Estimated PCB-area benefit |
| --- | --- | --- | --- | ---: | ---: |
| ESP32 Wi-Fi | Mandatory | ESP32-WROOM-32 | Retain for firmware compatibility; place antenna at cavity edge | 0 | 0 mm²; fixes antenna zone |
| PS1 interface | Mandatory | Provisional nine-position bare male-pin array plus CMD/CLK/ATT conditioning and DATA/ACK MOSFETs | Reserve pin/shell keepout; finalize drill and retention only after coupon-tested pin selection, then move small conditioning parts to B.Cu | 0 | 35–55 mm² top-side recovery |
| PS1 power | Mandatory | AP63203, inductor, fuse, TVS, input/output capacitors | Retain topology; verify smaller TVS/fuse/cap packages and compact shielded inductor | 0 | 20–35 mm² |
| USB-C | Mandatory | Amphenol receptacle, CC resistors, USBLC6 | Retain; direct fine-pitch escape and B.Cu passives | 0 | 10–20 mm² top-side recovery |
| USB-UART | Mandatory for current firmware workflow | CH340C SOIC-16 | Use verified CH340C QFN-16 variant or compact equivalent after pin/driver audit | 0 | 35–45 mm² |
| Source isolation | Mandatory while USB bench power remains | TPS2113A TSSOP-8 plus AP2112K | Retain unless a separately reviewed compact protected power path replaces it | 0 | 0–10 mm² |
| BOOT/RESET | Mandatory access | Two CK side switches | Replace with verified miniature side-actuated switches; keep both functions | 0 | 20–30 mm² |
| UART/rail test points | Non-MVP | TP1–TP4, 1.5 mm pads | Remove TP1/TP2; replace TX/RX with small labeled pogo pads | 2 footprints | 18–25 mm² |
| Status LED | Optional | LED plus resistor | Remove for minimum build, or retain only if enclosure indication is required | 2 footprints | 4–7 mm² |
| Auto-programming | Useful but reducible | Two transistors and two base resistors | Retain for normal flashing; aggressive option uses compact dual transistor/array after verification | 1–2 footprints | 8–15 mm² |
| Large passives | Mandatory electrically | 1206/0805 supply capacitors and fuse | Use verified voltage/current-rated 0805/0603 alternatives only where derating permits | 0 | 10–20 mm² |

Conservative reduction: remove two rail test points, convert UART points to pogo pads, use compact side switches, compact CH340C package, and move low-profile passives to B.Cu. Estimated reduction is **4–6 footprints and 100–160 mm² of top-side demand**.

Aggressive alternative: ESP32-S3 with native USB can remove CH340C and much of the auto-program block, but it changes the MCU, USB firmware, boot behavior, GPIO allocation, and PS1 timing validation. It is a separate product/firmware-port option and is not applied here.

## Placement decision

No replacement PCB placement was created. The existing 60-footprint design already failed at 42 × 42 mm with 40 courtyard overlaps; the measured cavity ceiling is only 38.51 × 35.39 mm before posts and connector intrusions. A legal placement requires the component/package reductions above and a PCB outline derived around the actual corner/post and plug-capture geometry. Compressing the existing parts again would not be an engineering result.
