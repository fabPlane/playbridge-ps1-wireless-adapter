# PlayBridge PS1 Wireless Adapter

PlayBridge is an ESP32-based adapter intended to plug directly into an original PlayStation controller port. A PC sends controller state over Wi-Fi, and the adapter presents that state to the console through the PS1 controller protocol.

## Current status

The PCB design is electrically complete, but physical PS1-contact validation is still required before fabrication release.

- Board: 55 x 52 mm, four copper layers
- ESP32-WROOM-32 module
- USB-C programming and development connection
- Nine-position provisional PS1 contact footprint
- Seven active PS1 contacts; pins 5 and 8 intentionally isolated
- Routing: complete, with zero unconnected items
- ERC: zero errors and zero warnings
- DRC: zero errors and zero warnings
- USB-C breakout and full-board routing are complete
- The PS1 metal contacts and printed alignment shell still require physical fit validation

## Repository layout

- `circuit.netlist.json` — FabDesk source design
- `board.kicad_*` — generated/current KiCad project files
- `PlayBridge.pretty/` — project-local footprints
- `PlayBridge.3dshapes/` — project-local provisional 3D models
- `fit-test-coupon/` — PS1 contact measurement and fit-test material
- `enclosure/` — provisional mechanical studies
- `manufacturing-notes.md` — current manufacturing constraints and open questions
- `docs/images/current/` — latest reviewed images only
- `docs/images/archive/` — superseded and diagnostic images kept for design history

## Important mechanical note

The nine PS1 contacts are currently provisional: 4.00 mm pitch, 32.00 mm first-to-last center span, nominal 0.64 mm square contact and 1.00 mm plated holes. Each pin projects 16.00 mm beyond the PCB edge: the inner 8.00 mm is reserved for printed-nose capture/alignment/support and only the outer 8.00 mm is the console mating section. The final contact manufacturer part number, retention method, plating and mating fit have not been released for production.

Do not order assembled boards until contact fit testing and the fabrication release checklist are complete.
