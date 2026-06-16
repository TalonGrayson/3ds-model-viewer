#!/usr/bin/env python3
"""
bump_to_normal.py — Convert a greyscale bump/height map to an RGB normal map.

The output is a PNG in tangent-space normal map format (R=X, G=Y, B=Z),
suitable for feeding into tex3ds for use on the 3DS.

Usage:
  python3 tools/bump_to_normal.py input_bump.jpg output_normal.png [strength]

  strength: how pronounced the normals are (default: 4.0 — higher = deeper bumps)
"""

import sys
from PIL import Image, ImageFilter

def bump_to_normal(src_path, dst_path, strength=4.0):
    img = Image.open(src_path).convert('L')  # greyscale
    w, h = img.size

    # Sobel kernels via a pair of linear filters
    # We compute per-pixel gradient with a 3x3 Sobel kernel
    pixels = list(img.get_flattened_data() if hasattr(img, 'get_flattened_data') else img.getdata())

    def px(x, y):
        x = max(0, min(w - 1, x))
        y = max(0, min(h - 1, y))
        return pixels[y * w + x] / 255.0

    out = Image.new('RGB', (w, h))
    out_pixels = []

    for y in range(h):
        for x in range(w):
            # Sobel in X: right - left (weighted centre row)
            dX = (px(x+1, y-1) + 2*px(x+1, y) + px(x+1, y+1)) \
               - (px(x-1, y-1) + 2*px(x-1, y) + px(x-1, y+1))
            # Sobel in Y: bottom - top
            dY = (px(x-1, y+1) + 2*px(x, y+1) + px(x+1, y+1)) \
               - (px(x-1, y-1) + 2*px(x, y-1) + px(x+1, y-1))

            dX *= strength
            dY *= strength

            # Normal = normalize(-dX, -dY, 1)
            length = (dX*dX + dY*dY + 1.0) ** 0.5
            nx = -dX / length
            ny = -dY / length
            nz =  1.0 / length

            # Encode to [0,255]: n * 0.5 + 0.5
            r = int((nx * 0.5 + 0.5) * 255)
            g = int((ny * 0.5 + 0.5) * 255)
            b = int((nz * 0.5 + 0.5) * 255)
            out_pixels.append((r, g, b))

    out.putdata(out_pixels)
    out.save(dst_path)
    print(f'[bump_to_normal] {w}x{h} → {dst_path}')

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    strength = float(sys.argv[3]) if len(sys.argv) > 3 else 4.0
    bump_to_normal(sys.argv[1], sys.argv[2], strength)
