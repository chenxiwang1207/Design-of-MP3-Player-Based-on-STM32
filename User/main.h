/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
  ******************************************************************************/
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H_
#define __MAIN_H_

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "string.h"
#include "stdlib.h"	
#include "stm32f10x.h"

/*-------app-------*/
#include "app_button.h"
#include "app_display_menu.h"
#include "app_setTime.h"
#include "app_music_application.h"
#include "app_display_lrc.h"

/*------device-----*/
#include "dev_button.h"
#include "dev_RTC.h"
#include "dev_button_config.h"
#include "com_ringbuffer.h"
#include "com_type.h"
#include "dev_w25q64.h"
#include "sdio_sdcard.h"
#include "spi_sdcard.h"
#include "spi_vs1053.h"

/*------drive------*/
#include "drv_wdt.h"
#include "drv_spi.h"
#include "drv_irq.h"
#include "drv_sys.h"
#include "drv_rcc.h"
#include "drv_gpio.h"
#include "drv_timer.h"
#include "drv_usart.h"
#include "drv_partition.h"

/*-------FATFS-----*/
#include "diskio.h"
#include "fatfs.h"

/*-------system-----*/
#include "Delay.h"
#include "sys.h"
#include "oled.h"
#include "OLED_Data.h"
//#include "oledfont.h"

/*----platform_config----*/
#include "platform_mcu_resource.h"
#include "platform_partition_config.h"

void Error_Handler(void);

#endif
