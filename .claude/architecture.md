# Architecture — model-viewer

<!-- 
FORMAT: Reverse-chronological. Each entry describes a structural element, layer, or key pattern.

## [YYYY-MM-DD] Component or pattern name
**Status:** CURRENT | DEPRECATED | PLANNED
**Tags:** optional, tags
**Relates-to:** [[decisions#decision-slug]]

Description of the component, why it's structured this way, and any invariants to preserve.

---
-->

## [2026-06-20] Dev environment and build/deploy workflow (slug: dev-environment)
**Status:** CURRENT
**Tags:** toolchain, workflow, deployment

- **Machine:** MacBook (macOS), IDE: Cursor
- **Toolchain:** devkitPro — devkitARM, libctru, citro3d, picasso, tex3ds. Installed via `dkp-pacman -S 3ds-dev`. Location: `/opt/devkitpro`
- **Required env vars** (in `~/.zshrc`):
  ```bash
  export DEVKITPRO=/opt/devkitpro
  export DEVKITARM="${DEVKITPRO}/devkitARM"
  export PATH="${DEVKITPRO}/tools/bin:${PATH}"
  ```
- **Build:** `make` → produces `model-viewer.3dsx` (also `.elf`, `.smdh`)
- **Deploy:** `3dslink -a 192.168.88.28 ./model-viewer.3dsx` — pushes over WiFi. The 3DS must have Homebrew Launcher open with network receiver active (press Y in hbmenu). Auto-discovery is unreliable — always pass `-a <IP>` explicitly.
- **Target hardware:** New 3DS LL (Japanese unit, region-switched to English, CFW installed)
- **Reference examples repo:** `~/code/3ds/3ds-examples` — useful starting points: `graphics/gpu/textured_cube` (project base), `graphics/gpu/toon_shading`, `graphics/gpu/fragment_light`, `graphics/gpu/stereoscopic_2d`

---

## [2026-06-20] Blender decimation pipeline (slug: blender-decimation)
**Status:** CURRENT
**Tags:** build, assets, tools

High-poly OBJ files need decimation before the OBJ-to-C step. Target: ~3,000–5,000 triangles for comfortable 3DS budget.

```bash
/Applications/Blender.app/Contents/MacOS/Blender --background \
  --python tools/decimate_obj.py -- input.obj output.obj 0.05
```

`0.05` = keep 5% of original faces. Decimated output feeds into `obj_to_c.py`. Current test model: `samus/00000000.obj` (Samus Varia Suit, 2,583 triangles, 2 texture groups — already decimated and confirmed working on hardware).

---

## [2026-06-20] SDF circle stroke (slug: sdf-circle-stroke)
**Status:** CURRENT
**Tags:** rendering, ui, bottom-screen
**Relates-to:** [[decisions#sdf-over-wu-for-aa-circle]]

`drawCircleAA(fb, cx, cy, r, halfW, cr, cg, cb)` in `source/ui.c`. Iterates the bounding annulus AABB (clamped to screen bounds). For each pixel: compute `dsq = dx²+dy²`, skip if outside `[r−margin, r+margin]²` (fast reject), then `dist = |sqrtf(dsq) − r|`, `alpha = clamp(halfW + 0.5 − dist, 0, 1)`. Blends foreground color against the bg color (#221A22 approx). Each pixel written exactly once — no octant symmetry, no duplicate-write artifacts. Orbit pad uses `halfW=1.0` (2px wide stroke).

---

## [2026-06-17] UIResult struct — ui.c / main.c interface (slug: uiresult-struct)
**Status:** CURRENT
**Tags:** ui, architecture
**Relates-to:** [[decisions#uiresult-struct-decoupling]]

`uiDraw()` returns `UIResult { UIEvent events; float orbitDX; float orbitDY; }` by value each frame. `UIEvent` is a bitmask: `UI_EVENT_RESET_VIEW` (FRAME button), `UI_EVENT_ZOOM_IN`, `UI_EVENT_ZOOM_OUT`. `orbitDX`/`orbitDY` are radians-per-frame deltas from touch drag on the orbit pad. `main.c` inspects the struct after the frame and applies it to `modelRot` / `camZ`. No shared globals between `ui.c` and `main.c`.

---

## [2026-06-17] Bottom screen framebuffer (slug: bottom-screen-framebuffer)
**Status:** CURRENT
**Tags:** rendering, bottom-screen, platform
**Relates-to:** [[decisions#cpu-direct-writes-for-bottom-screen]]

Bottom screen: 320×240 landscape, BGR8 pixel format, **column-major** storage. Pixel formula: `index = (x * 240 + (239 - y)) * 3`. `(0,0)` = top-left in logical coordinates. Access via `gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &w, &h)`; present via `gfxScreenSwapBuffers(GFX_BOTTOM, false)`. Called as `uiPresent()` **after** `C3D_FrameEnd()` — CPU writes don't need GPU sync. `uiDraw()` renders into an internal `u8 s_fb[320*240*3]` scratch buffer; `uiPresent()` copies it to the gfx framebuffer.

---

## [2026-06-17] Touch orbit pad (slug: touch-orbit-pad)
**Status:** CURRENT
**Tags:** ui, input, rotation
**Relates-to:** [[architecture#uiresult-struct]], [[architecture#screen-space-arcball-rotation]]

Circular drag region centered at `(ORBIT_CX=160, ORBIT_CY=156)` with `r=60px` on the bottom screen. On touch-start inside the circle, records `(s_orbitPX, s_orbitPY)`. Each frame while held: `orbitDX = (cur.px − prev.px) * ORBIT_SCALE`, `orbitDY = (cur.py − prev.py) * ORBIT_SCALE` (Y NOT negated — inverted drag feels natural on hardware). Returns deltas in `UIResult`; `main.c` applies them via `Mtx_RotateY` / `Mtx_RotateX` pre-multiplied onto `modelRot`. Circle stroke goes purple while dragging.

---

## [2026-06-16] Screen-space arcball rotation (slug: screen-space-arcball-rotation)
**Status:** CURRENT
**Tags:** rendering, rotation, input

`modelRot` is a `C3D_Mtx` accumulated incrementally each frame. Circle pad and touch orbit deltas build a `delta` matrix via `Mtx_RotateY` / `Mtx_RotateX` / `Mtx_RotateZ` (each with `bRightSide=true` so rotations are in the current frame), then `Mtx_Multiply(&modelRot, &delta, &modelRot)` **pre-multiplies** delta onto the accumulated rotation. Pre-multiply means each new rotation is applied in screen space, so axes never drift or couple regardless of the current orientation — this is the arcball invariant.

---

## [2026-06-16] Top screen stereoscopic rendering (slug: top-screen-rendering)
**Status:** CURRENT
**Tags:** rendering, top-screen, stereo
**Relates-to:** [[decisions#2x-oversample-ssaa]]

Two `C3D_RenderTarget`s at 480×800 (left + right eye), downscaled to 240×400 via GX transfer scaling (4×SSAA). `gfxSet3D(true)` enables the parallax barrier. When the 3D slider `osGet3DSliderState() > 0`, uses `Mtx_PerspStereoTilt` with `±iod` offset and `SCREEN_DEPTH=2.0f` convergence plane; otherwise mono via `Mtx_PerspTilt` into left eye only. `sceneRender(C3D_Mtx* proj)` takes the projection as a parameter so both eyes share the same draw call.

---

## [2026-06-16] OBJ-to-C pipeline and model_groups (slug: obj-to-c-pipeline)
**Status:** CURRENT
**Tags:** build, assets, textures
**Relates-to:** [[decisions#uv-v-flip-removed]]

`tools/obj_to_c.py <model.obj>` parses OBJ + MTL, groups faces by `map_Kd` texture, and emits `source/model.h` containing: `vertex model_vertices[]` (flat array), `model_group_t model_groups[]` (offset + count + texture path per group), `MODEL_GROUP_COUNT`. Textures referenced as `romfs:/gfx/<name>.t3x`; PNG → t3x conversion done via `tex3ds` at build time. One `C3D_DrawArrays` call per group, `C3D_TexBind(0, &modelTextures[g])` switched between them. Regenerate header: `python3 tools/obj_to_c.py samus/00000000.obj > source/model.h`.

---

## [2026-06-16] Vertex layout (slug: vertex-layout)
**Status:** CURRENT
**Tags:** rendering, gpu
**Relates-to:** [[architecture#obj-to-c-pipeline]]

`struct vertex { float pos[3]; float col[3]; float nrm[3]; float uv[2]; }` — 11 floats, 44 bytes. GPU attribute binding: `v0=pos`, `v1=col (tint)`, `v2=nrm`, `v3=uv`. BufInfo permutation `0x3210`. TexEnv stage 0: `GPU_TEXTURE0 × GPU_PRIMARY_COLOR` (`GPU_MODULATE`) — diffuse texture modulated by Phong lighting output from vertex shader. Per-vertex Phong computed in vertex shader via `uLoc_lightVec`, `uLoc_lightHalfVec`, `uLoc_lightClr`, `uLoc_material`.

---
