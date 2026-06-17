#include <3ds.h>
#include "ui.h"

// Bottom screen: 320x240 landscape, BGR8 framebuffer stored column-major.
// Pixel at screen (x, y) with (0,0) top-left: index = (x * 240 + (239 - y)) * 3
#define UI_W 320
#define UI_H 240

// ── Layout constants (all derived, don't change independently) ───────────────
#define HEADER_H    16
#define CARD_MARGIN  4

#define CARD_X      CARD_MARGIN
#define CARD_Y      (HEADER_H + CARD_MARGIN)
#define CARD_W      (UI_W - CARD_MARGIN * 2)
#define CARD_H      (UI_H - HEADER_H - CARD_MARGIN * 2)

// "CAMERA" label sits 4px from card top; divider is 3px below the label
#define DIVIDER_Y   (CARD_Y + 4 + FONT_H + 3)

// Tab bar sits 1px below the divider
#define TAB_Y       (DIVIDER_Y + 1)
#define TAB_H       18
#define TAB_COUNT   3
#define TAB_W       (CARD_W / TAB_COUNT)   // 104px each

// "FRAME" reset button — sits in card body below tab bar
#define BTN_MARGIN   6
#define BTN_X        (CARD_X + BTN_MARGIN)
#define BTN_Y        (TAB_Y + TAB_H + 1 + BTN_MARGIN)
#define BTN_W        80
#define BTN_H        14

// ── Font ─────────────────────────────────────────────────────────────────────
// 5x7 bitmap font, one byte per row, bit 4 = leftmost pixel.
// Indices 0-25 = A-Z, index 26 = '.'
#define FONT_W   5
#define FONT_H   7
#define FONT_GAP 2

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

// ── Primitives ────────────────────────────────────────────────────────────────

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

// ── State ─────────────────────────────────────────────────────────────────────

static int s_activeTab = 0;   // 0=ORBIT 1=PAN 2=ZOOM

static const char* TAB_LABELS[TAB_COUNT] = { "ORBIT", "PAN", "ZOOM" };

// ── Lifecycle ─────────────────────────────────────────────────────────────────

void uiInit(void) {}
void uiExit(void) {}

// Called inside C3D_FrameBegin/End — hidScanInput has already run this frame.
UIEvent uiDraw(void)
{
    UIEvent events = UI_EVENT_NONE;

    // Only react on the first frame of a touch (not hold)
    if (!(hidKeysDown() & KEY_TOUCH)) return events;

    touchPosition touch;
    hidTouchRead(&touch);
    int px = (int)touch.px;
    int py = (int)touch.py;

    // Tab bar
    if (py >= TAB_Y && py < TAB_Y + TAB_H) {
        int tx = px - CARD_X;
        if (tx >= 0 && tx < CARD_W) {
            s_activeTab = tx / TAB_W;
            if (s_activeTab >= TAB_COUNT) s_activeTab = TAB_COUNT - 1;
        }
    }

    // "FRAME" reset button
    if (px >= BTN_X && px < BTN_X + BTN_W &&
        py >= BTN_Y && py < BTN_Y + BTN_H) {
        events |= UI_EVENT_RESET_VIEW;
    }

    return events;
}

// ── Frame ─────────────────────────────────────────────────────────────────────

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

    // ── Header bar: #7B5CF0 ─────────────────────────────────────────────────
    fillRect(fb, 0, 0, UI_W, HEADER_H, 0x7B, 0x5C, 0xF0);
    const char* brand = "MODL.VIEW";
    drawStr(fb, (UI_W - strPixelW(brand)) / 2, (HEADER_H - FONT_H) / 2,
            brand, 0xFF, 0xFF, 0xFF);

    // ── Card: #222226 ───────────────────────────────────────────────────────
    fillRect(fb, CARD_X, CARD_Y, CARD_W, CARD_H, 0x22, 0x22, 0x26);

    // "CAMERA" label — muted blue-grey #9898B8
    drawStr(fb, CARD_X + 4, CARD_Y + 4, "CAMERA", 0x98, 0x98, 0xB8);

    // Divider below label: #363640
    fillRect(fb, CARD_X, DIVIDER_Y, CARD_W, 1, 0x36, 0x36, 0x40);

    // ── Tab bar ─────────────────────────────────────────────────────────────
    for (int t = 0; t < TAB_COUNT; t++) {
        int tx = CARD_X + t * TAB_W;
        // Last tab gets any leftover pixels from integer division
        int tw = (t == TAB_COUNT - 1) ? (CARD_X + CARD_W) - tx : TAB_W;

        if (t == s_activeTab) {
            // Active: purple fill, white label
            fillRect(fb, tx, TAB_Y, tw, TAB_H, 0x7B, 0x5C, 0xF0);
            int lw = strPixelW(TAB_LABELS[t]);
            drawStr(fb, tx + (tw - lw) / 2, TAB_Y + (TAB_H - FONT_H) / 2,
                    TAB_LABELS[t], 0xFF, 0xFF, 0xFF);
        } else {
            // Inactive: card bg (already filled), muted label
            int lw = strPixelW(TAB_LABELS[t]);
            drawStr(fb, tx + (tw - lw) / 2, TAB_Y + (TAB_H - FONT_H) / 2,
                    TAB_LABELS[t], 0x98, 0x98, 0xB8);
        }

        // Separator between tabs
        if (t < TAB_COUNT - 1)
            fillRect(fb, tx + tw, TAB_Y, 1, TAB_H, 0x36, 0x36, 0x40);
    }

    // Bottom edge of tab bar
    fillRect(fb, CARD_X, TAB_Y + TAB_H, CARD_W, 1, 0x36, 0x36, 0x40);

    // ── "FRAME" reset button ─────────────────────────────────────────────────
    // Outlined button: dark fill, purple border, white label
    fillRect(fb, BTN_X, BTN_Y, BTN_W, BTN_H, 0x28, 0x28, 0x2E);
    // Border (1px)
    fillRect(fb, BTN_X,             BTN_Y,              BTN_W, 1,     0x7B, 0x5C, 0xF0);
    fillRect(fb, BTN_X,             BTN_Y + BTN_H - 1,  BTN_W, 1,     0x7B, 0x5C, 0xF0);
    fillRect(fb, BTN_X,             BTN_Y,              1,     BTN_H, 0x7B, 0x5C, 0xF0);
    fillRect(fb, BTN_X + BTN_W - 1, BTN_Y,              1,     BTN_H, 0x7B, 0x5C, 0xF0);
    // Label centered
    const char* btnLabel = "FRAME";
    int blw = strPixelW(btnLabel);
    drawStr(fb, BTN_X + (BTN_W - blw) / 2, BTN_Y + (BTN_H - FONT_H) / 2,
            btnLabel, 0xFF, 0xFF, 0xFF);

    gfxScreenSwapBuffers(GFX_BOTTOM, false);
}
