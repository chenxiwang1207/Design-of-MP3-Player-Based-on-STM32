#include "stdio.h"
#include "string.h"
#include "stm32f10x.h"                  // Device header
#include "app_music_application.h"
#include "spi_vs1053.h"
#include "app_display_menu.h"
#include "drv_spi.h"
#include "drv_sys.h"
#include "drv_gpio.h"
#include "drv_usart.h"
#include "drv_wdt.h"
#include "ff.h"
#include "fatfs.h"


/**SPI1 GPIO Configuration
PA5     ------> SPI1_SCK   //复用推挽
PA6     ------> SPI1_MISO  //上拉输入
PA7     ------> SPI1_MOSI  //复用推挽
*/
#define SPI1_SCK  USER_GPIO_PA5
#define SPI1_MISO USER_GPIO_PA6
#define SPI1_MOSI USER_GPIO_PA7
/*
PA8      ------> VS_XCS   //推挽输出
PA4      ------> VS_XDCS  //推挽输出
PA12     ------> VS_DREQ  //上拉输入
PA11     ------> VS_RST   //推挽输出
*/
#define VS_XCS      USER_GPIO_PA8   //命令片选，低电平有效
#define VS_XDCS     USER_GPIO_PA4   //数据片选，低电平有效
#define VS_DREQ     USER_GPIO_PA12  //数据请求，高电平表示VS1053可接收数据
#define VS_XRESET   USER_GPIO_PA11  //复位，低电平复位，需拉高工作

#define VS1053_SCIVOL_ADDR  0x0B  //VS1053寄存器地址，0x0B是SCI_VOL寄存器地址，用于设置音量
#define VS1053_CLOCK_ADDR   0x03  //VS1053寄存器地址，0x03是SCI_CLOCKF寄存器地址，用于设置时钟频率
#define VS1053_AUDIO_ADDR   0x05  //VS1053寄存器地址，0x05是SCI_AUDATA寄存器地址，用于设置音频数据格式
#define VS1053_MODE_ADDR    0x00  //VS1053寄存器地址，0x00是SCI_MODE寄存器地址，用于设置工作模式

#define ENABLE_ITDMA_SEND    1    //是否启用DMA发送数据，0表示不启用，1表示启用

static vs1053_volume_t vs1053_volume = {
    
    .left_volume    = 0x50,
    .right_volume   = 0x50,
};

#if ENABLE_ITDMA_SEND
volatile uint8_t dma_tx_complete = 0;   //DMA传输完成标志,1表示传输完成
static uint8_t dma_rx_dummy[256];//DMA接收用的临时缓冲区，防止接收数据造成溢出（OVR）

void DMA1_Channel3_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC3))
    {
        
        DMA_ClearITPendingBit(DMA1_IT_TC3);//清中断标志，记下完成并停止 DMA 通道（也停止 RX 通道以免残留）
        DMA_ClearFlag(DMA1_FLAG_TC3);
        dma_tx_complete = 1;
        DMA_Cmd(DMA1_Channel3, DISABLE);
        DMA_Cmd(DMA1_Channel2, DISABLE);
    }
}
#endif

//写XCS 0-低电平 1-高电平
void vs1053_w_XCS(u8 value)
{
    drv_gpio_write(VS_XCS,value);
}

//写XDCS 0-低电平 1高电平
void vs1053_w_XDCS(u8 value)
{
    drv_gpio_write(VS_XDCS,value);
}

/*写XRESET 0-低电平 1-高电平
 *低电平复位，高电平vs1053工作
 */
void vs1053_w_XRESET(u8 value)
{
    drv_gpio_write(VS_XRESET,value);
}

//读DREQ 0-低电平 1高电平
u8 vs1053_r_DREQ(void)
{
    return drv_gpio_read_in(VS_DREQ);
}

//72M主频的简易延时
static void delay_ms(u32 ms)
{
    for(u32 i=0;i<ms;i++)
        for(u32 j=0;j<7200;j++);
}

//使用SPI2驱动VS1053的初始化
void vs1053_spi_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    
/*-----------VS1053-GPIO-Init-----------*/ 
    drv_gpio_config(VS_XCS,   DRV_GPIO_Mode_Out_PP, DRV_Speed_HIGH);
    drv_gpio_config(VS_XDCS,  DRV_GPIO_Mode_Out_PP, DRV_Speed_HIGH);
    drv_gpio_config(VS_DREQ,  DRV_GPIO_Mode_IPU,    DRV_Speed_HIGH);
    drv_gpio_config(VS_XRESET,DRV_GPIO_Mode_Out_PP, DRV_Speed_HIGH);
    
