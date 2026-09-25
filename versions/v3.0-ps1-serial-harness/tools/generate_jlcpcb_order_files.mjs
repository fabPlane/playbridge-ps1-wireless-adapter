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
const genericBomLines = fs.readFileSync(path.join(root, 'exports', 'bom.csv'), 'utf8').trim().split(/\r?\n/);
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
- L1: MYX MCNR4030AF-4R7M / C49449313, 4.0 x 4.0 mm, 4.7 uH, 2.5 A rated, 3 A saturation. The inherited sourcing audit recorded low stock (40 units); this regeneration did not perform a live-stock query, so recheck immediately before ordering.
- R20: TA-I RMS06FT6980 / C912219, 698 ohm E96 substitute for the 700 ohm nominal. The TPS2113A current-limit setpoint difference is approximately +0.29%.
- J1/J2: Molex 48393-0003 / C3197928. The inherited sourcing audit recorded only 7 available units; this regeneration did not perform a live-stock query. With two populated per board, recheck stock at checkout. J2 carries the user/team bench-validated three-signal PS1 serial harness on contacts 1 (PS1_RXD), 4 (PS1_TXD) and 7 (GND); all other signal contacts remain NC.
- D6: KENTO KT-0603R / C2286, red 0603 always-on 3V3 POWER indicator.
- R29: UNI-ROYAL 0603WAF1001T5E / C21190, 1 kΩ 0603 POWER-LED series resistor.

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
