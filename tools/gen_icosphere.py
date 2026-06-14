#!/usr/bin/env python3
"""
gen_icosphere.py — Generate a low-poly icosphere OBJ for testing.

Usage:
  python3 gen_icosphere.py [subdivisions] > icosphere.obj

  subdivisions: 0 = 20 faces, 1 = 80 faces, 2 = 320 faces (default: 1)

A subdivided icosphere looks rounder than a UV sphere at the same face count
and has no poles, making it a good generic test model.
"""

import sys
import math


PHI = (1.0 + math.sqrt(5.0)) / 2.0


def normalize(v):
    l = math.sqrt(v[0]**2 + v[1]**2 + v[2]**2)
    return (v[0]/l, v[1]/l, v[2]/l)


def midpoint(a, b):
    return normalize(((a[0]+b[0])/2, (a[1]+b[1])/2, (a[2]+b[2])/2))


def build_icosphere(subdivisions):
    # Base icosahedron vertices (normalized)
    verts = [
        normalize((-1,  PHI, 0)), normalize(( 1,  PHI, 0)),
        normalize((-1, -PHI, 0)), normalize(( 1, -PHI, 0)),
        normalize((0, -1,  PHI)), normalize((0,  1,  PHI)),
        normalize((0, -1, -PHI)), normalize((0,  1, -PHI)),
        normalize(( PHI, 0, -1)), normalize(( PHI, 0,  1)),
        normalize((-PHI, 0, -1)), normalize((-PHI, 0,  1)),
    ]

    faces = [
        (0,11,5),(0,5,1),(0,1,7),(0,7,10),(0,10,11),
        (1,5,9),(5,11,4),(11,10,2),(10,7,6),(7,1,8),
        (3,9,4),(3,4,2),(3,2,6),(3,6,8),(3,8,9),
        (4,9,5),(2,4,11),(6,2,10),(8,6,7),(9,8,1),
    ]

    for _ in range(subdivisions):
        new_faces = []
        edge_cache = {}

        def get_mid(a, b):
            key = (min(a, b), max(a, b))
            if key not in edge_cache:
                edge_cache[key] = len(verts)
                verts.append(midpoint(verts[a], verts[b]))
            return edge_cache[key]

        for (a, b, c) in faces:
            ab = get_mid(a, b)
            bc = get_mid(b, c)
            ca = get_mid(c, a)
            new_faces += [(a, ab, ca), (b, bc, ab), (c, ca, bc), (ab, bc, ca)]
        faces = new_faces

    return verts, faces


def main():
    subdivisions = int(sys.argv[1]) if len(sys.argv) > 1 else 1

    verts, faces = build_icosphere(subdivisions)

    lines = [f'# icosphere subdivisions={subdivisions}, {len(faces)} faces']
    for v in verts:
        lines.append(f'v {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}')
    # Normals equal positions on a unit sphere
    for v in verts:
        lines.append(f'vn {v[0]:.6f} {v[1]:.6f} {v[2]:.6f}')
    for (a, b, c) in faces:
        # OBJ is 1-indexed; include normal indices (same as position indices for sphere)
        lines.append(f'f {a+1}//{a+1} {b+1}//{b+1} {c+1}//{c+1}')

    print('\n'.join(lines))
    print(f'# {len(verts)} vertices, {len(faces)} faces', file=sys.stderr)


if __name__ == '__main__':
    main()
