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
  * @file    spi.c
  * @brief   This file provides code for the configuration
  *          of the SPI instances.
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
#include "platform_mcu_resource.h"
#include "com_type.h"
#include "drv_spi.h"
#include "drv_gpio.h"

/**SPI1 GPIO Configuration
PA4     ------> SPI1_NSS  //推挽输出
PA5     ------> SPI1_SCK  //复用推挽
PA6     ------> SPI1_MISO //上拉输入
PA7     ------> SPI1_MOSI //复用推挽
*/
#define SPI1_NSS  USER_GPIO_PA4  //CS (controlled as GPIO output in master)
#define SPI1_SCK  USER_GPIO_PA5
#define SPI1_MOSI USER_GPIO_PA7
#define SPI1_MISO USER_GPIO_PA6

/**SPI2 GPIO Configuration
PB12     ------> SPI2_NSS   //推挽输出
PB13     ------> SPI2_SCK   //复用推挽
PB14     ------> SPI2_MISO  //上拉输入
PB15     ------> SPI2_MOSI  //复用推挽
*/
#define SPI2_NSS  USER_GPIO_PB12  //CS (controlled as GPIO output in master)
#define SPI2_SCK  USER_GPIO_PB13
#define SPI2_MOSI USER_GPIO_PB15
#define SPI2_MISO USER_GPIO_PB14

static SPIx_Port_e  SPIx_Port_Config[USER_SPI_MAX] = 
{
    {SPI1,SPI1_NSS,SPI1_SCK,SPI1_MOSI,SPI1_MISO},
    {SPI2,SPI2_NSS,SPI2_SCK,SPI2_MOSI,SPI2_MISO},
    {SPI3,/*此处添加SPI3引脚配置*/},
};
//若想修改SPI的IO口的配置，修改下列数据即可
static SPI_IO_ALLConfig_e SPI_IO_ALLConfig[USER_SPI_MAX] = 
{
  { //spi1  使用SPI1读写SD卡
    {DRV_GPIO_Mode_Out_PP, DRV_Speed_HIGH},        //NSS
    {DRV_GPIO_Mode_AF_PP,  DRV_Speed_HIGH},        //SCK
    {DRV_GPIO_Mode_AF_PP,  DRV_Speed_HIGH},        //MOSI
    {DRV_GPIO_Mode_IPU,    DRV_Speed_HIGH},        //MISO
  },
  { //spi2  使用SPI2驱动vs1053
    {DRV_GPIO_Mode_Out_PP, DRV_Speed_HIGH},        //NSS
    {DRV_GPIO_Mode_AF_PP,  DRV_Speed_HIGH},        //SCK
    {DRV_GPIO_Mode_AF_PP,  DRV_Speed_HIGH},        //MOSI
    {DRV_GPIO_Mode_IPU,    DRV_Speed_HIGH},        //MISO
  },
/*  {
        此处添加SPI3-IO口的配置
    }   */
};

static const u32 prescaler_map[DRV_DIV_MAX] = {
  SPI_BaudRatePrescaler_2,
  SPI_BaudRatePrescaler_4,
  SPI_BaudRatePrescaler_8,
  SPI_BaudRatePrescaler_16,
  SPI_BaudRatePrescaler_32,
  SPI_BaudRatePrescaler_64,
  SPI_BaudRatePrescaler_128,
  SPI_BaudRatePrescaler_256,
};
static u32 spi_master_slave_mode[USER_SPI_MAX] = {0}; //按端口存储SPI主从模式，避免不同端口互相覆盖
SPI_InitTypeDef hspi[USER_SPI_MAX]; //SPI初始化配置句柄

/**
 * @brief 打开SPI相关设置
 * @param[in] spi_no     SPI端口号
 * @param[in] mode       DRV_SPI_MODE_MASTER SPI主模式 
 *                       DRV_SPI_MODE_SLAVE  SPI从模式
 * @return 返回执行结果
 * @retval DRV_OK        成功
 * @retval <0            失败原因
 * @attention 本接口只初始化SPI总线接口
 */
