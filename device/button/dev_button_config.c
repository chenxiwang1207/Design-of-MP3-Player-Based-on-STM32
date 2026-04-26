#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"                  // Device header
#include "drv_timer.h"
#include "drv_gpio.h"
#include "dev_button_config.h"

#define BUTTON_DEBUG              0          //按键打印使能
#if     BUTTON_DEBUG
#define button_debug(fmt, arg...) com_printf("[but]<%s><%d>" fmt, __func__, __LINE__, ##arg)
#define button_err(fmt, arg...)   com_printf("[but error]<%s><%d>" fmt, __func__, __LINE__, ##arg)
#else
#define button_debug(fmt, arg...)
#define button_err(fmt, arg...)
#endif

#define KEY_SN_CONFIRM       USER_GPIO_PB1   // 确认键 短按
#define KEY_SN_UP            USER_GPIO_PB0   // 上按键 短按 光标往上
//#define KEY_SN_DOWN          USER_GPIO_PB1   // 下按键 短按 光标往下
//#define KEY_SN_RIGHT         USER_GPIO_PB5   // 右按键 短按 光标往右/上一首    长按：歌曲快进
#define KEY_SN_LEFT          USER_GPIO_PB4   // 左按键 短按 光标往左/下一首    长按：歌曲回退


#define KEY_SN_RIGHT         USER_GPIO_PC5    // 右按键 短按 光标往右/上一首    长按：歌曲快进 KEY0
#define KEY_SN_DOWN          USER_GPIO_PA15   // 下按键 短按 光标往下

typedef struct
{
    user_gpio_e   gpio_btn_array[BTN_ARRAY_COUNT];
    flex_button_t user_button[BTN_ARRAY_COUNT];
    btn_cb        user_func;
}user_button_config_t;

static user_button_config_t user_button_config;

const user_gpio_e  gpio_btn_array[BTN_ARRAY_COUNT] = {KEY_SN_UP,KEY_SN_DOWN,KEY_SN_CONFIRM,KEY_SN_LEFT,KEY_SN_RIGHT};//按键    
const btn_value_t  btu_value[BTN_ARRAY_COUNT]      = {BUTTON_VALUE_UP,BUTTON_VALUE_DOWN,BUTTON_VALUE_CONFIRM,BUTTON_VALUE_LEFT,BUTTON_VALUE_RIGHT};//按键按下传入的值

/**
 * @brief 获取按键gpio引脚电平值
 * @param[in] arg 按键flex_button_t结构体指针
 * @return u8 返回按键电平值：0-低电平，1-高电平
 */
static u8 inner_read_gpio_level(void *arg)
{
    flex_button_t *btn = (flex_button_t *)arg;
    s32 ret = drv_gpio_read_in(user_button_config.gpio_btn_array[btn->id]);

    if(ret == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/**
 * @brief 按键按下事件回调函数
 * @param[in] arg 按键flex_button_t结构体指针
 * @return void 
 */
static void inner_btn_evt_cb(void *arg)
{
    flex_button_t *btn = (flex_button_t *)arg;
    if (btn->id < BTN_ARRAY_COUNT)
    {
        switch (btn->event)
        {
        case FLEX_BTN_PRESS_DOWN:
        case FLEX_BTN_PRESS_SHORT_START:
        case FLEX_BTN_PRESS_SHORT_UP:
        case FLEX_BTN_PRESS_LONG_START:
        case FLEX_BTN_PRESS_LONG_UP:
        case FLEX_BTN_PRESS_LONG_HOLD:
        case FLEX_BTN_PRESS_LONG_HOLD_UP:
            {
                if (user_button_config.user_func != NULL)
                {
                    user_button_config.user_func((flex_button_event_t)btn->event,btn->btu_value);
                }
                button_debug("btu_value=[%x],btn->event=%x\n",btn->btu_value, btn->event);
            }
            break;
        default:
            break;
        }
    }
    else
    {
        button_debug("invalid event, id=[%d]", btn->id);
    }
}

/**
 * @brief 按键扫描线程函数:使用定时器或者使用线程进行扫描
 * @param arg ：为了兼容线程扫描方式，使用定时器实际上没有用到
 * @return void
 */
void inner_btn_config_task(void *arg)
{
    dev_button_scan();
}

/**
 * @brief     按键初始化配置函数
 * @param[in] func 按键事件回调函数,主要用于处理用户操作按键时触发的按键事件
 * @return    void
 */
void dev_btn_config_init(btn_cb func)
{
    s32 i=0;
    dev_button_unregister();
    for(i = 0; i < BTN_ARRAY_COUNT;i++)
    {
        //初始化按键gpio引脚，配置成gpio浮空输入模式
        drv_gpio_close(gpio_btn_array[i]);
        drv_gpio_open (gpio_btn_array[i]);
        drv_gpio_config(gpio_btn_array[i], DRV_GPIO_Mode_IPU,DRV_Speed_HIGH);
    }
    for(i = 0; i < BTN_ARRAY_COUNT;i++)
    {
        //初始化按键结构体
        user_button_config.gpio_btn_array[i]                     = gpio_btn_array[i];              //初始化按键引脚
        user_button_config.user_button[i].id                     = i;                              //数值序号
        user_button_config.user_button[i].usr_button_read        = inner_read_gpio_level;          //读取按键电平函数
        user_button_config.user_button[i].cb                     = inner_btn_evt_cb;               //按键事件回调函数
        user_button_config.user_button[i].pressed_logic_level    = 0;                              //按键0按下逻辑电平是0
        user_button_config.user_button[i].short_press_start_tick = FLEX_MS_TO_SCAN_CNT(10);        //短按开始时间ms
        user_button_config.user_button[i].long_press_start_tick  = FLEX_MS_TO_SCAN_CNT(1800);      //长按开始时间ms
        user_button_config.user_button[i].long_hold_start_tick   = FLEX_MS_TO_SCAN_CNT(10000);     //长按持续时间ms
        user_button_config.user_button[i].btu_value              = btu_value[i];                   //按键初始状态值,把按键和按键值做映射关系
        user_button_config.user_button[i].next                   = NULL;
        dev_button_register(&user_button_config.user_button[i]);
    }
    user_button_config.user_func = func;

    //init button timer执行线程
    drv_timer_close(BUTTON_TIMER);
    drv_timer_open (BUTTON_TIMER);
    drv_timer_setup(BUTTON_TIMER,BUTTON_MANAGE_PERIOD,inner_btn_config_task);
    drv_timer_start(BUTTON_TIMER);
}
