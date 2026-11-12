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
#include <stdarg.h>
#include "stdio.h"
#include "string.h"
#include "stm32f10x.h"                  // Device header
#include "drv_gpio.h"
#include "drv_usart.h"
#include "com_ringbuffer.h"

/*此文件DMA中断接收暂时还有问题，若需要用串口设备请使用中断接收模式*/
#define USING_USART1 1
#define USING_USART2 0
#define USING_USART3 0

#define USING_DMA_1  1

static uart_mode_e uart_modes[USER_UART_MAX];         //设置一个全局数组 用来保存USART的模式


//设置环形缓存区
/*中断模式,每个串口独立*/
#define IT_RX_BUFFER_SIZE 512
static u8 it_rx_buffer_data[USER_UART_MAX][IT_RX_BUFFER_SIZE];
static ring_buffer_t it_rx_ringbuffers[USER_UART_MAX];
/*DMA模式,每个串口独立*/
#define DMA_RX_BUFFER_SIZE 256
__align(4) static u8 dma_rx_buffer_data[USER_UART_MAX][DMA_RX_BUFFER_SIZE];
static ring_buffer_t dma_rx_ringbuffers[USER_UART_MAX]; //每个串口一个DMA接收环形缓冲区

typedef struct{
  USART_TypeDef* serial_ports_t;       //串口号
  IRQn_Type USART_IRQ_Number;          //串口中断号
  IRQn_Type DMA_Channel_RX_IRQ;        //DMA接收通道中断号
}USART_DMA_NUmber_t;

static const USART_DMA_NUmber_t usart_dma_numbers[USER_UART_MAX] = {
  #if USING_USART1
  {USART1, USART1_IRQn, DMA1_Channel5_IRQn},
  #endif
  #if USING_USART2
  {USART2, USART2_IRQn,DMA1_Channel6_IRQn},
  #endif
  #if USING_USART3
  {USART3, USART3_IRQn,DMA1_Channel3_IRQn},
  #endif
  //根据实际情况添加其他串口的配置
};