void drv_spi_open(user_spi_e spi_no,spi_master_slave_t mode)
{
  if(spi_no >= USER_SPI_MAX)
  {
    return;
  }
//    drv_gpio_open(SPIx_Port_Config[spi_no].SPIx_NSS);
    drv_gpio_open(SPIx_Port_Config[spi_no].SPIx_SCK);
    drv_gpio_open(SPIx_Port_Config[spi_no].SPIx_MOSI);
    drv_gpio_open(SPIx_Port_Config[spi_no].SPIx_MISO);
  if(mode == DRV_SPI_MODE_MASTER)  //SPI主模式
  {
    spi_master_slave_mode[spi_no] = SPI_Mode_Master;
//    drv_gpio_config(SPIx_Port_Config[spi_no].SPIx_NSS,  SPI_IO_ALLConfig[spi_no].SPI_NSS_Config.SPI_IO_mode, \
//                    SPI_IO_ALLConfig[spi_no].SPI_NSS_Config.SPI_IO_speed);

    drv_gpio_config(SPIx_Port_Config[spi_no].SPIx_SCK,  SPI_IO_ALLConfig[spi_no].SPI_SCK_Config.SPI_IO_mode, \
                    SPI_IO_ALLConfig[spi_no].SPI_SCK_Config.SPI_IO_speed);
    
    drv_gpio_config(SPIx_Port_Config[spi_no].SPIx_MOSI, SPI_IO_ALLConfig[spi_no].SPI_MOSI_Config.SPI_IO_mode, \
                    SPI_IO_ALLConfig[spi_no].SPI_MOSI_Config.SPI_IO_speed);

    drv_gpio_config(SPIx_Port_Config[spi_no].SPIx_MISO, SPI_IO_ALLConfig[spi_no].SPI_MISO_Config.SPI_IO_mode, \
                    SPI_IO_ALLConfig[spi_no].SPI_MISO_Config.SPI_IO_speed);
  }
  else if(mode == DRV_SPI_MODE_SLAVE)  //SPI从模式
  {
    spi_master_slave_mode[spi_no] = SPI_Mode_Slave;
    /** 从模式下： MOSI - 浮空输入 MISO - 复用推挽输出
     *            NSS - 浮空输入
     *           SCK - 浮空输入
     */
//    drv_gpio_config(SPIx_Port_Config[spi_no].SPIx_NSS ,DRV_GPIO_Mode_IN_FLOATING,DRV_Speed_HIGH);
    drv_gpio_config(SPIx_Port_Config[spi_no].SPIx_SCK ,DRV_GPIO_Mode_IN_FLOATING,DRV_Speed_HIGH);
    drv_gpio_config(SPIx_Port_Config[spi_no].SPIx_MOSI,DRV_GPIO_Mode_IN_FLOATING,DRV_Speed_HIGH);
    drv_gpio_config(SPIx_Port_Config[spi_no].SPIx_MISO,DRV_GPIO_Mode_AF_PP,DRV_Speed_HIGH);
  }
  switch(spi_no)
  {
      case USER_SPI_1 : RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);  break;
      case USER_SPI_2 : RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);  break;
      case USER_SPI_3 : RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);  break;
      default         : break;
  }
}
/**
 * @brief 获取spi初始化配置的句柄
 * @param[in] spi_no     SPI端口号
 * @return 返回spi初始化配置的句柄
 */
