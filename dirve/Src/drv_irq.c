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
#include "stm32f10x.h"
#include "platform_mcu_resource.h"
#include "drv_timer.h"
#include "com_type.h"
#include "drv_irq.h"

volatile user_irq_callback irq_callback_fun[USER_TIMER_MAX] = {0};
volatile void *irq_callback_arg[USER_TIMER_MAX] = {0};

u32 sys_tick_num = 0;
u32 iqr_get_tick(void)
{
    return sys_tick_num;
}

enum
{
    USER_IQR_PRIORITY0_HIGHEST=0,
    USER_IQR_PRIORITY1,
    USER_IQR_PRIORITY2,
    USER_IQR_PRIORITY3,
    USER_IQR_PRIORITY4,
    USER_IQR_PRIORITY5,
    USER_IQR_PRIORITY6,
    USER_IQR_PRIORITY7_LOWEST,
};
const u8 drv_irq_priority[] = 
{
  USER_IQR_PRIORITY3,        /******  Cortex-M3 Processor Exceptions Numbers ***************************************************/
  USER_IQR_PRIORITY3,        //   NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                             */
  USER_IQR_PRIORITY3,        //   HardFault_IRQn              = -13,    /*!< 3 Cortex-M3 Hard Fault Interrupt                     */
  USER_IQR_PRIORITY3,        //   MemoryManagement_IRQn       = -12,    /*!< 4 Cortex-M3 Memory Management Interrupt              */
  USER_IQR_PRIORITY3,        //   BusFault_IRQn               = -11,    /*!< 5 Cortex-M3 Bus Fault Interrupt                      */
  USER_IQR_PRIORITY3,        //   UsageFault_IRQn             = -10,    /*!< 6 Cortex-M3 Usage Fault Interrupt                    */
  USER_IQR_PRIORITY3,        //   SVCall_IRQn                 = -5,     /*!< 11 Cortex-M3 SV Call Interrupt                       */
  USER_IQR_PRIORITY3,        //   DebugMonitor_IRQn           = -4,     /*!< 12 Cortex-M3 Debug Monitor Interrupt                 */
  USER_IQR_PRIORITY3,        //   PendSV_IRQn                 = -2,     /*!< 14 Cortex-M3 Pend SV Interrupt                       */
  USER_IQR_PRIORITY3,        //   SysTick_IRQn                = -1,     /*!< 15 Cortex-M3 System Tick Interrupt                   */
                             /******  STM32 specific Interrupt Numbers *********************************************************/
  USER_IQR_PRIORITY3,        //   WWDG_IRQn                   = 0,      /*!< Window WatchDog Interrupt                            */
  USER_IQR_PRIORITY3,        //   PVD_IRQn                    = 1,      /*!< PVD through EXTI Line detection Interrupt            */
  USER_IQR_PRIORITY3,        //   TAMPER_IRQn                 = 2,      /*!< Tamper Interrupt                                     */
  USER_IQR_PRIORITY3,        //   RTC_IRQn                    = 3,      /*!< RTC global Interrupt                                 */
  USER_IQR_PRIORITY3,        //   FLASH_IRQn                  = 4,      /*!< FLASH global Interrupt                               */
  USER_IQR_PRIORITY3,        //   RCC_IRQn                    = 5,      /*!< RCC global Interrupt                                 */
  USER_IQR_PRIORITY3,        //   EXTI0_IRQn                  = 6,      /*!< EXTI Line0 Interrupt                                 */
  USER_IQR_PRIORITY3,        //   EXTI1_IRQn                  = 7,      /*!< EXTI Line1 Interrupt                                 */
  USER_IQR_PRIORITY3,        //   EXTI2_IRQn                  = 8,      /*!< EXTI Line2 Interrupt                                 */
  USER_IQR_PRIORITY3,        //   EXTI3_IRQn                  = 9,      /*!< EXTI Line3 Interrupt                                 */
  USER_IQR_PRIORITY3,        //   EXTI4_IRQn                  = 10,     /*!< EXTI Line4 Interrupt                                 */
  USER_IQR_PRIORITY3,        //   DMA1_Channel1_IRQn          = 11,     /*!< DMA1 Channel 1 global Interrupt                      */
  USER_IQR_PRIORITY3,        //   DMA1_Channel2_IRQn          = 12,     /*!< DMA1 Channel 2 global Interrupt                      */
  USER_IQR_PRIORITY3,        //   DMA1_Channel3_IRQn          = 13,     /*!< DMA1 Channel 3 global Interrupt                      */
  USER_IQR_PRIORITY3,        //   DMA1_Channel4_IRQn          = 14,     /*!< DMA1 Channel 4 global Interrupt                      */
  USER_IQR_PRIORITY3,        //   DMA1_Channel5_IRQn          = 15,     /*!< DMA1 Channel 5 global Interrupt                      */
  USER_IQR_PRIORITY3,        //   DMA1_Channel6_IRQn          = 16,     /*!< DMA1 Channel 6 global Interrupt                      */
  USER_IQR_PRIORITY3,        //   DMA1_Channel7_IRQn          = 17,     /*!< DMA1 Channel 7 global Interrupt                      */
  USER_IQR_PRIORITY3,        //   ADC1_2_IRQn                 = 18,     /*!< ADC1 and ADC2 global Interrupt                       */
  USER_IQR_PRIORITY3,        //   USB_HP_CAN1_TX_IRQn         = 19,     /*!< USB Device High Priority or CAN1 TX Interrupts       */
  USER_IQR_PRIORITY3,        //   USB_LP_CAN1_RX0_IRQn        = 20,     /*!< USB Device Low Priority or CAN1 RX0 Interrupts       */
  USER_IQR_PRIORITY3,        //   CAN1_RX1_IRQn               = 21,     /*!< CAN1 RX1 Interrupt                                   */
  USER_IQR_PRIORITY3,        //   CAN1_SCE_IRQn               = 22,     /*!< CAN1 SCE Interrupt                                   */
  USER_IQR_PRIORITY3,        //   EXTI9_5_IRQn                = 23,     /*!< External Line[9:5] Interrupts                        */
  USER_IQR_PRIORITY3,        //   TIM1_BRK_IRQn               = 24,     /*!< TIM1 Break Interrupt                                 */
  USER_IQR_PRIORITY3,        //   TIM1_UP_IRQn                = 25,     /*!< TIM1 Update Interrupt                                */
  USER_IQR_PRIORITY3,        //   TIM1_TRG_COM_IRQn           = 26,     /*!< TIM1 Trigger and Commutation Interrupt               */
  USER_IQR_PRIORITY3,        //   TIM1_CC_IRQn                = 27,     /*!< TIM1 Capture Compare Interrupt                       */
  USER_IQR_PRIORITY3,        //   TIM2_IRQn                   = 28,     /*!< TIM2 global Interrupt                                */
  USER_IQR_PRIORITY3,        //   TIM3_IRQn                   = 29,     /*!< TIM3 global Interrupt                                */
  USER_IQR_PRIORITY3,        //   TIM4_IRQn                   = 30,     /*!< TIM4 global Interrupt                                */
  USER_IQR_PRIORITY3,        //   I2C1_EV_IRQn                = 31,     /*!< I2C1 Event Interrupt                                 */
  USER_IQR_PRIORITY3,        //   I2C1_ER_IRQn                = 32,     /*!< I2C1 Error Interrupt                                 */
  USER_IQR_PRIORITY3,        //   I2C2_EV_IRQn                = 33,     /*!< I2C2 Event Interrupt                                 */
  USER_IQR_PRIORITY3,        //   I2C2_ER_IRQn                = 34,     /*!< I2C2 Error Interrupt                                 */
  USER_IQR_PRIORITY3,        //   SPI1_IRQn                   = 35,     /*!< SPI1 global Interrupt                                */
  USER_IQR_PRIORITY3,        //   SPI2_IRQn                   = 36,     /*!< SPI2 global Interrupt                                */
  USER_IQR_PRIORITY3,        //   USART1_IRQn                 = 37,     /*!< USART1 global Interrupt                              */
  USER_IQR_PRIORITY3,        //   USART2_IRQn                 = 38,     /*!< USART2 global Interrupt                              */
  USER_IQR_PRIORITY3,        //   USART3_IRQn                 = 39,     /*!< USART3 global Interrupt                              */
  USER_IQR_PRIORITY3,        //   EXTI15_10_IRQn              = 40,     /*!< External Line[15:10] Interrupts                      */
  USER_IQR_PRIORITY3,        //   RTC_Alarm_IRQn              = 41,     /*!< RTC Alarm through EXTI Line Interrupt                */
  USER_IQR_PRIORITY3,        //   USBWakeUp_IRQn              = 42,     /*!< USB Device WakeUp from suspend through EXTI Line Interrupt */
  USER_IQR_PRIORITY3,        //   TIM8_BRK_IRQn               = 43,     /*!< TIM8 Break Interrupt                                 */
  USER_IQR_PRIORITY3,        //   TIM8_UP_IRQn                = 44,     /*!< TIM8 Update Interrupt                                */
  USER_IQR_PRIORITY3,        //   TIM8_TRG_COM_IRQn           = 45,     /*!< TIM8 Trigger and Commutation Interrupt               */
  USER_IQR_PRIORITY3,        //   TIM8_CC_IRQn                = 46,     /*!< TIM8 Capture Compare Interrupt                       */
  USER_IQR_PRIORITY3,        //   ADC3_IRQn                   = 47,     /*!< ADC3 global Interrupt                                */
  USER_IQR_PRIORITY3,        //   FSMC_IRQn                   = 48,     /*!< FSMC global Interrupt                                */
  USER_IQR_PRIORITY3,        //   SDIO_IRQn                   = 49,     /*!< SDIO global Interrupt                                */
  USER_IQR_PRIORITY3,        //   TIM5_IRQn                   = 50,     /*!< TIM5 global Interrupt                                */
  USER_IQR_PRIORITY3,        //   SPI3_IRQn                   = 51,     /*!< SPI3 global Interrupt                                */
  USER_IQR_PRIORITY3,        //   UART4_IRQn                  = 52,     /*!< UART4 global Interrupt                               */
  USER_IQR_PRIORITY3,        //   UART5_IRQn                  = 53,     /*!< UART5 global Interrupt                               */
  USER_IQR_PRIORITY3,        //   TIM6_IRQn                   = 54,     /*!< TIM6 global Interrupt                                */
  USER_IQR_PRIORITY3,        //   TIM7_IRQn                   = 55,     /*!< TIM7 global Interrupt                                */
  USER_IQR_PRIORITY3,        //   DMA2_Channel1_IRQn          = 56,     /*!< DMA2 Channel 1 global Interrupt                      */
  USER_IQR_PRIORITY3,        //   DMA2_Channel2_IRQn          = 57,     /*!< DMA2 Channel 2 global Interrupt                      */
  USER_IQR_PRIORITY3,        //   DMA2_Channel3_IRQn          = 58,     /*!< DMA2 Channel 3 global Interrupt                      */
  USER_IQR_PRIORITY3,        //   DMA2_Channel4_5_IRQn        = 59,     /*!< DMA2 Channel 4 and Channel 5 global Interrupt        */
};

