/**
 * @file drv_sys.h
 * @brief 系统抽象层接口定义
 * @details 系统抽象层接口定义，主要包含系统初始化、关机、开机、休眠、重启、复位等接口
 * 
 * @version 1.0
 * @date 2026-03-14
 */
#ifndef _DRV_SYS_H_
#define _DRV_SYS_H_

#include "com_type.h"
#include "platform_mcu_resource.h"

#define DRV_SYS_TICK_TIME           1000   //1ms
/**
* @brief 唤醒原因
*/
typedef enum
{
    WAKEUP_MCU_PIN,      ///< MCU指定引脚唤醒
    WAKEUP_KEY_PIN,      ///< 按键引脚唤醒
    WAKEUP_GPIO_PIN,     ///< GPIO引脚唤醒
    WAKEUP_USB_INPUT,    ///< USB插入唤醒
    WAKEUP_MAX
}wakeup_reson_e;

/**
 * @brief 系统滴答初始化,无操作系统情况下调用
 * @param[in] tick_time_us 设置tick间隔时间us   
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_systick_init(u32 tick_time_us);

/**
 * @brief 读取滴答时间
 * @param[out] tick 滴答值（精度必须转换成ms）
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_get_systick(u32 *tick);

/**
 * @brief 读取当前系统ms值
 * @return 返回获取的ms值
 */
u32 drv_get_timems(void);

/**
 * @brief 查询时间是否超时
 * @param[in] time 原时间，可用 drv_get_systick获取
 * @param[in] delay 间隔时间
 * @return 返回执行结果
 * @retval TRUE    超时
 * @retval FALSE   未超时
 */
s32 drv_querry_time(u32 time, u32 delay);

/**
 * @brief 延时
 * @param[in] us 时间 
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_delay_us(u32 us);

/**
 * @brief 延时
 * @param[in] ms 时间
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_delay_ms(u32 ms);

/**
 * @brief 睡眠，主要用于有RTOS情况下，不占用CPU
 * @param[in] ms 时间   
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_sleep_ms(u32 ms);

/**
 * @brief 系统初始化
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_sys_init(void);

/**
 * @brief 系统关机
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0	       失败原因
 */
s32 drv_sys_poweroff(void);

/**
 * @brief 系统开机上电
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_sys_poweron(void);

/**
 * @brief 系统休眠
 * @return s32 唤醒原因
 * @ref wakeup_reson_e
 */
s32 drv_sys_power_sleep(void);

/**
 * @brief 系统重启
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_sys_reboot(void);

/**
* @brief 系统复位
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_sys_reset(void);

/**
 * @brief 暂停编号>=0的中断，主动挂起kernel调度\n
 * 暂停编号>=0的中断, systick，hardfault等系统异常中断编号为负数仍保持开启;
 */
void drv_suspend_irq(void);

/**
 * @brief 恢复编号>=0的中断，恢复kernel调度
 */
void drv_resume_irq(void);

/**
 * @brief jump to 某个地址,主要是boot中调转到应用
 * @param[in] jumpaddr 跳转地址
 */
void drv_jump_to_app(u32 jumpaddr);
 
#endif
