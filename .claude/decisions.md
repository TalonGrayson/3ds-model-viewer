# Decisions — model-viewer

<!-- 
FORMAT: Reverse-chronological. Each entry:

## [YYYY-MM-DD] Decision title (slug: kebab-case-of-title)
**Status:** DECIDED | REVISIT | SUPERSEDED
**Tags:** optional, tags
**Relates-to:** [[architecture]], [[global:decisions#cross-project-pattern]]

What was decided and why. Include the alternatives considered and the reason this option won.

---
-->

## [2026-06-20] SDF over Wu's algorithm for AA circle stroke (slug: sdf-over-wu-for-aa-circle)
**Status:** DECIDED
**Tags:** rendering, ui, bottom-screen
**Relates-to:** [[architecture#sdf-circle-stroke]]

Wu's line/circle algorithm fills pixels using 8-fold octant symmetry: it iterates `x` from 0 to `r` and writes each pixel and its 7 symmetric counterparts. Pixels near 45° octant boundaries are written by two consecutive iterations with different alpha values — last write wins, producing inconsistent brightness and clearly visible patches (observed on hardware). Switched to an SDF approach: iterate the bounding annulus AABB, compute `dist = |sqrt(dx²+dy²) − r|`, blend `alpha = halfW + 0.5 − dist`. Each pixel is written exactly once, no symmetry, no conflicts. Result is smooth and clean.

---

## [2026-06-20] CPU direct writes for bottom screen display (slug: cpu-direct-writes-for-bottom-screen)
**Status:** DECIDED
**Tags:** rendering, bottom-screen, platform
**Relates-to:** [[bugs#bottom-screen-render-target-always-black]], [[architecture#bottom-screen-framebuffer]]

Three approaches tried for bottom screen display:
1. `C3D_RenderTargetCreate` + `C3D_RenderTargetSetOutput` — always produced black output; incompatible with our 2×-oversampled top screen + `gfxSet3D(true)` setup. Root cause unresolved.
2. `C2D_CreateScreenTarget` — returned NULL; same root cause.
3. Direct CPU writes to `gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &w, &h)` + `gfxScreenSwapBuffers(GFX_BOTTOM, false)` — confirmed working.

Chose option 3. The bottom screen doesn't need GPU acceleration for a simple 2D UI, so pure software rendering is fine.

---

## [2026-06-17] UIResult struct for ui.c / main.c decoupling (slug: uiresult-struct-decoupling)
**Status:** DECIDED
**Tags:** architecture, ui
**Relates-to:** [[architecture#uiresult-struct]]

`uiDraw()` needed to communicate both discrete events (FRAME button tap, zoom button held) and continuous float deltas (orbit drag DX/DY) back to `main.c` each frame. Alternatives: global state in `ui.c`, separate query functions per event, callback functions. Chose a `UIResult` struct returned by value: a `UIEvent` bitmask + `orbitDX`/`orbitDY` floats. Clean, zero-overhead, no shared mutable state between translation units.

---

## [2026-06-16] UV V-flip removed (slug: uv-v-flip-removed)
**Status:** DECIDED
**Tags:** rendering, textures, obj
**Relates-to:** [[architecture#obj-to-c-pipeline]]

Initially added a V-flip (`uv.v = 1.0 - uv.v`) in `obj_to_c.py` based on the assumption that OBJ and PICA200 have opposite UV Y conventions. Verified on hardware that no flip is needed — OBJ and PICA200 share the same UV origin (bottom-left). Removing the flip produced correct texture orientation.

---

## [2026-06-14] 2× oversampled render targets for SSAA (slug: 2x-oversample-ssaa)
**Status:** DECIDED
**Tags:** rendering, top-screen, quality
**Relates-to:** [[architecture#top-screen-rendering]]

Top screen render targets are created at 480×800 (2× the display's 240×400) and downscaled by the GX transfer hardware via `GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_XY)`. This gives effectively 4×SSAA (2× in each axis) on both eyes with no CPU cost. The side effect is that `C3D_RenderTargetSetOutput` for a bottom screen target doesn't work alongside this setup — see [[decisions#cpu-direct-writes-for-bottom-screen]].

---
