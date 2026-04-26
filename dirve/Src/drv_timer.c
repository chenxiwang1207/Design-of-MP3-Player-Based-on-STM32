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
 * @file drv_timer.c
 * @brief timer抽象层接口定义
 * @details timer抽象层接口定义，包括打开、关闭、设置、启动、停止、ioctl等接口
 * 
 * @version 1.0
 * @date 2026-03-12
 * @copyright Shenzhen Breo Technology Co., Ltd.
 * 
 */
#include "drv_timer.h"
#include "drv_irq.h"
#include "drv_usart.h"

static TIM_TimeBaseInitTypeDef  htimer[USER_TIMER_MAX] = {0};   //TIM初始化配置句柄
TIM_TypeDef* timer_instance[USER_TIMER_MAX] = 
{
  TIM1, TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM8
};
const IRQn_Type timer_irq[USER_TIMER_MAX] =
{
  TIM1_UP_IRQn, TIM2_IRQn, TIM3_IRQn, TIM4_IRQn, TIM5_IRQn, TIM6_IRQn, TIM7_IRQn, TIM8_UP_IRQn
};

/**
 * @brief 根据IRQ返回对应的TIM句柄指针
 * @param[in] irq 中断号
 * @return TIM_TimeBaseInitTypeDef* 找到返回指针，未找到返回 NULL
 */
TIM_TimeBaseInitTypeDef* drv_timer_get_handle_by_irq(IRQn_Type irq)
{
  for(int i = 0; i < USER_TIMER_MAX; i++)
  {
    if(timer_irq[i] == irq)
    {
      return &htimer[i];
    }
  }
  return NULL;
}
/**
 * @brief 根据IRQ返回对应的定时器号
 * @param[in] irq 中断号
 * @return USER_TIMER1-USER_TIMER8
 */
s32 drv_timer_get_index_by_irq(IRQn_Type irq)
{
  for(int i = 0; i < USER_TIMER_MAX; i++)
  {
    if(timer_irq[i] == irq)
      return i;
  }
  return -1;
}
/**
 * @brief 根据设置的定时器实例返回对应的定时器号
 * @param[in] instance 定时器实例
 * @return USER_TIMER1 - USER_TIMER8
 */
s32 drv_timer_get_index_by_instance(TIM_TypeDef *instance)
{
  for(int i = 0; i < USER_TIMER_MAX; i++)
  {
    if(timer_instance[i] == instance)
      return i;
  }
  return -1;
}
/**
 * @brief 打开TIMER功能
 * @param[in] no_time   timer编号
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败
 */
s32 drv_timer_open(user_timer_e no_time)
{
  if(no_time >= USER_TIMER_MAX)
  {
    return -1;
  }
  switch(no_time)
  {
    case USER_TIMER1: RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE); break;
    case USER_TIMER2: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE); break;
    case USER_TIMER3: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE); break;
    case USER_TIMER4: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE); break;
    case USER_TIMER5: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE); break;
    case USER_TIMER6: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE); break;
    case USER_TIMER7: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE); break;
    case USER_TIMER8: RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE); break;
    default         : break;
  }
  return 0;
}
/**
 * @brief 关闭Timer功能
 * @param[in] no_time   timer编号
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败
 */
s32 drv_timer_close(user_timer_e no_time)
{
  if(no_time >= USER_TIMER_MAX)
  {
    return -1;
  }
  switch(no_time)
  {
    case USER_TIMER1:  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,DISABLE);  break;
    case USER_TIMER2:  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,DISABLE);  break;
    case USER_TIMER3:  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,DISABLE);  break;
    case USER_TIMER4:  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,DISABLE);  break;
    case USER_TIMER5:  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,DISABLE);  break;
    case USER_TIMER6:  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,DISABLE);  break;
    case USER_TIMER7:  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,DISABLE);  break;
    case USER_TIMER8:  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,DISABLE);  break;
    default         :  break;
  }
  return 0;
}
/**
 * @brief 设置TIMER属性
 * @param[in] no_time    timer编号
 * @param[in] interval   定时器周期(单位ms)
 * @param[in] cb         定时器中断回调函数
 * @return 返回执行结果
 * @retval DRV_OK        成功
 * @retval <0            失败
 */