/*-------------SPI-GPIO-Init------------*/
    drv_gpio_config(SPI1_SCK, DRV_GPIO_Mode_AF_PP,DRV_Speed_HIGH);
    drv_gpio_config(SPI1_MOSI,DRV_GPIO_Mode_AF_PP,DRV_Speed_HIGH);
    drv_gpio_config(SPI1_MISO,DRV_GPIO_Mode_IPU,  DRV_Speed_HIGH);
    
    vs1053_w_XCS(1);
    vs1053_w_XDCS(1);
    vs1053_w_XRESET(1);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_Mode      = SPI_Mode_Master;                //设置主从模式
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//设置传输方向-全双工/半双工
    SPI_InitStructure.SPI_DataSize  = SPI_DataSize_8b;                //设置数据长度
    SPI_InitStructure.SPI_CPOL      = SPI_CPOL_High;                  //设置时钟极性  上升沿采样还是下降沿采样
    SPI_InitStructure.SPI_CPHA      = SPI_CPHA_2Edge;                 //设置时钟相位  第一个时钟采样还是第二个时钟采样
    SPI_InitStructure.SPI_NSS       = SPI_NSS_Soft;                   //设置片选CS是软件触发还是硬件触发
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; //指定用于配置发送和接收SCK时钟的波特率预分频值PSC
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;         //指定数据传输是从最高位有效(MSB)还是最低位有效(LSB)开始
    SPI_InitStructure.SPI_CRCPolynomial     = 12;                       //指定用于 CRC 计算的多项式，必须是奇数，偶数代表着CRC功能不使用
    SPI_Init(SPI1,&SPI_InitStructure);
    SPI_Cmd(SPI1,ENABLE);

#if ENABLE_ITDMA_SEND
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
    DMA_InitTypeDef DMA1_InitStructure;
    /* 先复位通道以避免遗留配置 */
    DMA_DeInit(DMA1_Channel3);
    DMA_DeInit(DMA1_Channel2);

    /* 配置 TX 通道 (Channel3): memory -> peripheral */
    DMA1_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI1->DR;
    DMA1_InitStructure.DMA_MemoryBaseAddr     = 0; /* 动态设置 */
    DMA1_InitStructure.DMA_BufferSize         = 0; /* 动态设置 */
    DMA1_InitStructure.DMA_DIR                = DMA_DIR_PeripheralDST;
    DMA1_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    DMA1_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    DMA1_InitStructure.DMA_Mode               = DMA_Mode_Normal;
    DMA1_InitStructure.DMA_Priority           = DMA_Priority_High;
    DMA1_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA1_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    DMA1_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3,&DMA1_InitStructure);

    /* 配置 RX 通道 (Channel2): peripheral -> memory，用于丢弃收到的数据防止 OVR */
    DMA1_InitStructure.DMA_MemoryBaseAddr = 0; /* 动态设置 */
    DMA1_InitStructure.DMA_BufferSize = 0; /* 动态设置 */
    DMA1_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; /* 外设->内存 */
    DMA1_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA1_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA1_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_Init(DMA1_Channel2, &DMA1_InitStructure);

    /* 启用DMA发送中断 (只对 TX 通道) */
    NVIC_SetPriority(DMA1_Channel3_IRQn,0);
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);
    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);
#endif
}

//提高VS1053的读写速度
void vs1053_spi_highSpeed(void)
{
    SPI_InitTypeDef SPI_InitStructure;
    SPI_InitStructure.SPI_Mode      = SPI_Mode_Master;                //设置主从模式
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//设置传输方向-全双工/半双工
    SPI_InitStructure.SPI_DataSize  = SPI_DataSize_8b;                //设置数据长度
    SPI_InitStructure.SPI_CPOL      = SPI_CPOL_High;                  //设置时钟极性  上升沿采样还是下降沿采样
    SPI_InitStructure.SPI_CPHA      = SPI_CPHA_2Edge;                 //设置时钟相位  第一个时钟采样还是第二个时钟采样
    SPI_InitStructure.SPI_NSS       = SPI_NSS_Soft;                   //设置片选CS是软件触发还是硬件触发
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//指定用于配置发送和接收SCK时钟的波特率预分频值PSC
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;       //指定数据传输是从最高位有效(MSB)还是最低位有效(LSB)开始
    SPI_InitStructure.SPI_CRCPolynomial     = 12;                     //指定用于 CRC 计算的多项式，必须是奇数
    SPI_Init(SPI1,&SPI_InitStructure);
    SPI_Cmd(SPI1,ENABLE);
}

//使用SPI2读写一个字节
u8 spi1_readwriteByte(u8 TxData)
{
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, TxData);
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI1);
}