//UART初始化配置，若想修改UART参数，修改这个结构体即可
static serial_configure_t serial_config[USER_UART_MAX] = {
   #if USING_USART1
   { 
        .usart_mode   = USART_MODE_IT,          //USART_MODE_IT_DMA_RX|USART_MODE_DMA_TX,USART_MODE_IT,
        .tx_pin       = USER_GPIO_PA9,
        .rx_pin       = USER_GPIO_PA10,
        .baud_rate    = 115200,                   //波特率115200
        .data_bits    = USART_WordLength_8b,      //8位数据位
        .stop_bits    = USART_StopBits_1,         //1位停止位
        .parity       = USART_Parity_No,          //无校验
        .direction    = USART_Mode_Rx|USART_Mode_Tx, //传输模式：收发模式
        .flowcontrol  = USART_HardwareFlowControl_None, //无硬件流控制
   },//usart1配置
   #endif
   #if USING_USART2
   {
        .usart_mode   = USART_MODE_IT,
        .tx_pin       = USER_GPIO_PA9,
        .rx_pin       = USER_GPIO_PA10,
        .baud_rate    = 115200,                   //波特率115200
        .data_bits    = USART_WordLength_8b,      //8位数据位
        .stop_bits    = USART_StopBits_1,         //1位停止位
        .parity       = USART_Parity_No,          //无校验
        .direction    = USART_Mode_Rx|USART_Mode_Rx, //传输模式：收发模式
        .flowcontrol  = USART_HardwareFlowControl_None, //无硬件流控制
   },//usart2配置
   #endif
   #if USING_USART3
   {
   
   },//usart3配置
   #endif
};
/*
  USART1_RX -> DMA1_Channel5, USART1_TX -> DMA1_Channel4
  USART2_RX -> DMA1_Channel6, USART2_TX -> DMA1_Channel7
  USART3_RX -> DMA1_Channel3, USART3_TX -> DMA1_Channel2
  DMA初始化配置，若想修改DMA参数，修改这个结构体数组即可
*/
static const dma_configure_t dma_configure[USER_UART_MAX] = {
  #if USING_USART1 & USING_DMA_1
  {
      {
        .dma_rx_channel          = DMA1_Channel5,
        .dma_memory_baseAddr     = 0,                          // 运行时设置
        .dma_peripheral_baseAddr = 0,                          // 运行时设置
        .dma_buffersize          = DMA_RX_BUFFER_SIZE,         /* DMA接收缓存区大小 */
        .dma_direction           = DMA_DIR_PeripheralSRC,      /* 外设->存储器 */
        .dma_periphInc           = DMA_PeripheralInc_Disable,  /* 外设地址不自增 */
        .dma_memInc              = DMA_MemoryInc_Enable,       /* 内存地址自增 */
        .PeriphDataAlignment     = DMA_PeripheralDataSize_Byte,/* 数据对齐根据 data_bits 配置：>8 位使用 halfword, 否则 byte */
        .MemDataAlignment        = DMA_MemoryDataSize_Byte,/* 数据对齐根据 data_bits 配置：>8 位使用 halfword, 否则 byte */
        .Mode                    = DMA_Mode_Circular,      /* 接收数据使用循环模式 */
        .Priority                = DMA_Priority_Medium,    /* 优先级依据波特率与流控：高速或有流控则高优先级 */
        .M2M                     = DMA_M2M_Disable,
      },
      {
        .dma_tx_channel          = DMA1_Channel4,
        .dma_memory_baseAddr     = 0,                          // 运行时设置
        .dma_peripheral_baseAddr = 0,                          /* 发送时由上层填充 */
        .dma_buffersize          = 0,                          /* 发送时动态设置缓冲区大小 */
        .dma_direction           = DMA_DIR_PeripheralDST,      /* 存储器->外设 */
        .dma_periphInc           = DMA_PeripheralInc_Disable,  /* 外设地址不自增 */
        .dma_memInc              = DMA_MemoryInc_Enable,       /* 内存地址自增 */
        .PeriphDataAlignment     = DMA_PeripheralDataSize_Byte,/* 数据对齐根据 data_bits 配置：>8 位使用 halfword, 否则 byte */
        .MemDataAlignment        = DMA_MemoryDataSize_Byte,/* 数据对齐根据 data_bits 配置：>8 位使用 halfword, 否则 byte */
        .Mode                    = DMA_Mode_Normal,        /* 发送数据使用普通模式 */
        .Priority                = DMA_Priority_Medium,    /* 优先级依据波特率与流控：高速或有流控则高优先级 */
        .M2M                     = DMA_M2M_Disable,
      }
  },
  #endif
  #if USING_USART2 & USING_DMA_1
  {
      {
        .dma_rx_channel          = DMA1_Channel6,
        .dma_memory_baseAddr     = 0,                          // 运行时设置
        .dma_peripheral_baseAddr = 0,                          // 运行时设置
        .dma_buffersize          = DMA_RX_BUFFER_SIZE,         /* DMA接收缓存区大小 */
        .dma_direction           = DMA_DIR_PeripheralSRC,      /* 外设->存储器 */
        .dma_periphInc           = DMA_PeripheralInc_Disable,  /* 外设地址不自增 */
        .dma_memInc              = DMA_MemoryInc_Enable,       /* 内存地址自增 */
        .PeriphDataAlignment     = DMA_PeripheralDataSize_Byte,/* 数据对齐根据 data_bits 配置：>8 位使用 halfword, 否则 byte */
        .MemDataAlignment        = DMA_MemoryDataSize_Byte,/* 数据对齐根据 data_bits 配置：>8 位使用 halfword, 否则 byte */
        .Mode                    = DMA_Mode_Circular,      /* 接收数据使用循环模式 */
        .Priority                = DMA_Priority_Medium,    /* 优先级依据波特率与流控：高速或有流控则高优先级 */
        .M2M                     = DMA_M2M_Disable,
      },
      {
        .dma_tx_channel          = DMA1_Channel7,
        .dma_memory_baseAddr     = 0,                          // 运行时设置
        .dma_peripheral_baseAddr = 0,                          /* 发送时由上层填充 */
        .dma_buffersize          = 0,                          /* 发送时动态设置缓冲区大小 */
        .dma_direction           = DMA_DIR_PeripheralDST,      /* 存储器->外设 */
        .dma_periphInc           = DMA_PeripheralInc_Disable,  /* 外设地址不自增 */
        .dma_memInc              = DMA_MemoryInc_Enable,       /* 内存地址自增 */
        .PeriphDataAlignment     = DMA_PeripheralDataSize_Byte,/* 数据对齐根据 data_bits 配置：>8 位使用 halfword, 否则 byte */
        .MemDataAlignment        = DMA_MemoryDataSize_Byte,/* 数据对齐根据 data_bits 配置：>8 位使用 halfword, 否则 byte */
        .Mode                    = DMA_Mode_Normal,        /* 发送数据使用普通模式 */
        .Priority                = DMA_Priority_Medium,    /* 优先级依据波特率与流控：高速或有流控则高优先级 */
        .M2M                     = DMA_M2M_Disable,
      }
  }
  #endif
  #if USING_USART3 & USING_DMA_1
  {
  
  }
  #endif
};

