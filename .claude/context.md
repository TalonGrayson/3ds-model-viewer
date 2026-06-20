# Project: model-viewer
**Stack:** C / devkitARM / libctru / citro3d / tex3ds — 3DS homebrew, Makefile build
**Last updated:** 2026-06-20
**Repo:** git@github.com:TalonGrayson/3ds-model-viewer.git

## Current Focus
PR #8 (feature/touchscreen-ui → main) open for review. Bottom screen UI complete: dark shell, MODL.VIEW header, CAMERA panel, ORBIT/PAN/ZOOM tabs, FRAME + zoom buttons, touch orbit pad with AA circle.

## Open Tasks (0)
<!-- - [ ] Task title → [[tasks#task-slug]] -->

## Recent Decisions
- SDF over Wu's algorithm for AA circle stroke [2026-06-20] → [[decisions#sdf-over-wu-for-aa-circle]]
- CPU direct writes for bottom screen display [2026-06-20] → [[decisions#cpu-direct-writes-for-bottom-screen]]
- UIResult struct for ui.c / main.c decoupling [2026-06-17] → [[decisions#uiresult-struct-decoupling]]
- UV V-flip removed [2026-06-16] → [[decisions#uv-v-flip-removed]]
- 2× oversampled render targets for SSAA [2026-06-14] → [[decisions#2x-oversample-ssaa]]

## Active Bugs
<!-- (none) -->

## Open Questions
<!-- - Question → [[questions#question-slug]] -->

## Security Flags
<!-- (none) -->

## Knowledge Files
[[plan]] | [[tasks]] | [[decisions]] | [[architecture]] | [[bugs]] | [[security]] | [[ideas]] | [[questions]]

## Related Projects
<!-- - [[other-project:context]] — description of relationship -->