// 定时器实例数组（在 drv_timer.c 中定义为全局）
extern TIM_TypeDef* timer_instance[];

// 辅助函数：通过 IRQ 获取定时器索引（由 drv_timer.c 实现）
s32 drv_timer_get_index_by_irq(IRQn_Type irq);

void TIM1_UP_IRQHandler(void)
{
    s32 idx = drv_timer_get_index_by_irq(TIM1_UP_IRQn);
    if (idx >= 0 && idx < USER_TIMER_MAX)
    {
        TIM_TypeDef* TIMx = timer_instance[idx];
        // 清除更新中断标志
        TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
        // 调用用户回调
        if (irq_callback_fun[idx])
        {
            irq_callback_fun[idx]((void*)irq_callback_arg[idx]);
        }
    }
}

void TIM2_IRQHandler(void)
{
    s32 idx = drv_timer_get_index_by_irq(TIM2_IRQn);
    if (idx >= 0 && idx < USER_TIMER_MAX)
    {
        TIM_TypeDef* TIMx = timer_instance[idx];
        TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
        if (irq_callback_fun[idx])
        {
            irq_callback_fun[idx]((void*)irq_callback_arg[idx]);
        }
    }
}

void TIM3_IRQHandler(void)
{
    s32 idx = drv_timer_get_index_by_irq(TIM3_IRQn);
    if (idx >= 0 && idx < USER_TIMER_MAX)
    {
        TIM_TypeDef* TIMx = timer_instance[idx];
        TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
        if (irq_callback_fun[idx])
        {
            irq_callback_fun[idx]((void*)irq_callback_arg[idx]);
        }
    }
}

