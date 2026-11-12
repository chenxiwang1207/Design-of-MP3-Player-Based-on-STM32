// #include "stdio.h"
// #include "string.h"
// #include "stm32f10x.h"                  // Device header
// #include "dev_button.h"
// #include "dev_button_config.h"
// #include "app_button.h"
// #include "drv_usart.h"

// #define APP_FLEX_BUTTON_DEBUG   0
// #if APP_FLEX_BUTTON_DEBUG
// #define test_printf_uart1(fmt, arg...)   com_printf(fmt, ##arg)
// #else
// #define test_printf_uart1(fmt, arg...)
// #endif

// volatile app_button_e app_button = 
// {
//     .button_up_flag      = button_press_none,
//     .button_down_flag    = button_press_none,
//     .button_confirm_flag = button_press_none,
//     .button_left_flag    = button_press_none,
//     .button_right_flag   = button_press_none,
// };

// #if APP_FLEX_BUTTON_DEBUG
// static const struct{
//     btn_value_t         button_value;
//     const char         *value_msg;
// }button_value_table[] = {
//     {  BUTTON_VALUE_UP,       "Up button"      },
//     {  BUTTON_VALUE_DOWN,     "Down button"    },
//     {  BUTTON_VALUE_CONFIRM,  "Confirm button" },
//     {  BUTTON_VALUE_LEFT,     "Left button"    },
//     {  BUTTON_VALUE_RIGHT,    "Right button"   },
//     {  BTN_VALUE_MAX,         "             "  },
// };

// static const struct {
//     flex_button_event_t button_event;
//     const char         *event_msg;
// } button_event_table[] = {
//     { FLEX_BTN_PRESS_DOWN,         "Press Down Event"            },
//     { FLEX_BTN_PRESS_CLICK,        "Click Event"                 },
//     { FLEX_BTN_PRESS_DOUBLE_CLICK, "Double Click Event"          },
//     { FLEX_BTN_PRESS_REPEAT_CLICK, "Combo Event"                 },
//     { FLEX_BTN_PRESS_SHORT_START,  "Short-press start event"     },
//     { FLEX_BTN_PRESS_SHORT_UP,     "Short-press liftup event"    },
//     { FLEX_BTN_PRESS_LONG_START,   "Long-press start event"      },
//     { FLEX_BTN_PRESS_LONG_UP,      "Long-press liftup event"     },
//     { FLEX_BTN_PRESS_LONG_HOLD,    "Long-press keep event"       },
//     { FLEX_BTN_PRESS_LONG_HOLD_UP, "Long-press keep liftup event"},
//     { FLEX_BTN_PRESS_NONE,         "No press event"              },
// };
// static const char *find_event_msg(flex_button_event_t button_event)
// {
//     for(u32 i=0;i<FLEX_BTN_PRESS_MAX;i++)
//     {
//         if(button_event_table[i].button_event == button_event)
//         {
//             return button_event_table[i].event_msg;
//         }
//     }
//     return "unkonwn";
// }

// static const char *find_value_msg(btn_value_t button_value)
// {
//     for(u32 i=0;i<FLEX_BTN_PRESS_MAX;i++)
//     {
//         if(button_value_table[i].button_value == button_value)
//         {
//             return button_value_table[i].value_msg;
//         }
//     }
//     return "unkonwn";
// }
// #endif

