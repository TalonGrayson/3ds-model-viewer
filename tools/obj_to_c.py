#!/usr/bin/env python3
"""
obj_to_c.py — Convert a Wavefront OBJ file to a C vertex array for citro3d.

Each output vertex contains:
  float position[3]   (x, y, z)
  float color[3]      (r, g, b) — tint; use 1 1 1 with textures (default)
  float normal[3]     (nx, ny, nz) — per-vertex normals, computed if absent
  float uv[2]         (u, v) — texture coordinates; (0, 0) if OBJ has no UVs

Multi-material OBJs are supported: faces are grouped by their resolved diffuse
texture (map_Kd from the MTL), and the output includes a model_groups[] array
for issuing one draw call per texture. Groups with the same texture are merged.

Usage:
  python3 obj_to_c.py model.obj [r g b] > model.h

  r g b are optional floats in [0,1] for the vertex colour tint (default: 1 1 1)

The output header defines:
  vertex              struct (position, color, normal, uv)
  model_group         struct (offset, count, texture romfs path)
  MODEL_GROUP_COUNT   number of draw groups
  model_vertices[]    all vertices, sorted by group
  model_groups[]      per-group draw info
  model_vertex_count  total vertex count
"""

import sys
import math
import os


def normalize(v):
    length = math.sqrt(sum(c * c for c in v))
    if length < 1e-8:
        return (0.0, 0.0, 1.0)
    return tuple(c / length for c in v)


def cross(a, b):
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    )


def sub(a, b):
    return (a[0] - b[0], a[1] - b[1], a[2] - b[2])


def face_normal(p0, p1, p2):
    return normalize(cross(sub(p1, p0), sub(p2, p0)))


def parse_mtl(mtl_path):
    """Returns {material_name: diffuse_texture_filename} from an MTL file."""
    materials = {}
    current = None
    try:
        with open(mtl_path) as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                parts = line.split()
                if parts[0] == 'newmtl':
                    current = parts[1]
                    materials[current] = None
                elif parts[0] == 'map_Kd' and current and len(parts) > 1:
                    materials[current] = parts[-1]  # last token is the filename
    except FileNotFoundError:
        print(f'warning: MTL file not found: {mtl_path}', file=sys.stderr)
    return materials


def parse_obj(path):
    """
    Returns (positions, uvs, normals, groups) where groups is an ordered list of
    (texture_filename_or_None, triangles) with faces merged by resolved texture.
    """
    positions = []
    uvs = []
    normals = []

    obj_dir = os.path.dirname(os.path.abspath(path))
    mtl_map = {}          # material_name -> texture filename (from MTL)
    current_texture = None
    pending = []          # triangles for the current texture group
    group_order = []      # texture names in encounter order (for stable output)
    by_texture = {}       # texture -> [triangles]

    def flush():
        nonlocal pending
        if not pending:
            return
        key = current_texture
        if key not in by_texture:
            by_texture[key] = []
            group_order.append(key)
        by_texture[key].extend(pending)
        pending = []

    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            parts = line.split()
            tag = parts[0]

            if tag == 'mtllib':
                for mtl_file in parts[1:]:
                    mtl_path = os.path.join(obj_dir, mtl_file)
                    mtl_map.update(parse_mtl(mtl_path))

            elif tag == 'v':
                positions.append(tuple(float(x) for x in parts[1:4]))

            elif tag == 'vt':
                uvs.append((float(parts[1]), float(parts[2])))

            elif tag == 'vn':
                normals.append(tuple(float(x) for x in parts[1:4]))

            elif tag == 'usemtl':
                mat_name = parts[1]
                new_tex = mtl_map.get(mat_name)  # None if no MTL or no map_Kd
                if new_tex != current_texture:
                    flush()
                    current_texture = new_tex

            elif tag == 'f':
                # Each token: v, v/vt, v/vt/vn, or v//vn (1-indexed)
                verts = []
                for tok in parts[1:]:
                    indices = tok.split('/')
                    pi = int(indices[0]) - 1
                    ti = int(indices[1]) - 1 if len(indices) > 1 and indices[1] else None
                    ni = int(indices[2]) - 1 if len(indices) > 2 and indices[2] else None
                    verts.append((pi, ti, ni))

                # Fan-triangulate polygon faces
                for i in range(1, len(verts) - 1):
                    pending.append((verts[0], verts[i], verts[i + 1]))

    flush()

    # Return as ordered list of (texture, triangles)
    groups = [(tex, by_texture[tex]) for tex in group_order]
    return positions, uvs, normals, groups


def centre_and_scale(positions):
    """Translate to origin and scale to fit in a unit cube."""
    if not positions:
        return positions
    mins = [min(p[i] for p in positions) for i in range(3)]
    maxs = [max(p[i] for p in positions) for i in range(3)]
    centre = [(mins[i] + maxs[i]) / 2.0 for i in range(3)]
    extent = max(maxs[i] - mins[i] for i in range(3))
    scale = 1.0 / extent if extent > 1e-8 else 1.0
    return [tuple((p[i] - centre[i]) * scale for i in range(3)) for p in positions]