SPI_InitTypeDef drv_get_spi_handlestruct(user_spi_e spi_no)
{
  return hspi[spi_no];
}
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
void drv_spi_init(user_spi_e spi_no, spi_communicate_speed max_khz,spi_clock_mode_e mode, spi_data_width bits)
{
  u32 data_width; //数据宽度 8位/16位
  u32 SPI_CPOL;   //时钟极性
  u32 SPI_CPHA;   //时钟相位
  if(bits == DRV_SPI_DATAWIDTH_8BIT)
  {
      data_width = SPI_DataSize_8b;     //8位数据宽度设置
  }
  else if(bits == DRV_SPI_DATAWIDTH_16BIT)
  {
      data_width = SPI_DataSize_16b;    //16位数据宽度设置
  }
  switch(mode)
  {
      case DRV_SPI_MODE_0: SPI_CPOL = SPI_CPOL_Low;  SPI_CPHA = SPI_CPHA_1Edge; break;
      case DRV_SPI_MODE_1: SPI_CPOL = SPI_CPOL_Low;  SPI_CPHA = SPI_CPHA_2Edge; break;
      case DRV_SPI_MODE_2: SPI_CPOL = SPI_CPOL_High; SPI_CPHA = SPI_CPHA_1Edge; break;
      case DRV_SPI_MODE_3: SPI_CPOL = SPI_CPOL_High; SPI_CPHA = SPI_CPHA_2Edge; break;
      default            : break;
  }
  //SPI传输速率max_khz = PSC_CLK(72MHz) / 预分频值PSC
  hspi[spi_no].SPI_Mode = spi_master_slave_mode[spi_no];        //设置主从模式
  hspi[spi_no].SPI_Direction = SPI_Direction_2Lines_FullDuplex; //设置传输方向-全双工/半双工
  hspi[spi_no].SPI_DataSize = data_width;                       //设置数据长度
  hspi[spi_no].SPI_CPOL = SPI_CPOL;                           //设置时钟极性  上升沿采样还是下降沿采样
  hspi[spi_no].SPI_CPHA = SPI_CPHA;                           //设置时钟相位  第一个时钟采样还是第二个时钟采样
  hspi[spi_no].SPI_NSS = SPI_NSS_Soft;                        //设置片选CS是软件触发还是硬件触发
  hspi[spi_no].SPI_BaudRatePrescaler = prescaler_map[max_khz];//指定用于配置发送和接收SCK时钟的波特率预分频值PSC
  hspi[spi_no].SPI_FirstBit = SPI_FirstBit_MSB;               //指定数据传输是从最高位有效(MSB)还是最低位有效(LSB)开始
  hspi[spi_no].SPI_CRCPolynomial = 12;                        //指定用于 CRC 计算的多项式，必须是奇数
  SPI_Init(SPIx_Port_Config[spi_no].SPIx,&hspi[spi_no]);
  SPI_Cmd(SPIx_Port_Config[spi_no].SPIx,ENABLE);
}
/**
 * @brief 设置NSS/CS引脚的电平
 * @param[in] spi_no    SPI端口号
 * @param[in] value     0为低电平，1为高电平
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败原因
 * @pre 执行drv_spi_open后执行才有效
 */
