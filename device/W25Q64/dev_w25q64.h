#ifndef __DEV_W25Q64_H
#define __DEV_W25Q64_H

#include "com_type.h"
#include "platform_mcu_resource.h"

u16  dev_w25q64_readID(void);
void w25q64_earse_chip(void);
void w25q64_earse_sector(u32 address);
void w25q64_write_sector(u32 address,u8* buff,u16 size);
void w25q64_read_sector(u32 address,u8* buff,u16 size);
#endif
