#include <nemesis/gpu.h>
#include <nemesis/print.h>
#include <nemesis/timer.h>

#define VIRTIO_MMIO_BASE 0x0a000000UL

#define REG(offset) (*(volatile u32 *)(VIRTIO_MMIO_BASE + (offset)))

#define VIRTIO_MMIO_MAGIC_VALUE 0x000
#define VIRTIO_MMIO_VERSION 0x004
#define VIRTIO_MMIO_DEVICE_ID 0x008
#define VIRTIO_MMIO_VENDOR_ID 0x00c
#define VIRTIO_MMIO_DEVICE_FEATURES 0x010
#define VIRTIO_MMIO_DEVICE_FEATURES_SEL 0x014
#define VIRTIO_MMIO_DRIVER_FEATURES 0x020
#define VIRTIO_MMIO_DRIVER_FEATURES_SEL 0x024
#define VIRTIO_MMIO_QUEUE_SEL 0x028
#define VIRTIO_MMIO_QUEUE_NUM_MAX 0x02c
#define VIRTIO_MMIO_QUEUE_NUM 0x030
#define VIRTIO_MMIO_QUEUE_READY 0x034
#define VIRTIO_MMIO_QUEUE_DESC_LOW 0x038
#define VIRTIO_MMIO_QUEUE_DESC_HIGH 0x03c
#define VIRTIO_MMIO_QUEUE_AVAIL_LOW 0x044
#define VIRTIO_MMIO_QUEUE_AVAIL_HIGH 0x04c
#define VIRTIO_MMIO_QUEUE_USED_LOW 0x050
#define VIRTIO_MMIO_QUEUE_USED_HIGH 0x058
#define VIRTIO_MMIO_QUEUE_NOTIFY 0x060
#define VIRTIO_MMIO_STATUS 0x07c
#define VIRTIO_MMIO_CONFIG 0x100

#define VIRTIO_STATUS_ACKNOWLEDGE 1UL
#define VIRTIO_STATUS_DRIVER 2UL
#define VIRTIO_STATUS_DRIVER_OK 4UL
#define VIRTIO_STATUS_FEATURES_OK 8UL
#define VIRTIO_STATUS_FAILED 0x80UL

#define VIRTIO_F_VERSION_1 32UL

#define VIRTIO_GPU_DEVICE_ID 16
#define CONTROL_QUEUE 0
#define QUEUE_SIZE 16

#define VIRTIO_GPU_CMD_GET_DISPLAY_INFO 0x0100
#define VIRTIO_GPU_CMD_RESOURCE_CREATE_2D 0x0101
#define VIRTIO_GPU_CMD_RESOURCE_UNREF 0x0102
#define VIRTIO_GPU_CMD_SET_SCANOUT 0x0103
#define VIRTIO_GPU_CMD_RESOURCE_FLUSH 0x0104
#define VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING 0x0109
#define VIRTIO_GPU_RESP_OK 0x1100

#define VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM 1

#define FB_WIDTH 1024
#define FB_HEIGHT 768

#define DESC_FLAG_NEXT 1UL
#define DESC_FLAG_WRITE 2UL

typedef struct {
    u32 type;
    u32 flags;
    u64 fence_id;
    u32 context_id;
    u32 padding;
} __attribute__((packed)) gpu_ctrl_hdr;

typedef struct {
    gpu_ctrl_hdr header;
    u32 format;
    u32 width;
    u32 height;
} __attribute__((packed)) gpu_create_2d;

typedef struct {
    u64 address;
    u32 length;
    u32 padding;
} __attribute__((packed)) gpu_mem_entry;

typedef struct {
    gpu_ctrl_hdr header;
    u32 resource_id;
    u32 entry_count;
    gpu_mem_entry entry;
} __attribute__((packed)) gpu_attach_backing;

typedef struct {
    gpu_ctrl_hdr header;
    u32 x;
    u32 y;
    u32 width;
    u32 height;
    u32 scanout_id;
    u32 resource_id;
} __attribute__((packed)) gpu_set_scanout;

typedef struct {
    gpu_ctrl_hdr header;
    u32 x;
    u32 y;
    u32 width;
    u32 height;
    u32 resource_id;
    u32 padding;
} __attribute__((packed)) gpu_resource_flush;

typedef struct {
    u64 address;
    u32 length;
    u16 flags;
    u16 next;
} __attribute__((packed)) virtq_desc;