void TIM4_IRQHandler(void)
{
    s32 idx = drv_timer_get_index_by_irq(TIM4_IRQn);
    if (idx >= 0 && idx < USER_TIMER_MAX)
    {
        TIM_TypeDef* TIMx = timer_instance[idx];
        TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
        if (irq_callback_fun[idx])
        {
            irq_callback_fun[idx]((void*)irq_callback_arg[idx]);
        }
    }
}

void TIM5_IRQHandler(void)
{
    s32 idx = drv_timer_get_index_by_irq(TIM5_IRQn);
    if (idx >= 0 && idx < USER_TIMER_MAX)
    {
        TIM_TypeDef* TIMx = timer_instance[idx];
        TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
        if (irq_callback_fun[idx])
        {
            irq_callback_fun[idx]((void*)irq_callback_arg[idx]);
        }
    }
}

void TIM6_IRQHandler(void)
{
    s32 idx = drv_timer_get_index_by_irq(TIM6_IRQn);
    if (idx >= 0 && idx < USER_TIMER_MAX)
    {
        TIM_TypeDef* TIMx = timer_instance[idx];
        TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
        if (irq_callback_fun[idx])
        {
            irq_callback_fun[idx]((void*)irq_callback_arg[idx]);
        }
    }
}

void TIM7_IRQHandler(void)
{
    s32 idx = drv_timer_get_index_by_irq(TIM7_IRQn);
    if (idx >= 0 && idx < USER_TIMER_MAX)
    {
        TIM_TypeDef* TIMx = timer_instance[idx];
        TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
        if (irq_callback_fun[idx])
        {
            irq_callback_fun[idx]((void*)irq_callback_arg[idx]);
        }
    }
}

void TIM8_UP_IRQHandler(void)
{
    s32 idx = drv_timer_get_index_by_irq(TIM8_UP_IRQn);
    if (idx >= 0 && idx < USER_TIMER_MAX)
    {
        TIM_TypeDef* TIMx = timer_instance[idx];
        TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
        if (irq_callback_fun[idx])
        {
            irq_callback_fun[idx]((void*)irq_callback_arg[idx]);
        }
    }
}

/**
 * @brief     注册中断处理函数
 * @param[in] s32 irq_no        中断号，不能小于0
 * @param[in] user_irq_callback fun  中断处理函数
 * @param[in] void *arg               中断处理函数参数
 * @return    无
 * @remark    注意事项
 */
void drv_irq_register_callback(s32 irq_no, user_irq_callback fun, void *arg)
{
    if (irq_no < 0) return;
    int idx = drv_timer_get_index_by_irq((IRQn_Type)irq_no);
    if (idx >= 0 && idx < USER_TIMER_MAX)
    {
        irq_callback_fun[idx] = fun;
        irq_callback_arg[idx] = arg;
    }
}
