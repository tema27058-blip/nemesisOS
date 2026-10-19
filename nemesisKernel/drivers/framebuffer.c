#include <nemesis/framebuffer.h>
#include <nemesis/ramfb.h>
#include <nemesis/types.h>

static u32 *framebuffer;
static u32 display_width;
static u32 display_height;
static u32 display_stride;

typedef struct {
    char character;
    const u8 glyph[8];
} font_glyph_t;

static const font_glyph_t font[] = {
    {' ', {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {'.', {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}},
    {'0', {0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x00}},
    {'3', {0x7e, 0xc3, 0x03, 0x1e, 0x03, 0xc3, 0x7e, 0x00}},
    {'L', {0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x7e, 0x00}},
    {'O', {0x3c, 0x66, 0xc3, 0xc3, 0xc3, 0x66, 0x3c, 0x00}},
    {'S', {0x3c, 0x66, 0x60, 0x38, 0x0c, 0x66, 0x38, 0x00}},
    {'a', {0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x7c, 0x00}},
    {'d', {0x0c, 0x0c, 0x7c, 0xcc, 0xcc, 0xcc, 0x7c, 0x00}},
    {'e', {0x00, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00}},
    {'g', {0x00, 0x00, 0x7c, 0xcc, 0xcc, 0x7c, 0x0c, 0x78}},
    {'i', {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00}},
    {'m', {0x00, 0x00, 0xec, 0xfe, 0xd6, 0xd6, 0xcc, 0x00}},
    {'n', {0x00, 0x00, 0xdc, 0xcc, 0xcc, 0xcc, 0xcc, 0x00}},
    {'o', {0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0x78, 0x00}},
    {'s', {0x00, 0x00, 0x78, 0xc0, 0x70, 0x0c, 0xf8, 0x00}},
    {'v', {0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00}},
};

void framebuffer_init(void)
{
    display_width = 800;
    display_height = 600;
    display_stride = display_width * sizeof(u32);

    ramfb_init(display_width, display_height);

    /* RAMFB maps the allocated buffer as the visible framebuffer. */
    framebuffer = (u32 *)0x40000000UL + 0x1000000UL;
}

void framebuffer_pixel(u32 x, u32 y, u32 color)
{
    if (x >= display_width || y >= display_height) {
        return;
    }

    framebuffer[y * (display_stride / sizeof(u32)) + x] = color;
}

void framebuffer_clear(u32 color)
{
    for (u32 y = 0; y < display_height; y++) {
        for (u32 x = 0; x < display_width; x++) {
            framebuffer_pixel(x, y, color);
        }
    }
}

void framebuffer_rect(u32 x, u32 y, u32 width, u32 height, u32 color)
{
    for (u32 row = 0; row < height; row++) {
        for (u32 column = 0; column < width; column++) {
            framebuffer_pixel(x + column, y + row, color);
        }
    }
}

static const u8 *find_glyph(char character)
{
    for (u32 index = 0; index < sizeof(font) / sizeof(font[0]); index++) {
        if (font[index].character == character) {
            return font[index].glyph;
        }
    }

    return font[0].glyph;
}

void framebuffer_text(u32 x, u32 y, const char *text, u32 color, u32 scale)
{
    u32 cursor_x = x;

    while (*text) {
        const u8 *glyph = find_glyph(*text);

        for (u32 row = 0; row < 8; row++) {
            for (u32 column = 0; column < 8; column++) {
                if (glyph[row] & (0x80 >> column)) {
                    framebuffer_rect(cursor_x + column * scale, y + row * scale,
                                     scale, scale, color);
                }
            }
        }

        cursor_x += 8 * scale;
        text++;
    }
}

void framebuffer_logo(u32 x, u32 y, u32 color, u32 scale)
{
    framebuffer_text(x, y, "nemesisOS", color, scale);
}

void framebuffer_loading_bar(u32 progress)
{
    u32 bar_width = 400;
    u32 bar_height = 10;
    u32 x = (display_width - bar_width) / 2;
    u32 y = display_height / 2 + 60;

    framebuffer_rect(x, y, bar_width, bar_height, 0x202028);

    if (progress > 100) {
        progress = 100;
    }

    framebuffer_rect(x, y, bar_width * progress / 100, bar_height, 0x7c5cff);
}
