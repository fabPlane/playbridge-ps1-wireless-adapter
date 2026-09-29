#!/usr/bin/env python3
"""Convert a legacy TraceParts/Three.js preview package into OBJ or VRML."""

from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path


def pad4(count: int) -> int:
    return (4 - count % 4) % 4


def u32(data: bytes, offset: int, count: int) -> tuple[int, ...]:
    return struct.unpack_from(f"<{count}I", data, offset)


def parse_mesh(path: Path) -> tuple[list[tuple[float, float, float]], list[tuple[int, int, int]]]:
    data = path.read_bytes()
    if data[:12] != b"Three.js 003":
        raise ValueError(f"Unsupported preview mesh: {path}")

    header_bytes, vertex_bytes, normal_bytes, uv_bytes = struct.unpack_from("<4B", data, 12)
    # TraceParts' "Three.js 003" variant includes one extra index-width byte
    # before the material width. Counts therefore start on the next 4-byte
    # boundary at byte 24 (the stock legacy loader expected byte 20).
    vertex_index_bytes, normal_index_bytes, uv_index_bytes, _color_index_bytes, material_index_bytes = struct.unpack_from("<5B", data, 16)
    counts = struct.unpack_from("<11I", data, 24)
    (
        nvertices,
        nnormals,
        nuvs,
        ntri_flat,
        ntri_smooth,
        ntri_flat_uv,
        ntri_smooth_uv,
        nquad_flat,
        nquad_smooth,
        nquad_flat_uv,
        nquad_smooth_uv,
    ) = counts

    if (vertex_bytes, normal_bytes, uv_bytes) != (4, 1, 4):
        raise ValueError(f"Unexpected coordinate widths in {path}")
    if (vertex_index_bytes, normal_index_bytes, uv_index_bytes) != (4, 4, 4) or material_index_bytes not in (2, 4):
        raise ValueError(f"Unexpected index widths in {path}")

    offset = header_bytes
    coords = struct.unpack_from(f"<{nvertices * 3}f", data, offset)
    vertices = [tuple(coords[i : i + 3]) for i in range(0, len(coords), 3)]
    offset += nvertices * 3 * vertex_bytes
    offset += nnormals * 3 * normal_bytes + pad4(nnormals * 3)
    offset += nuvs * 2 * uv_bytes

    tri_size = vertex_index_bytes * 3 + material_index_bytes
    quad_size = vertex_index_bytes * 4 + material_index_bytes
    lengths = {
        "tri_flat": ntri_flat * tri_size,
        "tri_smooth": ntri_smooth * (tri_size + normal_index_bytes * 3),
        "tri_flat_uv": ntri_flat_uv * (tri_size + uv_index_bytes * 3),
        "tri_smooth_uv": ntri_smooth_uv * (tri_size + normal_index_bytes * 3 + uv_index_bytes * 3),
        "quad_flat": nquad_flat * quad_size,
        "quad_smooth": nquad_smooth * (quad_size + normal_index_bytes * 4),
        "quad_flat_uv": nquad_flat_uv * (quad_size + uv_index_bytes * 4),
    }

    starts: dict[str, int] = {"tri_flat": offset}
    starts["tri_smooth"] = starts["tri_flat"] + lengths["tri_flat"] + pad4(ntri_flat * 2)
    starts["tri_flat_uv"] = starts["tri_smooth"] + lengths["tri_smooth"] + pad4(ntri_smooth * 2)
    starts["tri_smooth_uv"] = starts["tri_flat_uv"] + lengths["tri_flat_uv"] + pad4(ntri_flat_uv * 2)
    starts["quad_flat"] = starts["tri_smooth_uv"] + lengths["tri_smooth_uv"] + pad4(ntri_smooth_uv * 2)
    starts["quad_smooth"] = starts["quad_flat"] + lengths["quad_flat"] + pad4(nquad_flat * 2)
    starts["quad_flat_uv"] = starts["quad_smooth"] + lengths["quad_smooth"] + pad4(nquad_smooth * 2)
    starts["quad_smooth_uv"] = starts["quad_flat_uv"] + lengths["quad_flat_uv"] + pad4(nquad_flat_uv * 2)

    faces: list[tuple[int, int, int]] = []
    for name, count in (
        ("tri_flat", ntri_flat),
        ("tri_smooth", ntri_smooth),
        ("tri_flat_uv", ntri_flat_uv),
        ("tri_smooth_uv", ntri_smooth_uv),
    ):
        if count:
            indices = u32(data, starts[name], count * 3)
            faces.extend(tuple(indices[i : i + 3]) for i in range(0, len(indices), 3))

    for name, count in (
        ("quad_flat", nquad_flat),
        ("quad_smooth", nquad_smooth),
        ("quad_flat_uv", nquad_flat_uv),
        ("quad_smooth_uv", nquad_smooth_uv),
    ):
        if count:
            indices = u32(data, starts[name], count * 4)
            for i in range(0, len(indices), 4):
                a, b, c, d = indices[i : i + 4]
                faces.extend(((a, b, d), (b, c, d)))

    return vertices, faces


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("preview_dir", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()

    manifest = next(args.preview_dir.glob("*.js"))
    metadata = json.loads(manifest.read_text())
    all_vertices: list[tuple[float, float, float]] = []
    all_faces: list[tuple[int, int, int]] = []
    obj_lines = [
        "# Molex 48393-0003 visual mesh",
        "# Source: TraceParts preview package, generated from exact catalog part",
        "o Molex_48393-0003",
    ]
    for buffer_name in metadata["buffers"]:
        path = args.preview_dir / buffer_name
        vertices, faces = parse_mesh(path)
        vertex_offset = len(all_vertices)
        all_vertices.extend(vertices)
        shifted_faces = [(a + vertex_offset, b + vertex_offset, c + vertex_offset) for a, b, c in faces]
        all_faces.extend(shifted_faces)
        obj_lines.append(f"g {path.stem}")
        obj_lines.extend(f"v {x:.7g} {y:.7g} {z:.7g}" for x, y, z in vertices)
        obj_lines.extend(f"f {a + 1} {b + 1} {c + 1}" for a, b, c in shifted_faces)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    if args.output.suffix.lower() == ".wrl":
        lines = [
            "#VRML V2.0 utf8",
            "# Molex 48393-0003 exact-part visual mesh from TraceParts preview",
            "Shape {",
            "  appearance Appearance { material Material { diffuseColor 0.65 0.67 0.70 specularColor 0.8 0.8 0.8 shininess 0.35 } }",
            "  geometry IndexedFaceSet {",
            "    solid TRUE",
            "    creaseAngle 0.45",
            "    coord Coordinate { point [",
        ]
        lines.extend(f"      {x:.7g} {y:.7g} {z:.7g}," for x, y, z in all_vertices)
        lines.extend(("    ] }", "    coordIndex ["))
        lines.extend(f"      {a}, {b}, {c}, -1," for a, b, c in all_faces)
        lines.extend(("    ]", "  }", "}"))
    else:
        lines = obj_lines
    args.output.write_text("\n".join(lines) + "\n")


if __name__ == "__main__":
    main()