/**
 * @brief  USART1中断服务函数,在中断读取数据并放入环形缓冲区
 */
void USART1_IRQHandler(void)
{
  //使用 HAL 的中断处理函数，会调用HAL的回调
  #if USING_USART1
  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {
      uint8_t data = USART_ReceiveData(USART1);
      com_ringbuffer_put(&it_rx_ringbuffers[USER_USART_1], &data, 1);
  }
  #endif
}
/**
 * @brief  USART2中断服务函数,在中断读取数据并放入环形缓冲区
 */
void USART2_IRQHandler(void)
{
  #if USING_USART2
  if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
  {
      uint8_t data = USART_ReceiveData(USART2);
      com_ringbuffer_put(&it_rx_ringbuffers[USER_USART_2], &data, 1);
  }
  #endif
}
/**
 * @brief  USART3中断服务函数,在中断读取数据并放入环形缓冲区
 */
void USART3_IRQHandler(void)
{
  #if USING_USART3
  if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
  {
      uint8_t data = USART_ReceiveData(USART3);
      com_ringbuffer_put(&it_rx_ringbuffers[USER_USART_3], &data, 1);
  }
  #endif
}
/**
 * @brief  DMA1_Channel5中断服务函数,在DMA接收完成后调用回调函数处理数据
 */
void DMA1_Channel5_IRQHandler(void)
{
  #if (USING_USART1 & USING_DMA_1)
  if (DMA_GetITStatus(DMA1_IT_TC5) != RESET)
  {
      DMA_ClearITPendingBit(DMA1_IT_TC5);
      //将DMA缓冲区数据放入环形缓冲区（循环模式下，数据一直在更新，此处可做半满/全满处理，这里简化）
      //实际应用中，需要记录当前写入位置，但环形DMA模式下可不用额外处理，因为循环模式会自动覆盖。
      //为了简单，我们不再将数据搬移，而是由上层直接读取 dma_rx_buffer_data。
      //若需使用环形缓冲，可在DMA完成中断中搬运数据，但循环模式下频繁中断无意义。
      //这里留空，由上层直接读取dma_rx_buffer_data（因为DMA会一直刷新）。
  }
  #endif
}
/**
 * @brief  DMA1_Channel6中断服务函数,在DMA接收完成后调用回调函数处理数据
 */