s32 drv_timer_setup(user_timer_e no_time,u32 interval, timer_callback callback)
{
  if(no_time >= USER_TIMER_MAX || callback == NULL|| interval == 0)
  {
    return -1;
  }
  if(interval > 65535)
  {
    interval = 65535;
  }
  // interval_ms = [(PSC + 1) * (ARR + 1) / Timer_Clock]* 1000
  htimer[no_time].TIM_Prescaler     = 36000 - 1;         //设置预分频器PSC
  htimer[no_time].TIM_CounterMode   = TIM_CounterMode_Up;//向上计数模式
  htimer[no_time].TIM_Period        = 2*interval - 1;    //设置自动重装载值ARR，使定时器周期为interval毫秒
  htimer[no_time].TIM_ClockDivision = TIM_CKD_DIV1;      //时钟分频
  htimer[no_time].TIM_RepetitionCounter = 0;             //高级定时器的重复计数器，基本定时器无效，设置为0
  TIM_TimeBaseInit(timer_instance[no_time],&htimer[no_time]);
  TIM_Cmd(timer_instance[no_time],ENABLE);//启动定时器
  
  TIM_ITConfig(timer_instance[no_time],TIM_IT_Update,ENABLE);//使能更新中断
  NVIC_SetPriority(timer_irq[no_time],1);
  NVIC_EnableIRQ(timer_irq[no_time]);
  
  //注册回调函数
  drv_irq_register_callback(timer_irq[no_time], callback, NULL);
  return 0;
}
/**
 * @brief 设置下次中断时间(不用每次调用setup去设置一串寄存器)
 * @param[in] no_time    timer编号
 * @param[in] interval   设置下次中断时间间隔
 * @return 返回执行结果
 * @retval DRV_OK        成功
 * @retval <0            失败
 */
s32 drv_timer_interval(user_timer_e no_time, u32 interval)
{
  if(no_time >= USER_TIMER_MAX || interval == 0)
  {
    return -1;
  }
  TIM_TypeDef* TIMx = timer_instance[no_time];
  // 设置自动重装载值
  TIM_SetAutoreload(TIMx, interval - 1);
  // 重置计数器
  TIM_SetCounter(TIMx, 0);
  // 确保定时器使能且更新中断使能（若之前已使能则无需重复，但为保证可重复调用）
  TIM_Cmd(TIMx, ENABLE);
  TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
  return 0;
}
/**
 * @brief 启动定时计数器
 * @param[in] no_time   timer编号
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败
 */
s32 drv_timer_start(user_timer_e no_time)
{
  if(no_time >= USER_TIMER_MAX)
  {
    return -1;
  }
  TIM_Cmd(timer_instance[no_time], ENABLE);
  return 0;
}
/**
 * @brief 停止定时计数器
 * @param[in] no_time   timer编号
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败
 */
s32 drv_timer_stop(user_timer_e no_time)
{
  if(no_time >= USER_TIMER_MAX)
  {
    return -1;
  }
  TIM_Cmd(timer_instance[no_time], DISABLE);
  return 0;
}
/**
 * @brief TIM对外接口函数，对外只调用此函数来启动定时器
 * @param[in] no_time    timer编号
 * @param[in] interval   定时器周期(单位ms)
 * @param[in] cb         定时器中断回调函数
 * @return 返回执行结果
 * @retval DRV_OK        成功
 * @retval <0            失败
 */
void drv_timer_init(user_timer_e no_time,u32 interval,timer_callback callback)
{
  drv_timer_stop(no_time);
  drv_timer_close(no_time);
  drv_timer_open(no_time);
  drv_timer_setup(no_time,interval,callback);
  drv_timer_start(no_time);
}

#define TIM_TEST 1
#if     TIM_TEST

void timer_test_callback(void* arg)
{
    test_printf_uart1("\r\ntest TIM3");
}

void timer_test(void)
{
    drv_timer_init(USER_TIMER3,3000,timer_test_callback);
}
#endif
