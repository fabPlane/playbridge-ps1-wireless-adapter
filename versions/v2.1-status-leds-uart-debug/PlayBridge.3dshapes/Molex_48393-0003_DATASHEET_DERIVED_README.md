# Molex 48393-0003 datasheet-derived 3D model

Status: **DATASHEET_DERIVED - NOT VENDOR CAD**. Replace with the exact Molex STEP before enclosure freeze when the official archive becomes downloadable.

## Source and provenance

- Manufacturer: Molex
- Production part: 48393-0003 / normalized 483930003
- JLCPCB/LCSC: C3197928
- Official customer drawing: SD-48393-001, document `483930003_sd.pdf`, revision B
- Drawing URL: https://www.molex.com/content/dam/molex/molex-dot-com/products/automated/en-us/salesdrawingpdf/483/48393/483930003_sd.pdf
- Model asset: `Molex_48393-0003_DATASHEET_DERIVED_SD-48393-001.wrl`
- Render-only audit asset: `Molex_48393-0003_DATASHEET_DERIVED_SECTION_PROOF.wrl`; it contains exact duplicates of the 13 PCB-entry solids but omits the upper shell/housing so the solids can be seen through the real footprint holes. It is never attached to the production board.
- The official current and legacy STEP ZIP endpoints were unavailable on 2026-09-18. This model is not a conversion of the rejected TraceParts preview WRL and contains none of that mesh.

## Coordinate and KiCad conventions

- VRML coordinates are authored numerically in millimetres in the footprint coordinate frame.
- KiCad interprets one WRL unit as 2.54 mm; attach with scale `(0.3937007874, 0.3937007874, 0.3937007874)`.
- Attach with offset `(0,0,0)` and rotation `(0,0,0)`.
- PCB component-side seating datum is `Z=0`.
- Nominal electrical-tail and retention-post extent is `Z=-1.10..0.00 mm`.
- With the specified 1.20-mm PCB, nominal tail/post tips finish 0.10 mm above the PCB bottom plane.
- Open connector front/mating edge is at local `Y=11.75 mm`.

## Mechanically authoritative separate features

The model contains separate solids whose centerlines equal the footprint hole/slot centers exactly:

| Feature | X (mm) | Y (mm) | Section/envelope |
|---|---:|---:|---|
| Pin 1 | 0.00 | 0.00 | 0.50 x 0.26 mm |
| Pin 2 | 2.50 | 0.00 | 0.50 x 0.26 mm |
| Pin 3 | 4.50 | 0.00 | 0.50 x 0.26 mm |
| Pin 4 | 7.00 | 0.00 | 0.50 x 0.26 mm |
| Pin 5 | 7.50 | -1.50 | 0.50 x 0.26 mm |
| Pin 6 | 5.50 | -1.50 | 0.50 x 0.26 mm |
| Pin 7 | 3.50 | -1.50 | 0.50 x 0.26 mm |
| Pin 8 | 1.50 | -1.50 | 0.50 x 0.26 mm |
| Pin 9 | -0.50 | -1.50 | 0.50 x 0.26 mm |
| Rear-left shell post | -2.90 | -0.75 | 0.60 x 1.70 mm |
| Front-left shell post | -2.90 | 7.75 | 0.60 x 1.40 mm |
| Rear-right shell post | 9.90 | -0.75 | 0.60 x 1.70 mm |
| Front-right shell post | 9.90 | 7.75 | 0.60 x 1.40 mm |

External drawing envelope: 13.80 mm wide (`X=-3.40..10.40`), 14.25 mm deep (`Y=-2.50..11.75`), and 6.86 mm maximum above the seating datum. The metal shell is open at the mating face. Blue thermoplastic is represented separately.

## Limitations

- This is a placement and enclosure-envelope visualization model, not vendor CAD.
- The 13 PCB-entry solids and external shell envelope are mechanically controlled by the drawing/footprint coordinates.
- Internal tongue, blue frame, rear dielectric distribution, and visible mating-contact strips are illustrative because the customer drawing does not fully dimension every hidden/internal contour.
- Sheet-metal bends, embossments, latch cut-outs, radii, plating thickness, draft, and manufacturing clearances are omitted.
- Do not use this asset for interference decisions below the drawing tolerance or for tooling.
