/**
 * @file drv_gpio.h
 * @brief GPIO抽象层接口定义
 * @details 该文件定义了GPIO抽象层接口，主要包括GPIO的打开、关闭、配置、输出、输入、中断控制等功能。
 * 
 * @version 1.0
 * @date 2026-03-11
 */
#ifndef _DRV_GPIO_H_
#define _DRV_GPIO_H_

#include "com_type.h"
#include "platform_mcu_resource.h"

/**
 * @brief GPIO 高低电平
 */
typedef enum{
    DRV_GPIO_LOW  = 0,
    DRV_GPIO_HIGH = 1
}gpio_level_e;
/*
* GPIO_pull_define Pull definition
*/
//typedef enum
//{
//    DRV_GPIO_No_Pull,    /*!< 无上下拉激活 */
//    DRV_GPIO_Pull_Up,    /*!< 上拉激活 */
//    DRV_GPIO_Pull_Down   /*!< 下拉激活 */
//}gpio_pull_type_e;

/**
 * @brief GPIO引脚速度
 */
typedef enum
{
    DRV_Speed_LOW = 0,
    DRV_Speed_MEDIUM,
    DRV_Speed_HIGH,
}gpio_speed_e;

/*
* Pin configuration
*/
typedef enum {
    DRV_GPIO_Mode_AIN,                 /**< 模拟输入  */
    DRV_GPIO_Mode_IN_FLOATING,         /**< 浮空输入 */
    DRV_GPIO_Mode_IPD,                 /** 下拉输入 **/
    DRV_GPIO_Mode_IPU,                 /** 上拉输入 **/
    DRV_GPIO_Mode_Out_OD,              /**< 开漏输出  */
    DRV_GPIO_Mode_Out_PP,              /**< 推挽输出  */
    DRV_GPIO_Mode_AF_OD,               /**< 复用开漏输出  */
    DRV_GPIO_Mode_AF_PP,               /**< 复用推挽输出  */
}gpio_mode_e;

/**
 * @brief GPIO外部中断定义
 */
typedef enum
{
    DRV_GPIO_IRQ_LOW       = 0U,   /**< 低电平中断  */
    DRV_GPIO_IRQ_HIGH      = 1U,   /**< 高电平中断  */
    DRV_GPIO_IRQ_FALLING   = 2U,   /**< 下降沿中断  */
    DRV_GPIO_IRQ_RISING    = 3U,   /**< 上升沿中断  */
    DRV_GPIO_IRQ_BOTH_EDGE = 4U    /**< 上升下降沿中断  */
}gpio_irq_e;

void drv_gpio_open(user_gpio_e gpio);

void drv_gpio_close(user_gpio_e gpio);

/**
 * @brief 配置管脚属性
 * @param[in] gpio  GPIO编号
 *       @arg BREO_GPIO_PA0 - BREO_GPIO_PA15
 *       @arg BREO_GPIO_PB0 - BREO_GPIO_PB15
 *       @arg BREO_GPIO_PC0 - BREO_GPIO_PC15
 *       @arg BREO_GPIO_PD0 - BREO_GPIO_PD2
 * @param[in] mode  GPIO属性/模式
 *       @arg DRV_GPIO_Mode_AIN,             模拟输入
 *       @arg DRV_GPIO_Mode_IN_FLOATING,     浮空输入 
 *       @arg DRV_GPIO_Mode_IPD,             上拉输入 
 *       @arg DRV_GPIO_Mode_IPU,             下拉输入
 *       @arg DRV_GPIO_Mode_Out_OD,          开漏输出
 *       @arg DRV_GPIO_Mode_Out_PP,          推挽输出
 *       @arg DRV_GPIO_Mode_AF_OD,           复用开漏输出
 *       @arg DRV_GPIO_Mode_AF_PP,           复用推挽输出
 * @param[in] Speed  GPIO速度
 *       @arg DRV_Speed_LOW                 低速
 *       @arg DRV_Speed_MEDIUM              中速
 *       @arg DRV_Speed_HIGH                高速
 */
void drv_gpio_config(user_gpio_e gpio,gpio_mode_e mode,gpio_speed_e Speed);
/**
 * @brief 控制管脚输出值
 * @param[in] gpio GPIO编号   
 * @param[in] level \n
 *       @arg GPIO_PIN_RESET = 低电平
 *       @arg GPIO_PIN_SET   = 高电平
*/
void drv_gpio_write(user_gpio_e gpio,u8 level);
/**
* @brief 读取管脚输入值
* @param[in] gpio GPIO编号
* @return 返回管脚输入值
* @retval 0 = 输入低
* @retval 1 = 输入高
* @retval <0  失败原因
*/
s32 drv_gpio_read_in(user_gpio_e gpio);
#endif
