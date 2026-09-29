# Critical component 3D-model provenance

## U1 — ESP32-WROOM-32E-N4

- Model: `KICAD_OFFICIAL_ESP32-WROOM-32.step`
- Source: KiCad official `kicad-packages3D`, `RF_Module.3dshapes/ESP32-WROOM-32.step`.
- Classification: authoritative KiCad package model.
- Manufacturer dimensional cross-check: Espressif ESP32-WROOM-32E/32UE datasheet, 18.0 × 25.5 × 3.1 mm.

## SW1/SW2 — Korean Hroparts K2-1107ST-A4SW-06 / C2837195

- Model: `K2-1107ST-A4SW-06.step`
- Source: exact component asset record for LCSC C2837195.
- Classification: exact-part component-platform CAD; not claimed as manufacturer-native CAD.
- Cross-check: 6.0 × 3.6 × 2.5 mm, 8.0 mm terminal-center span.

## D2 — GOODWORK SMAJ12A / C908766

- Model: `KICAD_OFFICIAL_D_SMA_DO-214AC.step`
- Source: KiCad official `kicad-packages3D`, `Diode_SMD.3dshapes/D_SMA.step`.
- Classification: authoritative KiCad DO-214AC/SMA package model. The exact BOM device is GOODWORK SMAJ12A, not SMBJ12A.
- Polarity follows footprint pad 1/cathode and the package cathode band.

## L1 — MYX MCNR4030AF-4R7M / C49449313

- Model: `DATASHEET_DERIVED_MYX_MCNR4030AF-4R7M_C49449313.wrl`.
- Classification: **PROVISIONAL DATASHEET-DERIVED**, not vendor CAD.
- The exact-part EasyEDA/LCSC record identifies package `IND-SMD_L4.0-W4.0`; the BOM and footprint specify 4.0 × 4.0 × 3.0 mm. The linked exact-part STEP UUID was unavailable (HTTP 404).
- Body envelope: 4.0 × 4.0 × 3.0 mm. Seating plane: Z=0. Terminal envelopes: 1.10 × 3.70 × 0.30 mm centered at X=±1.50 mm, matching the authoritative production land geometry.
- Replace with manufacturer CAD if MYX later publishes it; do not represent this asset as vendor CAD.