void DMA1_Channel6_IRQHandler(void)
{
  #if (USING_USART2 & USING_DMA_1)
  if (DMA_GetITStatus(DMA1_IT_TC6) != RESET)
  {
      DMA_ClearITPendingBit(DMA1_IT_TC6);
      // 同上
  }
  #endif
}
/**
 * @brief  DMA1_Channel3中断服务函数,在DMA接收完成后调用回调函数处理数据
 */
  #if (USING_USART3 & USING_DMA_1)
void DMA1_Channel3_IRQHandler(void)
{
  if (DMA_GetITStatus(DMA1_IT_TC3) != RESET)
  {
      DMA_ClearITPendingBit(DMA1_IT_TC3);
      // 同上
  }
}
  #endif
/**
  * @brief  usart init.
  * @param  serial_dev:
  * 		@arg USER_USART_1 - USER_USART_3
  * @param  config : 配置USART参数
  * @param  mode   :
  * 		@arg USART_MODE_NORMAL - 普通模式
  * 		@arg USART_MODE_IT	   - 中断接收
  *     @arg USART_MODE_IT_DMA_RX  - DMA中断接收
  *     @arg USART_MODE_DMA_TX     - DMA发送
  */
void drv_uart_init(user_uart_e serial,serial_configure_t *config)
{
  if(serial >= USER_UART_MAX)
  {
    return;
  }
  USART_TypeDef* USARTx = usart_dma_numbers[serial].serial_ports_t;
  uart_modes[serial]    = config[serial].usart_mode;  //保存模式参数

  drv_gpio_open(config->tx_pin);
  drv_gpio_config(config->tx_pin, DRV_GPIO_Mode_AF_PP, DRV_Speed_HIGH);
  drv_gpio_open(config->rx_pin);
  drv_gpio_config(config->rx_pin, DRV_GPIO_Mode_IN_FLOATING, DRV_Speed_HIGH);
    
  switch(serial)
  {
      case USER_USART_1 : RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);  break;
      case USER_USART_2 : RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);  break;
      case USER_USART_3 : RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);  break;
      default           : break;
  }
  #if  USING_DMA_1
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
  #elif USING_DMA_2
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);
  #endif
  
  //初始化UART
  USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_BaudRate   = config->baud_rate;
  USART_InitStructure.USART_Mode       = config->direction;
  USART_InitStructure.USART_WordLength = config->data_bits;
  USART_InitStructure.USART_StopBits   = config->stop_bits;
  USART_InitStructure.USART_Parity     = config->parity;
  USART_InitStructure.USART_HardwareFlowControl = config->flowcontrol;
  USART_Init(USARTx,&USART_InitStructure);
  USART_Cmd (USARTx,ENABLE);
  
  if(config->usart_mode == USART_MODE_IT)   //中断接收时
  {
    com_ringbuffer_init(&it_rx_ringbuffers[serial], it_rx_buffer_data[serial], IT_RX_BUFFER_SIZE);
    //启用对应串口的中断
    NVIC_SetPriority(usart_dma_numbers[serial].USART_IRQ_Number,0);
    NVIC_EnableIRQ(usart_dma_numbers[serial].USART_IRQ_Number);
    //启用接收中断
    USART_ITConfig(USARTx,USART_IT_RXNE,ENABLE);
  }
  else if(config->usart_mode & USART_MODE_IT_DMA_RX)   //使用DMA接收并启用中断
  {
    com_ringbuffer_init(&dma_rx_ringbuffers[serial], dma_rx_buffer_data[serial], DMA_RX_BUFFER_SIZE);
    // 启用对应串口中断
    NVIC_SetPriority(usart_dma_numbers[serial].USART_IRQ_Number,0);
    NVIC_EnableIRQ(usart_dma_numbers[serial].USART_IRQ_Number);
    if(dma_configure[serial].dma_rx_config.dma_rx_channel != NULL)
    {
      //DMA接收通道初始化并链接到USART句柄
      DMA_InitTypeDef    hdma_usart_rx;
      hdma_usart_rx.DMA_MemoryBaseAddr     = (uint32_t)dma_rx_buffer_data[serial];
      hdma_usart_rx.DMA_PeripheralBaseAddr = (uint32_t)&USARTx->DR;
      hdma_usart_rx.DMA_BufferSize         = dma_configure[serial].dma_rx_config.dma_buffersize;
      hdma_usart_rx.DMA_DIR                = dma_configure[serial].dma_rx_config.dma_direction;
      hdma_usart_rx.DMA_PeripheralInc      = dma_configure[serial].dma_rx_config.dma_periphInc;
      hdma_usart_rx.DMA_MemoryInc          = dma_configure[serial].dma_rx_config.dma_memInc;
      hdma_usart_rx.DMA_Mode               = dma_configure[serial].dma_rx_config.Mode;
      hdma_usart_rx.DMA_Priority           = dma_configure[serial].dma_rx_config.Priority;
      hdma_usart_rx.DMA_PeripheralDataSize = dma_configure[serial].dma_rx_config.PeriphDataAlignment;
      hdma_usart_rx.DMA_MemoryDataSize     = dma_configure[serial].dma_rx_config.MemDataAlignment;
      hdma_usart_rx.DMA_M2M                = dma_configure[serial].dma_rx_config.M2M;
      DMA_Init(dma_configure[serial].dma_rx_config.dma_rx_channel,&hdma_usart_rx);
      DMA_Cmd(dma_configure[serial].dma_rx_config.dma_rx_channel,ENABLE);
      USART_DMACmd(USARTx,USART_DMAReq_Rx,ENABLE);//使USART链接DMA RX通道
      
      //启用DMA接收中断
      NVIC_SetPriority(usart_dma_numbers[serial].DMA_Channel_RX_IRQ,0);
      NVIC_EnableIRQ(usart_dma_numbers[serial].DMA_Channel_RX_IRQ);
      DMA_ITConfig(dma_configure[serial].dma_rx_config.dma_rx_channel,DMA_IT_TC,ENABLE);
    }
        //如果需要 TX DMA，则初始化TX通道并链接
    else if((config->usart_mode & USART_MODE_DMA_TX) && dma_configure[serial].dma_tx_config.dma_tx_channel != NULL)
    {
        DMA_InitTypeDef    hdma_usart_tx;
        hdma_usart_tx.DMA_MemoryBaseAddr     = 0;
        hdma_usart_tx.DMA_PeripheralBaseAddr = (uint32_t)&USARTx->DR;
        hdma_usart_tx.DMA_BufferSize         = 1;
        hdma_usart_tx.DMA_DIR                = dma_configure[serial].dma_tx_config.dma_direction;
        hdma_usart_tx.DMA_PeripheralInc      = dma_configure[serial].dma_tx_config.dma_periphInc;
        hdma_usart_tx.DMA_MemoryInc          = dma_configure[serial].dma_tx_config.dma_memInc;
        hdma_usart_tx.DMA_Mode               = dma_configure[serial].dma_tx_config.Mode;
        hdma_usart_tx.DMA_Priority           = dma_configure[serial].dma_tx_config.Priority;
        hdma_usart_tx.DMA_PeripheralDataSize = dma_configure[serial].dma_tx_config.PeriphDataAlignment;
        hdma_usart_tx.DMA_MemoryDataSize     = dma_configure[serial].dma_tx_config.MemDataAlignment;
        hdma_usart_tx.DMA_M2M                = dma_configure[serial].dma_tx_config.M2M;
        DMA_Init(dma_configure[serial].dma_tx_config.dma_tx_channel,&hdma_usart_tx);
        DMA_Cmd(dma_configure[serial].dma_tx_config.dma_tx_channel,ENABLE);
        USART_DMACmd(USARTx,USART_DMAReq_Tx,ENABLE);//使USART链接DMA RX通道
    }
  }
  else if(config->usart_mode == USART_MODE_NORMAL)
  {
    NVIC_DisableIRQ(usart_dma_numbers[serial].USART_IRQ_Number);
  }
}
/**
  * @brief  使用默认配置开启USART.
  *         若要对外开启USART，请调用此函数
  * @param  serial_dev:
  * 		USER_UART_1 - USER_UART_3
  */
