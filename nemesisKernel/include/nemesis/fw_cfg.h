#ifndef NEMESIS_FW_CFG_H
#define NEMESIS_FW_CFG_H

#include <nemesis/types.h>

void fw_cfg_init(void);
int fw_cfg_find_file(const char *name, u16 *selector);
void fw_cfg_read(u16 selector, void *buffer, u32 length);
void fw_cfg_write(u16 selector, void *buffer, u32 length);

#endif
