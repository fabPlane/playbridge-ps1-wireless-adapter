#!/usr/bin/env python3
"""Narrow, syntax-aware KiCad PCB GND dogbone repair utility."""

from __future__ import annotations

import argparse
import math
import re
import uuid
from pathlib import Path


def top_level_blocks(text: str):
    """Yield (start, end, head, block) for direct children of kicad_pcb."""
    depth = 0
    start = None
    in_string = False
    escaped = False
    for i, ch in enumerate(text):
        if in_string:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
        elif ch == "(":
            depth += 1
            if depth == 2:
                start = i
        elif ch == ")":
            if depth == 2 and start is not None:
                block = text[start : i + 1]
                m = re.match(r"\(([A-Za-z0-9_.-]+)", block)
                yield start, i + 1, m.group(1) if m else "", block
                start = None
            depth -= 1


def child_blocks(text: str):
    """Yield direct S-expression children from a block including its wrapper."""
    depth = 0
    start = None
    in_string = False
    escaped = False
    for i, ch in enumerate(text):
        if in_string:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == '"':
                in_string = False
            continue
        if ch == '"':
            in_string = True
        elif ch == "(":
            depth += 1
            if depth == 2:
                start = i
        elif ch == ")":
            if depth == 2 and start is not None:
                block = text[start:i + 1]
                m = re.match(r"\(([A-Za-z0-9_.-]+)", block)
                yield m.group(1) if m else "", block
                start = None
            depth -= 1


def first_xy(pattern: str, text: str, default=(0.0, 0.0)):
    m = re.search(pattern, text)
    return (float(m.group(1)), float(m.group(2))) if m else default


def transform_point(x, y, tx, ty, deg):
    a = math.radians(deg)
    # KiCad board coordinates use +Y down; positive footprint rotation is clockwise.
    return tx + x * math.cos(a) + y * math.sin(a), ty - x * math.sin(a) + y * math.cos(a)


def collect_smd_gnd_groups(text: str):
    groups = {}
    for _, _, head, fp in top_level_blocks(text):
        if head != "footprint":
            continue
        refm = re.search(r'\(property\s+"Reference"\s+"([^"]+)"', fp)
        tr = re.search(r'\(transform\s+\(translate\s+([-\d.]+)\s+([-\d.]+)\)\s+\(rotate\s+([-\d.]+)\)', fp)
        if not refm or not tr:
            continue
        ref = refm.group(1)
        tx, ty, rot = map(float, tr.groups())
        for phead, pad in child_blocks(fp):
            if phead != "pad" or not re.match(r'\(pad\s+"[^"]+"\s+smd\b', pad) or '(net "GND")' not in pad:
                continue
            num = re.match(r'\(pad\s+"([^"]+)"', pad).group(1)
            x, y = first_xy(r'\(at\s+([-\d.]+)\s+([-\d.]+)', pad)
            w, h = first_xy(r'\(size\s+([-\d.]+)\s+([-\d.]+)', pad)
            gx, gy = transform_point(x, y, tx, ty, rot)
            groups.setdefault((ref, num), []).append((gx, gy, w, h))
    return groups


def collect_geometry(text: str):
    pads, vias, segments = [], [], []
    for _, _, head, fp in top_level_blocks(text):
        if head != "footprint":
            continue
        refm = re.search(r'\(property\s+"Reference"\s+"([^"]+)"', fp)
        tr = re.search(r'\(transform\s+\(translate\s+([-\d.]+)\s+([-\d.]+)\)\s+\(rotate\s+([-\d.]+)\)', fp)
        if not refm or not tr:
            continue
        ref = refm.group(1)
        tx, ty, rot = map(float, tr.groups())
        for phead, pad in child_blocks(fp):
            if phead != "pad":
                continue
            pm = re.match(r'\(pad\s+"([^"]+)"\s+(\w+)', pad)
            if not pm:
                continue
            num, kind = pm.groups()
            x, y = first_xy(r'\(at\s+([-\d.]+)\s+([-\d.]+)', pad)
            w, h = first_xy(r'\(size\s+([-\d.]+)\s+([-\d.]+)', pad)
            netm = re.search(r'\(net\s+"([^"]*)"\)', pad)
            gx, gy = transform_point(x, y, tx, ty, rot)
            # Axis-aligned conservative box; rotate rectangular pad extents with footprint.
            if int(round(rot / 90)) % 2:
                w, h = h, w
            pads.append(dict(ref=ref, num=num, kind=kind, net=netm.group(1) if netm else "", x=gx, y=gy, w=w, h=h))
    for _, _, head, block in top_level_blocks(text):
        if head == "via":
            x, y = first_xy(r'\(at\s+([-\d.]+)\s+([-\d.]+)', block)
            sm = re.search(r'\(size\s+([-\d.]+)\)', block)
            nm = re.search(r'\(net\s+"([^"]*)"\)', block)
            um = re.search(r'\(uuid\s+"?([^"\s\)]+)', block)
            vias.append(dict(x=x, y=y, r=float(sm.group(1))/2, net=nm.group(1) if nm else "", uuid=um.group(1) if um else ""))
        elif head == "segment":
            a = first_xy(r'\(start\s+([-\d.]+)\s+([-\d.]+)', block)
            b = first_xy(r'\(end\s+([-\d.]+)\s+([-\d.]+)', block)
            wm = re.search(r'\(width\s+([-\d.]+)\)', block)
            nm = re.search(r'\(net\s+"([^"]*)"\)', block)
            lm = re.search(r'\(layer\s+"([^"]+)"\)', block)
            segments.append(dict(a=a, b=b, r=float(wm.group(1))/2, net=nm.group(1) if nm else "", layer=lm.group(1) if lm else ""))
    return pads, vias, segments


