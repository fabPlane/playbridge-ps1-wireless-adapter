import fs from 'node:fs';

const sourcePath = new URL('../circuit.netlist.json', import.meta.url);
const doc = JSON.parse(fs.readFileSync(sourcePath, 'utf8'));

const parts = {
  U1: ['Espressif Systems', 'ESP32-WROOM-32E-N4', 'C701341', 'ESP32-WROOM-32 module'],
  U2: ['Diodes Incorporated', 'AP63203WU-7', 'C780769', 'TSOT-23-6'],
  U3: ['Diodes Incorporated', 'AP2112K-3.3TRG1', 'C51118', 'SOT-25-5'],
  U4: ['Texas Instruments', 'TPS2113APWR', 'C130052', 'TSSOP-8'],
  U5: ['WCH', 'CH340C', 'C7464026', 'SOP-16'],
  U6: ['STMicroelectronics', 'USBLC6-2SC6', 'C7519', 'SOT-23-6L'],
  L1: ['MYX', 'MCNR4030AF-4R7M', 'C49449313', 'SMD 4.0x4.0mm, 4.7uH, 2.5A rated, 3A saturation'],
  D1: ['Everlight Electronics', '19-217/GHC-YR1S2/3T', 'C72043', '0603 green LED'],
  D2: ['GOODWORK', 'SMAJ12A', 'C908766', 'DO-214AC (SMA), 12V standoff, 400W unidirectional TVS'],
  D3: ['JSMSEMI', 'BAT54WS', 'C916997', 'SOD-323 Schottky diode'],
  D4: ['JSMSEMI', 'BAT54WS', 'C916997', 'SOD-323 Schottky diode'],
  D5: ['JSMSEMI', 'BAT54WS', 'C916997', 'SOD-323 Schottky diode'],
  F1: ['BHFUSE', 'BSMD1206-075-30V', 'C976305', '1206 resettable fuse, 750mA hold, 1.5A trip, 30V'],
  Q1: ['Jiangsu Changjing Electronics Technology', '2N7002', 'C8545', 'SOT-23 N-channel MOSFET'],
  Q2: ['Jiangsu Changjing Electronics Technology', '2N7002', 'C8545', 'SOT-23 N-channel MOSFET'],
  Q3: ['Jiangsu Changjing Electronics Technology', 'MMBT3904(RANGE:100-300)', 'C20526', 'SOT-23 NPN transistor'],
  Q4: ['Jiangsu Changjing Electronics Technology', 'MMBT3904(RANGE:100-300)', 'C20526', 'SOT-23 NPN transistor'],
};

const groups = [
  [['C1', 'C2'], 'Samsung Electro-Mechanics', 'CL31A226KAHNNNE', 'C12891', '1206, 22uF, 25V, X5R, +/-10%'],
  [['C3'], 'CCTC', 'TCC0805X5R106K160FT', 'C380332', '0805, 10uF, 16V, X5R, +/-10%'],
  [['C4', 'C7', 'C8', 'C10', 'C13'], 'Yageo', 'CC0603KRX7R9BB104', 'C14663', '0603, 100nF, 50V, X7R, +/-10%'],
  [['C5', 'C11', 'C12'], 'Samsung Electro-Mechanics', 'CL10A105KA8NNNC', 'C5673', '0603, 1uF, 25V, X5R, +/-10%'],
  [['C6'], 'CCTC', 'TCC0805X5R475K160FT', 'C380342', '0805, 4.7uF, 16V, X5R, +/-10%'],
  [['C9'], 'SANYEAR', 'C0805X5R226M6R3NT', 'C466795', '0805, 22uF, 6.3V, X5R, +/-20%'],
  [['R1', 'R3', 'R5', 'R13', 'R16'], 'UNI-ROYAL', '0603WAF1001T5E', 'C21190', '0603, 1k, +/-1%'],
  [['R2', 'R4', 'R6', 'R11', 'R12', 'R18', 'R19'], 'UNI-ROYAL', '0603WAF1002T5E', 'C25804', '0603, 10k, +/-1%'],
  [['R7', 'R9'], 'UNI-ROYAL', '0603WAF1000T5E', 'C22775', '0603, 100R, +/-1%'],
  [['R8', 'R10'], 'UNI-ROYAL', '0603WAF1003T5E', 'C25803', '0603, 100k, +/-1%'],
  [['R17'], 'UNI-ROYAL', '0603WAF2001T5E', 'C22975', '0603, 2k, +/-1%'],
  [['R20'], 'TA-I', 'RMS06FT6980', 'C912219', '0603, 698R, +/-1% (700R nominal ILIM; -0.29% substitution)'],
  [['R14', 'R15'], 'UNI-ROYAL', '0402WGF5101TCE', 'C25905', '0402, 5.1k, +/-1%'],
  [['R21', 'R22', 'R23', 'R24', 'R25', 'R26', 'R27', 'R28'], 'UNI-ROYAL', '0402WGF0000TCE', 'C17168', '0402, 0R jumper'],
];