void drv_spi_w_NSS(user_spi_e spi_no,u8 value)
{
    drv_gpio_write(SPIx_Port_Config[spi_no].SPIx_NSS,value);
}
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
s32 drv_spi_xfer(user_spi_e spi_no, void *write,void *read, u32 len)
{
  if(spi_no >= USER_SPI_MAX || write == NULL || read == NULL || len == 0)
  {
    return DRV_INVAL;
  }  
  SPI_TypeDef* SPIx = SPIx_Port_Config[spi_no].SPIx;
  u8 *p_write = (u8*)write;
  u8 *p_read = (u8*)read;
  u32 i;
  u32 timeout = 0xFFFF;
  for(i = 0; i < len; i++)
  {
    // 等待发送缓冲区空
    while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
    {
      if(--timeout == 0) return DRV_TIMEOUT;
    }
    // 发送数据
    SPI_I2S_SendData(SPIx, p_write[i]);

    // 等待接收缓冲区非空
    timeout = 0xFFFFFF;
    while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)
    {
      if(--timeout == 0) return DRV_TIMEOUT;
    }
    // 读取接收数据
    p_read[i] = SPI_I2S_ReceiveData(SPIx);
  }
  return DRV_OK;
}
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
s32 drv_spi_write(user_spi_e spi_no, void *buf, u32 len)
{
    if(spi_no >= USER_SPI_MAX || buf == NULL || len == 0)
    {
        return DRV_INVAL;
    }
    SPI_TypeDef* SPIx = SPIx_Port_Config[spi_no].SPIx;
    u8 *p_buf = (u8*)buf;
    u32 i;
    u32 timeout = 0xFFFF;
    u8 dummy;
    for(i = 0; i < len; i++)
    {
        // 等待发送缓冲区空
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
        {
            if(--timeout == 0) return DRV_TIMEOUT;
        }
        // 发送数据
        SPI_I2S_SendData(SPIx, p_buf[i]);

        // 等待接收缓冲区非空（丢弃接收到的数据）
        timeout = 0xFFFFFF;
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)
        {
            if(--timeout == 0) return DRV_TIMEOUT;
        }
        dummy = SPI_I2S_ReceiveData(SPIx);
        (void)dummy; // 避免未使用警告
    }
    // 等待传输完成（BSY标志清除）
    timeout = 0xFFFFFF;
    while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) == SET)
    {
        if(--timeout == 0) return DRV_TIMEOUT;
    }
    return (s32)len;
}
/**
 * @brief 收数据
 * @param[in] spi_no    SPI端口号
 * @param[out] buf      缓冲区指针
 * @param[in] len       缓冲区长度
 * @return 返回执行结果
 * @retval >0           接收数据长度
 * @retval =0           数据待接收(非阻塞式)，使用drv_spi_idle查询接收完成情况
 * @retval <0           失败原因
 * @pre 执行drv_spi_open后执行才有效。
 */
s32 drv_spi_read(user_spi_e spi_no, void *buf, u32 len)
{
    if(spi_no >= USER_SPI_MAX || buf == NULL || len == 0)
    {
        return DRV_INVAL;
    }
    SPI_TypeDef* SPIx = SPIx_Port_Config[spi_no].SPIx;
    u8 *p_buf = (u8*)buf;
    u32 i;
    u32 timeout;
    for(i = 0; i < len; i++)
    {
        // 等待发送缓冲区空（发送虚拟数据）
        timeout = 0xFFFFFF;
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
        {
            if(--timeout == 0) return DRV_TIMEOUT;
        }
        // 发送一个虚拟字节（如0xFF）以产生时钟
        SPI_I2S_SendData(SPIx, 0xFF);

        // 等待接收缓冲区非空
        timeout = 0xFFFFFF;
        while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET)
        {
            if(--timeout == 0) return DRV_TIMEOUT;
        }
        // 读取接收数据
        p_buf[i] = SPI_I2S_ReceiveData(SPIx);
    }

    // 等待传输完成（BSY标志清除）
    timeout = 0xFFFFFF;
    while(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) == SET)
    {
        if(--timeout == 0) return DRV_TIMEOUT;
    }
    return (s32)len;
}
/**
 * @brief 查询是否空闲
 * @param[in] spi_no    SPI端口号
 * @return 返回执行结果
 * @retval DRV_OK       处于空闲状态
 * @retval <0           失败原因
 * @pre 执行drv_spi_open后执行才有效。
 */
s32 drv_spi_idle(user_spi_e spi_no)
{
    if(spi_no >= USER_SPI_MAX)
    {
        return DRV_INVAL;
    }
    SPI_TypeDef* SPIx = SPIx_Port_Config[spi_no].SPIx;
    // 检查BSY标志，如果忙则返回BUSY，否则返回OK
    if(SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) != RESET)
    {
        return DRV_BUSY;
    }
    return DRV_OK;
}