#if ENABLE_ITDMA_SEND
/**
 * @brief       使用DMA发送一次音频数据
 *   @note      固定为32字节长度
 * @param       buf     : 音频数据缓冲区首地址
 */
uint8_t vs1053_write_data(uint8_t* buf)
{
    uint32_t timeout;

    /* 等待 VS1053 准备好 */
    while (vs1053_r_DREQ() == 0)
    {
        drv_wdt_feed();
    }

    /* 进入数据传输，先拉低 XDCS */
    vs1053_w_XDCS(0);

    /* 停止并清理通道，防止残留 */
    DMA_Cmd(DMA1_Channel3, DISABLE);
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_TC3);
    DMA_ClearFlag(DMA1_FLAG_GL3);
    DMA_ClearFlag(DMA1_FLAG_TC2);
    DMA_ClearFlag(DMA1_FLAG_GL2);
    dma_tx_complete = 0;

    /* 配置 RX DMA（外设->内存），避免 OVR */
    DMA1_Channel2->CMAR = (uint32_t)dma_rx_dummy;
    DMA1_Channel2->CNDTR = 32;

    /* 配置 TX DMA（内存->外设） */
    DMA1_Channel3->CMAR = (uint32_t)buf;
    DMA1_Channel3->CNDTR = 32;

    /* 启动接收通道再启动发送通道，确保接收数据被 DMA 吸走 */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, ENABLE);
    DMA_Cmd(DMA1_Channel2, ENABLE);

    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
    DMA_Cmd(DMA1_Channel3, ENABLE);

    /* 等待 DMA 完成，带超时并喂狗，避免死循环 */
    timeout = 0x20000;
    while (!dma_tx_complete && timeout--)
    {
        drv_wdt_feed();
    }

    if (!dma_tx_complete)
    {
        /* 超时处理：关闭 DMA、取消片选 */
        DMA_Cmd(DMA1_Channel3, DISABLE);
        DMA_Cmd(DMA1_Channel2, DISABLE);
        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
        SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);
        vs1053_w_XDCS(1);
        return 1; /* 错误 */
    }

    /* 等待 SPI 总线完全空闲（BSY=0），继续喂狗 */
    timeout = 0x20000;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET && timeout--)
    {
        drv_wdt_feed();
    }

    /* 停止 DMA 请求并清理通道 */
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, DISABLE);
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Rx, DISABLE);
    DMA_Cmd(DMA1_Channel3, DISABLE);
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA_ClearFlag(DMA1_FLAG_TC3);
    DMA_ClearFlag(DMA1_FLAG_TC2);

    vs1053_w_XDCS(1);
    return 0;
}
#else

/**
 * @brief       发送一次音频数据
 *   @note      固定为32字节长度
 * @param       buf     : 音频数据缓冲区首地址
 */
uint8_t vs1053_write_data(uint8_t* buf)
{
    do
    {
//      test_printf_uart1("\r\nDREQ:%d",vs1053_r_DREQ());
        drv_wdt_feed();
    }while (vs1053_r_DREQ() == 0);
    
    vs1053_w_XDCS(0);
    for(u32 i=0;i<32;i++)
    {
        spi1_readwriteByte(buf[i]);
    }
    vs1053_w_XDCS(1);
    return 0;
}
#endif

//VS1053-XRESET硬件复位
void vs1053_hard_XRESET(void)
{
    vs1053_w_XRESET(0);
    delay_ms(20);
    vs1053_w_XDCS(1);
    vs1053_w_XCS(1);
    vs1053_w_XRESET(1);
    
    while(vs1053_r_DREQ() == 0){}
    test_printf_uart1("\r\n hard reset success");
    delay_ms(20);
}

//VS1053-XRESET软件复位
void vs1053_soft_XRESET(void)
{
    while(vs1053_r_DREQ() == 0){}
    spi1_readwriteByte(0xFF);
    while(vs1053_read_regsiter(VS1053_MODE_ADDR) != 0x0800)
    {
        vs1053_write_regsiter(VS1053_MODE_ADDR,0x0804);
        delay_ms(20);
    }
    while(vs1053_r_DREQ() == 0){}
    test_printf_uart1("\r\n soft reset success");
}

