/**
 * @file drv_timer.h
 * @brief timer抽象层接口定义
 * @details timer抽象层接口定义，包括打开、关闭、设置、启动、停止、ioctl等接口
 * 
 * @version 1.0
 * @date 2026-03-12
 */
#ifndef DRV_TIMER_H
#define DRV_TIMER_H

#include "stm32f10x.h"                  // Device header
#include "com_type.h"
#include "platform_mcu_resource.h"

extern TIM_TypeDef* timer_instance[];

typedef void(*timer_callback)(void*arg);
void timer_test(void);

/**
 * @brief TIM对外接口函数，对外只调用此函数来启动定时器
 * @param[in] no_time    timer编号
 * @param[in] interval   定时器周期(单位ms)
 * @param[in] cb         定时器中断回调函数
 * @return 返回执行结果
 * @retval DRV_OK        成功
 * @retval <0            失败
 */
void drv_timer_init(user_timer_e no_time,u32 interval,timer_callback cb);

/**
 * @brief 打开TIMER功能
 * @param[in] no_time   timer编号
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败
 */
s32 drv_timer_open(user_timer_e no_time);

/**
 * @brief 关闭Timer功能
 * @param[in] no_time   timer编号
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败
 */
s32 drv_timer_close(user_timer_e no_time);

/**
 * @brief 设置下次中断时间(不用每次调用setup去设置一串寄存器)
 * @param[in] no_time    timer编号
 * @param[in] interval   设置下次中断时间间隔
 * @return 返回执行结果
 * @retval DRV_OK        成功
 * @retval <0            失败
 */
s32 drv_timer_interval(user_timer_e no_time, u32 interval);

/**
 * @brief 设置TIMER属性
 * @param[in] no_time    timer编号
 * @param[in] interval   定时器周期(单位ms)
 * @param[in] cb         定时器中断回调函数
 * @return 返回执行结果
 * @retval DRV_OK        成功
 * @retval <0            失败
 */
s32 drv_timer_setup(user_timer_e no_time,u32 interval, timer_callback callback);

/**
 * @brief 启动定时计数器
 * @param[in] no_time   timer编号
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败
 */
s32 drv_timer_start(user_timer_e no_time);

/**
 * @brief 停止定时计数器
 * @param[in] no_time   timer编号
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败
 */
s32 drv_timer_stop(user_timer_e no_time);

/**
 * @brief 根据IRQ返回对应的TIM句柄指针
 * @param[in] irq 中断号
 * @return TIM_TimeBaseInitTypeDef* 找到返回指针，未找到返回 NULL
 */
TIM_TimeBaseInitTypeDef* drv_timer_get_handle_by_irq(IRQn_Type irq);
/**
 * @brief 根据IRQ返回对应的定时器号
 * @param[in] irq 中断号
 * @return USER_TIMER1-USER_TIMER8
 */
s32 drv_timer_get_index_by_irq(IRQn_Type irq);
/**
 * @brief 根据设置的定时器实例返回对应的定时器号
 * @param[in] instance 定时器实例
 * @return USER_TIMER1 - USER_TIMER8
 */
s32 drv_timer_get_index_by_instance(TIM_TypeDef *instance);

#endif
