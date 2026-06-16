#include <3ds.h>
#include "ui.h"

// Bottom screen: 320x240 landscape, BGR8 framebuffer stored column-major.
// Pixel at screen (x, y) with (0,0) top-left: index = (x * 240 + (239 - y)) * 3
#define UI_W 320
#define UI_H 240

// Layout
#define HEADER_H    16   // purple top bar
#define CARD_MARGIN  4   // gap from screen edge to card

// 5x7 bitmap font, one byte per row, bit 4 = leftmost pixel.
// Indices 0-25 = A-Z, index 26 = '.'
#define FONT_W   5
#define FONT_H   7
#define FONT_GAP 2   // horizontal gap between glyphs (px)

static const u8 FONT[27][FONT_H] = {
    { 0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 }, // A
    { 0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E }, // B
    { 0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E }, // C
    { 0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E }, // D
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F }, // E
    { 0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10 }, // F
    { 0x0E, 0x11, 0x10, 0x13, 0x11, 0x11, 0x0E }, // G
    { 0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11 }, // H
    { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F }, // I
    { 0x0F, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C }, // J
    { 0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11 }, // K
    { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F }, // L
    { 0x11, 0x1B, 0x15, 0x11, 0x11, 0x11, 0x11 }, // M
    { 0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11 }, // N
    { 0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E }, // O
    { 0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10 }, // P
    { 0x0E, 0x11, 0x11, 0x11, 0x15, 0x13, 0x0F }, // Q
    { 0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11 }, // R
    { 0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E }, // S
    { 0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 }, // T
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E }, // U
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04 }, // V
    { 0x11, 0x11, 0x11, 0x15, 0x15, 0x1B, 0x11 }, // W
    { 0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11 }, // X
    { 0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04 }, // Y
    { 0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F }, // Z
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00 }, // .
};

static inline int glyphOf(char ch)
{
    if (ch >= 'A' && ch <= 'Z') return ch - 'A';
    if (ch == '.') return 26;
    return -1;
}

// ── primitives ──────────────────────────────────────────────────────────────

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
        u8 bits = FONT[gi][row];
        for (int col = 0; col < FONT_W; col++)
            if ((bits >> (FONT_W - 1 - col)) & 1)
                setPixel(fb, sx + col, sy + row, r, g, b);
    }
}

static void drawStr(u8* fb, int sx, int sy, const char* str, u8 r, u8 g, u8 b)
{
    for (int i = 0; str[i]; i++) {
        int gi = glyphOf(str[i]);
        if (gi >= 0) drawGlyph(fb, sx + i * (FONT_W + FONT_GAP), sy, gi, r, g, b);
    }
}

static int strPixelW(const char* str)
{
    int n = 0;
    for (; str[n]; n++) {}
    return n > 0 ? n * FONT_W + (n - 1) * FONT_GAP : 0;
}

// ── lifecycle ────────────────────────────────────────────────────────────────

void uiInit(void) {}
void uiDraw(void) {}
void uiExit(void) {}

// ── frame ────────────────────────────────────────────────────────────────────

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

    // ── Header bar ──────────────────────────────────────────────────────────
    // Purple: #7B5CF0
    fillRect(fb, 0, 0, UI_W, HEADER_H, 0x7B, 0x5C, 0xF0);

    // "MODL.VIEW" centered in header, white
    const char* brand = "MODL.VIEW";
    int brandX = (UI_W - strPixelW(brand)) / 2;
    int brandY = (HEADER_H - FONT_H) / 2;
    drawStr(fb, brandX, brandY, brand, 0xFF, 0xFF, 0xFF);

    // ── Camera panel card ───────────────────────────────────────────────────
    int cardX = CARD_MARGIN;
    int cardY = HEADER_H + CARD_MARGIN;
    int cardW = UI_W - CARD_MARGIN * 2;
    int cardH = UI_H - HEADER_H - CARD_MARGIN * 2;

    // Card background: #222226
    fillRect(fb, cardX, cardY, cardW, cardH, 0x22, 0x22, 0x26);

    // "CAMERA" label — left-aligned, 4px from card edge, muted blue-grey #9898B8
    drawStr(fb, cardX + 4, cardY + 4, "CAMERA", 0x98, 0x98, 0xB8);

    // Divider below label: #363640
    fillRect(fb, cardX, cardY + 4 + FONT_H + 3, cardW, 1, 0x36, 0x36, 0x40);

    gfxScreenSwapBuffers(GFX_BOTTOM, false);
}
