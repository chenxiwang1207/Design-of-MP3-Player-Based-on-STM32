/**
 * @file platform_partition_config.h
 * @brief 代码分区配置
 * @details 
 * 
 * @version 1.0
 * @date 2025-08-14
 * @copyright Shenzhen Breo Technology Co., Ltd.
 * 
 */

#ifndef _PLATFORM_PARTITION_CONFIG_H_
#define _PLATFORM_PARTITION_CONFIG_H_

//芯片页大小
#define FLASH_PAGE_INT_SIZE                 (0x800)        //内部FLASH页大小/* 2KB */
#define FLASH_PAGE_EXT_SIZE                 (0x800)        //外挂FLASH页大小/* 2KB */

//mcu 内部flash起始地址，大小
#define MCU_FLASH_INT_BASE                  (0x08000000)  /* flash起始地址 */
#define MCU_FLASH_INT_SIZE                  (0x40000)     /* 256 KBytes */
#define MCU_FLASH__INT_END_ADDRESS          (MCU_FLASH_INT_BASE + BR_FLASH_SIZE)

//mcu 内部SRAM 48K,起始地址，结束地址
#define MCU_SRAM_INT_START_ADDR             (0x20000000)
#define MCU_SRAM_INT_END_ADDR               (MCU_SRAM_INT_START_ADDR + 0xC000)//48k

//boot 区 0K
#define BOOT_FIRMWARE_ADDR                  MCU_FLASH_INT_BASE
#define BOOT_FIRMWARE_SIZE                  (0 * 1024)

//core 区 250K
#define CORE_FIRMWARE_ADDR                  (BOOT_FIRMWARE_ADDR + BOOT_FIRMWARE_SIZE)
#define CORE_FIRMWARE_SIZE                  (254 * 1024)

//临时存放区 2K
#define UPDATE_TEMP_ADDR                    (CORE_FIRMWARE_ADDR + CORE_FIRMWARE_SIZE)
#define UPDATE_TEMP_SIZE                    (2 * 1024)

//更新标准位 0K
#define UPDATA_FLAG_ADDR                    (UPDATE_TEMP_ADDR + UPDATE_TEMP_SIZE)
#define UPDATE_FLAG_SIZE                    (0 * 1024)  //0kB

//mcu芯片硬件引脚初始化电平配置
#define BREO_DCD_ADDR                       (UPDATA_FLAG_ADDR+UPDATE_FLAG_SIZE)
#define BREO_DCD_SIZE                       (0 * 1024)  //0kB


#endif
