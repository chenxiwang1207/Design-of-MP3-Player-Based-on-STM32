#ifndef __DRV_WDG_H
#define __DRV_WDG_H

#include "com_type.h"

/**
 * @brief  看门狗初始化
 *  配置IWDG
 *  超时时间 = (重载值+1) * (预分频值) / LSI频率 
 *  timeout_ms   load         256      32.768KHz
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
void drv_wdt_init(u32 timeout_ms);
/**
 * @brief 喂狗看门狗
 */
void drv_wdt_feed(void);

#endif