def point_segment_distance(p, a, b):
    px, py = p; ax, ay = a; bx, by = b
    dx, dy = bx-ax, by-ay
    if dx*dx + dy*dy == 0:
        return math.hypot(px-ax, py-ay)
    t = max(0.0, min(1.0, ((px-ax)*dx + (py-ay)*dy)/(dx*dx+dy*dy)))
    return math.hypot(px-(ax+t*dx), py-(ay+t*dy))


def point_rect_distance(p, q):
    dx = max(abs(p[0]-q['x']) - q['w']/2, 0)
    dy = max(abs(p[1]-q['y']) - q['h']/2, 0)
    return math.hypot(dx, dy)


def clear_candidate(center, source, pads, vias, segments, chosen, clearance=0.20):
    vr = 0.30
    x, y = center
    if x < 0.55 or x > 59.45 or y < 0.55 or y > 61.45:
        return False
    for p in pads:
        if p['net'] == 'GND':
            continue
        if point_rect_distance(center, p) < vr + clearance:
            return False
        for frac in (0.25, 0.5, 0.75):
            sp = (source[0]+frac*(x-source[0]), source[1]+frac*(y-source[1]))
            if point_rect_distance(sp, p) < 0.125 + clearance:
                return False
    for v in vias:
        if v['net'] == 'GND':
            continue
        if math.hypot(x-v['x'], y-v['y']) < vr + v['r'] + clearance:
            return False
        if point_segment_distance((v['x'], v['y']), source, center) < v['r'] + 0.125 + clearance:
            return False
    for s in segments:
        if s['net'] == 'GND':
            continue
        if point_segment_distance(center, s['a'], s['b']) < vr + s['r'] + clearance:
            return False
        # Endpoint sampling gives a conservative local crossing screen; DRC is authoritative.
        if s['layer'] == 'F.Cu':
            for frac in (0.25, 0.5, 0.75):
                p = (source[0]+frac*(x-source[0]), source[1]+frac*(y-source[1]))
                if point_segment_distance(p, s['a'], s['b']) < 0.125 + s['r'] + clearance:
                    return False
    for c in chosen:
        if math.hypot(x-c[0], y-c[1]) < 0.80:
            return False
    return True