typedef struct {
    u16 flags;
    u16 index;
    u16 ring[QUEUE_SIZE];
    u16 used_event;
} virtq_avail;

typedef struct {
    u32 id;
    u32 length;
} virtq_used_elem;

typedef struct {
    u16 flags;
    u16 index;
    virtq_used_elem ring[QUEUE_SIZE];
    u16 used_event;
} virtq_used;

static u8 queue_memory[8192] __attribute__((aligned(4096)));
static u8 request_buffer[128] __attribute__((aligned(16)));
static u8 response_buffer[512] __attribute__((aligned(16)));

static u32 last_used_index;
static int gpu_ready;

static u32 framebuffer[FB_WIDTH * FB_HEIGHT];

static virtq_desc *descriptors = (virtq_desc *)queue_memory;
static virtq_avail *available = (virtq_avail *)(queue_memory + 256);
static virtq_used *used = (virtq_used *)(queue_memory + 4096);

static u32 read32(u32 offset)
{
    return REG(offset);
}

static void write32(u32 offset, u32 value)
{
    REG(offset) = value;
}

static void memory_barrier(void)
{
    __asm__ volatile("dsb sy");
}

static int gpu_command(const void *request, u32 request_size,
                       void *response, u32 response_size)
{
    if (!gpu_ready) {
        return -1;
    }

    descriptors[0].address = (u64)request;
    descriptors[0].length = request_size;
    descriptors[0].flags = DESC_FLAG_NEXT;
    descriptors[0].next = 1;

    descriptors[1].address = (u64)response;
    descriptors[1].length = response_size;
    descriptors[1].flags = DESC_FLAG_WRITE;
    descriptors[1].next = 0;

    available->ring[available->index % QUEUE_SIZE] = 0;
    available->index++;

    memory_barrier();
    write32(VIRTIO_MMIO_QUEUE_NOTIFY, CONTROL_QUEUE);

    while (used->index == last_used_index) {
        __asm__ volatile("nop");
    }

    last_used_index++;
    memory_barrier();

    gpu_ctrl_hdr *header = (gpu_ctrl_hdr *)response;
    return header->type == VIRTIO_GPU_RESP_OK ? 0 : -2;
}

static int setup_queue(void)
{
    write32(VIRTIO_MMIO_QUEUE_SEL, CONTROL_QUEUE);

    u32 maximum = read32(VIRTIO_MMIO_QUEUE_NUM_MAX);
    if (maximum < QUEUE_SIZE) {
        return -1;
    }

    write32(VIRTIO_MMIO_QUEUE_NUM, QUEUE_SIZE);

    u64 descriptor_address = (u64)queue_memory;
    u64 available_address = (u64)(queue_memory + 256);
    u64 used_address = (u64)(queue_memory + 4096);

    write32(VIRTIO_MMIO_QUEUE_DESC_LOW, (u32)descriptor_address);
    write32(VIRTIO_MMIO_QUEUE_DESC_HIGH, (u32)(descriptor_address >> 32));
    write32(VIRTIO_MMIO_QUEUE_AVAIL_LOW, (u32)available_address);
    write32(VIRTIO_MMIO_QUEUE_AVAIL_HIGH, (u32)(available_address >> 32));
    write32(VIRTIO_MMIO_QUEUE_USED_LOW, (u32)used_address);
    write32(VIRTIO_MMIO_QUEUE_USED_HIGH, (u32)(used_address >> 32));

    write32(VIRTIO_MMIO_QUEUE_READY, 1);
    return 0;
}

