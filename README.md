# 3DS Model Viewer

A homebrew Nintendo 3DS application that loads a 3D model and lets you rotate it with the circle pad. Built with [devkitPro](https://devkitpro.org/), [libctru](https://github.com/devkitPro/libctru), and [citro3d](https://github.com/devkitPro/citro3d).

## Features

- Renders a 3D model on the top screen with per-vertex colour and Phong lighting
- Circle pad rotates the model on X and Y axes
- START exits to the Homebrew Launcher

## Requirements

- devkitPro with devkitARM, libctru, citro3d, and picasso installed:
  ```bash
  dkp-pacman -S 3ds-dev
  ```
- Environment variables set in your shell profile:
  ```bash
  export DEVKITPRO=/opt/devkitpro
  export DEVKITARM="${DEVKITPRO}/devkitARM"
  export PATH="${DEVKITPRO}/tools/bin:${PATH}"
  ```
- Python 3 (for the asset pipeline scripts in `tools/`)
- A 3DS with CFW and the Homebrew Launcher (deployment via `3dslink` over WiFi)

## Building

```bash
make
```

Produces `model-viewer.3dsx`.

## Deploying

With the Homebrew Launcher open on your 3DS and the network receiver active (press **Y** in hbmenu):

```bash
3dslink -a <3DS_IP_ADDRESS> ./model-viewer.3dsx
```

## Swapping the model

The model is baked into the binary at build time as a C vertex array. To replace it:

1. **If the source OBJ is high-poly**, decimate it first with Blender:
   ```bash
   /Applications/Blender.app/Contents/MacOS/Blender --background \
     --python tools/decimate_obj.py -- input.obj decimated.obj 0.05
   ```
   The third argument is the decimate ratio (0–1). `0.05` = 5% of original faces,
   which typically brings a high-res scan down to a 3DS-friendly ~3–5k triangles.

2. **Convert the OBJ to a C header:**
   ```bash
   python3 tools/obj_to_c.py decimated.obj R G B > source/model.h
   ```
   `R G B` are optional floats in `[0, 1]` for the model's base colour (default: `0.8 0.8 0.8`).
   The script auto-centres and scales the model to fit in a unit cube.

3. **Rebuild and deploy:**
   ```bash
   make && 3dslink -a <3DS_IP_ADDRESS> ./model-viewer.3dsx
   ```

### Generating a test model

A low-poly icosphere is included as a quick test target:

```bash
python3 tools/gen_icosphere.py 1 > /tmp/icosphere.obj
python3 tools/obj_to_c.py /tmp/icosphere.obj 0.6 0.8 1.0 > source/model.h
make
```

`gen_icosphere.py` accepts a subdivision level (0 = 20 faces, 1 = 80 faces, 2 = 320 faces).

## Project structure

```
source/
  main.c          — application logic, render loop, circle pad input
  vshader.v.pica  — PICA200 vertex shader (lit per-vertex colour)
  model.h         — generated C vertex array (not checked in — regenerate with tools/)
gfx/              — placeholder graphics directory (unused)
obj/              — source OBJ files
tools/
  obj_to_c.py     — converts a Wavefront OBJ to a C vertex array header
  decimate_obj.py — Blender headless script to decimate a high-poly OBJ
  gen_icosphere.py — generates a subdivided icosphere OBJ for testing
Makefile          — standard devkitPro 3DS Makefile
```

## Technical notes

- The 3DS GPU is the **PICA200** — no OpenGL or GLSL. Vertex shaders are written in PICA200 assembly (`.v.pica`) and compiled by the `picasso` tool included with devkitPro.
- The top screen framebuffer is **240×400** (rotated), so the projection matrix uses `Mtx_PerspTilt` with a matching aspect ratio.
- Models are baked into the binary as `const vertex[]` arrays — no runtime file loading. The eventual goal is an SD card model picker with a runtime loader.

## Status

See [HANDOFF.md](HANDOFF.md) for the full project context and planned next steps.