def repair(path: Path):
    text = path.read_text()
    original = text
    removed = []
    kept = []
    for start, end, head, block in top_level_blocks(text):
        drop = False
        if head == 'via':
            at = first_xy(r'\(at\s+([-\d.]+)\s+([-\d.]+)', block)
            netm = re.search(r'\(net\s+"([^"]*)"\)', block)
            net = netm.group(1) if netm else ''
            if net == 'USB_DP' and math.hypot(at[0]-28.7875, at[1]-10.075) < 0.002:
                drop = True
            if net == 'PS_3V3' and math.hypot(at[0]-13.9425, at[1]-27.475) < 0.002:
                drop = True
            if '203a1711-' in block:
                drop = True
        if drop:
            removed.append((start, end, head, block))
        else:
            kept.append((start, end, head, block))
    for start, end, _, _ in reversed(removed):
        text = text[:start] + text[end:]

    pads, vias, segments = collect_geometry(text)
    groups = collect_smd_gnd_groups(text)
    chosen = []
    additions = []
    plan = []
    angles = [i * 11.25 for i in range(32)]
    for (ref, num), members in groups.items():
        # Use the largest constituent pad (the exposed-pad group includes one 4.2 mm pad).
        srcpad = max(members, key=lambda p: p[2]*p[3])
        sx, sy, w, h = srcpad
        found = None
        for extra in (0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.25, 1.5, 2.0, 2.5, 3.0):
            for deg in angles:
                a = math.radians(deg)
                ux, uy = math.cos(a), math.sin(a)
                # Rectangle boundary along ray + via radius + 0.05 mm copper overlap margin.
                denom = max(abs(ux)/(w/2 if w else 0.01), abs(uy)/(h/2 if h else 0.01))
                edge = 1/denom if denom else max(w, h)/2
                d = edge + 0.30 - 0.05 + extra
                c = (round(sx+ux*d, 4), round(sy+uy*d, 4))
                if clear_candidate(c, (sx, sy), pads, vias, segments, chosen):
                    found = c
                    break
            if found:
                break
        if not found:
            raise RuntimeError(f'No collision-screened dogbone candidate for {ref}.{num} at {(sx,sy)}')
        chosen.append(found)
        suid, vuid = str(uuid.uuid4()), str(uuid.uuid4())
        additions.append(f'''\n\t(segment\n\t\t(start {sx:.4f} {sy:.4f})\n\t\t(end {found[0]:.4f} {found[1]:.4f})\n\t\t(width 0.25)\n\t\t(layer "F.Cu")\n\t\t(net "GND")\n\t\t(uuid "{suid}")\n\t)\n\t(via\n\t\t(at {found[0]:.4f} {found[1]:.4f})\n\t\t(size 0.6)\n\t\t(drill 0.3)\n\t\t(layers "F.Cu" "B.Cu")\n\t\t(net "GND")\n\t\t(uuid "{vuid}")\n\t)''')
        plan.append((ref, num, sx, sy, found[0], found[1]))

    # Insert before the first board-level zone, leaving filled zone data for KiCad to refill.
    zone_start = next(start for start, _, head, _ in top_level_blocks(text) if head == 'zone')
    text = text[:zone_start] + ''.join(additions) + '\n' + text[zone_start:]

    # Discard stale cached fills. KiCad/FabDesk will regenerate these from zone polygons.
    for start, end, head, zone in reversed(list(top_level_blocks(text))):
        if head != 'zone':
            continue
        spans = []
        depth = 0; child_start = None; in_string = False; escaped = False
        for i, ch in enumerate(zone):
            if in_string:
                if escaped: escaped = False
                elif ch == '\\': escaped = True
                elif ch == '"': in_string = False
                continue
            if ch == '"': in_string = True
            elif ch == '(':
                depth += 1
                if depth == 2: child_start = i
            elif ch == ')':
                if depth == 2 and child_start is not None:
                    child = zone[child_start:i+1]
                    if child.startswith('(filled_polygon') or child.startswith('(fill_segments'):
                        spans.append((child_start, i+1))
                    child_start = None
                depth -= 1
        for a, b in reversed(spans):
            zone = zone[:a] + zone[b:]
        text = text[:start] + zone + text[end:]

    # Exact 1.20 mm physical stack: reduce the central dielectric by the 0.02 mm excess.
    text, nstack = re.subn(r'(\(layer "dielectric 2"[\s\S]*?\(thickness\s+)0\.86(\))', r'\g<1>0.84\2', text, count=1)
    if nstack != 1:
        raise RuntimeError('Could not locate central 0.86 mm dielectric')

    # Add the normalized manufacturer field to J1/J2 embedded footprints.
    for ref in ('J1', 'J2'):
        blocks = list(top_level_blocks(text))
        hit = next((b for b in blocks if b[2] == 'footprint' and f'(property "Reference" "{ref}"' in b[3]), None)
        if not hit:
            raise RuntimeError(f'Missing {ref} footprint')
        start, end, _, fp = hit
        if '(property "Normalized Manufacturer Number"' not in fp:
            anchor = fp.find('\n\t\t(property "Footprint Provenance"')
            if anchor < 0:
                anchor = fp.find('\n\t\t(property "Pinout"')
            prop = f'''\n\t\t(property "Normalized Manufacturer Number" "483930003"\n\t\t\t(at 0 0 0)\n\t\t\t(unlocked yes)\n\t\t\t(layer "F.Fab")\n\t\t\t(hide yes)\n\t\t\t(uuid "{uuid.uuid4()}")\n\t\t\t(effects\n\t\t\t\t(font\n\t\t\t\t\t(size 1 1)\n\t\t\t\t\t(thickness 0.15)\n\t\t\t\t)\n\t\t\t)\n\t\t)'''
            fp = fp[:anchor] + prop + fp[anchor:]
            text = text[:start] + fp + text[end:]

    if text == original:
        raise RuntimeError('No changes made')
    path.write_text(text)
    print('removed_vias', len(removed))
    for item in removed:
        print('REMOVED', item[3].replace('\n',' ')[:220])
    print('dogbones', len(plan))
    for row in plan:
        print('DOGBONE', *row)


