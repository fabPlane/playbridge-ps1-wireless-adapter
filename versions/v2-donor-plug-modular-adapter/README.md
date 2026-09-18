# V2 — donor-plug modular adapter

V2 is the planned replacement for the PCB-mounted PlayStation pin array.

The main PlayBridge board will expose removable multi-contact connectors. A console-specific adapter will use a genuine donor controller cable and molded PlayStation male plug soldered to a matching breakout board. Swapping the adapter cable will make it possible to support other console systems without manufacturing their proprietary plug contacts directly on the main PCB.

## Initial plan

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

## Current development checkpoint

V2 PCB development is active on branch `v2/donor-plug-modular-adapter`. The working source remains at the repository root until V2 is frozen as a release snapshot.

- Board: 55 × 62 mm, four layers.
- J1: Molex 48393-0003 / JLCPCB C3197928 used as a nine-contact **non-USB** PS1 donor-harness interface.
- J2: the same connector reserved for a future rear-serial harness; its contacts remain intentionally unassigned until that interface is characterized.
- J1/J2 now use `Molex_48393-0003_DATASHEET_DERIVED_SD-48393-001.wrl`, built from the official Molex drawing and documented beside the model. It is explicitly a datasheet-derived placement/enclosure model, not vendor CAD; replace it with the exact Molex STEP before enclosure freeze.
- `renders/` contains the current full-board, edge, side, perspective, and pin-alignment proof views.
- ERC currently reports zero errors and zero warnings. Routing and DRC cleanup are not complete, so V2 is not order-ready.

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

The model is mechanically controlled only for the 13 PCB-entry features and
the drawing's external envelope. Internal tongue/frame details are illustrative.
See `PlayBridge.3dshapes/Molex_48393-0003_DATASHEET_DERIVED_README.md` for
provenance, dimensions, transforms, and limitations.
