/*            哦弥陀佛 我佛慈悲  
                   _ooOoo_
                  o8888888o
                  88" . "88
                  (| -_- |)
                  O\  =  /O
               ____/`---'\____
             .'  \\|     |//  `.
            /  \\|||  :  |||//  \
           /  _||||| -:- |||||-  \
           |   | \\\  -  /// |   |
           | \_|  ''\---/''  |   |
           \  .-\__  `-`  ___/-. /
         ___`. .'  /--.--\  `. . __
      ."" '<  `.___\_<|>_/___.'  >'"".
     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
     \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
                   `=---='
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
            佛祖保佑       永无BUG^-^
*/
/**
 * @file drv_wdt.c
 * @brief 看门狗抽象层接口定义
 * @details 看门狗打开、关闭、喂狗、设置超时时间、设置超时中断回调函数等接口定义
 * @version 1.0
 * @date 2026-03-13
 */
#include "stm32f10x.h"                  // Device header
#include "platform_mcu_resource.h"
#include "drv_wdt.h"
#include "drv_sys.h"

#define LSI_Freq 32768

/**
 * @brief  看门狗初始化
 *  配置IWDG
 *  超时时间 = (重载值+1) * (预分频值) / LSI频率 
 *  timeout_ms   load         256      32.768KHz
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
void drv_wdt_init(u32 timeout_ms)
{
    // 使能 LSI 时钟
    RCC_LSICmd(ENABLE);
    // 等待 LSI 稳定（简单延时）
    for(u32 i=0; i<5000; i++) {}
    
    // 1. 使能写访问
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    
    // 2. 设置预分频器
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    
    // 3. 计算重载值
    u32 reload = (timeout_ms * LSI_Freq) / 256;
    // 确保重载值不超过 IWDG 范围（0xFFF）
    if (reload > 0xFFF) reload = 0xFFF;
    if (reload < 1) reload = 1;
    IWDG_SetReload(reload);
    
    // 4. 喂狗（重载计数器）
    IWDG_ReloadCounter();
    
    // 5. 启动看门狗
    IWDG_Enable();
}
/**
 * @brief 喂狗看门狗
 */
void drv_wdt_feed(void)
{
    IWDG_ReloadCounter();
}
