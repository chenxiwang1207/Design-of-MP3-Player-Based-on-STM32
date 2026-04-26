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
  * @file    usb.c
  * @brief   This file provides code for the configuration
  *          of the USB instances.
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
#include "drv_usb.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

PCD_HandleTypeDef hpcd_USB_FS;

/* USB init function */

void DRV_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.low_power_enable = ENABLE;
  hpcd_USB_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

}

void HAL_PCD_MspInit(PCD_HandleTypeDef* pcdHandle)
{

  if(pcdHandle->Instance==USB)
  {
  /* USER CODE BEGIN USB_MspInit 0 */

  /* USER CODE END USB_MspInit 0 */
    /* USB clock enable */
    __HAL_RCC_USB_CLK_ENABLE();

    /* USB interrupt Init */
    if(pcdHandle->Init.low_power_enable == 1)
    {
      /* Enable EXTI Line 20 for USB wakeup */
      __HAL_USB_WAKEUP_EXTI_CLEAR_FLAG();
      __HAL_USB_WAKEUP_EXTI_ENABLE_RISING_EDGE();
      __HAL_USB_WAKEUP_EXTI_ENABLE_IT();
      HAL_NVIC_SetPriority(USBWakeUp_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(USBWakeUp_IRQn);
    }
  /* USER CODE BEGIN USB_MspInit 1 */

  /* USER CODE END USB_MspInit 1 */
  }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef* pcdHandle)
{

  if(pcdHandle->Instance==USB)
  {
  /* USER CODE BEGIN USB_MspDeInit 0 */

  /* USER CODE END USB_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USB_CLK_DISABLE();

    /* USB interrupt Deinit */
    HAL_NVIC_DisableIRQ(USBWakeUp_IRQn);
  /* USER CODE BEGIN USB_MspDeInit 1 */

  /* USER CODE END USB_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

