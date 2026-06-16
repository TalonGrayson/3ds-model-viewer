#pragma once

void uiInit(void);
void uiDraw(void);     // call inside C3D_FrameBegin / C3D_FrameEnd
void uiPresent(void);  // call after C3D_FrameEnd — blits to the gfx framebuffer
void uiExit(void);