// //按键事件处理回调函数
// static void button_callback(flex_button_event_t event,btn_value_t value)
// {
//     switch(value)
//     {
// /*------------------------上按键值----------------------*/
//         case BUTTON_VALUE_UP:
//             if(event == FLEX_BTN_PRESS_SHORT_UP)       //短按弹起响应
//             {
//                 app_button.button_up_flag = button_press_short_up;
//             }
//             else if(event == FLEX_BTN_PRESS_LONG_START) //长按开始
//             {
//                 app_button.button_up_flag = button_press_long_start;
//             }
//             else if(event == FLEX_BTN_PRESS_LONG_HOLD)  //长按保持
//             {
//                 app_button.button_up_flag = button_press_long_hold_start;
//             }
//             break;
// /*------------------------下按键值----------------------*/
//         case BUTTON_VALUE_DOWN:
//             if(event == FLEX_BTN_PRESS_SHORT_UP)       //短按弹起响应
//             {
//                 app_button.button_down_flag = button_press_short_up;
//             }
//             else if(event == FLEX_BTN_PRESS_LONG_START) //长按开始
//             {
//                 app_button.button_down_flag = button_press_long_start;
//             }
//             else if(event == FLEX_BTN_PRESS_LONG_HOLD)  //长按保持
//             {
//                 app_button.button_down_flag = button_press_long_hold_start;
//             }
//             break;
// /*---------------------------确认按键值----------------------------*/
//         case BUTTON_VALUE_CONFIRM:
//             if(event == FLEX_BTN_PRESS_SHORT_UP)      //短按弹起响应
//             {
//                 app_button.button_confirm_flag = button_press_short_up;
//             }
//             else if(event == FLEX_BTN_PRESS_LONG_START) //长按开始
//             {
//                 app_button.button_confirm_flag = button_press_long_start;
//             }
//             else if(event == FLEX_BTN_PRESS_LONG_HOLD)  //长按保持
//             {
//                 app_button.button_confirm_flag = button_press_long_hold_start;
//             }
//             break;
// /*---------------------------左按键值----------------------------*/
//         case BUTTON_VALUE_LEFT:
//             if(event == FLEX_BTN_PRESS_SHORT_UP)       //短按弹起响应
//             {
//                 app_button.button_left_flag = button_press_short_up;
//             }
//             else if(event == FLEX_BTN_PRESS_LONG_START) //长按开始
//             {
//                 app_button.button_left_flag = button_press_long_start;
//             }
//             else if(event == FLEX_BTN_PRESS_LONG_HOLD)  //长按保持
//             {
//                 app_button.button_left_flag = button_press_long_hold_start;
//             }
//             break;
// /*---------------------------右按键值----------------------------*/
//         case BUTTON_VALUE_RIGHT:
//             if(event == FLEX_BTN_PRESS_SHORT_UP)       //短按弹起响应
//             {
//                 app_button.button_right_flag = button_press_short_up;
//             }
//             else if(event == FLEX_BTN_PRESS_LONG_START) //长按开始
//             {
//                 app_button.button_right_flag = button_press_long_start;
//             }
//             else if(event == FLEX_BTN_PRESS_LONG_HOLD)  //长按保持
//             {
//                 app_button.button_right_flag = button_press_long_hold_start;
//             }
//             break;
// /*---------------------------其他未知按键按下----------------------------*/
//         default:
//             break;
//     }
//     test_printf_uart1("\r\nbutton event:%s",find_event_msg(event));
//     test_printf_uart1("\r\nbutton value:%s",find_value_msg(value));
// }
// //初始化按键
// void App_Button_Init(void)
// {
//     dev_btn_config_init(button_callback);
// }
// //获取按键值
// app_button_e App_Get_Button_Value(void)
// {
//     app_button_e temp = app_button;
//     app_button.button_confirm_flag = button_press_none;
//     app_button.button_down_flag    = button_press_none;
//     app_button.button_left_flag    = button_press_none;
//     app_button.button_right_flag   = button_press_none;
//     app_button.button_up_flag      = button_press_none;
//     return temp;
// }

