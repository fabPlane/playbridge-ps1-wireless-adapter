# PlayBridge hardware v2.0.0 release candidate

## Scope

V2 replaces the provisional PCB-mounted PS1 contact array with a modular
donor-plug harness. J1 carries the native PS1 controller interface through a
keyed 9-contact connector. J2 reserves the same mechanical interface for a
future rear-serial harness and is intentionally all-NC. USB-C J3 remains the
programming and user power connection.

J1 and J2 use USB 3 Type-A connector hardware but do not carry USB. Both ports
must be marked **NOT USB** in the enclosure and harness.

## Validated board

- Dimensions: 56.295 × 62.000 mm.
- Stackup: four copper layers, 1.20 mm finished thickness.
- Population: 67 footprints; 63 fitted references and four DNP test pads.
- J1/J2: Molex 48393-0003 / JLCPCB C3197928, outward at the lower edge.
- J3: HRO TYPE-C-31-M-12 / JLCPCB C165948, outward at the right edge.
- Board SHA-256: `4e4068be3e2941f74bfe923bd5959de7a7d2209ca3e7ce8036483f3a8c5df5ab`.

## Release gate

- KiCad DRC: 0 errors, 0 warnings, 0 unconnected items.
- KiCad ERC: 0 errors, 0 warnings.
- Schematic-to-PCB parity: 0 findings.
- FabDesk release gate: PASS with no warning baselines.
- Connectivity: 135/135 connections routed.

## 3D inspection coverage

- Exact/authoritative component models: J1, J2, J3, U1, SW1, SW2, and D2.
- L1: dimensionally controlled datasheet-derived 4 × 4 × 3 mm model because
  exact vendor CAD was unavailable.
- Remaining fitted packages use their KiCad/project models or dimensionally
  representative package envelopes in the populated audit renders.
- Missing footprint representations: 0.

Accepted technical renders are in [`renders/`](renders/). They are render-only
artifacts and do not modify PCB geometry, placement, routing, footprints, or nets.

## Manufacturing package

The repository includes:

- `ordering/gerbers-jlcpcb.zip`
- `ordering/jlcpcb-bom.csv`
- `ordering/jlcpcb-cpl.csv`
- `ordering/PCBA-SOURCING-AUDIT.md`

## Checkout hold

Before placing the PCBA order, recheck live stock for J1/J2 C3197928 and L1
C49449313, then inspect the JLCPCB BOM/CPL placement preview—especially both
USB-A connectors, USB-C, switches, D2, and L1. This is a commercial/assembly
review hold; no electrical-layout or connector-orientation defect is open.
