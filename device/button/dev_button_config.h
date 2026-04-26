#ifndef _DEV_BUTTON_CONFIG_H_
#define _DEV_BUTTON_CONFIG_H_
#include "dev_button.h"
#include "com_type.h"

#define BUTTON_MANAGE_PERIOD     20              ///< 按键执行周期
#define BUTTON_TIMER             USER_TIMER1     ///< 定时器号
#define BTN_ARRAY_COUNT          5               ///< 按键数量，不准对点阵按键

typedef void (*btn_cb)(flex_button_event_t event,btn_value_t value);

/**
 * @brief     按键初始化配置函数
 * @param[in] func 按键事件回调函数,主要用于处理用户操作按键时触发的按键事件
 * @return    void
 */
void dev_btn_config_init(btn_cb func);

#endif
