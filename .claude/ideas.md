# Ideas — model-viewer

<!-- 
FORMAT: Reverse-chronological. Each entry:

## [YYYY-MM-DD] Idea title
**Status:** OPEN | PROMOTED | DROPPED
**Tags:** optional, tags
**Relates-to:** [[plan]], [[tasks#task-slug]]

Description. The idea and its potential value. Why deferred for now.

---
-->

## [2026-06-20] Phase 2: Normal mapping via C3D_LightEnv (slug: phase-2-normal-mapping)
**Status:** OPEN
**Tags:** rendering, lighting, pbr
**Relates-to:** [[architecture#vertex-layout]]

Replace per-vertex Phong lighting with the PICA200's hardware `C3D_LightEnv` + fragment lighting pipeline, adding normal map support. Requires: (1) tangent/bitangent attributes or `normalquat` output from vertex shader, (2) normal map textures in romfs, (3) `C3D_LightEnv` + `C3D_Light` setup instead of manual uniform-based Phong. Would significantly improve surface detail on the Samus model. Deferred until the UI/interaction layer is stable.

---

## [2026-06-20] PAN tab functionality (slug: pan-tab-functionality)
**Status:** OPEN
**Tags:** ui, input, camera
**Relates-to:** [[architecture#touch-orbit-pad]]

The ORBIT/PAN/ZOOM tab bar is in place but PAN and ZOOM tabs currently have no distinct touch behaviour — the orbit pad always orbits regardless of active tab. Could wire the touch drag area to camX/camY when PAN is active, and to camZ when ZOOM is active. Low priority; D-pad already covers pan and zoom.

---