def sync_u1_library(board_path: Path, library_path: Path):
    text = board_path.read_text()
    fp = next(block for _, _, head, block in top_level_blocks(text)
              if head == 'footprint' and '(property "Reference" "U1"' in block)
    children = []
    for head, child in child_blocks(fp):
        if head in {'placed', 'uuid', 'transform'}:
            continue
        if head == 'property' and child.startswith('(property "Reference"'):
            child = child.replace('(property "Reference" "U1"', '(property "Reference" "REF**"', 1)
        if head == 'pad':
            child = re.sub(r'\n\s*\(net\s+"[^"]*"\)', '', child)
        children.append('\n' + '\n'.join('\t' + line for line in child.splitlines()))
    out = '(footprint "ESP32-WROOM-32_D0.3mm_SILK_CLEAN"\n'
    out += '\t(version 20260206)\n\t(generator "pcbnew")\n\t(generator_version "10.0")'
    out += ''.join(children) + '\n)\n'
    library_path.write_text(out)
    print('synced_library', library_path)


def fix_connector_fields(board_path: Path):
    text = board_path.read_text()
    for ref in ('J1', 'J2'):
        hit = next((b for b in top_level_blocks(text)
                    if b[2] == 'footprint' and f'(property "Reference" "{ref}"' in b[3]), None)
        if not hit:
            raise RuntimeError(f'Missing {ref}')
        start, end, _, fp = hit
        if '(property "JLCPCB/LCSC Part"' in fp:
            continue
        anchor = fp.find('\n\t\t(property "Footprint Provenance"')
        if anchor < 0:
            anchor = fp.find('\n\t\t(property "Pinout"')
        prop = f'''\n\t\t(property "JLCPCB/LCSC Part" "C3197928"\n\t\t\t(at 0 0 0)\n\t\t\t(unlocked yes)\n\t\t\t(layer "F.Fab")\n\t\t\t(hide yes)\n\t\t\t(uuid "{uuid.uuid4()}")\n\t\t\t(effects\n\t\t\t\t(font\n\t\t\t\t\t(size 1 1)\n\t\t\t\t\t(thickness 0.15)\n\t\t\t\t)\n\t\t\t)\n\t\t)'''
        fp = fp[:anchor] + prop + fp[anchor:]
        text = text[:start] + fp + text[end:]
    board_path.write_text(text)
    print('fixed_connector_fields', board_path)


def summarize(path: Path):
    text = path.read_text()
    net_names = {}
    counts = {}
    gnd_items = []
    experimental = []
    for start, end, head, block in top_level_blocks(text):
        counts[head] = counts.get(head, 0) + 1
        if head == "net":
            m = re.match(r'\(net\s+(\d+)\s+"([^"]*)"', block)
            if m:
                net_names[int(m.group(1))] = m.group(2)
        if "203a1711-" in block:
            experimental.append((head, start, block[:180]))
    gnd_id = next((n for n, name in net_names.items() if name == "GND"), None)
    if gnd_id is not None:
        for start, end, head, block in top_level_blocks(text):
            if head in {"segment", "arc", "via", "zone"} and re.search(rf"\(net\s+{gnd_id}\)(?!\d)", block):
                uid = re.search(r"\(uuid\s+([^\s\)]+)", block)
                gnd_items.append((head, uid.group(1) if uid else "", block[:240]))
    print("counts", counts)
    print("nets", len(net_names), "gnd_id", gnd_id)
    print("gnd_items", len(gnd_items))
    for row in gnd_items:
        print("GND", *row[:2], row[2].replace("\n", " "))
    print("experimental", len(experimental))
    for row in experimental:
        print("EXP", row[0], row[2].replace("\n", " "))
    groups = collect_smd_gnd_groups(text)
    print("smd_gnd_groups", len(groups))
    for key, pads in groups.items():
        print("PADGROUP", key, pads)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("board", type=Path)
    ap.add_argument("--summarize", action="store_true")
    ap.add_argument("--repair", action="store_true")
    ap.add_argument("--sync-u1-library", type=Path)
    ap.add_argument("--fix-connector-fields", action="store_true")
    args = ap.parse_args()
    if args.summarize:
        summarize(args.board)
    if args.repair:
        repair(args.board)
    if args.sync_u1_library:
        sync_u1_library(args.board, args.sync_u1_library)
    if args.fix_connector_fields:
        fix_connector_fields(args.board)


if __name__ == "__main__":
    main()