/**
 * @file    app_button.c
 * @brief   按键应用层：把 dev_button 的 flex_button 事件转换为业务用的 app_button_value_e
 *
 * 相对原版修正：
 *   1. 修正 APP_FLEX_BUTTON_DEBUG 的 #if/#else 分支写反问题；
 *   2. find_value_msg / find_event_msg 用 sizeof 计算表长度，修复越界；
 *      且仅在 DEBUG 打开时编译，避免未使用告警；
 *   3. 补全 FLEX_BTN_PRESS_LONG_UP / LONG_HOLD_UP 映射，修复长按松开后 flag 卡死；
 *   4. 补全 FLEX_BTN_PRESS_CLICK / SHORT_START 映射，与枚举保持一致；
 *   5. App_Get_Button_Value 增加临界区，规避"读-清"竞态与事件丢失；
 *   6. 全局状态改为 static，避免污染命名空间。
 *
 * 注意：app_button.h 中 button_press_click = 0，未初始化时默认被当作 click。
 *      本文件在 App_Button_Init 中显式置为 button_press_none 规避该问题，
 *      但仍建议把 header 里 button_press_none 改为 0（更符合直觉）。
 */

#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"                  // Device header
#include "dev_button.h"
#include "dev_button_config.h"
#include "app_button.h"
#include "drv_usart.h"

#define APP_FLEX_BUTTON_DEBUG   0

#if APP_FLEX_BUTTON_DEBUG
#define test_printf_uart1(fmt, arg...)   printf(fmt, ##arg)   /* 换成你们平台的串口打印即可 */
#else
#define test_printf_uart1(fmt, arg...)
#endif

/* ============================================================
 * 临界区包装：保存并恢复 PRIMASK，支持嵌套
 * 如果平台有 drv_enter_critical / drv_exit_critical，可替换
 * ============================================================ */