void drv_uart_open(user_uart_e serial)
{
  drv_uart_init(serial,&serial_config[serial]);
}
/**
* @brief从串行设备读取数据
* @param serial_dev 要读取的串行设备
* @param buffer 用于存储读取数据的缓冲区指针
* @param size 读取的字节数
* @return s32读取的字节数，失败时为负值
*/
s32 drv_uart_read(user_uart_e serial,void* buffer,u32 size)
{
    // 判断参数
    if( serial >= USER_UART_MAX || buffer == NULL || size == 0 )
    {
      return DRV_ERROR;
    }
    //中断模式
    if(uart_modes[serial] & USART_MODE_IT)
    {
      if(it_rx_ringbuffers[serial].buffer != NULL)
      {
        u8 *data_ptr = (u8*)buffer;
        return com_ringbuffer_get(&it_rx_ringbuffers[serial], data_ptr, size);
      }
    }
    //DMA接收中断模式
    else if(uart_modes[serial] & USART_MODE_IT_DMA_RX)
    {
      if(dma_rx_ringbuffers[serial].buffer != NULL)
      {
        u8 *data_ptr = (u8*)buffer;
        // 直接从DMA缓冲区读取数据（环形）
        // 注意：DMA循环模式一直更新，需要记录读指针，这里用环形缓冲管理
        return com_ringbuffer_get(&dma_rx_ringbuffers[serial], data_ptr, size);
      }
    }
    //轮询模式：使用阻塞接收
    if(uart_modes[serial] == USART_MODE_NORMAL)
    {
      /*轮询模式*/
      return size;
    }
    return -1;  //不支持的串口设备
}
/**
 * @brief向串口设备写入数据
 * @param serial_dev 要写入的串行设备
 * @param buffer 要写的数据指针
 * @param size 写入的字节数
 * @return s32 写入的字节数,失败时为负值
 */
