/**
 * @File:    flexible_button.c
 * @Author:  MurphyZhao
 * @Date:    2018-09-29
 * 
 * Copyright (c) 2018-2019 MurphyZhao <d2014zjt@163.com>
 *               https://github.com/murphyzhao
 * All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Change logs:
 * Date        Author       Notes
 * 2018-09-29  MurphyZhao   First add
 * 2019-08-02  MurphyZhao   Migrate code to github.com/murphyzhao account
 * 2019-12-26  MurphyZhao   Refactor code and implement multiple clicks
 * 
*/

#include "dev_button.h"

#define BUTTON_DEBUG              0           //按键打印使能
#if     BUTTON_DEBUG
#define button_debug(fmt, arg...) breo_printf("[but]<%s><%d>" fmt, __func__, __LINE__, ##arg)
#define button_err(fmt, arg...)   breo_printf("[but error]<%s><%d>" fmt, __func__, __LINE__, ##arg)
#else
#define button_debug(fmt, arg...)
#define button_err(fmt, arg...)
#endif

#define EVENT_SET_AND_EXEC_CB(btn, evt)                                        \
    do                                                                         \
    {                                                                          \
        btn->event = evt;                                                      \
        if(btn->cb)                                                            \
            btn->cb((flex_button_t*)btn);                                      \
    } while(0)

/**
 * BTN_IS_PRESSED
 *
 * 1: is pressed
 * 0: is not pressed
*/
#define BTN_IS_PRESSED(i) (g_btn_status_reg & (1 << i))

enum FLEX_BTN_STAGE
{
    FLEX_BTN_STAGE_DEFAULT        = 0,
    FLEX_BTN_STAGE_DOWN           = 1,
    FLEX_BTN_STAGE_MULTIPLE_CLICK = 2
};

typedef u32 btn_type_t;

static flex_button_t *btn_head = NULL;

/**
* g_logic_level
* 按下的按钮的逻辑电平
* 每个位代表一个按钮
* 首先点击注册按钮后，该按钮被按下时的逻辑电平
* 就会存储在 g_logic_level 的最低位中
*/
btn_type_t g_logic_level = (btn_type_t)0;

/**
* g_btn_status_reg
* 所有按钮的状态寄存器，每个位都记录着一个按钮的按下状态
* 首先注册的按钮，其按钮的按下状态位于 g_btn_status_reg 的最低位
*/
btn_type_t g_btn_status_reg = (btn_type_t)0;

static u8 button_cnt = 0;

s32 dev_button_unregister(void)
{
    button_cnt = 0;
    g_btn_status_reg = (btn_type_t)0;
    g_logic_level = (btn_type_t)0;
    btn_head = NULL;
    return 0;
}
/**
 * @brief 注册用户按钮结构实例
 * @param[in] lex_button_tbutton *button: 按钮结构实例
 * @return 返回以及注册的按钮数量，失败返回-1
*/
s32 dev_button_register(flex_button_t *button)
{
    flex_button_t *curr = btn_head;

    if (!button || (button_cnt > sizeof(btn_type_t) * 8))
    {
        return -1;
    }

    while (curr)
    {
        if (curr == button)
        {
            return -1;  /* already exist. */
        }
        curr = curr->next;
    }

    /**
    * 第一个注册按钮位于“链表”的末尾
    * btn_head 指向“链表”的头部
    */
    button->next = btn_head;
    button->status = FLEX_BTN_STAGE_DEFAULT;
    button->event = FLEX_BTN_PRESS_NONE;
    button->scan_cnt = 0;
    button->click_cnt = 0;
    button->max_multiple_clicks_interval = MAX_MULTIPLE_CLICKS_INTERVAL;
    btn_head = button;

    /**
    * 首先点击注册按钮后，该按钮被按下时的逻辑电平
    * 就会存储在 g_logic_level 的最低位中
    */
    g_logic_level |= (button->pressed_logic_level << button_cnt);
    button_cnt ++;

    return button_cnt;
}

/**
 * @brief 在一个扫描周期内读取所有键盘按键的状态位，并更新按键状态
 * @param void
 * @return 无
*/
static void dev_button_read(void)
{
    u8 i;
    flex_button_t *target;

    /*首先注册的按钮，其值位于raw_data的低位*/
    btn_type_t raw_data = 0;

//     for (target = btn_head, i = button_cnt - 1;(target != NULL) && (target->usr_button_read != NULL);target = target->next, i--)
//     {
//         raw_data = raw_data | ((target->usr_button_read)(target) << i);
//     }

    target = btn_head;
    i = button_cnt;
    while(i!=0)
    {
        i--;
        if((target != NULL) && (target->usr_button_read != NULL))
        {
            raw_data = raw_data | ((target->usr_button_read)(target) << i);
            target = target->next;
        }
    }
    g_btn_status_reg = (~raw_data) ^ g_logic_level;
}