int fb_init(void)
{
    gpu_ready = 0;

    if (read32(VIRTIO_MMIO_MAGIC_VALUE) != 0x74726976UL) {
        return -1;
    }

    if (read32(VIRTIO_MMIO_VERSION) != 2) {
        return -2;
    }

    if (read32(VIRTIO_MMIO_DEVICE_ID) != VIRTIO_GPU_DEVICE_ID) {
        return -3;
    }

    write32(VIRTIO_MMIO_STATUS, 0);
    write32(VIRTIO_MMIO_STATUS, VIRTIO_STATUS_ACKNOWLEDGE);
    write32(VIRTIO_MMIO_STATUS, VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER);

    write32(VIRTIO_MMIO_DEVICE_FEATURES_SEL, 1);
    u32 high_features = read32(VIRTIO_MMIO_DEVICE_FEATURES);

    if (!(high_features & 1UL)) {
        return -4;
    }

    write32(VIRTIO_MMIO_DRIVER_FEATURES_SEL, 1);
    write32(VIRTIO_MMIO_DRIVER_FEATURES, 1);
    write32(VIRTIO_MMIO_DRIVER_FEATURES_SEL, 0);
    write32(VIRTIO_MMIO_DRIVER_FEATURES, 0);

    write32(VIRTIO_MMIO_STATUS,
            VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER | VIRTIO_STATUS_FEATURES_OK);

    if (!(read32(VIRTIO_MMIO_STATUS) & VIRTIO_STATUS_FEATURES_OK)) {
        return -5;
    }

    if (setup_queue() != 0) {
        return -6;
    }

    write32(VIRTIO_MMIO_STATUS,
            VIRTIO_STATUS_ACKNOWLEDGE | VIRTIO_STATUS_DRIVER |
            VIRTIO_STATUS_FEATURES_OK | VIRTIO_STATUS_DRIVER_OK);

    gpu_ready = 1;

    gpu_ctrl_hdr display_request;
    display_request.type = VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
    display_request.flags = 0;
    display_request.fence_id = 0;
    display_request.context_id = 0;
    display_request.padding = 0;

    if (gpu_command(&display_request, sizeof(display_request),
                    response_buffer, sizeof(response_buffer)) != 0) {
        gpu_ready = 0;
        return -7;
    }

    gpu_create_2d create;
    create.header.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
    create.header.flags = 0;
    create.header.fence_id = 0;
    create.header.context_id = 0;
    create.header.padding = 0;
    create.format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;
    create.width = FB_WIDTH;
    create.height = FB_HEIGHT;

    if (gpu_command(&create, sizeof(create), response_buffer, sizeof(response_buffer)) != 0) {
        gpu_ready = 0;
        return -8;
    }

    gpu_attach_backing attach;
    attach.header.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
    attach.header.flags = 0;
    attach.header.fence_id = 0;
    attach.header.context_id = 0;
    attach.header.padding = 0;
    attach.resource_id = 1;
    attach.entry_count = 1;
    attach.entry.address = (u64)framebuffer;
    attach.entry.length = sizeof(framebuffer);
    attach.entry.padding = 0;

    if (gpu_command(&attach, sizeof(attach), response_buffer, sizeof(response_buffer)) != 0) {
        gpu_ready = 0;
        return -9;
    }

    gpu_set_scanout scanout;
    scanout.header.type = VIRTIO_GPU_CMD_SET_SCANOUT;
    scanout.header.flags = 0;
    scanout.header.fence_id = 0;
    scanout.header.context_id = 0;
    scanout.header.padding = 0;
    scanout.x = 0;
    scanout.y = 0;
    scanout.width = FB_WIDTH;
    scanout.height = FB_HEIGHT;
    scanout.scanout_id = 0;
    scanout.resource_id = 1;

    if (gpu_command(&scanout, sizeof(scanout), response_buffer, sizeof(response_buffer)) != 0) {
        gpu_ready = 0;
        return -10;
    }

    fb_clear(0xff101418);
    fb_flush();
    return 0;
}

void fb_flush(void)
{
    if (!gpu_ready) {
        return;
    }

    memory_barrier();

    gpu_resource_flush flush;
    flush.header.type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
    flush.header.flags = 0;
    flush.header.fence_id = 0;
    flush.header.context_id = 0;
    flush.header.padding = 0;
    flush.x = 0;
    flush.y = 0;
    flush.width = FB_WIDTH;
    flush.height = FB_HEIGHT;
    flush.resource_id = 1;
    flush.padding = 0;

    gpu_command(&flush, sizeof(flush), response_buffer, sizeof(response_buffer));
}

void fb_clear(u32 color)
{
    for (int y = 0; y < FB_HEIGHT; y++) {
        for (int x = 0; x < FB_WIDTH; x++) {
            framebuffer[y * FB_WIDTH + x] = color;
        }
    }
}

void fb_pixel(int x, int y, u32 color)
{
    if (x < 0 || y < 0 || x >= FB_WIDTH || y >= FB_HEIGHT) {
        return;
    }

    framebuffer[y * FB_WIDTH + x] = color;
}

void fb_rect(int x, int y, int width, int height, u32 color)
{
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            fb_pixel(x + dx, y + dy, color);
        }
    }
}

int fb_width(void)
{
    return FB_WIDTH;
}

int fb_height(void)
{
    return FB_HEIGHT;
}
