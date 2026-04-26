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
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"                  // Device header
#include "drv_gpio.h"
#include "com_type.h"
#include "platform_mcu_resource.h"

GPIO_TypeDef* gpio_type_def[4]={GPIOA,GPIOB,GPIOC,GPIOD};

const static u16 gpio_pin_num[USER_GPIO_MAX]={GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2, GPIO_Pin_3,  \
                                       GPIO_Pin_4, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7,  \
                                       GPIO_Pin_8, GPIO_Pin_9, GPIO_Pin_10,GPIO_Pin_11, \
                                       GPIO_Pin_12,GPIO_Pin_13,GPIO_Pin_14,GPIO_Pin_15};

void drv_gpio_open(user_gpio_e gpio)
{
  if(gpio >= USER_GPIO_MAX)
  {
     return;
  }
  u32 gpio_port = gpio / 16;
  switch(gpio_port)
  {
    case 0: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);    break;
    case 1: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);    break;
    case 2: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);    break;
    case 3: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);    break;
  }
}

void drv_gpio_close(user_gpio_e gpio)
{
  GPIO_TypeDef* GPIOx = gpio_type_def[gpio/16];
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  GPIO_InitStructure.GPIO_Pin = gpio_pin_num[gpio];
  GPIO_Init(GPIOx,&GPIO_InitStructure);
}
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
void drv_gpio_config(user_gpio_e gpio,gpio_mode_e mode,gpio_speed_e Speed)
{
  if(gpio >= USER_GPIO_MAX)
  {
     return;
  }
  GPIO_TypeDef* GPIOx = gpio_type_def[gpio/16];
  u16 gpio_pin = gpio_pin_num[gpio%16];
  u32 gpio_mode = 0;
  u32 gpio_speed = 0;
  GPIO_InitTypeDef GPIO_InitStructure = {0};
  switch(mode)
  {
    case DRV_GPIO_Mode_AIN        : gpio_mode = GPIO_Mode_AIN;          break;
    case DRV_GPIO_Mode_IN_FLOATING: gpio_mode = GPIO_Mode_IN_FLOATING;  break;
    case DRV_GPIO_Mode_IPD        : gpio_mode = GPIO_Mode_IPD;          break;
    case DRV_GPIO_Mode_IPU        : gpio_mode = GPIO_Mode_IPU;          break;
    case DRV_GPIO_Mode_Out_OD     : gpio_mode = GPIO_Mode_Out_OD;       break;
    case DRV_GPIO_Mode_Out_PP     : gpio_mode = GPIO_Mode_Out_PP;       break;
    case DRV_GPIO_Mode_AF_OD      : gpio_mode = GPIO_Mode_AF_OD;        break;
    case DRV_GPIO_Mode_AF_PP      : gpio_mode = GPIO_Mode_AF_PP;        break;
    default                       :                                     break;
  }
  switch(Speed)
  {
    case DRV_Speed_LOW      : gpio_speed = GPIO_Speed_2MHz;    break;
    case DRV_Speed_MEDIUM   : gpio_speed = GPIO_Speed_10MHz;   break;
    case DRV_Speed_HIGH     : gpio_speed = GPIO_Speed_50MHz;   break;
    default                 :                                  break;
  }
  GPIO_InitStructure.GPIO_Pin   = gpio_pin;
  GPIO_InitStructure.GPIO_Mode  = (GPIOMode_TypeDef) gpio_mode;
  GPIO_InitStructure.GPIO_Speed = (GPIOSpeed_TypeDef)gpio_speed;
  GPIO_Init(GPIOx, &GPIO_InitStructure);
}
/**
 * @brief 控制管脚输出值
 * @param[in] gpio GPIO编号   
 * @param[in] level \n
 *       @arg 0 = 低电平
 *       @arg 1 = 高电平
*/
void drv_gpio_write(user_gpio_e gpio,u8 level)
{
  GPIO_TypeDef* GPIOx = gpio_type_def[gpio/16];
  u16 gpio_pin = gpio_pin_num[gpio%16];

  GPIO_WriteBit(GPIOx, gpio_pin, (BitAction)level);
}
/**
* @brief 读取管脚输入值
* @param[in] gpio GPIO编号
* @return 返回管脚输入值
* @retval 0 = 输入低
* @retval 1 = 输入高
* @retval <0  失败原因
*/
s32 drv_gpio_read_in(user_gpio_e gpio)
{
  GPIO_TypeDef* GPIOx = gpio_type_def[gpio/16];
  u16 gpio_pin = gpio_pin_num[gpio%16];
  return GPIO_ReadInputDataBit(GPIOx, gpio_pin);
}