//vs1053写寄存器，reg_addr寄存器地址，data要写入的数据
//用来控制VS1053的各种功能，比如设置音量、选择解码模式等
void vs1053_write_regsiter(u8 reg_addr,u16 data)
{
    while(vs1053_r_DREQ() == 0);
    vs1053_w_XDCS(1);
    vs1053_w_XCS(0);   //命令片选拉低，选中VS1053的命令寄存器
    spi1_readwriteByte(0x02);
    spi1_readwriteByte(reg_addr);
    spi1_readwriteByte(data >> 8);   //先写高8位
    spi1_readwriteByte(data & 0xFF); //再写低8位
    vs1053_w_XCS(1);   //命令片选拉高，结束对VS1053命令寄存器的访问
    vs1053_spi_highSpeed();
}

//vs1053读寄存器，reg_addr寄存器地址，返回值为读取到的数据
//用来获取VS1053的状态信息，比如当前解码模式、音量等
u16 vs1053_read_regsiter(u8 reg_addr)
{
    u16 data;
    while(vs1053_r_DREQ() == 0);
    vs1053_w_XDCS(1);
    vs1053_w_XCS(0);    //命令片选拉低，选中VS1053的命令寄存器
    spi1_readwriteByte(0x03);
    spi1_readwriteByte(reg_addr);
    data  = spi1_readwriteByte(0xFF) << 8;   //先读高8位
    data |= spi1_readwriteByte(0xFF);        //再读低8
    vs1053_w_XCS(1);    //命令片选拉高，结束对VS1053命令寄存器的访问
    vs1053_spi_highSpeed();
    return data;
}

//设置VS1053的音量，left为左声道音量，right为右声道音量，范围0-255，数值越大音量越小
void vs1053_set_volume(u8 left,u8 right)
{
    vs1053_write_regsiter(VS1053_SCIVOL_ADDR,(left << 8)| right);
}

/**
 * @brief       设置播放速度
 *   @note      该函数仅对 VS1053 有效
 * @param       speed   : 播放倍速
 *   @arg       0/1     : 正常速度
 *   @arg       其他    : speed倍
 * @retval      无
 */
void vs1053_set_speed(u8 speed)
{
    vs1053_write_regsiter(0x07,0x1E04);
    while(vs1053_r_DREQ() == 0);
    vs1053_write_regsiter(0x06,speed);
}

/**
 * @brief       重设解码时间
 * @param       无
 * @retval      无
 */
void vs1053_reset_decode_time(void)
{
    vs1053_write_regsiter(0x04, 0x0000);
    vs1053_write_regsiter(0x04, 0x0000); /* 操作两次 */
}

/**
 * @brief       vs1053 写RAM
 * @param       address : RAM地址
 * @param       data    : 要写入的值
 * @retval      无
 */
void vs1053_write_ram(u16 addr,u16 data)
{
    vs1053_write_regsiter(0x07,addr);/* 写RAM地址 */
    while (vs1053_r_DREQ() == 0);    /* 等待空闲 */
    vs1053_write_regsiter(0x06,data);/* 写RAM值 */
}

/**
 * @brief       vs1053 读RAM
 * @param       address : RAM地址
 * @retval      读取到的数据
 */
u16 vs1053_read_ram(u16 address)
{
    uint16_t res;
    vs1053_write_regsiter(SPI_WRAMADDR, address);
    res = vs1053_read_regsiter(SPI_WRAM);
    return res;
}

/**
 * @brief       板载喇叭开/关设置函数
 *   @note      战舰开发板 板载了HT6872功放, 通过ATK_MO1053 的GPIO4(36脚), 控制其工作/关闭
 *              该函数实际上是VS1053的GPIO设置函数
 *              GPIO4 = 1, HT6872正常工作
 *              GPIO4 = 0, HT6872关闭(默认)
 * @param       sw      : 0, 关闭; 1, 开启;
 * @retval      无
 */
void vs1053_set_speaker(u8 sw)
{
    vs1053_write_ram(0xC017,1 << 4);
    vs1053_write_ram(0xC019, sw << 4); /* 控制ATK_MO1053的GPIO4输出值(0/1) */
}

/**
 * @brief   重启播放
 */
