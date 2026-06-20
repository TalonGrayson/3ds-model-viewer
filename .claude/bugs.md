# Bugs & Code Smells — model-viewer

<!-- 
FORMAT: Reverse-chronological. Each entry:

## [YYYY-MM-DD] Bug or smell title
**Status:** OPEN | INVESTIGATING | FIXED | WONTFIX
**Tags:** optional, tags
**Severity:** LOW | MEDIUM | HIGH | CRITICAL
**Relates-to:** [[tasks#task-slug]], [[security#concern-slug]]

Description. What's wrong, how to reproduce if relevant, suspected cause.

---
-->

## [2026-06-20] Bottom screen render target always black (slug: bottom-screen-render-target-always-black)
**Status:** WONTFIX
**Tags:** rendering, bottom-screen, platform
**Severity:** LOW
**Relates-to:** [[decisions#cpu-direct-writes-for-bottom-screen]]

`C3D_RenderTargetCreate` + `C3D_RenderTargetSetOutput(target, GFX_BOTTOM, GFX_LEFT, ...)` consistently produces a black bottom screen. Attempted mitigations: `GSPGPU_InvalidateDataCache`, moving the blit call inside `C3D_FrameBegin/End`, different transfer flags. None worked. Suspected root cause: incompatibility between the 2×-oversampled top screen targets + `gfxSet3D(true)` and bottom screen display transfers in the same frame. `C2D_CreateScreenTarget` also returned NULL for the same reason. Workaround: direct CPU writes to `gfxGetFramebuffer` — see [[decisions#cpu-direct-writes-for-bottom-screen]]. Not worth further investigation since the workaround is clean and sufficient.

---
