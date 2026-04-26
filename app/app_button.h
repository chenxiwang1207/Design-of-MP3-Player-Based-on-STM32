#ifndef __APP_BUTTON_H
#define __APP_BUTTON_H

#include "com_type.h"
#include "platform_mcu_resource.h"

typedef enum {
    button_press_click = 0,         //按键单击
    button_press_short_start,       //按键短按开始
    button_press_short_up,          //按键短按抬起
    button_press_long_start,        //按键长按开始
    button_press_long_hold_start,   //按键长按保持
    button_press_long_up,           //按键长按(/长按保持)抬起
    button_press_none,
}app_button_value_e;

typedef struct {
    app_button_value_e button_up_flag;
    app_button_value_e button_down_flag;
    app_button_value_e button_confirm_flag;
    app_button_value_e button_left_flag;
    app_button_value_e button_right_flag;
}app_button_e;

//初始化按键
void App_Button_Init(void);
//获取按键值
app_button_e App_Get_Button_Value(void);

#endif
