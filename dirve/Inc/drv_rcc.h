/**
 * @file drv_rcc.h
 * @brief 
 * @details 
 * 
 * @version 1.0
 * @date 2026-03-14
 */
#ifndef __DRV_RCC_H__
#define __DRV_RCC_H__
#include "com_type.h"
#include "platform_mcu_resource.h"

/**
 * @brief 系统sysclk、PCLK、hclk时钟设置
 * @remark 所有时钟默认HSI+PLL：72M
 */
void drv_rcc_sysclk_init(void);
/**
 * @brief 获取系统sysclk时钟频率
 * @return u32 系统sysclk时钟频率
 */
u32 drv_rcc_get_sysclk_freq(void);

#endif /* __DRV_RCC_H__ */