s32 drv_uart_write(user_uart_e serial, const void *buffer, u32 size)
{
   if (serial >= USER_UART_MAX || buffer == NULL || size == 0) 
    {
        return -1;
    }
    if (uart_modes[serial] & USART_MODE_DMA_TX)  //启用DMA发送时
    {
        DMA_Channel_TypeDef* dma_ch = dma_configure[serial].dma_tx_config.dma_tx_channel;
        if (dma_ch == NULL)
            return -1;
        
        //根据DMA通道确定对应的传输完成标志
        uint32_t flag_tc = 0;
        if (dma_ch == DMA1_Channel2)      flag_tc = DMA1_FLAG_TC2;
        else if (dma_ch == DMA1_Channel3) flag_tc = DMA1_FLAG_TC3;
        else if (dma_ch == DMA1_Channel4) flag_tc = DMA1_FLAG_TC4;
        else if (dma_ch == DMA1_Channel5) flag_tc = DMA1_FLAG_TC5;
        else if (dma_ch == DMA1_Channel6) flag_tc = DMA1_FLAG_TC6;
        else if (dma_ch == DMA1_Channel7) flag_tc = DMA1_FLAG_TC7;

        DMA_Cmd(dma_ch, DISABLE);//禁用DMA通道
        dma_ch->CMAR = (uint32_t)buffer;//设置内存地址
        dma_ch->CNDTR = size;//设置传输数量
        DMA_Cmd(dma_ch, ENABLE);//重新使能DMA通道

        //等待传输完成
        if (flag_tc)
        {
            while (DMA_GetFlagStatus(flag_tc) == RESET);
            DMA_ClearFlag(flag_tc);
        }
        return (s32)size;
    }
    else  //普通发送（轮询）
    {
        USART_TypeDef* USARTx = usart_dma_numbers[serial].serial_ports_t;
        const u8 *p = (const u8*)buffer;
        u32 sent = 0;
        
        while (sent < size)
        {
            // 等待发送缓冲区空
            while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
            USART_SendData(USARTx, *p++);
            sent++;
        }
        // 等待发送完成
        while (USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET);
        return (s32)size;
    }
}

