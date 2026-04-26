#include "stdio.h"
#include "stm32f10x.h"                  // Device header
#include "dev_RTC.h"
#include "app_button.h"
#include "app_display_menu.h"
#include "oled.h"
#include "OLED_Data.h"
#include "drv_sys.h"

//年选项设置
void app_interface3_clock_setYear(void)
{
    app_button_e app_menu_button;
    while(1)
    {
        app_menu_button = App_Get_Button_Value();
        if(app_menu_button.button_up_flag == button_press_short_up ||
           app_menu_button.button_up_flag == button_press_long_hold_start )   //上按键短按抬起/长按
        {
            MyRTC_Time[0] += 1;
            if(MyRTC_Time[0] > 2099) MyRTC_Time[0] = 2000;
            app_interface3_clock_firstPage();
            OLED_ReverseArea(16,0,32,16);    //光标只覆盖年选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_down_flag == button_press_short_up ||
                app_menu_button.button_up_flag   == button_press_long_hold_start )   //下按键短按抬起/长按
        {
            MyRTC_Time[0] -= 1;
            if(MyRTC_Time[0] < 2000) MyRTC_Time[0] = 2099;
            app_interface3_clock_firstPage();
            OLED_ReverseArea(16,0,32,16);    //光标只覆盖年选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_confirm_flag == button_press_short_up)   //按下确认按键
        {
            OLED_ShowPicture(14,7,100,50,(u8*)success,1);
            drv_delay_ms(1000);
            return;
        }
    }
}
//月选项设置
void app_interface3_clock_setMonth(void)
{
    app_button_e app_menu_button;
    while(1)
    {
        app_menu_button = App_Get_Button_Value();
        if(app_menu_button.button_up_flag == button_press_short_up ||
           app_menu_button.button_up_flag == button_press_long_hold_start )   //上按键短按抬起/长按
        {
            MyRTC_Time[1] += 1;
            if(MyRTC_Time[1] > 13) MyRTC_Time[1] = 0;
            app_interface3_clock_firstPage();
            OLED_ReverseArea(16,16,16,16);    //光标只覆盖月选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_down_flag == button_press_short_up ||
                app_menu_button.button_up_flag   == button_press_long_hold_start )   //下按键短按抬起/长按
        {
            MyRTC_Time[1] -= 1;
            if(MyRTC_Time[1] < 0) MyRTC_Time[1] = 12;
            app_interface3_clock_firstPage();
            OLED_ReverseArea(16,16,16,16);    //光标只覆盖月选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_confirm_flag == button_press_short_up)   //按下确认按键
        {
            OLED_ShowPicture(14,7,100,50,(u8*)success,1);
            drv_delay_ms(1000);
            return;
        }
    }
}
//日选项设置
void app_interface3_clock_setDay(void)
{
    app_button_e app_menu_button;
    while(1)
    {
        app_menu_button = App_Get_Button_Value();
        if(app_menu_button.button_up_flag == button_press_short_up ||
           app_menu_button.button_up_flag == button_press_long_hold_start )   //上按键短按抬起/长按
        {
            MyRTC_Time[2] += 1;
            if(MyRTC_Time[2] > 32) MyRTC_Time[2] = 1;
            app_interface3_clock_firstPage();
            OLED_ReverseArea(16,32,16,16);    //光标只覆盖日选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_down_flag == button_press_short_up ||
                app_menu_button.button_up_flag   == button_press_long_hold_start )   //下按键短按抬起/长按
        {
            MyRTC_Time[2] -= 1;
            if(MyRTC_Time[2] < 1) MyRTC_Time[2] = 31;
            app_interface3_clock_firstPage();
            OLED_ReverseArea(16,32,16,16);    //光标只覆盖日选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_confirm_flag == button_press_short_up)   //按下确认按键
        {
            OLED_ShowPicture(14,7,100,50,(u8*)success,1);
            drv_delay_ms(1000);
            return;
        }
    }
}
//时选项设置
void app_interface3_clock_setHour(void)
{
    app_button_e app_menu_button;
    while(1)
    {
        app_menu_button = App_Get_Button_Value();
        if(app_menu_button.button_up_flag == button_press_short_up ||
           app_menu_button.button_up_flag == button_press_long_hold_start )   //上按键短按抬起/长按
        {
            MyRTC_Time[3] += 1;
            if(MyRTC_Time[3] > 25) MyRTC_Time[3] = 0;
            app_interface3_clock_firstPage();
            OLED_ReverseArea(16,48,16,16);    //光标只覆盖时选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_down_flag == button_press_short_up ||
                app_menu_button.button_up_flag   == button_press_long_hold_start )   //下按键短按抬起/长按
        {
            MyRTC_Time[3] -= 1;
            if(MyRTC_Time[3] < 0) MyRTC_Time[3] = 24;
            app_interface3_clock_firstPage();
            OLED_ReverseArea(16,48,16,16);    //光标只覆盖时选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_confirm_flag == button_press_short_up)   //按下确认按键
        {
            OLED_ShowPicture(14,7,100,50,(u8*)success,1);
            drv_delay_ms(1000);
            return;
        }
    }
}
//分选项设置
void app_interface3_clock_setMinute(void)
{
    app_button_e app_menu_button;
    while(1)
    {
        app_menu_button = App_Get_Button_Value();
        if(app_menu_button.button_up_flag == button_press_short_up ||
           app_menu_button.button_up_flag == button_press_long_hold_start )   //上按键短按抬起/长按
        {
            MyRTC_Time[4] += 1;
            if(MyRTC_Time[4] > 61) MyRTC_Time[4] = 0;
            app_interface3_clock_secondPage();
            OLED_ReverseArea(16,0,16,16);    //光标只覆盖分选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_down_flag == button_press_short_up ||
                app_menu_button.button_up_flag   == button_press_long_hold_start )   //下按键短按抬起/长按
        {
            MyRTC_Time[4] -= 1;
            if(MyRTC_Time[4] < 0) MyRTC_Time[4] = 60;
            app_interface3_clock_secondPage();
            OLED_ReverseArea(16,0,16,16);    //光标只覆盖分选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_confirm_flag == button_press_short_up)   //按下确认按键
        {
            OLED_ShowPicture(14,7,100,50,(u8*)success,1);
            drv_delay_ms(1000);
            return;
        }
    }
}
//秒选项设置
void app_interface3_clock_setSec(void)
{
    app_button_e app_menu_button;
    while(1)
    {
        app_menu_button = App_Get_Button_Value();
        if(app_menu_button.button_up_flag == button_press_short_up ||
           app_menu_button.button_up_flag == button_press_long_hold_start )   //上按键短按抬起/长按
        {
            MyRTC_Time[5] += 1;
            if(MyRTC_Time[5] > 61) MyRTC_Time[5] = 0;
            app_interface3_clock_secondPage();
            OLED_ReverseArea(16,16,16,16);    //光标只覆盖秒选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_down_flag == button_press_short_up ||
                app_menu_button.button_up_flag   == button_press_long_hold_start )   //下按键短按抬起/长按
        {
            MyRTC_Time[5] -= 1;
            if(MyRTC_Time[5] < 0) MyRTC_Time[5] = 60;
            app_interface3_clock_secondPage();
            OLED_ReverseArea(16,16,16,16);    //光标只覆盖秒选项的数字
            OLED_Update();
            MyRTC_SetTime();
        }
        else if(app_menu_button.button_confirm_flag == button_press_short_up)   //按下确认按键
        {
            OLED_ShowPicture(14,7,100,50,(u8*)success,1);
            drv_delay_ms(1000);
            return;
        }
    }
}
