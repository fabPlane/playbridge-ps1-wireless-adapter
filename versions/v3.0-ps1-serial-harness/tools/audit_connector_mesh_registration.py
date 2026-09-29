#!/usr/bin/env python3
"""Measure disconnected low-Z connector mesh solids against footprint holes."""
import sys

path = sys.argv[1]
verts=[]; faces=[]; active=False; seen=False
for line in open(path, encoding="utf-8", errors="ignore"):
    if line.startswith("v "):
        verts.append(tuple(float(x) for x in line.split()[1:4]))
    elif line.startswith("g "):
        now=line.startswith("g Molex_48393")
        if now and not seen: active=True; seen=True
        elif active: break
    elif active and line.startswith("f "):
        faces.append([int(x.split("/")[0])-1 for x in line.split()[1:]])

parent={}
def find(x):
    parent.setdefault(x,x)
    while parent[x]!=x:
        parent[x]=parent[parent[x]]; x=parent[x]
    return x
def union(a,b):
    a,b=find(a),find(b)
    if a!=b: parent[b]=a
for f in faces:
    for i in range(1,len(f)): union(f[0],f[i])
groups={}
for x in parent: groups.setdefault(find(x),[]).append(x)
rows=[]
for ids in groups.values():
    vv=[verts[i] for i in ids]
    lo=[min(v[k] for v in vv) for k in range(3)]
    hi=[max(v[k] for v in vv) for k in range(3)]
    cen=[(lo[k]+hi[k])/2 for k in range(3)]
    # Scene axes: x=board X, y=height, z=board Y.
    if lo[1] < 0.00125:
        rows.append((lo[1],hi[1],cen[0],cen[2],hi[0]-lo[0],hi[2]-lo[2],len(ids)))
for r in sorted(rows): print(" ".join(f"{x:.6f}" if isinstance(x,float) else str(x) for x in r))
