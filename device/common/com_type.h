/**
 * @file com_type.h
 * @details 
 * @version 1.0
 * @date 2026-03-11
 */
#ifndef _COM_TYPE_H_
#define _COM_TYPE_H_

#define SDCARD_SPI_OR_SDIO 1    //1为使用SPI读写SD，0为使用SDIO读写SD
#define HAL_or_SPL         0    //1为使用HAL库，0为使用SPL库

typedef unsigned char u8;
typedef signed   char s8;

typedef unsigned short u16;
typedef short s16;

typedef unsigned int u32;
typedef int s32;

//drv层设备是否打开
#define DRV_STATUS_OPEN     1
#define DRV_STATUS_CLOSE    0

//drv层 函数返回值类型
#define DRV_OK               0   //成功
#define DRV_ERROR           -1   //错误
#define DRV_TIMEOUT         -2   //超时
#define DRV_BUSY            -3   //繁忙
#define DRV_INVAL           -4   //参数无效
#define DRV_DATA_ERR        -5   //数据错误
#define DRV_PROTOCOL_ERR    -6   //协议错误
#define DRV_TRANSPORT_ERR   -7   //传输错误
#define DRV_NO_MEMORY       -8   //没有内存
#define DRV_NO_SUPPORTED    -9   //暂不支持

//dev层 函数返回值类型
#define DEV_OK               0   //成功
#define DEV_ERROR           -1   //错误
#define DEV_TIMEOUT         -2   //超时
#define DEV_BUSY            -3   //繁忙
#define DEV_INVAL           -4   //参数无效
#define DEV_DATA_ERR        -5   //数据错误
#define DEV_PROTOCOL_ERR    -6   //协议错误
#define DEV_TRANSPORT_ERR   -7   //传输错误
#define DEV_NO_MEMORY       -8   //没有内存
#define DEV_NO_SUPPORTED    -9   //暂不支持

#undef NULL
#if defined (__cplusplus)
#define NULL 0
#else
#define NULL ((void *)0)
#endif

#undef  TRUE
#define TRUE  1
#undef  FALSE
#define FALSE 0

typedef void(*user_irq_callback)(void *arg);

#endif