/**
 * @brief 在一个扫描周期内处理所有关键事件。必须在'flex_button_read'函数调用之后使用
 * @param void
 * @return 返回按键按下的按键数量
*/
static u8 dev_button_process(void)
{
    uint8_t i;
    uint8_t active_btn_cnt = 0;
    flex_button_t* target;
    
    for (target = btn_head, i = button_cnt - 1; target != NULL; target = target->next, i--)
    {
        if (target->status > FLEX_BTN_STAGE_DEFAULT)
        {
            target->scan_cnt ++;
            if (target->scan_cnt >= ((1 << (sizeof(target->scan_cnt) * 8)) - 1))
            {
                target->scan_cnt = target->long_hold_start_tick;
            }
        }

        switch (target->status)
        {
        case FLEX_BTN_STAGE_DEFAULT: /* stage: 默认（按钮抬起状态）*/
            if (BTN_IS_PRESSED(i))   /* 被按压 */
            {
                target->scan_cnt = 0;
                target->click_cnt = 0;

                EVENT_SET_AND_EXEC_CB(target, FLEX_BTN_PRESS_DOWN);

                /* 转到按钮按下阶段 */
                target->status = FLEX_BTN_STAGE_DOWN;
            }
            else
            {
                target->event = FLEX_BTN_PRESS_NONE;
            }
            break;

        case FLEX_BTN_STAGE_DOWN: /* stage：按钮按下*/
            if (BTN_IS_PRESSED(i)) /* 被按压 */
            {
                if (target->click_cnt > 0) /* 多次点击 */
                {
                    if (target->scan_cnt > target->max_multiple_clicks_interval)
                    {
                        EVENT_SET_AND_EXEC_CB(target, 
                            target->click_cnt < FLEX_BTN_PRESS_REPEAT_CLICK ? 
                                target->click_cnt :
                                FLEX_BTN_PRESS_REPEAT_CLICK);

                        /*切换到按下按钮阶段*/
                        target->status = FLEX_BTN_STAGE_DOWN;
                        target->scan_cnt = 0;
                        target->click_cnt = 0;
                    }
                }
                else if (target->scan_cnt >= target->long_hold_start_tick)
                {
                    if (target->event != FLEX_BTN_PRESS_LONG_HOLD)
                    {
                        EVENT_SET_AND_EXEC_CB(target, FLEX_BTN_PRESS_LONG_HOLD);
                    }
                }
                else if (target->scan_cnt >= target->long_press_start_tick)
                {
                    if (target->event != FLEX_BTN_PRESS_LONG_START)
                    {
                        EVENT_SET_AND_EXEC_CB(target, FLEX_BTN_PRESS_LONG_START);
                    }
                }
                else if (target->scan_cnt >= target->short_press_start_tick)
                {
                    if (target->event != FLEX_BTN_PRESS_SHORT_START)
                    {
                        EVENT_SET_AND_EXEC_CB(target, FLEX_BTN_PRESS_SHORT_START);
                    }
                }
            }
            else /* 按键弹起 */
            {
                if (target->scan_cnt >= target->long_hold_start_tick)
                {
                    EVENT_SET_AND_EXEC_CB(target, FLEX_BTN_PRESS_LONG_HOLD_UP);
                    target->status = FLEX_BTN_STAGE_DEFAULT;
                }
                else if (target->scan_cnt >= target->long_press_start_tick)
                {
                    EVENT_SET_AND_EXEC_CB(target, FLEX_BTN_PRESS_LONG_UP);
                    target->status = FLEX_BTN_STAGE_DEFAULT;
                }
                else if (target->scan_cnt >= target->short_press_start_tick)
                {
                    EVENT_SET_AND_EXEC_CB(target, FLEX_BTN_PRESS_SHORT_UP);
                    target->status = FLEX_BTN_STAGE_DEFAULT;
                }
                else
                {
                    /* swtich to multiple click stage */
                    /*切换到多次点击阶段*/
                    target->status = FLEX_BTN_STAGE_MULTIPLE_CLICK;
                    target->click_cnt ++;
                }
            }
            break;

        case FLEX_BTN_STAGE_MULTIPLE_CLICK: /* stage: 多次点击 */
            if (BTN_IS_PRESSED(i)) /* 被按压 */
            {
                /* swtich to button down stage */
                /*切换到按下按钮阶段*/
                target->status = FLEX_BTN_STAGE_DOWN;
                target->scan_cnt = 0;
            }
            else
            {
                if (target->scan_cnt > target->max_multiple_clicks_interval)
                {
                    EVENT_SET_AND_EXEC_CB(target, 
                        target->click_cnt < FLEX_BTN_PRESS_REPEAT_CLICK ? 
                            target->click_cnt :
                            FLEX_BTN_PRESS_REPEAT_CLICK);

                    /* swtich to default stage */
                    /*切换到默认阶段*/
                    target->status = FLEX_BTN_STAGE_DEFAULT;
                }
            }
            break;
        }
        
        if (target->status > FLEX_BTN_STAGE_DEFAULT)
        {
            active_btn_cnt ++;
        }
    }
    
    return active_btn_cnt;
}

/**
 * @brief 获取指定按钮的按钮事件。
 * @param[in] lex_button_tbutton: 按钮结构实例
 * @return 按钮事件flex_button_event_t
*/
flex_button_event_t dev_button_event_read(flex_button_t *button)
{
    return (flex_button_event_t)(button->event);
}

/**
 * @brief 开始钥匙扫描,需要在指定时间内循环调用。采样周期：5-20ms
 * @param void
 * @return 返回按键按下的按键数量
*/
u8 dev_button_scan(void)
{
    dev_button_read();
    return dev_button_process();
}