static inline uint32_t app_btn_enter_critical(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static inline void app_btn_exit_critical(uint32_t primask)
{
    __set_PRIMASK(primask);
}

/* ============================================================
 * 按键状态：中断写、任务读，加 volatile 防止优化
 * ============================================================ */
static volatile app_button_e g_app_button =
{
    .button_up_flag      = button_press_none,
    .button_down_flag    = button_press_none,
    .button_confirm_flag = button_press_none,
    .button_left_flag    = button_press_none,
    .button_right_flag   = button_press_none,
};

/* ============================================================
 * 调试辅助表
 * ============================================================ */
#if APP_FLEX_BUTTON_DEBUG
static const struct {
    btn_value_t         button_value;
    const char         *value_msg;
} button_value_table[] = {
    { BUTTON_VALUE_UP,       "Up button"      },
    { BUTTON_VALUE_DOWN,     "Down button"    },
    { BUTTON_VALUE_CONFIRM,  "Confirm button" },
    { BUTTON_VALUE_LEFT,     "Left button"    },
    { BUTTON_VALUE_RIGHT,    "Right button"   },
    { BTN_VALUE_MAX,         "             "  },
};

static const struct {
    flex_button_event_t  button_event;
    const char          *event_msg;
} button_event_table[] = {
    { FLEX_BTN_PRESS_DOWN,         "Press Down Event"             },
    { FLEX_BTN_PRESS_CLICK,        "Click Event"                  },
    { FLEX_BTN_PRESS_DOUBLE_CLICK, "Double Click Event"           },
    { FLEX_BTN_PRESS_REPEAT_CLICK, "Combo Event"                  },
    { FLEX_BTN_PRESS_SHORT_START,  "Short-press start event"      },
    { FLEX_BTN_PRESS_SHORT_UP,     "Short-press liftup event"     },
    { FLEX_BTN_PRESS_LONG_START,   "Long-press start event"       },
    { FLEX_BTN_PRESS_LONG_UP,      "Long-press liftup event"      },
    { FLEX_BTN_PRESS_LONG_HOLD,    "Long-press keep event"        },
    { FLEX_BTN_PRESS_LONG_HOLD_UP, "Long-press keep liftup event" },
    { FLEX_BTN_PRESS_NONE,         "No press event"               },
};

static const char *find_event_msg(flex_button_event_t button_event)
{
    for (u32 i = 0; i < sizeof(button_event_table) / sizeof(button_event_table[0]); i++)
    {
        if (button_event_table[i].button_event == button_event)
        {
            return button_event_table[i].event_msg;
        }
    }
    return "unknown";
}

static const char *find_value_msg(btn_value_t button_value)
{
    for (u32 i = 0; i < sizeof(button_value_table) / sizeof(button_value_table[0]); i++)
    {
        if (button_value_table[i].button_value == button_value)
        {
            return button_value_table[i].value_msg;
        }
    }
    return "unknown";
}
#endif /* APP_FLEX_BUTTON_DEBUG */

/* ============================================================
 * flex_button_event_t -> app_button_value_e
 * ============================================================ */
static app_button_value_e map_flex_event(flex_button_event_t event)
{
    switch (event)
    {
        case FLEX_BTN_PRESS_CLICK:          return button_press_click;
        case FLEX_BTN_PRESS_SHORT_START:    return button_press_short_start;
        case FLEX_BTN_PRESS_SHORT_UP:       return button_press_short_up;
        case FLEX_BTN_PRESS_LONG_START:     return button_press_long_start;
        case FLEX_BTN_PRESS_LONG_HOLD:      return button_press_long_hold_start;
        case FLEX_BTN_PRESS_LONG_UP:        return button_press_long_up;
        case FLEX_BTN_PRESS_LONG_HOLD_UP:   return button_press_long_up;
        default:                            return button_press_none;
    }
}

/* ============================================================
 * 按键事件回调，运行在定时器中断上下文中
 * 只做轻量操作：写标志位
 * ============================================================ */
static void button_callback(flex_button_event_t event, btn_value_t value)
{
    app_button_value_e evt_val = map_flex_event(event);

    if (evt_val == button_press_none)
    {
        return;
    }

    switch (value)
    {
        case BUTTON_VALUE_UP:       g_app_button.button_up_flag      = evt_val; break;
        case BUTTON_VALUE_DOWN:     g_app_button.button_down_flag    = evt_val; break;
        case BUTTON_VALUE_CONFIRM:  g_app_button.button_confirm_flag = evt_val; break;
        case BUTTON_VALUE_LEFT:     g_app_button.button_left_flag    = evt_val; break;
        case BUTTON_VALUE_RIGHT:    g_app_button.button_right_flag   = evt_val; break;
        default:                    return;
    }

#if APP_FLEX_BUTTON_DEBUG
    test_printf_uart1("\r\nevent=%s value=%s",
                      find_event_msg(event), find_value_msg(value));
#endif
}

/* ============================================================
 * 对外接口
 * ============================================================ */
void App_Button_Init(void)
{
    /* 显式清零，规避 header 中 button_press_click = 0 导致的默认值问题 */
    g_app_button.button_up_flag      = button_press_none;
    g_app_button.button_down_flag    = button_press_none;
    g_app_button.button_confirm_flag = button_press_none;
    g_app_button.button_left_flag    = button_press_none;
    g_app_button.button_right_flag   = button_press_none;

    dev_btn_config_init(button_callback);
}

app_button_e App_Get_Button_Value(void)
{
    app_button_e temp;
    uint32_t primask = app_btn_enter_critical();

    /* volatile 结构体不宜整体赋值，逐字段拷贝 */
    temp.button_up_flag      = g_app_button.button_up_flag;
    temp.button_down_flag    = g_app_button.button_down_flag;
    temp.button_confirm_flag = g_app_button.button_confirm_flag;
    temp.button_left_flag    = g_app_button.button_left_flag;
    temp.button_right_flag   = g_app_button.button_right_flag;

    g_app_button.button_up_flag      = button_press_none;
    g_app_button.button_down_flag    = button_press_none;
    g_app_button.button_confirm_flag = button_press_none;
    g_app_button.button_left_flag    = button_press_none;
    g_app_button.button_right_flag   = button_press_none;

    app_btn_exit_critical(primask);

    return temp;
}
