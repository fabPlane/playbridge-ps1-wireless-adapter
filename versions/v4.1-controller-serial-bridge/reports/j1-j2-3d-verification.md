# J1/J2 exact-model verification

Date: 2026-10-01

## Model source

- Part: Hong Cheng HC-USB3.0-L168-ZP, LCSC C7501856
- Project model: `PlayBridge.3dshapes/HC-USB3.0-L168-ZP_C7501856.wrl`
- STEP SHA-256: `f520a450f9d80237587fb2a4dddd8b4d12c20504a3b4a4d833d242cad28aa385`
- WRL SHA-256: `cd701c8fe0843f3be053458451192e131f831f955c700702210e23dc49a00f45`
- Both hashes exactly match the supplied v3.0 reference assets.

## Authoritative transform and geometry

Both supplied reference footprints assign the WRL HC model with:

- offset: `(0, 0, 0) mm`
- scale: `(1, 1, 1)`
- rotation: `(0, 0, 0) degrees`

The current J1 and J2 footprints retain the same anchor-relative signal-hole and
retention-slot centers as the supplied references:

| Feature | Local X (mm) | Local Y (mm) |
| --- | ---: | ---: |
| Pin 1 | -3.5 | -0.3 |
| Pin 2 | -1.0 | -0.3 |
| Pin 3 | 1.0 | -0.3 |
| Pin 4 | 3.5 | -0.3 |
| Pin 5 | 4.0 | -1.8 |
| Pin 6 | 2.0 | -1.8 |
| Pin 7 | 0.0 | -1.8 |
| Pin 8 | -2.0 | -1.8 |
| Pin 9 | -4.0 | -1.8 |
| Left retention slot | -6.4 | 1.8 |
| Right retention slot | 6.4 | 1.8 |

J1 anchor: `(14.0, 48.5) mm`; retention-slot centers:
`(7.6, 50.3)` and `(20.4, 50.3) mm`.

J2 anchor: `(39.2, 48.5) mm`; retention-slot centers:
`(32.8, 50.3)` and `(45.6, 50.3) mm`.

No connector footprint anchor, pad, slot, connected route, net, zone, or
board-outline coordinate was changed. The board-specific connector body
overhang is 2.0 mm at the lower edge; every signal pin and retention slot
remains supported by PCB material.

## Through-board Z verification

The board stackup thickness is `1.600 mm`. Direct measurement of the bound WRL
geometry, after applying its native VRML scale, gives a minimum model Z of
`-2.800096 mm` relative to the footprint datum. With the model bound at the
authoritative zero Z offset, the nine signal tails and both shell-retention
legs therefore extend `1.200096 mm` beyond the PCB underside. This is the
solderable protrusion visible in the close side and oblique-underside renders.

No visual-only X, Y, or Z correction was applied. The PCB footprints and the
model bindings remain at their validated coordinates and zero transform.

The STEP file was also rendered at the same zero transform. It does not share
the WRL model's effective KiCad frame and visibly mis-registers the body/pin
field. It is retained as source geometry but is not bound to J1/J2. No
compensating visual offset was introduced.

## Visual verification

- `renders/j1-orthographic-top.png`
- `renders/j1-orthographic-bottom.png`
- `renders/j1-orthographic-side.png`
- `renders/j1-mechanical-underside.png`
- `renders/j1-mechanical-side.png`
- `renders/j2-orthographic-top.png`
- `renders/j2-orthographic-bottom.png`
- `renders/j2-orthographic-side.png`
- `renders/j2-mechanical-underside.png`
- `renders/j2-mechanical-side.png`

The independent orthographic bottom views expose all nine plated-hole centers
and both retention slots. The new close oblique-underside views show all nine
metal signal tails emerging from those holes, and the close edge-on views show
the tails and both retention legs extending below the 1.6 mm PCB. The modeled
pins/tabs are concentric with their corresponding PCB holes for J1 and J2.

## Electrical verification

- ERC: 0 errors, 0 warnings.
- Final parity-enabled DRC: 0 violations, 0 unconnected items, and 0 schematic
  parity issues (`reports/drc-final.rpt`).
