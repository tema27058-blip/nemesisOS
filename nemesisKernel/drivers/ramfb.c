#include <nemesis/fw_cfg.h>
#include <nemesis/mm.h>
#include <nemesis/ramfb.h>
#include <nemesis/types.h>

#define RAMFB_FOURCC_XRGB8888 0x34325258UL

typedef struct {
    u64 address;
    u32 fourcc;
    u32 width;
    u32 height;
    u32 stride;
} ramfb_config_t;

int ramfb_init(u32 width, u32 height)
{
    u16 selector;
    u32 *framebuffer;
    ramfb_config_t config;

    if (fw_cfg_find_file("etc/ramfb", &selector) != 0) {
        return -1;
    }

    framebuffer = (u32 *)pmm_alloc((usize)width * height * sizeof(u32));

    config.address = (u64)framebuffer;
    config.fourcc = RAMFB_FOURCC_XRGB8888;
    config.width = width;
    config.height = height;
    config.stride = width * sizeof(u32);

    fw_cfg_write(selector, &config, sizeof(config));

    return 0;
}
