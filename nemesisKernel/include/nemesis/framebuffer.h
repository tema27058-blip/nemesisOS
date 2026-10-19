#ifndef NEMESIS_FRAMEBUFFER_H
#define NEMESIS_FRAMEBUFFER_H

#include <nemesis/types.h>

void framebuffer_init(void);
void framebuffer_clear(u32 color);
void framebuffer_pixel(u32 x, u32 y, u32 color);
void framebuffer_rect(u32 x, u32 y, u32 width, u32 height, u32 color);
void framebuffer_text(u32 x, u32 y, const char *text, u32 color, u32 scale);
void framebuffer_logo(u32 x, u32 y, u32 color, u32 scale);
void framebuffer_loading_bar(u32 progress);

#endif
