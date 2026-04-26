/**
 * @file      dev_button.h
 * @brief     button抽象层接口定义,参考:https://github.com/murphyzhao/FlexibleButton.git
 * @date      2025-04-07
 * @copyright https://www.breo.com/
 * 
 */
#ifndef __DEV_BUTTON_H__
#define __DEV_BUTTON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "com_type.h"

#define FLEX_BTN_SCAN_FREQ_HZ         30 // How often flex_button_scan () is called
#define FLEX_MS_TO_SCAN_CNT(ms)       (ms / (2000 / FLEX_BTN_SCAN_FREQ_HZ))

/* Multiple clicks interval, default 300ms */
#define MAX_MULTIPLE_CLICKS_INTERVAL  (FLEX_MS_TO_SCAN_CNT(300))

typedef void (*flex_button_response_callback)(void *);

typedef enum
{
    FLEX_BTN_PRESS_DOWN=0,              // 按下事件
    FLEX_BTN_PRESS_CLICK,               // 单击事件
    FLEX_BTN_PRESS_DOUBLE_CLICK,        // 双击事件
    FLEX_BTN_PRESS_REPEAT_CLICK,        // 连击事件，使用 flex_button_t 中的 click_cnt 断定连击次数
    FLEX_BTN_PRESS_SHORT_START,         // 短按开始事件
    FLEX_BTN_PRESS_SHORT_UP,            // 短按抬起事件
    FLEX_BTN_PRESS_LONG_START,          // 长按开始事件
    FLEX_BTN_PRESS_LONG_UP,             // 长按抬起事件
    FLEX_BTN_PRESS_LONG_HOLD,           // 长按保持事件
    FLEX_BTN_PRESS_LONG_HOLD_UP,        // 长按保持的抬起事件
    FLEX_BTN_PRESS_NONE,                // 无按键按下
    FLEX_BTN_PRESS_MAX,
}flex_button_event_t;

typedef enum {
//MP3_key_value
    BUTTON_VALUE_UP,                    //上按键值
    BUTTON_VALUE_DOWN,                  //下按键值
    BUTTON_VALUE_CONFIRM,               //确认按键值
    BUTTON_VALUE_LEFT,                  //左按键值
    BUTTON_VALUE_RIGHT,                 //右按键值
    BTN_VALUE_MAX,
}btn_value_t;

/**
 * flex_button_t
 *
 * @brief 按钮数据结构
 *        以下是需要在扫描前由用户初始化的成员。
 *
 * @member next
 *         内部使用。
 *         单向链表，指向下一个按钮。
 *
 * @member usr_button_read
 *         用户函数，用于读取按钮值。
 *
 * @member cb
 *         按钮事件回调函数。
 *
 * @member scan_cnt
 *         内部使用，用户只读。
 *         扫描计数，按钮按下时计数，每个扫描周期加一。
 *
 * @member click_cnt
 *         内部使用，用户只读。
 *         按钮点击次数
 *
 * @member max_multiple_clicks_interval
 *         多次点击间隔。默认值为 'MAX_MULTIPLE_CLICKS_INTERVAL'。
 *         需要使用 FLEX_MS_TO_SCAN_CNT 将毫秒转换为扫描计数。
 *
 * @member debounce_tick
 *         消抖时间。尚未使用。
 *         需要使用 FLEX_MS_TO_SCAN_CNT 将毫秒转换为扫描计数。
 *
 * @member short_press_start_tick
 *         短按开始时间。需要用户配置。
 *         需要使用 FLEX_MS_TO_SCAN_CNT 将毫秒转换为扫描计数。
 *
 * @member long_press_start_tick
 *         长按开始时间。需要用户配置。
 *         需要使用 FLEX_MS_TO_SCAN_CNT 将毫秒转换为扫描计数。
 *
 * @member long_hold_start_tick
 *         长保持按开始时间。需要用户配置。
 *
 * @member id
 *         按钮ID。需要用户配置。
 *         当多个按钮使用相同的按钮回调函数时，
 *         用于区分不同按钮。
 *         每个按钮ID必须是唯一的。
 *
 * @member pressed_logic_level
 *         需要用户配置。
 *         按钮按下时的逻辑电平，每位代表一个按钮。
 *
 * @member event
 *         内部使用，用户可以调用 'flex_button_event_read' 来获取当前按钮事件。
 *         用于记录当前按钮事件。
 *
 * @member status
 *         内部使用，用户不可访问。
 *         用于记录按钮的当前状态。
 *
*/
typedef struct flex_button
{
    struct flex_button *next;          //内部使用，单链表，指向下一个按钮
    u8 (*usr_button_read)(void *);     //用户设备的按键引脚电平读取函数，重要
    flex_button_response_callback  cb; //设置按键事件回调，用于应用层对按键事件的分类处理
    u16 scan_cnt;                      //用于记录扫描次数，按键按下是开始从零计数
    u16 click_cnt;                     //记录单击次数，用于判定单击、连击
    u16 max_multiple_clicks_interval;  //连击间隙，用于判定是否结束连击计数，有默认值 MAX_MULTIPLE_CLICKS_INTERVAL
    u16 debounce_tick;                 //消抖时间，暂未使用，依靠扫描间隙进行消抖
    u16 short_press_start_tick;        //短按开始时间,这个时间是函数扫描的执行周期的次数，周期*start_tick
    u16 long_press_start_tick;         //长按开始时间,这个时间是函数扫描的执行周期的次数，周期*start_tick
    u16 long_hold_start_tick;          //长按持续时间,这个时间是函数扫描的执行周期的次数，周期*start_tick
    u8  id;                            //单链表序号ID
    u8  pressed_logic_level : 1;       //设置按键按下的逻辑电平。1：标识按键按下的时候为高电平；0：标识按键按下的时候为低电平
    u8  event               : 4;       //用于记录当前按键事件
    u8  status              : 3;       //用于记录当前按键的状态，用于内部状态机
    btn_value_t   btu_value;           //按键的当前状态值
} flex_button_t;

/**
 * @brief 释放注册函数
 * @param void
 * @return 返回返回0成功，其他失败
*/
s32 dev_button_unregister(void);
/**
 * @brief 在一个扫描周期内处理所有关键事件。必须在'flex_button_read'函数调用之后使用
 * @param void
 * @return 返回按键按下的按键数量
*/
s32 dev_button_register(flex_button_t *button);
/**
 * @brief 获取指定按钮的按钮事件。
 * @param[in] lex_button_tbutton: 按钮结构实例
 * @return 按钮事件flex_button_event_t
*/
flex_button_event_t dev_button_event_read(flex_button_t *button);
/**
 * @brief 开始钥匙扫描,需要在指定时间内循环调用。采样周期：5-20ms
 * @param void
 * @return 返回按键按下的按键数量
*/
u8 dev_button_scan(void);

#ifdef __cplusplus
}
#endif

#endif
