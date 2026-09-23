# V2.0.0 — donor-plug modular adapter

V2 replaces the PCB-mounted PlayStation pin array with a removable donor-plug harness.

The main PlayBridge board will expose removable multi-contact connectors. A console-specific adapter will use a genuine donor controller cable and molded PlayStation male plug soldered to a matching breakout board. Swapping the adapter cable will make it possible to support other console systems without manufacturing their proprietary plug contacts directly on the main PCB.

## Architecture

- Keep the ESP32, signal protection and protocol-generation circuitry on the main board.
- Replace the console-facing PCB pin array with a removable cable interface.
- Use the genuine molded PlayStation donor plug for console alignment, contact geometry and insertion loads.
- Map every donor wire by continuity testing; never rely on wire color.
- Treat the observed missing pin 8 as a donor-plug property that must be verified electrically.
- Define connector pin allocation, power/backfeed protection, shield policy and cable requirements before designing the V2 PCB.
- Add other console adapters only after the PS1 harness is validated.

## Safety warning

The concept currently uses USB 3 Type-A connector hardware because it provides enough inexpensive contacts and readily available breakout boards. The signals would be native console signals—not USB. Any implementation must be clearly marked **NOT USB**, protected against accidental connection to computers/chargers, and reviewed before hardware is powered.

## References

- [`concepts/`](concepts/) contains the supplied system architecture, breakout-board example and illustrative product layout.
- [`references/ps1-donor-plug/`](references/ps1-donor-plug/) contains photographs of the actual donor plug, including the empty pin-8 position.

## V2.0.0 release checkpoint

This directory is the self-contained V2.0.0 release snapshot. It includes the
KiCad project, FabDesk netlist/configuration, local symbol/footprint/3D-model
libraries, verification reports, manufacturing package, release tools,
references, and accepted renders.

FabDesk still uses the matching root-level files as its live working copy.
Keeping that live copy avoids breaking FabDesk's fixed project path, while this
directory keeps V2 independent from subsequent hardware versions.

- Board: 56.295 × 62.000 mm, four layers, 1.20 mm thick.
- J1: Molex 48393-0003 / JLCPCB C3197928 used as a nine-contact **non-USB** PS1 donor-harness interface.
- J2: the same connector reserved for a future rear-serial harness; its contacts remain intentionally unassigned until that interface is characterized.
- J1/J2 use `Molex_48393-0003_TraceParts_exact_visual.wrl`, converted from the exact TraceParts catalog preview for 48393-0003. Registration is documented in `PlayBridge.3dshapes/README_MOLEX_48393-0003_TRACEPARTS.md`.
- J1 is at (4.525, 50.525) mm, J2 at (22.525, 50.525) mm, both at 0°, with their mating mouths facing outward through the lower edge. F1 is at (12.500, 38.000) mm. J3 remains at (51.025, 9.625) mm, 90°, facing outward through the right edge.
- ERC, DRC, schematic parity, and connectivity currently pass with zero errors, warnings, or unconnected items. Ordering remains subject to the sourcing and PCBA-completeness checks in `ordering/`.

See [`RELEASE-v2.0.0.md`](RELEASE-v2.0.0.md) for the release gate,
manufacturing-package status, 3D-model provenance, and remaining checkout hold.

## Snapshot layout

- `board.kicad_*`, `circuit.netlist.json`, and `fabdesk.json` — complete V2 design.
- `PlayBridge.pretty/`, `PlayBridge.3dshapes/`, and the symbol libraries — project-local libraries required to reopen the design.
- `ordering/` — Gerber archive, BOM, CPL, sourcing audit, and checkout hold.
- `reports/` — release build, DRC, ERC, statistics, and gate reports.
- `tools/` — scripts used for model inspection and order-file generation.
- `renders/` — accepted V2.0.0 renders only.

## USB-A mechanical fit

J1 and J2 use the official Molex 48393-0003 through-hole pattern: nine 0.70 mm
signal holes plus four polarized shell-retention slots. The 13.80 × 14.25 mm
connector body fits the footprint and requires no PCB cutout. With the current
placement, its mating lip projects 0.275 mm beyond the straight board edge so
the cable remains accessible, while every signal pin and retention leg remains
inside the board outline. The V2 PCB thickness is 1.20 mm, matching the PCB
thickness stated on Molex drawing `483930003_sd.pdf`.

## 3D-model validation

The datasheet-derived model contains separate solids for all nine signal tails
and four shell-retention posts. Their centerlines match the corresponding
footprint holes/slots with a reported XY delta of 0.000 mm. The model uses zero
offset and zero rotation in the footprint frame; its WRL scale is
`0.3937007874` on all axes. The nominal tails/posts extend 1.10 mm below the
top seating datum on the 1.20 mm PCB.

The production footprints retain the official Molex entry geometry. The
TraceParts visual model is used for populated-board inspection, while the
datasheet-derived section proof remains available for dimensional audit.
See `PlayBridge.3dshapes/README_MOLEX_48393-0003_TRACEPARTS.md` and
`PlayBridge.3dshapes/Molex_48393-0003_DATASHEET_DERIVED_README.md`.
