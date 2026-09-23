# V1.0.0 — PCB-mounted PS1 pin array

PlayBridge is an ESP32-based adapter intended to plug directly into an original PlayStation controller port. A PC sends controller state over Wi-Fi, and the adapter presents that state to the console through the PS1 controller protocol.

Open this directory directly in FabDesk. This is the frozen V1.0.0 project;
start later hardware work from a separate version directory.

## Current status

The PCB design is electrically complete, but physical PS1-contact validation is still required before fabrication release.

- Board: 55 x 52 mm, four copper layers
- ESP32-WROOM-32 module
- USB-C programming and development connection
- Nine-position provisional PS1 contact footprint with eight metal contacts
- Seven active PS1 contacts; pin 5 is physically present but isolated, and pin 8 is physically absent
- Routing: complete, with zero unconnected items
- ERC: zero errors and zero warnings
- DRC: zero errors and zero unconnected items; 25 physical warnings remain documented
- Schematic/PCB parity: 66 archived `extra_footprint` warnings because this
  checkpoint predates synchronized schematic-to-board identity metadata
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
- `docs/images/current/` — latest reviewed images only
- `docs/images/archive/` — superseded and diagnostic images kept for design history

## Important mechanical note

The nine PS1 contacts use 4.00 mm pitch and a 32.00 mm first-to-last center span. A printed guide carrying all nine nominal 0.64 mm square pins was successfully inserted into a spare original PS1 controller port. Each pin projects 16.00 mm beyond the PCB edge: the inner 8.00 mm is reserved for printed-nose capture/alignment/support and only the outer 8.00 mm is the console mating section. The final contact manufacturer part number, production hole/tolerance, retention method and plating have not been released.

J1 pin 5 is `PS_3V5_CONSOLE`, measured at 3.58 V relative to pin 4/GND with the console powered and ESP32 disconnected. It remains intentionally isolated from board 3V3 and every USB/console power path to avoid backfeed. Pin 8 is the only true console NC.

This checkpoint is suitable for team review and prototype-output preparation, but it is not yet an approved assembly release. Resolve or formally accept the remaining DRC and parity warnings, finish J1 contact sourcing/assembly validation, and verify the production-board USB-C 3D transform before ordering assembled boards.
