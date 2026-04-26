/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    spi.h
  * @brief   This file contains all the function prototypes for
  *          the spi.c file
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
#ifndef __DRV_SPI_H__
#define __DRV_SPI_H__

#include "stm32f10x.h"
#include "com_type.h"
#include "drv_gpio.h"
#include "platform_mcu_resource.h"

/** SPI mode flags */
#define	DRV_SPI_CPHA    0x01                 /**< clock phase */
#define	DRV_SPI_CPOL    0x02                 /**< clock polarity */

typedef struct
{
  SPI_TypeDef* SPIx;
  user_gpio_e SPIx_NSS;
  user_gpio_e SPIx_SCK;
  user_gpio_e SPIx_MOSI;
  user_gpio_e SPIx_MISO;
}SPIx_Port_e;   //SPI引脚配置

typedef struct
{
  gpio_mode_e  SPI_IO_mode;
  gpio_speed_e SPI_IO_speed;
}SPI_IO_Config_e;   //SPI的IO口设置

typedef struct
{
  SPI_IO_Config_e SPI_NSS_Config;
  SPI_IO_Config_e SPI_SCK_Config;
  SPI_IO_Config_e SPI_MOSI_Config;
  SPI_IO_Config_e SPI_MISO_Config;
}SPI_IO_ALLConfig_e;

typedef enum
{
  DRV_SPI_MODE_0 = (0|0),                      /**< 第一个上升沿采样*/     //spi低电平采样
  DRV_SPI_MODE_1 = (0|DRV_SPI_CPHA),           /**< 第二个下降沿采样*/     //spi上升沿采样
  DRV_SPI_MODE_2 = (DRV_SPI_CPOL|0),           /**< 第一个下降沿采样*/     //spi下降沿采样
  DRV_SPI_MODE_3 = (DRV_SPI_CPOL|DRV_SPI_CPHA) /**< 第二个上升沿采样*/     //spi高电平采样
}spi_clock_mode_e;

typedef enum
{
  DRV_DIV2_Speed_36000=0,
  DRV_DIV4_Speed_18000,
  DRV_DIV8_Speed_9000,
  DRV_DIV16_Speed_4500,
  DRV_DIV32_Speed_2250,
  DRV_DIV64_Speed_1125,
  DRV_DIV128_Speed_562_5,
  DRV_DIV256_Speed_281_25,
  DRV_DIV_MAX,
}spi_communicate_speed;

/** SPI transfer flags */
typedef enum
{
  DRV_SPI_MODE_MASTER,          /**< spi主模式 */
  DRV_SPI_MODE_SLAVE,          /**< spi从模式 */
}spi_master_slave_t;

typedef enum
{
    DRV_SPI_DATAWIDTH_8BIT,
    DRV_SPI_DATAWIDTH_16BIT,
}spi_data_width;   //定义SPI数据宽度

/**
 * @brief SPI初始化
 * @param[in] spi_no     SPI端口号
 * @param[in] mode       SPI主模式 或者 SPI从模式
 * @return 返回执行结果
 * @retval DRV_OK        成功
 * @retval <0            失败原因
 * @attention 本接口只初始化SPI总线接口
 */
void drv_spi_open(user_spi_e spi_no,spi_master_slave_t mode);
/**
 * @brief 初始化SPI配置
 * @param[in] spi_no    SPI端口号
 * @param[in] max_khz   SPI通信速率,单位是khz
 * @param[in] mode      SPI时钟采用级性
 * @param[in] bits      SPI发送数据长度(目前只支持发送8位数据)
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败原因
 * @pre 执行drv_spi_open后执行才有效
 */
void drv_spi_init(user_spi_e spi_no, spi_communicate_speed max_khz,spi_clock_mode_e mode, spi_data_width bits);
/**
 * @brief 获取spi初始化配置的句柄
 * @param[in] spi_no     SPI端口号
 * @return 返回spi初始化配置的句柄
 */
SPI_InitTypeDef drv_get_spi_handlestruct(user_spi_e spi_no);
/**
 * @brief 全双工收发数据(目前只支持发送8位数据)
 * @param[in] spi_no    SPI端口号
 * @param[in] write_buf 发送缓冲区指针
 * @param[in] read_buf  接收缓冲区指针
 * @param[in] len       发送和接收缓冲区长度
 * @warning 发送接收缓冲区必须一样大小，否则可能出现程序崩溃等异常
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败原因
 * @pre 执行drv_spi_open后执行才有效
 */
s32 drv_spi_xfer(user_spi_e spi_no, void *write,void *read, u32 len);
/**
 * @brief 发数据
 * @param[in] spi_no    SPI端口号
 * @param[in] buf       缓冲区指针
 * @param[in] len       缓冲区长度
 * @return 返回执行结果
 * @retval >0           发送数据长度(阻塞式),发送数据后等待数据接收回复(此内容本函数暂不实现)
 * @retval =0           数据待发送(非阻塞式)，使用drv_spi_idle查询发送完成情况
 * @retval <0           失败原因
 * @pre 执行drv_spi_open后执行才有效
 */
s32 drv_spi_write(user_spi_e spi_no, void *buf, u32 len);
/**
 * @brief 收数据
 * @param[in] spi_no    SPI端口号
 * @param[out] buf      缓冲区指针
 * @param[in] len       缓冲区长度
 * @return 返回执行结果
 * @retval >0           接收数据长度
 * @retval =0           数据待接收(非阻塞式)，使用drv_spi_idle查询接收完成情况
 * @retval <0           失败原因
 * @pre 执行drv_spi_open后执行才有效
 */
s32 drv_spi_read(user_spi_e spi_no, void *buf, u32 len);
/**
 * @brief 查询是否空闲
 * @param[in] spi_no    SPI端口号
 * @return 返回执行结果
 * @retval DRV_OK       处于空闲状态
 * @retval <0           失败原因
 * @pre 执行drv_spi_open后执行才有效
 */
s32 drv_spi_idle(user_spi_e spi_no);


#endif
