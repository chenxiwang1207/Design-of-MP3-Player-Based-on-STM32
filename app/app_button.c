#include "stdio.h"
#include "string.h"
#include "stm32f10x.h"                  // Device header
#include "dev_button.h"
#include "dev_button_config.h"
#include "app_button.h"
#include "drv_usart.h"

#define APP_FLEX_BUTTON_DEBUG   0
#if     APP_FLEX_BUTTON_DEBUG
#else
#define test_printf_uart1(fmt, arg...)
#endif

volatile app_button_e app_button = 
{
    .button_up_flag      = button_press_none,
    .button_down_flag    = button_press_none,
    .button_confirm_flag = button_press_none,
    .button_left_flag    = button_press_none,
    .button_right_flag   = button_press_none,
};

#if APP_FLEX_BUTTON_DEBUG
static const struct{
    btn_value_t         button_value;
    const char         *value_msg;
}button_value_table[] = {
    {  BUTTON_VALUE_UP,       "Up button"      },
    {  BUTTON_VALUE_DOWN,     "Down button"    },
    {  BUTTON_VALUE_CONFIRM,  "Confirm button" },
    {  BUTTON_VALUE_LEFT,     "Left button"    },
    {  BUTTON_VALUE_RIGHT,    "Right button"   },
    {  BTN_VALUE_MAX,         "             "  },
};

static const struct {
    flex_button_event_t button_event;
    const char         *event_msg;
} button_event_table[] = {
    { FLEX_BTN_PRESS_DOWN,         "Press Down Event"            },
    { FLEX_BTN_PRESS_CLICK,        "Click Event"                 },
    { FLEX_BTN_PRESS_DOUBLE_CLICK, "Double Click Event"          },
    { FLEX_BTN_PRESS_REPEAT_CLICK, "Combo Event"                 },
    { FLEX_BTN_PRESS_SHORT_START,  "Short-press start event"     },
    { FLEX_BTN_PRESS_SHORT_UP,     "Short-press liftup event"    },
    { FLEX_BTN_PRESS_LONG_START,   "Long-press start event"      },
    { FLEX_BTN_PRESS_LONG_UP,      "Long-press liftup event"     },
    { FLEX_BTN_PRESS_LONG_HOLD,    "Long-press keep event"       },
    { FLEX_BTN_PRESS_LONG_HOLD_UP, "Long-press keep liftup event"},
    { FLEX_BTN_PRESS_NONE,         "No press event"              },
};
static const char *find_event_msg(flex_button_event_t button_event)
{
    for(u32 i=0;i<FLEX_BTN_PRESS_MAX;i++)
    {
        if(button_event_table[i].button_event == button_event)
        {
            return button_event_table[i].event_msg;
        }
    }
    return "unkonwn";
}

static const char *find_value_msg(btn_value_t button_value)
{
    for(u32 i=0;i<FLEX_BTN_PRESS_MAX;i++)
    {
        if(button_value_table[i].button_value == button_value)
        {
            return button_value_table[i].value_msg;
        }
    }
    return "unkonwn";
}
#endif

//按键事件处理回调函数
static void button_callback(flex_button_event_t event,btn_value_t value)
{
    switch(value)
    {
/*------------------------上按键值----------------------*/
        case BUTTON_VALUE_UP:
            if(event == FLEX_BTN_PRESS_SHORT_UP)       //短按弹起响应
            {
                app_button.button_up_flag = button_press_short_up;
            }
            else if(event == FLEX_BTN_PRESS_LONG_START) //长按开始
            {
                app_button.button_up_flag = button_press_long_start;
            }
            else if(event == FLEX_BTN_PRESS_LONG_HOLD)  //长按保持
            {
                app_button.button_up_flag = button_press_long_hold_start;
            }
            break;
/*------------------------下按键值----------------------*/
        case BUTTON_VALUE_DOWN:
            if(event == FLEX_BTN_PRESS_SHORT_UP)       //短按弹起响应
            {
                app_button.button_down_flag = button_press_short_up;
            }
            else if(event == FLEX_BTN_PRESS_LONG_START) //长按开始
            {
                app_button.button_down_flag = button_press_long_start;
            }
            else if(event == FLEX_BTN_PRESS_LONG_HOLD)  //长按保持
            {
                app_button.button_down_flag = button_press_long_hold_start;
            }
            break;
/*---------------------------确认按键值----------------------------*/
        case BUTTON_VALUE_CONFIRM:
            if(event == FLEX_BTN_PRESS_SHORT_UP)      //短按弹起响应
            {
                app_button.button_confirm_flag = button_press_short_up;
            }
            else if(event == FLEX_BTN_PRESS_LONG_START) //长按开始
            {
                app_button.button_confirm_flag = button_press_long_start;
            }
            else if(event == FLEX_BTN_PRESS_LONG_HOLD)  //长按保持
            {
                app_button.button_confirm_flag = button_press_long_hold_start;
            }
            break;
/*---------------------------左按键值----------------------------*/
        case BUTTON_VALUE_LEFT:
            if(event == FLEX_BTN_PRESS_SHORT_UP)       //短按弹起响应
            {
                app_button.button_left_flag = button_press_short_up;
            }
            else if(event == FLEX_BTN_PRESS_LONG_START) //长按开始
            {
                app_button.button_left_flag = button_press_long_start;
            }
            else if(event == FLEX_BTN_PRESS_LONG_HOLD)  //长按保持
            {
                app_button.button_left_flag = button_press_long_hold_start;
            }
            break;
/*---------------------------右按键值----------------------------*/
        case BUTTON_VALUE_RIGHT:
            if(event == FLEX_BTN_PRESS_SHORT_UP)       //短按弹起响应
            {
                app_button.button_right_flag = button_press_short_up;
            }
            else if(event == FLEX_BTN_PRESS_LONG_START) //长按开始
            {
                app_button.button_right_flag = button_press_long_start;
            }
            else if(event == FLEX_BTN_PRESS_LONG_HOLD)  //长按保持
            {
                app_button.button_right_flag = button_press_long_hold_start;
            }
            break;
/*---------------------------其他未知按键按下----------------------------*/
        default:
            break;
    }
    test_printf_uart1("\r\nbutton event:%s",find_event_msg(event));
    test_printf_uart1("\r\nbutton value:%s",find_value_msg(value));
}
//初始化按键
void App_Button_Init(void)
{
    dev_btn_config_init(button_callback);
}
//获取按键值
app_button_e App_Get_Button_Value(void)
{
    app_button_e temp = app_button;
    app_button.button_confirm_flag = button_press_none;
    app_button.button_down_flag    = button_press_none;
    app_button.button_left_flag    = button_press_none;
    app_button.button_right_flag   = button_press_none;
    app_button.button_up_flag      = button_press_none;
    return temp;
}
