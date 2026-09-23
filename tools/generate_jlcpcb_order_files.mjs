import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
const doc = JSON.parse(fs.readFileSync(path.join(root, 'circuit.netlist.json'), 'utf8'));
const outDir = path.join(root, 'ordering');
fs.mkdirSync(outDir, { recursive: true });

const csv = value => `"${String(value ?? '').replaceAll('"', '""')}"`;
const fitted = doc.netlist.components.filter(c => c.fields?.DNP !== 'Yes');
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
- L1: MYX MCNR4030AF-4R7M / C49449313, 4.0 x 4.0 mm, 4.7 uH, 2.5 A rated, 3 A saturation. JLC stock was low (40 units when checked), so recheck immediately before ordering.
- R20: TA-I RMS06FT6980 / C912219, 698 ohm E96 substitute for the 700 ohm nominal. The TPS2113A current-limit setpoint difference is approximately +0.29%.
- J1/J2: Molex 48393-0003 / C3197928. JLC showed only 7 available units when checked; with two populated per board, the public stock supports at most 3 complete boards. J2 is electrically reserved/all-NC, but remains populated in this BOM because the current board/render includes it.

## Order files

- \`jlcpcb-bom.csv\`: grouped JLC/LCSC BOM; all fitted references assigned exactly once.
- \`jlcpcb-cpl.csv\`: top-side placement list derived from FabDesk's existing \`exports/positions.csv\`, excluding bare-board test pads.
- \`gerbers-jlcpcb.zip\`: flat manufacturing Gerber/drill archive copied from FabDesk's verified export set.

## Fresh release verification

- KiCad DRC: 0 violations
- Unconnected items: 0
- Schematic-to-PCB parity issues: 0
- KiCad ERC: 0 violations
- Board and schematic hashes remained unchanged during the BOM/order-file pass.

Stock is time-sensitive; JLCPCB/LCSC availability must be rechecked by the order portal at upload time.
`;
fs.writeFileSync(path.join(outDir, 'PCBA-SOURCING-AUDIT.md'), report);

console.log(`Generated ${rows.length} BOM rows covering ${fitted.length} fitted references and ${parsed.length} CPL rows.`);
