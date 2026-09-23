# PlayBridge PS1 Wireless Adapter

PlayBridge is an ESP32-based wireless bridge that receives PC controller state
over Wi-Fi and presents it to an original PlayStation through the native PS1
controller protocol.

## Hardware versions

| Version | Design | Status |
| --- | --- | --- |
| `v1.0.0` | Direct PCB-mounted PS1 pin array | Frozen prototype checkpoint under [`versions/v1-pcb-mounted-pin-array/`](versions/v1-pcb-mounted-pin-array/) |
| `v2.0.0` | Modular donor-plug harness through board-mounted 9-contact connectors | Current release candidate under [`versions/v2-donor-plug-modular-adapter/`](versions/v2-donor-plug-modular-adapter/) |

The repository root contains the current V2 production design. V1 remains
self-contained in its version directory.

## V2.0.0 status

- Board: 56.295 × 62.000 mm, four copper layers, 1.20 mm thick.
- J1: 9-contact non-USB connector for a genuine PS1 donor-plug harness.
- J2: mechanically identical reserved connector; every contact is intentionally NC.
- J3: USB-C programming/power receptacle on the right edge, facing outward.
- J1 and J2: lower-edge mating mouths face outward and remain registered to the approved Molex footprints.
- Routing: complete; zero unconnected items.
- DRC: zero errors and zero warnings.
- ERC: zero errors and zero warnings.
- Schematic/PCB parity: zero findings.
- FabDesk release gate: PASS.
- PCBA data: 63 fitted references, 32 grouped BOM lines, and four DNP test pads.
- Controller-port bench firmware passed visible original-PS1 system-menu testing over Wi-Fi: all supported digital buttons worked, quick clicks were reliable, and the final check recorded 17,913 complete polls.

The design release is electrically and mechanically validated. Ordering still
requires a final live JLCPCB stock check and review of the uploaded BOM/CPL
rotation previews; see [`ordering/DO-NOT-ORDER.md`](ordering/DO-NOT-ORDER.md).

## Repository layout

- `circuit.netlist.json` — FabDesk design source and BOM metadata.
- `board.kicad_*` — current V2 KiCad project.
- `PlayBridge.pretty/` — project-local production footprints.
- `PlayBridge.3dshapes/` — connector and critical-component 3D models.
- `ordering/` — JLCPCB Gerber bundle, BOM, CPL, and sourcing audit.
- `reports/` — latest build, DRC, ERC, statistics, and release-gate reports.
- `versions/v1-pcb-mounted-pin-array/` — frozen V1 design.
- `versions/v2-donor-plug-modular-adapter/` — V2 architecture, release notes, references, and accepted renders.
- `fit-check/` and `fit-test-coupon/` — legacy V1 physical-contact test material.
- `docs/` — diagrams, concepts, and design-history images.
- `firmware/playbridge_wifi_test/` — experimental ESP32 Wi-Fi controller firmware and bench instructions

## Why the live V2 files are at the repository root

FabDesk opens and regenerates the active project from fixed root-level paths,
including `circuit.netlist.json`, `board.kicad_*`, the local footprint/model
libraries, `reports/`, and `ordering/`. Moving only those files into
`versions/v2-donor-plug-modular-adapter/` would break the current FabDesk
project and its library references.

The `versions/` directories are release records. V1 is a frozen, fully copied
snapshot; V2 currently keeps its release notes, references, concepts, and
accepted renders there while the authoritative FabDesk project remains at the
root. A fully self-contained V2 archive can be generated when the release is
tagged, without removing the root working project.

## Important connector warning

J1 and J2 use USB 3 Type-A connector hardware only as inexpensive keyed
9-contact interconnects. They carry native console signals and are **NOT USB**.
Never connect either port to a computer, charger, or ordinary USB peripheral.
