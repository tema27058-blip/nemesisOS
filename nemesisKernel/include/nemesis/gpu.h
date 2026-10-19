#ifndef NEMESIS_GPU_H
#define NEMESIS_GPU_H

#include <nemesis/types.h>

int fb_init(void);
void fb_flush(void);

void fb_clear(u32 color);
void fb_pixel(int x, int y, u32 color);
void fb_rect(int x, int y, int width, int height, u32 color);

int fb_width(void);
int fb_height(void);

#endif
