#ifndef __DRV_USART_H
#define __DRV_USART_H

#include "com_type.h"
#include "platform_mcu_resource.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"

typedef enum
{
    USART_MODE_NORMAL    = 0x01,   //不开启中断
    USART_MODE_IT        = 0x02,   //普通中断接收
    USART_MODE_IT_DMA_RX = 0x04,   //DMA中断接收
    USART_MODE_DMA_TX    = 0x08    //DMA发送
}uart_mode_e;

typedef struct
{
    uart_mode_e usart_mode;
    user_gpio_e tx_pin;
    user_gpio_e rx_pin;
    u32 baud_rate;
    u16 data_bits;
    u16 stop_bits;
    u16 parity;
    u16 direction;
    u16 flowcontrol;    //硬件流控制，默认为无流控制
//    u16 bit_order;      //数据位顺序，默认为 LSB 先行
//    u16 oversampling;   //过采样率，默认为16倍，某些芯片支持8倍
}serial_configure_t;

typedef struct {
    DMA_Channel_TypeDef *dma_rx_channel;//DMA接收通道号
    u32 dma_peripheral_baseAddr;//指定DMA通道的外设基地址
    u32 dma_memory_baseAddr;    //指定DMA通道的存储器基地址
    u32 dma_buffersize;         //指定指定通道的缓冲区大小（以数据单位计）
    u32 dma_direction;          //DMA传输方向：外设到存储器或存储器到外设
    u32 dma_periphInc;          //DMA外设地址是否递增
    u32 dma_memInc;             //DMA存储器地址是否递增
    u32 PeriphDataAlignment;    //指定外设数据宽度
    u32 MemDataAlignment;       //指定存储器数据宽度
    u32 Mode;                   //指定DMA模式：普通或循环
    u32 Priority;               //指定DMA优先级：/* 优先级依据波特率与流控：高速或有流控则高优先级 */
    u32 M2M;                    //指定DMA通道是否用于存储器到存储器传输
}dma_rx_config_t;   //dma接收初始化结构体

typedef struct{
    DMA_Channel_TypeDef *dma_tx_channel;//DMA发送通道号
    u32 dma_peripheral_baseAddr;//指定DMA通道的外设基地址
    u32 dma_memory_baseAddr;    //指定DMA通道的存储器基地址
    u32 dma_buffersize;         //指定指定通道的缓冲区大小（以数据单位计）
    u32 dma_direction;          //DMA传输方向：外设到存储器或存储器到外设
    u32 dma_periphInc;          //DMA外设地址是否递增
    u32 dma_memInc;             //DMA存储器地址是否递增
    u32 PeriphDataAlignment;    //指定外设数据宽度
    u32 MemDataAlignment;       //指定存储器数据宽度
    u32 Mode;                   //指定DMA模式：普通或循环
    u32 Priority;               //指定DMA优先级：/* 优先级依据波特率与流控：高速或有流控则高优先级 */
    u32 M2M;                    //指定DMA通道是否用于存储器到存储器传输
}dma_tx_config_t;   //dma发送初始化结构体

typedef struct{
    dma_rx_config_t dma_rx_config;   //DMA接收配置
    dma_tx_config_t dma_tx_config;   //DMA发送配置
}dma_configure_t;

void drv_uart_open(user_uart_e serial);
s32 drv_uart_read(user_uart_e serial, void *buffer, u32 size);
s32 drv_uart_write(user_uart_e serial, const void *buffer, u32 size);

void test_printf_uart1(const char *format, ...);
void test_printf_uart2(const char *format, ...);
void test_printf_uart3(const char *format, ...);
void uart_test(void);
void drv_uart_write_from_hardfault(user_uart_e serial, const char *buffer, u32 size);
#endif
