/**
 * @file platform_mcu_resource.h
 * @brief MCU硬件资源接口,根据不同MCU平台进行对应的资源适配
 * @version stm32f103rct6
 * @date 2026-03-11
 */
#ifndef _PLATFORM_MCU_RESOURCE_H_
#define _PLATFORM_MCU_RESOURCE_H_

typedef enum
{
    USER_GPIO_PA0=0, //PA0
    USER_GPIO_PA1,   //PA1
    USER_GPIO_PA2,   //PA2
    USER_GPIO_PA3,   //PA3
    USER_GPIO_PA4,   //PA4
    USER_GPIO_PA5,   //PA5
    USER_GPIO_PA6,   //PA6
    USER_GPIO_PA7,   //PA7
    USER_GPIO_PA8,   //PA8
    USER_GPIO_PA9,   //PA9
    USER_GPIO_PA10,  //PA10
    USER_GPIO_PA11,  //PA11
    USER_GPIO_PA12,  //PA12
    USER_GPIO_PA13,  //PA13
    USER_GPIO_PA14,  //PA14
    USER_GPIO_PA15,  //PA15

    USER_GPIO_PB0,   //PB0
    USER_GPIO_PB1,   //PB1
    USER_GPIO_PB2,   //PB2
    USER_GPIO_PB3,   //PB3
    USER_GPIO_PB4,   //PB4
    USER_GPIO_PB5,   //PB5
    USER_GPIO_PB6,   //PB6
    USER_GPIO_PB7,   //PB7
    USER_GPIO_PB8,   //PB8
    USER_GPIO_PB9,   //PB9
    USER_GPIO_PB10,  //PB10
    USER_GPIO_PB11,  //PB11
    USER_GPIO_PB12,  //PB12
    USER_GPIO_PB13,  //PB13
    USER_GPIO_PB14,  //PB14
    USER_GPIO_PB15,  //PB15

    USER_GPIO_PC0,   //PC0
    USER_GPIO_PC1,   //PC1
    USER_GPIO_PC2,   //PC2
    USER_GPIO_PC3,   //PC3
    USER_GPIO_PC4,   //PC4
    USER_GPIO_PC5,   //PC5
    USER_GPIO_PC6,   //PC6
    USER_GPIO_PC7,   //PC7
    USER_GPIO_PC8,   //PC8
    USER_GPIO_PC9,   //PC9
    USER_GPIO_PC10,  //PC10
    USER_GPIO_PC11,  //PC11
    USER_GPIO_PC12,  //PC12
    USER_GPIO_PC13,  //PC13
    USER_GPIO_PC14,  //PC14
    USER_GPIO_PC15,  //PC15

    USER_GPIO_PD0,   //PD0
    USER_GPIO_PD1,   //PD1
    USER_GPIO_PD2,   //PD2
    // USER_GPIO_PD3,   //PD3
    // USER_GPIO_PD4,   //PD4
    // USER_GPIO_PD5,   //PD5
    // USER_GPIO_PD6,   //PD6
    // USER_GPIO_PD7,   //PD7
    // USER_GPIO_PD8,   //PD8
    // USER_GPIO_PD9,   //PD9
    // USER_GPIO_PD10,  //PD10
    // USER_GPIO_PD11,  //PD11
    // USER_GPIO_PD12,  //PD12
    // USER_GPIO_PD13,  //PD13
    // USER_GPIO_PD14,  //PD14
    // USER_GPIO_PD15,  //PD15

    // USER_GPIO_PE0,   //PE0
    // USER_GPIO_PE1,   //PE1
    // USER_GPIO_PE2,   //PE2
    // USER_GPIO_PE3,   //PE3
    // USER_GPIO_PE4,   //PE4
    // USER_GPIO_PE5,   //PE5
    // USER_GPIO_PE6,   //PE6
    // USER_GPIO_PE7,   //PE7
    // USER_GPIO_PE8,   //PE8
    // USER_GPIO_PE9,   //PE9
    // USER_GPIO_PE10,  //PE10
    // USER_GPIO_PE11,  //PE11
    // USER_GPIO_PE12,  //PE12
    // USER_GPIO_PE13,  //PE13
    // USER_GPIO_PE14,  //PE14
    // USER_GPIO_PE15,  //PE15

    // USER_GPIO_PF0,   //PF0
    // USER_GPIO_PF1,   //PF1
    // USER_GPIO_PF2,   //PF2
    // USER_GPIO_PF3,   //PF3
    // USER_GPIO_PF4,   //PF4
    // USER_GPIO_PF5,   //PF5
    // USER_GPIO_PF6,   //PF6
    // USER_GPIO_PF7,   //PF7
    // USER_GPIO_PF8,   //PF8
    // USER_GPIO_PF9,   //PF9
    // USER_GPIO_PF10,  //PF10
    // USER_GPIO_PF11,  //PF11
    // USER_GPIO_PF12,  //PF12
    // USER_GPIO_PF13,  //PF13
    // USER_GPIO_PF14,  //PF14
    // USER_GPIO_PF15,  //PF15
    USER_GPIO_MAX
}user_gpio_e;

