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
#include "stm32f10x.h"                  // Device header
#include "drv_sys.h"
#include "drv_rcc.h"
#include "platform_mcu_resource.h"

#define DRV_SYS_DEBUG   0

#if DRV_SYS_DEBUG
#define drv_sys_debug(fmt, arg...) breo_printf("[sys]<%s><%d>" fmt, __func__, __LINE__, ##arg)
#define drv_sys_err(fmt, arg...)   breo_printf("[sys]<%s><%d>" fmt, __func__, __LINE__, ##arg)
#else
#define drv_sys_debug(fmt, arg...)
#define drv_sys_err(fmt, arg...)
#endif
extern u32 iqr_get_tick(void);
u32 tick_timeus_set = 0;
/**
 * @brief     ms硬延时，loop实现
 * @param[in] int ms  毫秒数值
 * @return    无
 * @remark    注意事项
 */
void inner_delayms_const(u32 ms)
{
    volatile u32 k = 0;
    //one loop need 7 machine cycles
    volatile u32 counter_ms = SystemCoreClock/1000/7;
    while(ms --)
    {
        while(k < counter_ms)k ++;
        k = 0;
    }
}
/**
 * @brief     us硬延时，loop实现
 * @param[in] int us  微秒数值
 * @return    无
 * @remark    注意事项
 */
void inner_delayus_const(s32 us)
{
    volatile u32 k = 0;
    volatile u32 counter_us = (SystemCoreClock/1000000/7);
    while(us --)
    {
        while(k < counter_us)k ++;
        k = 0;
    }
}

/**
 * @brief 系统滴答初始化,无操作系统情况下调用
 * @param[in] tick_time_us 设置systick间隔时间us
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_systick_init(u32 tick_time_us)
{
    u32 sysclk = drv_rcc_get_sysclk_freq();
    SysTick_Config(sysclk/tick_time_us);
    tick_timeus_set = tick_time_us;
    return DRV_OK;
}

/**
 * @brief 读取滴答时间
 * @param[out] tick 滴答值（精度必须转换成ms）
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_get_systick(u32 *tick)
{
    *tick = iqr_get_tick();
    return DRV_OK;
}

/**
 * @brief 读取当前系统ms值
 * @return 返回获取的ms值
 */
u32 drv_get_timems(void)
{
    u32 time;
    drv_get_systick(&time);
    return time;
}

/**
 * @brief 查询时间是否超时
 * @param[in] time 原时间，可用 drv_get_systick获取
 * @param[in] delay 间隔时间
 * @return 返回执行结果
 * @retval TRUE    超时
 * @retval FALSE   未超时
 */
s32 drv_querry_time(u32 time, u32 delay)
{
    u32 curtime;
    u32 passtime;

    curtime = drv_get_timems();
    if (curtime >= time)
    {
        passtime = curtime - time;
    }
    else
    {
        passtime = (u32)(~0) - (time - curtime);
    }

    if (passtime >= delay)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief 延时
 * @param[in] us 时间 
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_delay_us(u32 us)
{
    inner_delayus_const(us);
    return DRV_OK;
}

/**
 * @brief 延时
 * @param[in] ms 时间
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_delay_ms(u32 ms)
{
    inner_delayms_const(ms);
    return DRV_OK;
}

/**
 * @brief 睡眠，主要用于有RTOS情况下，不占用CPU
 * @param[in] ms 时间   
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_sleep_ms(u32 ms)
{
    u32 curtime;
    drv_get_systick(&curtime);
    while (1)
    {
        if (drv_querry_time(curtime, ms) == TRUE)
        {
            break;
        }
    }
    return DRV_OK;
}

/**
 * @brief 系统初始化
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_sys_init(void)
{
    drv_systick_init(DRV_SYS_TICK_TIME);
    return DRV_OK;
}

/**
 * @brief 系统关机
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0	       失败原因
 */
s32 drv_sys_poweroff(void)
{
    return DRV_OK;
}

/**
 * @brief 系统开机上电
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_sys_poweron(void)
{
    return DRV_OK;
}

/**
 * @brief 系统休眠
 * @return s32 唤醒原因
 * @ref wakeup_reson_e
 */
s32 drv_sys_power_sleep(void)
{
    return DRV_OK;
}

/**
 * @brief 系统重启
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_sys_reboot(void)
{
    __disable_irq();                                 	//关闭所有中断
    NVIC_SystemReset();                                 //复位,以上两个函数位于 core_cm4.h 文件中
    return DRV_OK;
}

/**
* @brief 系统复位
 * @return 返回执行结果
 * @retval DRV_OK      成功
 * @retval <0          失败原因
 */
s32 drv_sys_reset(void)
{
    return DRV_OK;
}

/**
 * @brief 暂停编号>=0的中断，主动挂起kernel调度\n
 * 暂停编号>=0的中断, systick，hardfault等系统异常中断编号为负数仍保持开启;
 */
void drv_suspend_irq(void)
{
}

/**
 * @brief 恢复编号>=0的中断，恢复kernel调度
 */
void drv_resume_irq(void)
{
}
/**
 * @brief jump to 某个地址,主要是boot中调转到应用
 * @param[in] jumpaddr 跳转地址
 */
typedef  void (*pFunction)(void);
void drv_jump_to_app(u32 jumpaddr)
{
  uint32_t JumpAddress;
  pFunction Jump_To_Application;
  
  CLEAR_BIT(RCC->CFGR, RCC_CFGR_SW);
  while (0 != READ_BIT(RCC->CFGR, RCC_CFGR_SWS));
  
  CLEAR_BIT(RCC->CR, RCC_CR_PLLON);

  //if (((*(__IO uint32_t*)jumpaddr) & 0x2FFC0000 ) == 0x2FFC0000)
  {
    /* Jump to user application */
    JumpAddress = *(__IO uint32_t*) (jumpaddr + 4);
    Jump_To_Application = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) jumpaddr);
    Jump_To_Application();
  }
}
