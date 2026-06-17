#pragma once

// Bitmask returned by uiDraw — main.c checks these each frame.
typedef enum {
    UI_EVENT_NONE        = 0,
    UI_EVENT_RESET_VIEW  = 1 << 0,  // "FRAME" button tapped
    UI_EVENT_ZOOM_IN     = 1 << 1,  // "+" button held
    UI_EVENT_ZOOM_OUT    = 1 << 2,  // "-" button held
} UIEvent;

void     uiInit(void);
UIEvent  uiDraw(void);    // call inside C3D_FrameBegin / C3D_FrameEnd
void     uiPresent(void); // call after C3D_FrameEnd — blits to the gfx framebuffer
void     uiExit(void);
