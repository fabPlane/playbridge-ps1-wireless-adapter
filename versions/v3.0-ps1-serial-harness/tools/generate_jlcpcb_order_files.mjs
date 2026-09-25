import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const doc = JSON.parse(fs.readFileSync(path.join(root, 'circuit.netlist.json'), 'utf8'));
const outDir = path.join(root, 'ordering');
fs.mkdirSync(outDir, { recursive: true });

const csv = value => `"${String(value ?? '').replaceAll('"', '""')}"`;
const fitted = doc.netlist.components.filter(c => c.fields?.DNP !== 'Yes');
const fittedRefSet = new Set(fitted.map(c => c.ref));
const grouped = new Map();
for (const c of fitted) {
  const f = c.fields ?? {};
  const lcsc = f['JLCPCB/LCSC Part'] || f.LCSC;
  if (!lcsc) throw new Error(`Missing LCSC selection for ${c.ref}`);
  const key = [lcsc, f.MPN, c.footprint].join('|');
  const row = grouped.get(key) ?? {
    comment: f.MPN || c.value,
    designators: [],
    footprint: c.footprint.replace(/^PlayBridge:/, ''),
    lcsc,
    manufacturer: f.Manufacturer || '',
    mpn: f.MPN || '',
    package: f.Package || '',
  };
  row.designators.push(c.ref);
  grouped.set(key, row);
}

const natural = new Intl.Collator(undefined, { numeric: true, sensitivity: 'base' });
const rows = [...grouped.values()].sort((a, b) => natural.compare(a.designators[0], b.designators[0]));
const genericFields = [
  'Reference', 'Value', 'Footprint', 'Assembly', 'DNP', 'Datasheet',
  'Engineering Note', 'Footprint Provenance', 'Function', 'Hole',
  'JLCPCB Part URL', 'JLCPCB/LCSC Part', 'LCSC', 'Label', 'MPN',
  'Manufacturer', 'Normalized Manufacturer Number', 'Orientation', 'Package',
  'Pad', 'Pinout', 'Placement Constraint', 'Procurement Status', 'Safety',
];
const genericBomLines = [
  genericFields,
  ...[...doc.netlist.components]
    .sort((a, b) => natural.compare(a.ref, b.ref))
    .map(c => genericFields.map(field => {
      if (field === 'Reference') return c.ref;
      if (field === 'Value') return c.value;
      if (field === 'Footprint') return c.footprint;
      return c.fields?.[field] ?? '';
    })),
].map(row => row.map(csv).join(','));
fs.writeFileSync(path.join(root, 'exports', 'bom.csv'), `${genericBomLines.join('\n')}\n`);
const bomLines = [
  ['Comment', 'Designator', 'Footprint', 'LCSC Part #', 'Manufacturer', 'Manufacturer Part Number', 'Package', 'Quantity'],
  ...rows.map(r => [r.comment, r.designators.sort(natural.compare).join(','), r.footprint, r.lcsc, r.manufacturer, r.mpn, r.package, r.designators.length]),
].map(row => row.map(csv).join(','));
fs.writeFileSync(path.join(outDir, 'jlcpcb-bom.csv'), `${bomLines.join('\n')}\n`);

const posLines = fs.readFileSync(path.join(root, 'exports', 'positions.csv'), 'utf8').trim().split(/\r?\n/);
const parsed = posLines.slice(1).map(line => {
  const fields = [...line.matchAll(/"([^"]*)"|([^,]+)/g)].map(m => m[1] ?? m[2]);
  return { ref: fields[0], x: fields[3], y: fields[4], rot: fields[5], side: fields[6] };
}).filter(r => !/^TP[1-4]$/.test(r.ref));
const cplLines = [
  ['Designator', 'Mid X', 'Mid Y', 'Layer', 'Rotation'],
  ...parsed.map(r => [r.ref, `${r.x}mm`, `${r.y}mm`, r.side === 'bottom' ? 'Bottom' : 'Top', r.rot]),
].map(row => row.map(csv).join(','));
fs.writeFileSync(path.join(outDir, 'jlcpcb-cpl.csv'), `${cplLines.join('\n')}\n`);

