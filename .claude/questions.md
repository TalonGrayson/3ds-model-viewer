# Open Questions — model-viewer

<!-- 
FORMAT: Reverse-chronological. Each entry:

## [YYYY-MM-DD] Question (slug: kebab-case)
**Status:** OPEN | ANSWERED | DROPPED
**Tags:** optional, tags
**Relates-to:** [[decisions]], [[architecture]]

The question. What would the answer unblock? Any partial info gathered so far.

---
-->

## [2026-06-20] Why does C3D_RenderTargetSetOutput black out the bottom screen? (slug: render-target-bottom-screen-black)
**Status:** OPEN
**Tags:** rendering, bottom-screen, platform
**Relates-to:** [[bugs#bottom-screen-render-target-always-black]], [[decisions#cpu-direct-writes-for-bottom-screen]]

`C3D_RenderTargetCreate` + `C3D_RenderTargetSetOutput(target, GFX_BOTTOM, GFX_LEFT, flags)` always produces a black bottom screen regardless of timing or cache invalidation (`GSPGPU_InvalidateDataCache`). `C2D_CreateScreenTarget` also returns NULL. The issue appears to be specific to running alongside 2×-oversampled top screen targets + `gfxSet3D(true)` in the same frame — the combination may exhaust some GX display transfer resource or conflict with the parallax barrier setup. Answering this would let us use GPU-accelerated rendering (C2D, citro2d sprites) on the bottom screen instead of pure CPU software rendering. Partial leads: check libctru source for `C3D_RenderTargetSetOutput` interaction with `gfxSet3D`; check if creating the bottom target *before* the top targets changes behavior.

---
