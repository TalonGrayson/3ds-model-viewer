#include <3ds.h>
#include <string.h>
#include "ui.h"

// Bottom screen: 320x240 landscape, BGR8 framebuffer stored column-major.
// Pixel at screen (x, y) with (0,0) top-left: index = (x * 240 + (239 - y)) * 3
#define UI_W 320
#define UI_H 240

static void fillRect(u8* fb, int sx, int sy, int w, int h, u8 r, u8 g, u8 b)
{
    for (int y = sy; y < sy + h; y++) {
        for (int x = sx; x < sx + w; x++) {
            int idx = (x * 240 + (239 - y)) * 3;
            fb[idx + 0] = b;
            fb[idx + 1] = g;
            fb[idx + 2] = r;
        }
    }
}

void uiInit(void)  {}
void uiDraw(void)  {}
void uiExit(void)  {}

void uiPresent(void)
{
    u16 fbW, fbH;
    u8* fb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &fbW, &fbH);
    if (!fb) return;

    // Background: #1A1A1C
    int total = UI_W * UI_H * 3;
    for (int i = 0; i < total; i += 3) {
        fb[i + 0] = 0x1C; // B
        fb[i + 1] = 0x1A; // G
        fb[i + 2] = 0x1A; // R
    }

    // Purple accent bar across the top: #7B5CF0 → BGR = (0xF0, 0x5C, 0x7B)
    fillRect(fb, 0, 0, UI_W, 16, 0x7B, 0x5C, 0xF0);

    gfxScreenSwapBuffers(GFX_BOTTOM, false);
}
