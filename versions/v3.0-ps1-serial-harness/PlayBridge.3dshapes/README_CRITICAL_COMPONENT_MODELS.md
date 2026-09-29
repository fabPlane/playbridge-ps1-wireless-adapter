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

## L1 — Sunlord WPN4020H4R7MT / C98363

- Model: `DATASHEET_DERIVED_SUNLORD_WPN4020H4R7MT_C98363.wrl`.
- Classification: **PROVISIONAL DATASHEET-DERIVED**, not vendor CAD.
- The Sunlord WPN4020H land pattern specifies a 1.9 mm pad gap with 1.10 × 3.70 mm pads centered at X=±1.50 mm, exactly matching the inherited copper.
- Body envelope: 4.0 × 4.0 × 2.0 mm. Seating plane: Z=0. Terminal envelopes: 1.10 × 3.70 × 0.30 mm centered at X=±1.50 mm.
- Replace with manufacturer CAD if a qualified exact-part model is obtained; do not represent this envelope as vendor CAD.