typedef enum
{
    USER_USART_1=0,
    USER_USART_2,
    USER_USART_3,
    USER_UART_4,
    USER_UART_5,
    USER_UART_MAX
}user_uart_e;

typedef enum
{
    USER_ADC_CH0=0,
    USER_ADC_CH1,
    USER_ADC_CH2,
    USER_ADC_CH3,
    USER_ADC_CH4,
    USER_ADC_CH5,
    USER_ADC_CH6,
    USER_ADC_CH7,
    USER_ADC_CH8,
    USER_ADC_CH9,
    USER_ADC_CH10,
    USER_ADC_CH11,
    USER_ADC_CH12,
    USER_ADC_CH13,
    USER_ADC_CH14,
    USER_ADC_CH15,
    USER_ADC_MAX
}user_adc_channel_e;

typedef enum
{
    USER_TIMER1=0,  //高级定时器
    USER_TIMER2,    //通用定时器
    USER_TIMER3,    //通用定时器
    USER_TIMER4,    //通用定时器
    USER_TIMER5,    //通用定时器
    USER_TIMER6,    //基本定时器
    USER_TIMER7,    //基本定时器
    USER_TIMER8,    //高级定时器
    USER_TIMER_MAX
}user_timer_e;

typedef enum
{
    USER_SDIO = 0,
    USER_SDIO_MAX
}user_sdio_e;

// typedef enum
// {
//     //tim1
//     BREO_TIME1_PWM_OR_CAPTURE_1 = 0,
//     BREO_TIME1_PWM_OR_CAPTURE_2,
//     BREO_TIME1_PWM_OR_CAPTURE_3,
//     BREO_TIME1_PWM_OR_CAPTURE_4,
//     //tim3
//     BREO_TIME3_PWM_OR_CAPTURE_1,
//     BREO_TIME3_PWM_OR_CAPTURE_2,
//     BREO_TIME3_PWM_OR_CAPTURE_3,
//     BREO_TIME3_PWM_OR_CAPTURE_4,
//     //tim14
//     BREO_TIME14_PWM_OR_CAPTURE_1,
//     //tim16
//     BREO_TIME16_PWM_OR_CAPTURE_1,
//     //tim17
//     BREO_TIME17_PWM_OR_CAPTURE_1,
//     BREO_PWM_OR_CAPTURE_MAX
// }breo_pwm_or_capture_e;

typedef enum 
{
    USER_USB_CDC_HOST   = 0,//usb做主
    // USER_USB_CDC_DEVICE = 1,//usb从设备当cdc串口使用
    // USER_USB_HID_DEVICE = 2,//usb从设备当hid键盘使用
    USER_USB_MAX
}user_usb_e;

typedef enum 
{
    USER_SPI_1 = 0,
    USER_SPI_2,
    USER_SPI_3,
    USER_SPI_MAX
}user_spi_e;

typedef enum 
{
    USER_I2C1 = 0,
    USER_I2C2,
    // USER_I2C3,
    // USER_I2C4,
    USER_I2C_MAX,
}user_i2c_e;

typedef enum
{
    USER_FLASH_INT = 0,//内部flash
    USER_FLASH_EXT = 1,//外部flash
    USER_FLASH_MAX
}user_flash_e;

/** @brief DMA控制器：共 2 个 DMA 控制器，总计12 个独立通道（DMA1 7 通道、DMA2 5 通道）
 *  @brief 支持外设：定时器、ADC、DAC、SDIO、I2S、SPI、I2C、USART
 *  @brief 支持内存到内存、外设到内存、内存到外设传输
 *  @brief 支持循环缓冲区管理，每通道可软件触发，支持硬件 DMA 请求
 */

#endif
