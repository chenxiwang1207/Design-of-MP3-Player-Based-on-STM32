#include "main.h"

void sys_stm32_clock_init(uint32_t plln);
extern char directory_name[50];

const drv_partition_table_t test_table[] =
{
    {DRV_PARTITION_UPDATE_TEMP,DRV_PARTITION_FLASH_INT,UPDATE_TEMP_ADDR,  UPDATE_TEMP_SIZE,  DRV_PAR_READ_EN|DRV_PAR_WRITE_EN ,"updata"},
};

int main(void)
{
    sys_stm32_clock_init(9);
    drv_wdt_init(30000);
    drv_partition_init(test_table,sizeof(test_table)/sizeof(test_table[0]));
    drv_uart_open(USER_USART_1);
    test_printf_uart1("\r\nEnter App Success");
    App_Button_Init();
    OLED_Init();
    sd_fatfs_init();

    while(1)
    {
        app_menu_UI();
        IWDG_ReloadCounter();
    }
}

/**
 * @brief       时钟设置函数
 * @param       plln: PLL倍频系数(PLL倍频), 取值范围: 2~16
 * @note
 *
 *              PLLCLK: PLL输出时钟
 *              PLLSRC: PLL输入时钟频率, 可以是 HSI/2, HSE/2, HSE等, 一般选择HSE.
 *              SYSCLK: 系统时钟, 可选来自 HSI/PLLCLK/HSE, 一般选择来自PLLCLK
 *              FCLK  : Cortex M3内核时钟, 等于HCLK
 *              HCLK  : AHB总线时钟, 来自 SYSCLK 的分频, 可以是1...512分频, 一般不分频
 *              PCLK2 : APB2总线时钟, 来自 HCLK 的分频(最大72Mhz), 可以是1/2/4/8/16分频, 一般不分频
 *              PCLK1 : APB1总线时钟, 来自 HCLK 的分频(最大36Mhz), 可以是1/2/4/8/16分频, 一般二分频
 *
 *              PLLCLK = PLLSRC * plln;
 *              FCLK = HCLK = SYSCLK;
 *              PCLK2 = HCLK;
 *              PCLK1 = HCLK / 2;
 *
 *              我们一般选择PLLSRC来自HSE, 即来自外部晶振.
 *              当外部晶振为 8M的时候, 推荐: plln = 9, AHB不分频, 得到:
 *              PLLCLK = 8 * 9 = 72Mhz
 *              FCLK = HCLK = SYSCLK = PLLCLK / 1 = 72Mhz
 *              PCLK2 = HCLK = 72Mhz
 *              PCLK1 = HCLK / 2 = 36Mhz
 *
 *              关于STM32F103的PLL说明详见: <<STM32中文参考手册 V10>>第六章相关内容
 *
 * @retval      错误代码: 0, 成功; 1, HSE错误;
 */
uint8_t sys_clock_set(uint32_t plln)
{
    uint32_t retry = 0;
    uint8_t retval = 0;
    RCC->CR |= 0x00010000;          /* 外部高速时钟使能HSEON */

    while (retry < 0XFFF0)
    {
        __nop();

        /* 注意, MDK5.29或以后版本, 在使能HSEON以后, 如果不加一定的延时
         * 再开始其他配置, 会导致仿真器下载完代码, 延时函数运行不正常的 bug
         * 需要按复位按键, 延时才会正常, 在这里加一定的延时, 可以解决这个 bug
         * 这里, 我们设置的延时时间, 至少是 0X8000 个 nop时间
         */
        if (RCC->CR & (1 << 17) && retry > 0X8000)
        {
            break;
        }

        retry++;        /* 等待HSE RDY */
    }

    if (retry >= 0XFFF0)
    {
        retval = 1;     /* HSE无法就绪 */
    }
    else
    {
        RCC->CFGR = 0X00000400;     /* PCLK1 = HCLK / 2; PCLK2 = HCLK; HCLK = SYSCLK; */
        plln -= 2;                  /* 抵消2个单位(因为是从2开始的, 设置0就是2) */
        RCC->CFGR |= plln << 18;    /* 设置PLL值 2~16 */
        RCC->CFGR |= 1 << 16;       /* PLLSRC = 1, 选择 HSE 作为 PLL 输入时钟 */

        /* FLASH_ACR寄存器的描述详见: <<STM32F10xx闪存编程手册>> */
        FLASH->ACR = 1 << 4;        /* PRFTBE = 1 开启预取缓冲区 */
        FLASH->ACR |= 2 << 0;       /* LATENCY[2:0] = 2 FLASH两个等待周期 */

        RCC->CR |= 1 << 24;         /* PLLON = 1, 使能PLL */

        while (!(RCC->CR >> 25));   /* 等待PLL锁定 */

        RCC->CFGR |= 2 << 0;        /* SW[1:0] = 2, 选择PLL输出作为系统时钟 */

        while (((RCC->CFGR >> 2) & 0X03) != 2); /* 等待PLL作为系统时钟设置成功 */
    }

    return retval;
}
/**
 * @brief       设置中断向量表偏移地址
 * @param       baseaddr: 基址
 * @param       offset: 偏移量(必须是0, 或者0X100的倍数)
 * @retval      无
 */
void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset)
{
    /* 设置NVIC的向量表偏移寄存器,VTOR低9位保留,即[8:0]保留 */
    SCB->VTOR = baseaddr | (offset & (uint32_t)0xFFFFFE00);
}
/**
 * @brief       系统时钟初始化函数
 * @param       plln: PLL倍频系数(PLL倍频), 取值范围: 2~16
 * @retval      无
 */
void sys_stm32_clock_init(uint32_t plln)
{
    RCC->APB1RSTR = 0x00000000;     /* 复位结束 */
    RCC->APB2RSTR = 0x00000000;
    
    RCC->AHBENR = 0x00000014;       /* 睡眠模式闪存和SRAM时钟使能.其他关闭 */
    RCC->APB2ENR = 0x00000000;      /* 外设时钟关闭 */
    RCC->APB1ENR = 0x00000000;
    
    RCC->CR |= 0x00000001;          /* 使能内部高速时钟HSION */
    RCC->CFGR &= 0xF8FF0000;        /* 复位SW[1:0], SWS[1:0], HPRE[3:0], PPRE1[2:0], PPRE2[2:0], ADCPRE[1:0], MCO[2:0] */
    RCC->CR &= 0xFEF6FFFF;          /* 复位HSEON, CSSON, PLLON */
    RCC->CR &= 0xFFFBFFFF;          /* 复位HSEBYP */
    RCC->CFGR &= 0xFF80FFFF;        /* 复位PLLSRC, PLLXTPRE, PLLMUL[3:0] 和 USBPRE/OTGFSPRE */
    RCC->CIR = 0x009F0000;          /* 关闭所有RCC中断并清除中断标志 */

    sys_clock_set(plln);            /* 设置时钟 */

    /* 配置中断向量偏移 */
#ifdef  VECT_TAB_RAM
    sys_nvic_set_vector_table(SRAM_BASE, 0x0);
#else
    sys_nvic_set_vector_table(FLASH_BASE, 0x0);
#endif
}
