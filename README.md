# PlayBridge PS1 Wireless Adapter

PlayBridge is an ESP32-based adapter intended to plug directly into an original PlayStation controller port. A PC sends controller state over Wi-Fi, and the adapter presents that state to the console through the PS1 controller protocol.

## Hardware versions

PlayBridge hardware is versioned independently so the existing PCB-mounted pin-array prototype can remain reproducible while the modular donor-plug design is developed.

| Version | Design | Status |
| --- | --- | --- |
| `v1.0.0` | PCB-mounted pin array | Frozen prototype checkpoint under [`versions/v1-pcb-mounted-pin-array/`](versions/v1-pcb-mounted-pin-array/) |
| `v1.x` | Compatible revisions of the PCB-mounted pin-array design | Reserved for future V1 corrections that retain the same architecture |
| `v2.0.0` | Donor PlayStation plug with a removable custom USB-connector harness | Planned; development material is under [`versions/v2-donor-plug-modular-adapter/`](versions/v2-donor-plug-modular-adapter/) |

## Current status

The PCB design is electrically complete, but physical PS1-contact validation is still required before fabrication release.

- Board: 55 x 52 mm, four copper layers
- ESP32-WROOM-32 module
- USB-C programming and development connection
- Nine-position provisional PS1 connector footprint with eight metal contacts
- Seven active signal/power contacts; pin 5 is present but isolated, and pin 8 is physically omitted
- Routing: complete, with zero unconnected items
- ERC: zero errors and zero warnings
- DRC: zero errors and zero unconnected items; 25 physical warnings remain documented
- USB-C breakout and full-board routing are complete
- The nine-pin 4.00 mm spacing/insertion concept passed a physical guide fit; the final contact MPN, assembly tolerances, retention and printed alignment shell still require manufacturing validation

## Repository layout

- `circuit.netlist.json` — FabDesk source design
- `board.kicad_*` — generated/current KiCad project files
- `PlayBridge.pretty/` — project-local footprints
- `PlayBridge.3dshapes/` — project-local provisional 3D models
- `fit-test-coupon/` — PS1 contact measurement and fit-test material
- `fit-check/` — printable STL fixtures and physical controller-port test photos
- `enclosure/` — provisional mechanical studies
- `manufacturing-notes.md` — current manufacturing constraints and open questions
- `docs/diagrams/` — editable Mermaid sources and rendered modular-interconnect concepts
- `docs/images/concepts/` — illustrative product and assembly concept renders
- `docs/images/current/` — latest reviewed images only
- `docs/images/archive/` — superseded and diagnostic images kept for design history
- `firmware/playbridge_wifi_test/` — experimental ESP32 Wi-Fi controller firmware and bench instructions

## Help wanted: source the PS1 male contacts

We need an orderable manufacturer part number for the eight individual metal contacts at `J1`. The connector has nine indexed positions, but standard controllers omit the metal contact at position 8. These are not a standard 2.54 mm pin header and not a complete donor plug.

| Requirement | Current target |
| --- | --- |
| Quantity | 8 identical straight male contacts per board |
| Contact arrangement | Nine indexed positions in one row on continuous 4.00 mm centers; position 8 intentionally empty |
| First-to-last span | 32.00 mm across eight equal gaps |
| Mating cross-section | Nominal 0.64 × 0.64 mm square, as used in the successful fit check |
| Projection beyond PCB edge | 16.00 mm total |
| Console insertion section | Outermost 8.00 mm |
| Printed-nose support section | Inner 8.00 mm between PCB edge and insertion section |
| Current provisional PCB land | 1.00 mm finished plated hole with 1.70 mm pad; final hole must follow the selected contact datasheet and assembly tolerance |
| Contact finish | Gold-plated mating area preferred; supplier must state base material and plating thickness |
| Tip | Smooth/chamfered or rounded entry; no sharp burrs that could damage the console socket |
| Assembly | Must support repeatable PCB assembly and retention; loose hand-cut jumper wires are not the production solution |

When proposing a part, please provide the manufacturer, exact MPN, dimensioned drawing/datasheet, cross-section, total length and tail geometry, plating specification, recommended PCB hole/tolerance, retention method, packaging, supplier link, stock/lead time, and any JLCPCB/LCSC part number. JLCPCB in-stock assembly availability is preferred, but a readily obtainable external part is also useful for fit testing.

Looking directly at the male mating face, the order is `1 2 3 | 4 5 6 | 7 8 9`; the second position from the right is pin 8 and has no metal contact. In the current PCB top view this order is mirrored as `9 8 7 | 6 5 4 | 3 2 1`. Pin 5 is physically present but electrically isolated as `PS_3V5_CONSOLE`. Any candidate must pass the connector-only fit coupon test before it is approved for populated-board production.

## Important mechanical note

The nine-position PS1 pattern uses 4.00 mm pitch and a 32.00 mm first-to-last center span, but only eight metal contacts are installed because position 8 is NC and physically empty on the reference controller plug. A printed guide carrying nine nominal 0.64 mm square test pins was successfully inserted into a spare original PS1 controller port; that test validated spacing only and does not define the final population. Each installed contact projects 16.00 mm beyond the PCB edge: the inner 8.00 mm is reserved for printed-nose capture/alignment/support and only the outer 8.00 mm is the console mating section. The final contact manufacturer part number, production hole/tolerance, retention method and plating have not been released.

J1 pin 5 is `PS_3V5_CONSOLE`, measured at 3.58 V relative to pin 4/GND with the console powered and ESP32 disconnected. It remains intentionally isolated from board 3V3 and every USB/console power path to avoid backfeed. Pin 8 is the only true console NC and must not be populated.

This checkpoint is suitable for team review and prototype-output preparation, but it is not yet an approved assembly release. Resolve or formally accept the remaining DRC warnings, finish J1 contact sourcing/assembly validation, and verify the production-board USB-C 3D transform before ordering assembled boards.
