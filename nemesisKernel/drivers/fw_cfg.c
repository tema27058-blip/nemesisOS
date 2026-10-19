#include <nemesis/fw_cfg.h>
#include <nemesis/types.h>

#define FW_CFG_BASE 0x0a000000UL
#define FW_CFG_DATA_OFFSET 0x00
#define FW_CFG_SELECTOR_OFFSET 0x08
#define FW_CFG_DMA_OFFSET 0x10

#define FW_CFG_DMA_CTL_ERROR 0x01
#define FW_CFG_DMA_CTL_READ 0x02
#define FW_CFG_DMA_CTL_SKIP 0x04
#define FW_CFG_DMA_CTL_SELECT 0x08
#define FW_CFG_DMA_CTL_WRITE 0x10

#define FW_CFG_FILE_DIR 0x19
#define FW_CFG_MAX_FILES 64

typedef struct {
    u32 control;
    u32 length;
    u64 address;
    u32 selector;
} fw_cfg_dma_t;

typedef struct {
    u32 size;
    u16 selector;
    u16 reserved;
    char name[56];
} fw_cfg_file_t;

static volatile u8 *const fw_cfg_base = (volatile u8 *)FW_CFG_BASE;

void fw_cfg_init(void)
{
}

static void fw_cfg_dma_transfer(u16 selector, void *buffer, u32 length, u32 operation)
{
    static fw_cfg_dma_t operation_block;

    operation_block.control = operation | ((u32)selector << 16);
    operation_block.length = length;
    operation_block.address = (u64)buffer;
    operation_block.selector = 0;

    *(volatile u64 *)(fw_cfg_base + FW_CFG_DMA_OFFSET) = (u64)&operation_block;

    while (operation_block.control & (FW_CFG_DMA_CTL_SELECT | FW_CFG_DMA_CTL_READ |
                                      FW_CFG_DMA_CTL_WRITE | FW_CFG_DMA_CTL_SKIP)) {
        __asm__ volatile("nop");
    }
}

void fw_cfg_read(u16 selector, void *buffer, u32 length)
{
    fw_cfg_dma_transfer(selector, buffer, length,
                        FW_CFG_DMA_CTL_SELECT | FW_CFG_DMA_CTL_READ);
}

void fw_cfg_write(u16 selector, void *buffer, u32 length)
{
    fw_cfg_dma_transfer(selector, buffer, length,
                        FW_CFG_DMA_CTL_SELECT | FW_CFG_DMA_CTL_WRITE);
}

int fw_cfg_find_file(const char *name, u16 *selector)
{
    static fw_cfg_file_t files[FW_CFG_MAX_FILES];
    u32 file_count = 0;
    u32 index;

    fw_cfg_read(FW_CFG_FILE_DIR, &file_count, sizeof(file_count));

    if (file_count > FW_CFG_MAX_FILES) {
        file_count = FW_CFG_MAX_FILES;
    }

    fw_cfg_read(FW_CFG_FILE_DIR, files, sizeof(u32) + file_count * sizeof(fw_cfg_file_t));

    for (index = 0; index < file_count; index++) {
        const fw_cfg_file_t *file = (const fw_cfg_file_t *)((u8 *)files + sizeof(u32) + index * sizeof(fw_cfg_file_t));

        int matches = 1;
        for (u32 character = 0; character < 56; character++) {
            if (name[character] == '\0') {
                break;
            }
            if (file->name[character] != name[character]) {
                matches = 0;
                break;
            }
        }

        if (matches && file->name[0] != '\0') {
            *selector = file->selector;
            return 0;
        }
    }

    return -1;
}