#if HAL_or_SPL
//每串口IT模式用的单字节接收缓冲
static uint8_t it_rx_byte[USER_UART_MAX];

/**
 * 回调：接收完成（此处用于DMA或IT完成）
 * 此回调函数在HAL_UART_IRQHandler()或DMA1_Channel5_IRQHandler()完成后会自动调用
 * 此函数用于兼容原HAL回调，SPL库未使用，作为保留
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  // 找到对应串口索引
  for(int i=0;i<USER_UART_MAX;i++)
  {
    if(huart == &huarts[i])
    {
      if(uart_modes[i] & USART_MODE_IT)
      {
        //IT 模式：把单字节放入对应串口的环形缓冲，并重新启动IT接收
        com_ringbuffer_put(&it_rx_ringbuffers[i], &it_rx_byte[i], 1);
        HAL_UART_Receive_IT(&huarts[i], &it_rx_byte[i], 1);
      }
      else if(uart_modes[i] & USART_MODE_IT_DMA_RX)
      {
        //DMA 模式：把DMA 缓冲区的数据拷贝到对应的DMA环形缓冲区
        com_ringbuffer_put(&dma_rx_ringbuffers[i], dma_rx_buffer_data[i], DMA_RX_BUFFER_SIZE);
        //对于循环模式，通常不会每次触发回调，若使用完成/半完成回调请根据需要处理
      }
      break;
    }
  }
}

/* HardFault 下安全的直接寄存器写入发送（阻塞轮询 TXE/TC） */
void drv_uart_write_from_hardfault(user_uart_e serial, const char *buffer, u32 size)
{
  if (serial >= USER_UART_MAX || buffer == NULL || size == 0)
    return;

  USART_TypeDef *inst = huarts[serial].Instance;
  if (inst == NULL)
    return;

  for (u32 i = 0; i < size; i++)
  {
    /* 等待 TXE */
    while (!(inst->SR & USART_SR_TXE))
      ;
    inst->DR = (uint8_t)buffer[i];
  }
  /* 等待发送完成 TC */
  while (!(inst->SR & USART_SR_TC))
    ;
}
void HAL_USART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  //可在此处理发送完成事件
}
void HAL_USART_ErrorCallback(UART_HandleTypeDef *huart)
{
  //错误处理（可选）
}
#endif

void test_printf_uart1(const char *format, ...)
{
    char buffer[256];                         // 输出缓冲区，可根据需要调整大小
    va_list args;                             // 可变参数列表
    va_start(args, format);                   // 初始化参数列表
    vsnprintf(buffer, sizeof(buffer), format, args);  // 格式化字符串到缓冲区
    va_end(args);                             // 结束可变参数处理
    drv_uart_write(USER_USART_1, (u8*)buffer, strlen(buffer)); // 发送到串口
}
void test_printf_uart2(const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    drv_uart_write(USER_USART_2, (u8*)buffer, strlen(buffer));
}
void test_printf_uart3(const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    drv_uart_write(USER_USART_3, (u8*)buffer, strlen(buffer));
}

#define UASRT_TEST_  1
#if UASRT_TEST_

#define UART_TEST_NO USER_USART_1

void uart_test(void)
{
    static u8 flag_init = 0;
    static u8 test_str1[] = "\r\nhello world!";
    if(flag_init == 0)
    {
        flag_init = 1;
        drv_uart_open(UART_TEST_NO);
    }
    drv_uart_write(UART_TEST_NO, test_str1, sizeof(test_str1)-1);

    while(1)
    {
        u8 test_str[256]={0};
        s32 len = drv_uart_read(UART_TEST_NO, test_str, sizeof(test_str));
        if(len>0)
        {
            drv_uart_write(UART_TEST_NO, test_str, len);
        }
    }
}
#endif
