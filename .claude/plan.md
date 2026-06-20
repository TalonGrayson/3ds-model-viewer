# Plan — model-viewer

<!-- 
FORMAT: Reverse-chronological. Each entry:

## [YYYY-MM-DD] Milestone or phase title
**Status:** ACTIVE | COMPLETE | PAUSED | CANCELLED
**Relates-to:** [[tasks]], [[decisions#relevant-decision]]

What this phase covers and why.

---
-->

## [2026-06-20] Phase 4: SD card runtime model loader (slug: phase-4-sd-card-loader)
**Status:** PAUSED
**Relates-to:** [[ideas#phase-2-normal-mapping]]

Allow the user to pick a model from `sdmc:/models/` on the 3DS without rebuilding. Requires: (1) a file browser UI on the bottom screen, (2) a compact custom binary format (raw interleaved floats + small header — NOT OBJ text, which is too slow to parse on a 133MHz ARM11), (3) an offline `obj_to_bin.py` converter, (4) a runtime loader (`fread` into `linearAlloc` + VBO swap). The Python tools become an offline converter step rather than a build step. Deferred until rendering quality is satisfying.

---

## [2026-06-20] Phase 3: Normal mapping via C3D_LightEnv (slug: phase-3-normal-mapping)
**Status:** PAUSED
**Relates-to:** [[ideas#phase-2-normal-mapping]], [[architecture#vertex-layout]]

Replace per-vertex Phong with hardware fragment lighting via `C3D_LightEnv` + `C3D_Light`. Adds normal map support: tangent/bitangent attributes or `normalquat` output from vertex shader, normal map textures in romfs, full `C3D_LightEnv` setup. Would significantly improve surface detail on the Samus model. Deferred until touchscreen UI PR is merged.

---

## [2026-06-17] Phase 2: Touchscreen UI (slug: phase-2-touchscreen-ui)
**Status:** COMPLETE
**Relates-to:** [[decisions#cpu-direct-writes-for-bottom-screen]], [[decisions#uiresult-struct-decoupling]], [[decisions#sdf-over-wu-for-aa-circle]]

Bottom screen UI shell with full camera controls accessible via touch. Delivered iteratively with hardware tests between each step:
- Step 1: Dark bg (#1A1A1C) + purple header (#7B5CF0) + "MODL.VIEW" branding
- Step 2: CAMERA panel chrome (dark card, label, divider)
- Step 3: ORBIT/PAN/ZOOM tab bar with touch detection + active highlight
- Step 4: FRAME reset button wired to view reset logic (R button equivalent)
- Step 5: − and + zoom buttons wired to camZ
- Step 6: Touch orbit pad — drag inside AA circle rotates model; SDF circle stroke

Key discovery: C3D render targets for the bottom screen don't work alongside 2×-oversampled top screen + `gfxSet3D(true)`. Workaround: direct CPU writes to `gfxGetFramebuffer`. See [[decisions#cpu-direct-writes-for-bottom-screen]].

PR: https://github.com/TalonGrayson/3ds-model-viewer/pull/8 (open as of 2026-06-20)

---

## [2026-06-14] Phase 1: Per-vertex Phong + diffuse texture + OBJ pipeline (slug: phase-1-phong-texture)
**Status:** COMPLETE
**Relates-to:** [[architecture#obj-to-c-pipeline]], [[architecture#vertex-layout]], [[decisions#uv-v-flip-removed]], [[decisions#2x-oversample-ssaa]]

Core rendering pipeline and asset workflow:
- PICA200 vertex shader with per-vertex Phong lighting (ambient + diffuse + specular via material uniform)
- Diffuse texture via TexEnv `GPU_MODULATE` (texture × lighting)
- Multi-material OBJ pipeline: `obj_to_c.py` parses MTL, groups faces by `map_Kd`, emits `model_groups[]`
- PNG → `.t3x` via `tex3ds`, loaded at runtime from romfs via `Tex3DS_TextureImportStdio`
- Stereoscopic 3D: two 480×800 render targets (4×SSAA), `Mtx_PerspStereoTilt` per eye
- Circle pad + ZL/ZR + D-pad controls; screen-space arcball rotation; L toggles auto-rotate; R resets
- Test model: Samus Varia Suit (`samus/00000000.obj`) — 2,583 triangles, 2 texture groups

---

## [2026-06-10] Phase 0: Project bootstrap (slug: phase-0-bootstrap)
**Status:** COMPLETE

Forked from `textured_cube` example in `~/code/3ds/3ds-examples/graphics/gpu/textured_cube`. Replaced static cube geometry with OBJ-loaded model. Stripped C2D texture handling, added per-vertex color and Phong lighting shader. Confirmed basic circle pad rotation on hardware with orange cat model (~3,500 triangles at 5% decimation).

---
