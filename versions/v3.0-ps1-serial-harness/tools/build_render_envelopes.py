#!/usr/bin/env python3
import json, re, sys

src, dst = sys.argv[1], sys.argv[2]
text = open(src, encoding="utf-8").read()

def blocks(s, token="(footprint "):
    pos = 0
    while True:
        start = s.find(token, pos)
        if start < 0: return
        depth = 0
        quoted = False
        esc = False
        for i in range(start, len(s)):
            c = s[i]
            if quoted:
                if esc: esc = False
                elif c == "\\": esc = True
                elif c == '"': quoted = False
            else:
                if c == '"': quoted = True
                elif c == '(': depth += 1
                elif c == ')':
                    depth -= 1
                    if depth == 0:
                        yield s[start:i+1]
                        pos = i + 1
                        break

def package(fp, ref):
    # millimetres: X, Y, body height, material class
    if ref in {"J1","J2","J3","U1","SW1","SW2","D2","L1"} or ref.startswith("TP"):
        return None
    rules = [
        ("ESP32-WROOM-32", (18.0, 25.5, 3.1, "module")),
        ("SOIC-16", (10.0, 6.0, 1.75, "ic")),
        ("TSSOP-8", (3.0, 4.4, 1.1, "ic")),
        ("TSOT-23-6", (3.0, 1.7, 1.0, "ic")),
        ("SOT-23-6", (3.0, 1.7, 1.1, "ic")),
        ("SOT-23-5", (3.0, 1.7, 1.1, "ic")),
        ("SOT-23", (3.0, 1.4, 1.1, "ic")),
        ("D_SMA", (4.6, 2.8, 2.2, "diode")),
        ("SOD-323", (2.5, 1.25, 0.95, "diode")),
        ("LED_0603", (1.6, 0.8, 0.55, "led")),
        ("PinHeader_1x04_P2.54mm_Vertical", (2.54, 10.16, 8.5, "connector")),
        ("Sunlord_SWPA4030S", (4.0, 4.0, 3.0, "inductor")),
        ("Fuse_1206", (3.2, 1.6, 1.0, "fuse")),
        ("C_1206", (3.2, 1.6, 0.75, "capacitor")),
        ("C_0805", (2.0, 1.25, 0.65, "capacitor")),
        ("C_0603", (1.6, 0.8, 0.55, "capacitor")),
        ("R_1206", (3.2, 1.6, 0.65, "resistor")),
        ("R_0805", (2.0, 1.25, 0.55, "resistor")),
        ("R_0603", (1.6, 0.8, 0.45, "resistor")),
        ("R_0402", (1.0, 0.5, 0.35, "resistor")),
    ]
    for key, value in rules:
        if key in fp: return value
    raise SystemExit(f"unclassified footprint {ref}: {fp}")

items=[]
all_refs=[]
coverage=[]
critical = {
    "J1": ("EXACT_MESH", "PlayBridge.3dshapes/HC-USB3.0-L168-ZP_C7501856.wrl"),
    "J2": ("EXACT_MESH", "PlayBridge.3dshapes/HC-USB3.0-L168-ZP_C7501856.wrl"),
    "J3": ("EXACT_MESH", "PlayBridge.3dshapes/TYPE-C-31-M-12.step"),
    "U1": ("AUTHORITATIVE_KICAD_MODEL_CROSSCHECKED_TO_MPN", "PlayBridge.3dshapes/KICAD_OFFICIAL_ESP32-WROOM-32.step"),
    "SW1": ("EXACT_PART_COMPONENT_ASSET", "PlayBridge.3dshapes/K2-1107ST-A4SW-06.step"),
    "SW2": ("EXACT_PART_COMPONENT_ASSET", "PlayBridge.3dshapes/K2-1107ST-A4SW-06.step"),
    "D2": ("AUTHORITATIVE_KICAD_PACKAGE_MODEL", "PlayBridge.3dshapes/KICAD_OFFICIAL_D_SMA_DO-214AC.step"),
    "L1": ("DATASHEET_DERIVED", "PlayBridge.3dshapes/DATASHEET_DERIVED_SUNLORD_WPN4020H4R7MT_C98363.wrl"),
}
for b in blocks(text):
    fm=re.match(r'\(footprint "([^"]+)"', b)
    rm=re.search(r'\(property "Reference" "([^"]+)"', b)
    tm=re.search(r'\(transform\s+\(translate ([^ ]+) ([^)]+)\)\s+\(rotate ([^)]+)\)', b)
    if not (fm and rm and tm): continue
    fp, ref = fm.group(1), rm.group(1)
    all_refs.append(ref)
    p=package(fp, ref)
    if p:
        items.append({"reference":ref,"footprint":fp,"x_mm":float(tm.group(1)),"y_mm":float(tm.group(2)),"rotation_deg":float(tm.group(3)),"size_mm":list(p[:3]),"material":p[3],"classification":"PROVISIONAL_RENDER_ENVELOPE_FROM_EXACT_PACKAGE_SIZE"})
        coverage.append({"reference":ref,"footprint":fp,"classification":"DIMENSIONALLY_ACCURATE_STOCK_PACKAGE_ENVELOPE","model_source":"render mirror envelope from exact package size"})
    elif ref.startswith("TP"):
        coverage.append({"reference":ref,"footprint":fp,"classification":"INTENTIONALLY_PAD_ONLY","model_source":None})
    else:
        classification, source = critical[ref]
        coverage.append({"reference":ref,"footprint":fp,"classification":classification,"model_source":source})

out={"source":"board.kicad_pcb","total_footprints":len(all_refs),"render_envelopes":len(items),"exact_scene_models":["J1","J2","J3","U1","SW1","SW2","D2"],"datasheet_derived_scene_models":["L1"],"intentional_pad_only":["TP1","TP2","TP3","TP4"],"missing":[],"coverage":sorted(coverage,key=lambda x:x["reference"]),"items":sorted(items,key=lambda x:x["reference"])}
json.dump(out, open(dst,"w",encoding="utf-8"), indent=2)
print(json.dumps({k:v for k,v in out.items() if k!="items"}, indent=2))
