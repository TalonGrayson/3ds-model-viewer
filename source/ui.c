#include <3ds.h>
#include <string.h>
#include "ui.h"

// Bottom screen: 320x240 landscape, BGR8 framebuffer stored column-major.
// Pixel at screen (x, y) with (0,0) top-left: index = (x * 240 + (239 - y)) * 3
#define UI_W 320
#define UI_H 240

// Header bar
#define HEADER_H 16

// 5x7 bitmap font — one byte per row, bit 4 = leftmost pixel.
// Glyphs for "MODL.VIEW" in order.
#define FONT_W     5
#define FONT_H     7
#define FONT_SCALE 1
#define FONT_GAP   2   // px between glyphs (at scaled size)

static const char BRAND[] = "MODL.VIEW";

static const u8 GLYPHS[][FONT_H] = {
    { 0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11 }, // M
    { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E }, // O
    { 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E }, // D
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F }, // L
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00 }, // .
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 }, // V
    { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F }, // I
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F }, // E
    { 0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x0A }, // W
};

static inline void setPixel(u8* fb, int x, int y, u8 r, u8 g, u8 b)
{
    int idx = (x * 240 + (239 - y)) * 3;
    fb[idx + 0] = b;
    fb[idx + 1] = g;
    fb[idx + 2] = r;
}

static void fillRect(u8* fb, int sx, int sy, int w, int h, u8 r, u8 g, u8 b)
{
    for (int y = sy; y < sy + h; y++)
        for (int x = sx; x < sx + w; x++)
            setPixel(fb, x, y, r, g, b);
}

static void drawGlyph(u8* fb, int sx, int sy, int gi, u8 r, u8 g, u8 b)
{
    for (int row = 0; row < FONT_H; row++) {
        u8 bits = GLYPHS[gi][row];
        for (int col = 0; col < FONT_W; col++) {
            if (!((bits >> (FONT_W - 1 - col)) & 1)) continue;
            for (int dy = 0; dy < FONT_SCALE; dy++)
                for (int dx = 0; dx < FONT_SCALE; dx++)
                    setPixel(fb,
                        sx + col * FONT_SCALE + dx,
                        sy + row * FONT_SCALE + dy,
                        r, g, b);
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
        fb[i + 0] = 0x1C;
        fb[i + 1] = 0x1A;
        fb[i + 2] = 0x1A;
    }

    // Purple header bar: #7B5CF0
    fillRect(fb, 0, 0, UI_W, HEADER_H, 0x7B, 0x5C, 0xF0);

    // "MODL.VIEW" centered in header, white text
    int nchars  = sizeof(BRAND) - 1;
    int textW   = nchars * FONT_W * FONT_SCALE + (nchars - 1) * FONT_GAP;
    int textX   = (UI_W - textW) / 2;
    int textY   = (HEADER_H - FONT_H * FONT_SCALE) / 2;
    for (int i = 0; i < nchars; i++)
        drawGlyph(fb, textX + i * (FONT_W * FONT_SCALE + FONT_GAP), textY, i,
                  0xFF, 0xFF, 0xFF);

    gfxScreenSwapBuffers(GFX_BOTTOM, false);
}
