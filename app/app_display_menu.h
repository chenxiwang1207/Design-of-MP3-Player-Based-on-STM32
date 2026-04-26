#ifndef __APP_DISPALY_MENU_H
#define __APP_DISPALY_MENU_H

#include "com_type.h"
#include "platform_mcu_resource.h"

/*
* 
*                                                ----> 三级界面-菜单-游戏
*            ----> 菜单   ---->   二级界面菜单    ----> 三级界面-菜单-音乐
*                                                ---->      退出           ----> 二级界面菜单
*
*                        ---->       退出        ---->  一级界面
*                                            
*   一级界面                                 
*                                               ----> 三级界面-设置-系统设置
*                                               ----> 三级界面-设置-时钟
*            ----> 设置 ---->   二级界面设置     ----> 三级界面-设置-音乐设置
*                                               ---->         退出         ----> 二级界面设置
*
*                      ---->       退出         ---->  一级界面
*/

typedef enum
{
    app_menu_first = 0,

    app_menu_second_interface,   //二级界面菜单
    app_menu_second_setting,     //二级界面设置

    app_menu_third_game,    app_game_application,
    app_menu_third_music,   app_menu_music_application,

    app_menu_third_clock,
    app_menu_third_music_setting,
    app_menu_third_system_setting,

    app_menu_none,
    app_menu_max,
}menu_address;  //OLED此时显示是几级界面


void app_interface1_main_clock(void);
void app_interface1_main_UI(u8 value1,u8 value2);    //0反色显示，1原色显示
menu_address app_interface1_main(void);//一级界面

void app_interface2_menu_UI(void);
menu_address app_interface2_menu(void);//二级界面-菜单

void app_interface2_setting_UI(void);
menu_address app_interface2_setting(void);//二级界面-设置

menu_address app_interface3_game(void);//三级界面-菜单-游戏

menu_address app_interface3_music(void);//三级界面-菜单-音乐

void app_interface3_clock_firstPage(void);
void app_interface3_clock_secondPage(void);
menu_address app_interface3_clock(void);//三级界面-设置-时钟设置

void app_menu_UI(void);

#endif