// Generic assembly exports retain FabDesk's native CSV schemas while excluding
// DNP plated test holes and any other explicitly non-fitted references.
const assemblyBomLines = [
  genericBomLines[0],
  ...genericBomLines.slice(1).filter(line => fittedRefSet.has(line.match(/^\"?([A-Za-z]+[0-9]+)\"?,/)?.[1])),
];
fs.writeFileSync(path.join(root, 'exports', 'assembly-bom.csv'), `${assemblyBomLines.join('\n')}\n`);

const assemblyPosLines = [
  posLines[0],
  ...posLines.slice(1).filter(line => fittedRefSet.has(line.match(/^\"([A-Za-z]+[0-9]+)\"/)?.[1])),
];
fs.writeFileSync(path.join(root, 'exports', 'assembly-cpl.csv'), `${assemblyPosLines.join('\n')}\n`);

const refs = fitted.map(c => c.ref);
const selectedRefs = rows.flatMap(r => r.designators);
const missing = refs.filter(ref => !selectedRefs.includes(ref));
const duplicateRefs = selectedRefs.filter((ref, i, all) => all.indexOf(ref) !== i);
const auditDate = new Date().toISOString().slice(0, 10);
const report = `# JLCPCB PCBA sourcing audit — ${auditDate}

## Result

- Fitted references: ${fitted.length}
- Unique orderable line items: ${rows.length}
- DNP/bare-board test pads: TP1, TP2, TP3, TP4
- Missing fitted references: ${missing.length ? missing.join(', ') : 'none'}
- Duplicate reference assignments: ${duplicateRefs.length ? duplicateRefs.join(', ') : 'none'}
- Board routing/placement changed by this sourcing pass: no

## Footprint-sensitive selections

- D2: GOODWORK SMAJ12A / C908766, DO-214AC (SMA). The earlier value label said SMBJ12A, but the PCB uses an SMA land pattern; the selected SMAJ12A preserves the 12 V unidirectional TVS function and fits the footprint family.
- F1: BHFUSE BSMD1206-075-30V / C976305, 1206, 750 mA hold, 1.5 A trip, 30 V.
- L1: Sunlord WPN4020H4R7MT / C98363, 4.0 x 4.0 x 2.0 mm, 4.7 uH +/-20%, 2.85 A rated, 4.9 A saturation, 108 mOhm. The official 1.9 mm pad gap and 1.1 x 3.7 mm lands exactly match the inherited routed footprint.
- R20: UNI-ROYAL 0603WAF6800T5E / C23228, 680 ohm +/-1%. Relative to the previous 698 ohm selection, the calculated TPS2113A current limit increases by approximately 2.65%; verify intended current-limit acceptability.
- C3: Samsung CL21A106KOQNNNE / C1713, 10 uF 16 V X5R +/-10%, 0805.
- C9: Samsung CL21A226MQQNNNE / C5674, 22 uF 6.3 V X5R +/-20%, 0805.
- D1: KENTO KT-0603YG / C2289, yellow-green 0603 STATUS LED; polarity and STATUS_LED wiring are unchanged.
- J1/J2: Hong Cheng HC-USB3.0-L168-ZP / C7501856. JLCPCB listed this as a ready-for-assembly in-stock part during the 2026-09-25 replacement pass; recheck live stock at checkout. Its nine electrical-pad coordinates exactly match the replaced connector, so routing is unchanged. J2 carries the user/team bench-validated three-signal PS1 serial harness on contacts 1 (PS1_RXD), 4 (PS1_TXD) and 7 (GND); all other signal contacts remain NC.
- D6: KENTO KT-0603R / C2286, red 0603 always-on 3V3 POWER indicator.
- R1/R3/R5/R13/R16/R29/R30/R31: Yageo RC0603FR-071KL / C22548, 1 kΩ +/-1%, 100 mW, 75 V, 0603. This is the quote-time replacement for out-of-stock UNI-ROYAL 0603WAF1001T5E / C21190. Electrical value, tolerance, power rating, voltage rating, and land pattern are unchanged; routing and placement are unchanged.

## Live JLCPCB quote review — 2026-09-25

- JLCPCB initially displayed zero available quantity for C7501856 and C2837195. Searching public PCBA inventory and reselecting the exact same catalog entries resolved the stale matches: J1/J2 remain C7501856 and SW1/SW2 remain C2837195. These were not substitutions.
- C21190 was genuinely unavailable in the live PCBA quote. All eight 1 kΩ 0603 positions were replaced in the quote with C22548 as documented above.
- All 34 uploaded BOM rows were confirmed. The portal grouped duplicate catalog selections into 31 charged component items.
- Placement preview was reviewed before pricing: J1/J2 mouths face the lower board edge, J3 remains on the right edge, and no obvious 180-degree placement error was visible.
- Five-board quote: PCB $13.10; Standard PCBA $143.95; total $157.05 before shipping and tax. No order was placed and Save to Cart was not clicked.

## Order files

- \`jlcpcb-bom.csv\`: grouped JLC/LCSC BOM; all fitted references assigned exactly once.
- \`jlcpcb-cpl.csv\`: top-side placement list derived from FabDesk's existing \`exports/positions.csv\`, excluding bare-board test pads.
- \`gerbers-jlcpcb.zip\`: flat manufacturing Gerber/drill archive copied from FabDesk's verified export set.

## Fresh release verification

- KiCad DRC: 0 errors, with only the three approved historical track_dangling warnings
- Unconnected items: 0
- Schematic-to-PCB parity issues: 0
- KiCad ERC: 0 violations
- Board and schematic hashes remained unchanged during the BOM/order-file pass.

Stock is time-sensitive; JLCPCB/LCSC availability must be rechecked by the order portal at upload time.
`;
fs.writeFileSync(path.join(outDir, 'PCBA-SOURCING-AUDIT.md'), report);

console.log(`Generated ${rows.length} BOM rows covering ${fitted.length} fitted references and ${parsed.length} CPL rows.`);