for (const [refs, manufacturer, mpn, lcsc, pkg] of groups) {
  for (const ref of refs) parts[ref] = [manufacturer, mpn, lcsc, pkg];
}

for (const component of doc.netlist.components) {
  component.fields ??= {};
  if (parts[component.ref]) {
    const [manufacturer, mpn, lcsc, pkg] = parts[component.ref];
    Object.assign(component.fields, {
      Manufacturer: manufacturer,
      MPN: mpn,
      'JLCPCB/LCSC Part': lcsc,
      LCSC: lcsc,
      Package: pkg,
      'Procurement Status': 'JLCPCB in-stock selection verified 2026-09-22',
      'JLCPCB Part URL': `https://jlcpcb.com/partdetail/${lcsc}`,
    });
  }
  if (component.ref === 'R20') {
    component.fields['Engineering Note'] = '698R is the stocked E96 substitute for 700R; TPS2113A current-limit setpoint changes by approximately +0.29%.';
  }
  if (component.ref === 'D2') {
    component.fields['Engineering Note'] = 'Corrected from SMBJ12A: PCB footprint is DO-214AC/SMA. Electrical rating remains 12V standoff, unidirectional TVS.';
  }
  if (component.ref === 'J3') {
    Object.assign(component.fields, {
      Manufacturer: 'Korean Hroparts Elec',
      'JLCPCB/LCSC Part': 'C165948',
      LCSC: 'C165948',
      Package: 'USB Type-C 16P right-angle SMD receptacle',
      'Procurement Status': 'JLCPCB in-stock selection verified 2026-09-22',
      'JLCPCB Part URL': 'https://jlcpcb.com/partdetail/C165948',
    });
  }
  if (component.ref === 'J1' || component.ref === 'J2') {
    component.fields.Manufacturer = 'Hong Cheng';
    component.fields.MPN = 'HC-USB3.0-L168-ZP';
    component.fields['JLCPCB/LCSC Part'] = 'C7501856';
    component.fields.LCSC = 'C7501856';
    component.fields.Package = 'USB 3.0 Type-A right-angle through-hole receptacle, 9 signal pins + 2 shell slots';
    component.fields['Procurement Status'] = 'JLCPCB in-stock assembly part verified 2026-09-25; recheck live stock at order time';
    component.fields['JLCPCB Part URL'] = 'https://jlcpcb.com/partdetail/HongCheng-HC_USB3_0_L168ZP/C7501856';
    component.fields['Normalized Manufacturer Number'] = 'HC-USB3.0-L168-ZP';
    component.fields.Datasheet = 'https://jlcpcb.com/partdetail/HongCheng-HC_USB3_0_L168ZP/C7501856';
    component.fields['Footprint Provenance'] = 'Official JLCPCB/EasyEDA C7501856 land geometry for Hong Cheng HC-USB3.0-L168-ZP, converted locally with easyeda2kicad.';
    component.fields.Orientation = 'Right-angle top-mount receptacle at the lower board edge; mating mouth faces outward. Signal-pad coordinates are preserved from the validated board routing.';
  }
  if (component.ref === 'SW1' || component.ref === 'SW2') {
    component.fields['JLCPCB/LCSC Part'] = 'C2837195';
    component.fields['Procurement Status'] = 'JLCPCB in-stock selection verified 2026-09-22';
    component.fields['JLCPCB Part URL'] = 'https://jlcpcb.com/partdetail/C2837195';
  }
  if (/^TP[1-4]$/.test(component.ref)) {
    Object.assign(component.fields, {
      DNP: 'Yes',
      Assembly: 'DNP - bare PCB test pad; no purchased component',
    });
  }
}

const missing = doc.netlist.components
  .filter(c => !/^TP[1-4]$/.test(c.ref))
  .filter(c => !(c.fields?.['JLCPCB/LCSC Part'] || c.fields?.LCSC))
  .map(c => c.ref);
if (missing.length) throw new Error(`Missing orderable part metadata: ${missing.join(', ')}`);

fs.writeFileSync(sourcePath, `${JSON.stringify(doc, null, 2)}\n`);
console.log(`Updated ${doc.netlist.components.length} components; all fitted references have JLC/LCSC selections.`);