void vs1053_restart_play(void)
{
    uint16_t temp;
    uint16_t i;
    uint8_t n;
    uint8_t vsbuf[32] = {0};

    temp = vs1053_read_regsiter(SPI_MODE);           /* 读取SPI_MODE的内容 */
    temp |= 1 << 3;                                  /* 设置SM_CANCEL位 */
    temp |= 1 << 2;                                  /* 设置SM_LAYER12位,允许播放MP1,MP2 */
    vs1053_write_regsiter(SPI_MODE, temp);           /* 设置取消当前解码指令 */
    
    for (i = 0; i < 2048;)                           /* 发送2048个0,期间读取SM_CANCEL位.如果为0,则表示已经取消了当前解码 */
    {
        if (vs1053_write_data(vsbuf) == 0) /* 每发送32个字节后检测一次 */
        {
            i += 32;                                 /* 发送了32个字节 */
            temp = vs1053_read_regsiter(SPI_MODE);   /* 读取SPI_MODE的内容 */

            if ((temp & (1 << 3)) == 0)
            {
                break;                               /* 成功取消了 */
            }
        }
    }
    
    if (i < 2048)                                    /* SM_CANCEL正常 */
    {
        temp = vs1053_read_ram(0x1E06) & 0xff;       /* 读取填充字节 */

        for (n = 0; n < 32; n++)
        {
            vsbuf[n] = temp;                         /* 填充字节放入数组 */
        }

        for (i = 0; i < 2052;)
        {
            if (vs1053_write_data(vsbuf) == 0)
            {
                i += 32;                             /* 填充 */
            }
        }
    }
    else
    {
        vs1053_soft_XRESET();                        /* SM_CANCEL不成功,坏情况,需要软复位 */
    }
    temp = vs1053_read_regsiter(SPI_HDAT0);
    temp += vs1053_read_regsiter(SPI_HDAT1);
    
    if (temp)                                        /* 软复位,还是没有成功取消,放杀手锏,硬复位 */
    {
        vs1053_hard_XRESET();                        /* 硬复位 */
        vs1053_soft_XRESET();                        /* 软复位 */
    }
}

//设置音量，时钟频率，立体声等信息
void vs1053_set_all(void)
{
    vs1053_write_regsiter(VS1053_CLOCK_ADDR,0x9800);   //设置时钟频率，0x9800表示使用外部晶振，倍频为3.5倍，时钟频率为12MHz*3.5=42MHz，满足VS1053的工作要求
    vs1053_write_regsiter(VS1053_AUDIO_ADDR,0xB000);   //设置音频数据格式，0xB000表示16位立体声，采样率为44.1kHz
    vs1053_write_regsiter(VS1053_MODE_ADDR,0x0800);    //设置工作模式，0x0800表示进入解码模式
    /* 启用板载扬声器/外放（若硬件有控制）并把音量调大，排查放大器未打开或音量过低导致无声 */
    vs1053_set_speaker(1);
    vs1053_set_volume(vs1053_volume.left_volume,vs1053_volume.right_volume); //设置初始音量，0x10表示左声道和右声道的音量都为16，范围0-255，数值越大音量越小
}

/**
 * @brief       vs1053 正弦测试
 * @param       无
 * @retval      无
 */
void vs1053_sin_test(void)
{
    vs1053_hard_XRESET();
    vs1053_write_regsiter(0x0b, 0X2020);     /* 设置音量 */
    vs1053_write_regsiter(SPI_MODE, 0x0820); /* 进入ATK_MO1053 的测试模式 */

    while (vs1053_r_DREQ() == 0);            /* 等待DREQ为高 */
                                             /* printf("mode sin:%x\n", vs10xx_read_reg(SPI_MODE));
                                              * 向VS10XX发送正弦测试命令：0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
                                              * 其中n = 0x24, 设定VS10XX所产生的正弦波的频率值，具体计算方法见VS10XX的datasheet
                                              */
    vs1053_spi_init();                       /* 低速 */    
    vs1053_w_XDCS(0);                        /* 选中数据传输 */
    spi1_readwriteByte(0x53);
    spi1_readwriteByte(0xef);
    spi1_readwriteByte(0x6e);
    spi1_readwriteByte(0x24);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    delay_ms(100);
    vs1053_w_XDCS(1);
    /* 退出正弦测试 */
    vs1053_w_XDCS(0);                         /* 选中数据传输 */
    spi1_readwriteByte(0x45);
    spi1_readwriteByte(0x78);
    spi1_readwriteByte(0x69);
    spi1_readwriteByte(0x74);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    delay_ms(100);
    vs1053_w_XDCS(1);

    /* 再次进入正弦测试并设置n值为0x44，即将正弦波的频率设置为另外的值 */
    vs1053_w_XDCS(0);                         /* 选中数据传输 */
    spi1_readwriteByte(0x53);
    spi1_readwriteByte(0xef);
    spi1_readwriteByte(0x6e);
    spi1_readwriteByte(0x44);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    delay_ms(100);
    vs1053_w_XDCS(1);
    /* 退出正弦测试 */
    vs1053_w_XDCS(0);                         /* 选中数据传输 */
    spi1_readwriteByte(0x45);
    spi1_readwriteByte(0x78);
    spi1_readwriteByte(0x69);
    spi1_readwriteByte(0x74);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    spi1_readwriteByte(0x00);
    delay_ms(100);
    vs1053_w_XDCS(1);
}