def build_group_vertices(positions, uvs, normals, triangles, colour):
    """Build the flat vertex list for one material group."""
    out = []
    fallback_uv = (0.0, 0.0)
    fn_cache = {}

    for (p0i, t0i, n0i), (p1i, t1i, n1i), (p2i, t2i, n2i) in triangles:
        p0, p1, p2 = positions[p0i], positions[p1i], positions[p2i]

        key = (p0i, p1i, p2i)
        if key not in fn_cache:
            fn_cache[key] = face_normal(p0, p1, p2)
        fn = fn_cache[key]

        n0 = normalize(normals[n0i]) if (n0i is not None and n0i < len(normals)) else fn
        n1 = normalize(normals[n1i]) if (n1i is not None and n1i < len(normals)) else fn
        n2 = normalize(normals[n2i]) if (n2i is not None and n2i < len(normals)) else fn

        uv0 = (uvs[t0i][0], uvs[t0i][1]) if (t0i is not None and t0i < len(uvs)) else fallback_uv
        uv1 = (uvs[t1i][0], uvs[t1i][1]) if (t1i is not None and t1i < len(uvs)) else fallback_uv
        uv2 = (uvs[t2i][0], uvs[t2i][1]) if (t2i is not None and t2i < len(uvs)) else fallback_uv

        out.append((p0, colour, n0, uv0))
        out.append((p1, colour, n1, uv1))
        out.append((p2, colour, n2, uv2))
    return out


def tex_to_romfs_path(tex_filename):
    """Convert a texture filename (e.g. samusvaria_d.png) to its romfs path."""
    if tex_filename is None:
        return None
    base = os.path.splitext(os.path.basename(tex_filename))[0]
    return f'romfs:/gfx/{base}.t3x'


def emit_header(groups_vertices, group_textures, source_name):
    """
    groups_vertices: list of flat vertex lists, one per group
    group_textures:  list of romfs paths (or None), one per group
    """
    total = sum(len(g) for g in groups_vertices)
    n_groups = len(groups_vertices)

    lines = []
    lines.append(f'// Auto-generated by obj_to_c.py from {os.path.basename(source_name)}')
    lines.append(f'// {total} vertices ({total // 3} triangles), {n_groups} material group(s)')
    lines.append('#pragma once')
    lines.append('')
    lines.append('typedef struct {')
    lines.append('    float position[3];')
    lines.append('    float color[3];')
    lines.append('    float normal[3];')
    lines.append('    float uv[2];')
    lines.append('} vertex;')
    lines.append('')
    lines.append('typedef struct { int offset; int count; const char* texture; } model_group;')
    lines.append('')
    lines.append(f'#define MODEL_GROUP_COUNT {n_groups}')
    lines.append('')
    lines.append(f'static const vertex model_vertices[] = {{')

    offset = 0
    group_offsets = []
    for verts in groups_vertices:
        group_offsets.append(offset)
        for (px, py, pz), (cr, cg, cb), (nx, ny, nz), (u, v) in verts:
            lines.append(
                f'    {{ {{{px:.6f}f, {py:.6f}f, {pz:.6f}f}}, '
                f'{{{cr:.4f}f, {cg:.4f}f, {cb:.4f}f}}, '
                f'{{{nx:.6f}f, {ny:.6f}f, {nz:.6f}f}}, '
                f'{{{u:.6f}f, {v:.6f}f}} }},'
            )
        offset += len(verts)

    lines.append('};')
    lines.append('')
    lines.append(f'static const model_group model_groups[MODEL_GROUP_COUNT] = {{')
    for i, (tex, off, verts) in enumerate(zip(group_textures, group_offsets, groups_vertices)):
        tex_str = f'"{tex}"' if tex else 'NULL'
        lines.append(f'    {{ {off}, {len(verts)}, {tex_str} }},')
    lines.append('};')
    lines.append('')
    lines.append(f'static const int model_vertex_count = {total};')
    lines.append('')
    return '\n'.join(lines)


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    obj_path = sys.argv[1]
    colour = (1.0, 1.0, 1.0)
    if len(sys.argv) >= 5:
        colour = (float(sys.argv[2]), float(sys.argv[3]), float(sys.argv[4]))

    positions, uvs, normals, groups = parse_obj(obj_path)
    if not groups:
        print(f'error: no faces found in {obj_path}', file=sys.stderr)
        sys.exit(1)

    positions = centre_and_scale(positions)

    groups_vertices = []
    group_textures = []
    for tex_file, triangles in groups:
        verts = build_group_vertices(positions, uvs, normals, triangles, colour)
        groups_vertices.append(verts)
        group_textures.append(tex_to_romfs_path(tex_file))

    print(emit_header(groups_vertices, group_textures, obj_path), end='')

    total_tris = sum(len(g) // 3 for g in groups_vertices)
    print(f'// {total_tris} triangles, {len(groups)} group(s):', file=sys.stderr)
    for (tex_file, _), tex_path in zip(groups, group_textures):
        print(f'//   {tex_path or "(no texture)"} <- {tex_file}', file=sys.stderr)


if __name__ == '__main__':
    main()
