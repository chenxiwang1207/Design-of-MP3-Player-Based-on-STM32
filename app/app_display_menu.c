#include "stdio.h"
#include "string.h"
#include "stm32f10x.h"                  // Device header
#include "oled.h"
#include "Delay.h"
#include "fatfs.h"
#include "dev_RTC.h"
#include "drv_timer.h"
#include "drv_usart.h"
#include "drv_sys.h"
#include "drv_wdt.h"
#include "app_music_application.h"
#include "app_display_menu.h"
#include "app_setTime.h"
#include "app_button.h"
#include "OLED_Data.h"

#define APP_DISPLAY_MENU_DEBUG   0
#if     APP_DISPLAY_MENU_DEBUG
#else
#define test_printf_uart1(...)
#endif

static entry_t* g_music_list;    //全局变量，指向当前音乐列表,文件信息
static int      g_music_count;   //全局变量，当前音乐列表的文件数


/**
 * @brief 读取 RTC 并在 OLED 上显示当前日期与时间。
 *
 * 从 RTC 缓存读取时间数据到 MyRTC_Time，并以指定字体格式在屏幕上绘制日期和时分秒。
 */
void app_interface1_main_clock(void)
{
    MyRTC_ReadTime();
    OLED_Printf(0,0,_6x8,0,"%d-%d-%d",MyRTC_Time[0],MyRTC_Time[1],MyRTC_Time[2]);
    OLED_Printf(16,16,_12x24,0,"%02d:%02d:%02d",MyRTC_Time[3],MyRTC_Time[4],MyRTC_Time[5]);
    OLED_ShowPicture(110,0,16,8,(u8*)charge,1);
}

/**
 * @brief 绘制一级界面主 UI（两组中文和时钟）。
 *
 * @param value1 左侧中文显示模式（0：反色显示，1：原色显示）
 * @param value2 右侧中文显示模式（0：反色显示，1：原色显示）
 *
 * 会调用 app_interface1_main_clock 更新时钟显示并刷新 OLED。
 */
void app_interface1_main_UI(u8 value1,u8 value2)    //0反色显示，1原色显示
{
    for(u32 i=12;i<=13;i++)
    {
        u32 index = i-12;
        OLED_ShowChinese(0+index*16,48,i,_8x16,value1);
    }
    for(u32 i=14;i<=15;i++)
    {
        u32 index = i-14;
        OLED_ShowChinese(96+index*16,48,i,_8x16,value2);
    }
    app_interface1_main_clock();
    OLED_Update();
}

/**
 * @brief 一级界面主循环，处理左右切换与确认键。
 *
 * 在本循环中响应左右按键切换菜单/设置指针，确认键返回下一级界面地址。
 *
 * @return menu_address 下一步要进入的界面地址。
 */
menu_address app_interface1_main(void)
{
    s8 pointer_addr  = 0;  //光标位置，0为在菜单处，1为在设置处
    
    menu_address interface1_flag;   //返回值
    app_button_e app_menu_button;   //按键值
    
    app_interface1_main_UI(0,1);
    while(1)
    {
        drv_wdt_feed();
        app_menu_button = App_Get_Button_Value();
        if(app_menu_button.button_left_flag == button_press_short_up)   //左按键短按抬起
        {
            pointer_addr -= 1;
            if(pointer_addr < 0)
            {
                pointer_addr = 1;
            }
            test_printf_uart1("\r\nleft button:%d",app_menu_button.button_left_flag);
        }
        else if(app_menu_button.button_right_flag == button_press_short_up) //右按键短按抬起
        {
            pointer_addr += 1;
            if(pointer_addr > 1)
            {
                pointer_addr = 0;
            }
            test_printf_uart1("\r\nright button:%d",app_menu_button.button_right_flag);
        }
        if(pointer_addr == 0)   { app_interface1_main_UI(0,1); }
        else                    { app_interface1_main_UI(1,0); }

        if(app_menu_button.button_confirm_flag == button_press_short_up)   //按下确认按键
        {
            if(pointer_addr == 0)   //在菜单处按下确认
            {
                interface1_flag = app_menu_second_interface;
            }
            else    //在设置处按下确认
            {
                interface1_flag = app_menu_second_setting;
            }
            OLED_Clear();
            return interface1_flag;
        }
    }
}

/**
 * @brief 绘制二级菜单页面（音乐、游戏、退出）。
 */
void app_interface2_menu_UI(void)
{
    OLED_Printf(0,8,_8x16,0,"1.");
    OLED_ShowChinese(16,8,  18,_8x16,1); /* 音 */
    OLED_ShowChinese(32,8, 19,_8x16,1);  /* 乐 */

    OLED_Printf(0,24,_8x16,0,"2.");
    OLED_ShowChinese(16,24,22,_8x16,1);  /* 游 */
    OLED_ShowChinese(32,24,23,_8x16,1);  /* 戏 */

    /* 退出选项保持无编号 */
    OLED_ShowChinese(0,40, 10,_8x16,1); /* 退 */
    OLED_ShowChinese(16,40,11,_8x16,1); /* 出 */
}

/**
 * @brief 二级菜单主循环，处理上下/确认按键并返回所选界面地址。
 *
 * @return menu_address 选择的下一级界面地址。
 */
menu_address app_interface2_menu(void)
{
    s8 pointer_addr = 0;          //当前光标位置 0为音乐，1为游戏，2为退出
    s8 last_addr = 0;             //上一次光标位置
    app_button_e app_menu_button;
    menu_address interface2_meun_flag;

    app_interface2_menu_UI();
    /* 高亮第一项：高度改为16以覆盖16像素的中文 */
    OLED_ReverseArea(0, 8, 128, 16);
    OLED_Update();

    while (1)
    {
        drv_wdt_feed();
        app_menu_button = App_Get_Button_Value();
        Delay_ms(5);
        if (app_menu_button.button_up_flag == button_press_short_up)
        {
            OLED_ReverseArea(0, 8 + last_addr * 16, 128, 16);//擦除旧光标（将旧光标区域取反，恢复为正常显示）
            pointer_addr--;//更新光标位置
            if (pointer_addr < 0)
            {
                pointer_addr = 2;
            }
            last_addr = pointer_addr;
            OLED_ReverseArea(0, 8 + pointer_addr * 16, 128, 16);//绘制新光标（将新光标区域取反，高亮显示）
            OLED_Update();//一次性刷新全屏，将两处修改同时生效
        }
        else if (app_menu_button.button_down_flag == button_press_short_up)
        {
            OLED_ReverseArea(0, 8 + last_addr * 16, 128, 16);//擦除旧光标
            pointer_addr++;//更新光标位置
            if (pointer_addr > 2) 
            {
                pointer_addr = 0;
            }
            last_addr = pointer_addr;
            OLED_ReverseArea(0, 8 + pointer_addr * 16, 128, 16);//绘制新光标
            OLED_Update();
        }
        else if (app_menu_button.button_confirm_flag == button_press_short_up)
        {
            switch (pointer_addr)
            {
                case 0: interface2_meun_flag = app_menu_third_music; break;
                case 1: interface2_meun_flag = app_menu_third_game;  break;
                case 2: interface2_meun_flag = app_menu_first;       break;
            }
            OLED_Clear();
            return interface2_meun_flag;
        }
    }
}

/**
 * @brief 绘制二级设置界面（时钟设置/音乐设置/系统设置/退出）。
 */
void app_interface2_setting_UI(void)
{
    /* 使用字库中的中文：时钟设置 / 音乐设置 / 系统设置 / 退出 */
    OLED_Printf(0,0,_8x16,0,"1.");
    OLED_ShowChinese(16,0,16,_8x16,1); /* 时 */
    OLED_ShowChinese(32,0,17,_8x16,1); /* 钟 */
    OLED_ShowChinese(48,0,14,_8x16,1); /* 设 */
    OLED_ShowChinese(64,0,15,_8x16,1); /* 置 */
    OLED_Printf(0,16,_8x16,0,"2.");
    OLED_ShowChinese(16,16,18,_8x16,1); /* 音 */
    OLED_ShowChinese(32,16,19,_8x16,1); /* 乐 */
    OLED_ShowChinese(48,16,14,_8x16,1); /* 设 */
    OLED_ShowChinese(64,16,15,_8x16,1); /* 置 */
    OLED_Printf(0,32,_8x16,0,"3.");
    OLED_ShowChinese(16,32,20,_8x16,1); /* 系 */
    OLED_ShowChinese(32,32,21,_8x16,1); /* 统 */
    OLED_ShowChinese(48,32,14,_8x16,1); /* 设 */
    OLED_ShowChinese(64,32,15,_8x16,1); /* 置 */
    OLED_ShowChinese(0, 48,10,_8x16,1); /* 退 */
    OLED_ShowChinese(16,48,11,_8x16,1); /* 出 */
    OLED_Update();
}

/**
 * @brief 二级设置主循环，处理光标移动与确认，返回选择的界面地址。
 *
 * @return menu_address 选择的下一级界面地址。
 */
menu_address app_interface2_setting(void)
{
    menu_address interface2_setting_flag;
    static app_button_e app_menu_button;
    s8 pointer_addr = 0;          //当前光标位置 0~3
    s8 last_addr = 0;             //上一次光标位置

    app_interface2_setting_UI();
    /* 高亮首项（16像素高） */
    OLED_ReverseArea(0, 0, 128, 16);
    OLED_Update();

    while (1)
    {
        drv_wdt_feed();
        app_menu_button = App_Get_Button_Value();

        if (app_menu_button.button_up_flag == button_press_short_up)
        {
            OLED_ReverseArea(0, last_addr * 16, 128, 16);//擦除旧光标
            pointer_addr--;//更新光标位置
            if (pointer_addr < 0) pointer_addr = 3;
            last_addr = pointer_addr;
            OLED_ReverseArea(0, pointer_addr * 16, 128, 16);//绘制新光标
            OLED_Update();
        }
        else if (app_menu_button.button_down_flag == button_press_short_up)
        {
            OLED_ReverseArea(0, last_addr * 16, 128, 16);//擦除旧光标
            pointer_addr++;//更新光标位置
            if (pointer_addr > 3) pointer_addr = 0;
            last_addr = pointer_addr;
            OLED_ReverseArea(0, pointer_addr * 16, 128, 16);//绘制新光标
            OLED_Update();
        }
        else if (app_menu_button.button_confirm_flag == button_press_short_up)
        {
            switch (pointer_addr)
            {
                case 0: interface2_setting_flag = app_menu_third_clock;          break;
                case 1: interface2_setting_flag = app_menu_third_music_setting;  break;
                case 2: interface2_setting_flag = app_menu_third_system_setting; break;
                case 3: interface2_setting_flag = app_menu_first;                break;
            }
            OLED_Clear();
            return interface2_setting_flag;
        }
    }
}

/**
 * @brief 三级菜单-游戏界面，显示游戏选项并等待确认返回。
 *
 * @return menu_address 返回上一级界面地址（通常为 app_menu_first）。
 */
menu_address app_interface3_game(void)
{
    static app_button_e app_menu_button;
//    menu_address interface3_meun_game_flag;
//    s8 pointer_addr = 0;          //当前光标位置 0~3
//    s8 last_addr = 0;             //上一次光标位置

    /* 显示中文：贪 吃 蛇 / 退出 */
    OLED_Printf(0,0,_8x16,0,"1.");
    OLED_ShowChinese(16,0,24,_8x16,1);  /* 贪 */
    OLED_ShowChinese(32,0,25,_8x16,1);  /* 吃 */
    OLED_ShowChinese(48,0,26,_8x16,1);  /* 蛇 */
    OLED_ShowChinese(0,16,10,_8x16,1);  /* 退 */
    OLED_ShowChinese(16,16,11,_8x16,1); /* 出 */
    OLED_ReverseArea(0,0,128,16); //光标取反表示画面指针，起始在时钟位置
    OLED_Update();

    while(1)
    {
        drv_wdt_feed();
        app_menu_button = App_Get_Button_Value();
        if(app_menu_button.button_up_flag == button_press_short_up)   //上按键短按抬起
        {
        }
        else if(app_menu_button.button_down_flag == button_press_short_up)  //下按键短按抬起
        {
        }
        else if(app_menu_button.button_confirm_flag == button_press_short_up)   //按下确认按键
        {
            return app_menu_first;
        }
    }
}

/**
 * @brief 显示音乐列表的指定页（每页 items_per_page 项）
 *
 * @param list 指向文件名数组的指针
 * @param count 列表总项数
 * @param page 要显示的页索引（从 0 开始）
 * @param items_per_page 每页显示的项数（通常为4，第一页实际显示3个文件+1个退出图标）
 */
static void app_music_show_page(entry_t *list, int count, int page, int items_per_page)
{
    int i;
    if (items_per_page < 1) items_per_page = 4;
    int per_first = items_per_page - 1; //第一页文件数（3个）
    int last_page;                      //最后一页是第几页（从0开始）
    int mid_page_count;                 //中间页数（不含第一页和最后一页）
    
    //计算总页数及最后一页索引
    if (count <= per_first)      //文件总数不超过第一页容量（3个）
    {
        last_page = 0;           //只有第一页
    }
    else
    {
        int remaining = count - per_first;                 //去掉第一页后的剩余文件数
        mid_page_count = (remaining - 1) / items_per_page; //中间完整页数（每页4个）
        last_page = mid_page_count + 1;                    //最后一页索引
    }
    
    OLED_Clear();
/* ---------- 第一页：退出图标 + 最多 per_first 个文件 ---------- */
    if (page == 0)
    {
        
        OLED_ShowPicture(0, 0, 16, 16, (u8*)Return, 1);
        for (i = 0; i < per_first; i++)
        {
            int idx = i; // 第一页文件索引从 0 开始
            if (idx >= count) break;
            OLED_Printf(0, (i + 1) * 16, _8x16, 0, "%d.", idx + 1);
            OLED_Printf(16, (i + 1) * 16, _16x16, 1, "%s", list[idx].name);
        }
    }        
/* ---------- 最后一页：显示剩余文件（最多 items_per_page 个）---------- */
    else if (page == last_page)
    {

        int start = per_first + (page - 1) * items_per_page;//起始索引 = 第一页已占用的 per_first 个 + 前面所有完整页的文件数
        int remaining = count - start;                      //剩余文件数
        int show_count = (remaining > items_per_page) ? items_per_page : remaining;

        for (i = 0; i < show_count; i++)
        {
            int idx = start + i;
            OLED_Printf(0, i * 16, _8x16, 0, "%d.", idx + 1);
            OLED_Printf(16, i * 16, _16x16, 1, "%s", list[idx].name);
        }
    }
/* ---------- 中间页：满页 items_per_page 个文件 ---------- */
    else
    {
        int start = per_first + (page - 1) * items_per_page;//起始索引 = 第一页占用的 per_first + (page-1) * items_per_page
        for (i = 0; i < items_per_page; i++)
        {
            int idx = start + i;
            if (idx >= count) break; //理论上中间页不会提前结束，但保留安全检查
            OLED_Printf(0, i * 16, _8x16, 0, "%d.", idx + 1);
            OLED_Printf(16, i * 16, _16x16, 1, "%s", list[idx].name);
        }
    }
    OLED_Update();
}

/**
 * @brief 初始化音乐列表缓存并显示第一页
 *
 * 从 SD 卡读取目录信息到全局缓存 g_music_list/g_music_count，并显示第一页内容
 */
void app_interface3_music_UI(void)
{
    /* 从 SD 读取到文件级缓存 g_music_list/g_music_count */
    g_music_list  = get_dir_information();  //获取目录信息
    g_music_count = get_dir_count();        //获取目录下文件/文件夹数量
    app_music_show_page(g_music_list, g_music_count, 0, 4); //显示第一页，第一页含退出图标+3项
}

/**
 * @brief 三级菜单-音乐主循环，支持分页浏览歌曲并响应按键选择。
 *
 * 短按确认键返回上级，当前实现不自动播放，仅支持查看文件名与翻页。
 *
 * @return menu_address 返回要进入的下一个界面地址。
 */
/*全局变量，为了提供给其他模块使用*/
volatile u32 g_page = 0;
volatile u32 g_pointer = 0;

menu_address app_interface3_music(void)
{
    static app_button_e app_menu_button;
    const int rows = 4;             //屏幕总行数（包含第一页的退出图标）
    const int per_first = rows - 1; //第1页的文件项数（不含退出图标）
    int page = 0;             //当前页（0 为带退出图标的第一页）
    int pointer = 0;          //页内指针
    int last_pointer = 0;

    app_interface3_music_UI();
    if (g_music_count == 0)
    {
        return app_menu_second_interface;
    }
    last_pointer = pointer;

    /* 初始光标显示：第一页第0行为退出图标，取反显示代表当前选中 */
    OLED_ReverseArea(0,0,16,16); //初始光标在退出图标
    OLED_Update();

    while (1)
    {
        drv_wdt_feed();
        app_menu_button = App_Get_Button_Value();

        if (app_menu_button.button_up_flag == button_press_short_up)    //上按键短按抬起
        {
            /* 上键：先擦除当前光标显示 */
            if (last_pointer == 0 && page == 0)
            {
                OLED_ReverseArea(0,0,16,16);
            }
            else
            {
                OLED_ReverseArea(0, last_pointer * 16, 128, 16);
            }

            /* 光标上移 */
            pointer--;
            if (pointer < 0)
            {
                /* 如果不是第一页，则翻到上一页末尾 */
                if (page > 0)
                {
                    page--;
                    if (page == 0)//返回到第一页，计算第一页实际显示的文件数
                    {
                        
                        int items_in_first = (g_music_count < per_first) ? g_music_count : per_first;
                        pointer = items_in_first; //上移到退出图标位置，方便用户选择退出
                    }
                    else
                    {
                        pointer = rows - 1;/* 中间页末尾为 rows-1 */
                    }
                    app_music_show_page(g_music_list, g_music_count, page, rows);
                }
                else    //第一页上移则循环到最后一页末尾
                {
                    int last_page;
                    if (g_music_count <= per_first)
                    {
                        last_page = 0;
                    }
                    else
                    {
                        int remaining = g_music_count - per_first;
                        last_page = 1 + ((remaining - 1) / rows);
                    }
                    page = last_page;
                    int items_in_last;
                    if (page == 0)
                    {
                        items_in_last = (g_music_count < per_first) ? g_music_count : per_first;
                    }
                    else
                    {
                        int remaining = g_music_count - page * per_first;
                        items_in_last = remaining;
                        if (items_in_last > rows) items_in_last = rows;
                    }
                    pointer = items_in_last - 1;
                    app_music_show_page(g_music_list, g_music_count, page, rows);
                }
            }
            /* 绘制新光标 */
            last_pointer = pointer;
            if (last_pointer == 0 && page == 0)
            {
                OLED_ReverseArea(0,0,16,16);
            }
            else
            {
                OLED_ReverseArea(0, last_pointer * 16, 128, 16);
            }
            OLED_Update();
        }
        else if (app_menu_button.button_down_flag == button_press_short_up) //下按键短按抬起
        {
            pointer++;
            int items_in_current;
            //擦除旧光标
            if (last_pointer == 0 && page == 0)
            {
                OLED_ReverseArea(0,0,16,16);
            }
            else
            {
                OLED_ReverseArea(0, last_pointer * 16, 128, 16);
            }

            if (page == 0)  //第一页，计算当前页实际显示的文件数（不含退出图标）
            {
                items_in_current =((g_music_count < per_first) ? g_music_count : per_first) + 1; //加1是因为第一页有退出图标占位
            }
            else    //其他页，计算当前页实际显示的文件数
            {
                int remaining = g_music_count - page * per_first;
                items_in_current = remaining;
                if (items_in_current > rows) items_in_current = rows;
                if (items_in_current < 0) items_in_current = 0;
            }

            if (pointer >= items_in_current)    //当前页内越界，则翻到下一页首项
            {
                int last_page;
                if (g_music_count <= per_first) //文件数不超过第一页容量，则只有第一页，下一页即第一页
                {
                    last_page = 0;
                }
                else
                {
                    int remaining_total = g_music_count - per_first;
                    last_page = 1 + ((remaining_total - 1) / rows); //计算最后一页索引
                }

                if (page < last_page)   //如果不是最后一页，则翻到下一页
                {
                    page++;
                    pointer = 0;
                    app_music_show_page(g_music_list, g_music_count, page, rows);
                }
                else    //最后一页下移则循环回第一页
                {
                    page = 0;
                    pointer = 0;
                    app_music_show_page(g_music_list, g_music_count, page, rows);
                }
            }

            last_pointer = pointer;
            //绘制新光标
            if (last_pointer == 0 && page == 0)
            {
                OLED_ReverseArea(0,0,16,16);
            }
            else
            {
                OLED_ReverseArea(0, last_pointer * 16, 128, 16);
            }
            OLED_Update();
        }
        else if (app_menu_button.button_confirm_flag == button_press_short_up)
        {
            if(page == 0 && pointer == 0)   //第一页的退出处
            {
                OLED_Clear();
                return app_menu_second_interface;
            }
            else
            {
                g_page = page;
                g_pointer = pointer;
                OLED_Clear();
                return app_menu_music_application;  //进入播放音乐界面
            }
        }
    }
}

/**
 * @brief 绘制时钟设置第一页（年、月、日、时）。
 */
void app_interface3_clock_firstPage(void)
{
    OLED_Printf(0,0 ,_8x16,0,"1.%04d",MyRTC_Time[0]);
    OLED_ShowChinese(48,0,0,_8x16,1);
    OLED_Printf(0,16,_8x16,0,"2.%02d",MyRTC_Time[1]);
    OLED_ShowChinese(32,16,1,_8x16,1);
    OLED_Printf(0,32,_8x16,0,"3.%02d",MyRTC_Time[2]);
    OLED_ShowChinese(32,32,2,_8x16,1);
    OLED_Printf(0,48,_8x16,0,"4.%02d",MyRTC_Time[3]);
    OLED_ShowChinese(32,48,3,_8x16,1);
}

/**
 * @brief 绘制时钟设置第二页（分、秒与退出）。
 */
void app_interface3_clock_secondPage(void)
{
    OLED_Printf(0,0 ,_8x16,0,"5.%02d",MyRTC_Time[4]);
    OLED_ShowChinese(32,0,4,_8x16,1);
    OLED_Printf(0,16,_8x16,0,"6.%02d",MyRTC_Time[5]);
    OLED_ShowChinese(32,16,5,_8x16,1);

    OLED_ShowChinese(0,32,10,_8x16,1);
    OLED_ShowChinese(16,32,11,_8x16,1);
}

/**
 * @brief 三级界面-时钟设置主循环，处理翻页、导航与进入子项设置。
 *
 * 按确认键进入对应的年/月/日/时/分/秒设置函数，完成后返回主菜单。
 *
 * @return menu_address 设置完成后要返回的界面地址。
 */
menu_address app_interface3_clock(void)
{
    static app_button_e app_menu_button;
    s8 pointer_addr = 0;          //当前光标位置 0~3
    s8 last_addr = 0;             //上一次光标位置
    s8 page_flag = 0;             //页标志，0为年月日页，1为时分秒页
    app_interface3_clock_firstPage();
    OLED_ReverseArea(0,0,128,16); //光标取反表示画面指针，起始在年位置
    OLED_Update();
    while(1)
    {
        drv_wdt_feed();
        app_menu_button = App_Get_Button_Value();
        if(app_menu_button.button_up_flag == button_press_short_up ||      //上按键短按抬起
           app_menu_button.button_up_flag == button_press_long_start)      //上按键长按开始
        {
            OLED_ReverseArea(0,last_addr*16,128,16); //擦除旧光标
            pointer_addr -= 1;
            if(page_flag == 0)  //在第一页
            {
                //第一页有 4 项，索引 0..3
                if(pointer_addr < 0)
                {
                    //跳转到第二页，定位到第二页末尾
                    page_flag = 1;
                    OLED_Clear();
                    app_interface3_clock_secondPage();
                    pointer_addr = 2; //第二页末尾索引（假定为 0..2）
                }
            }
            else    //在第二页
            {
                //第二页有 3 项，索引 0..2
                if(pointer_addr < 0)
                {
                    //跳转到第一页，定位到第一页末尾
                    page_flag = 0;
                    OLED_Clear();
                    app_interface3_clock_firstPage();
                    pointer_addr = 3; //第一页末尾索引（0..3）
                }
            }
            last_addr = pointer_addr;
            OLED_ReverseArea(0,pointer_addr*16,128,16); //绘制新光标
            OLED_Update();
        }
        else if(app_menu_button.button_down_flag == button_press_short_up ||  //下按键短按抬起
                app_menu_button.button_down_flag == button_press_long_start)  //下按键长按开始
        {
            OLED_ReverseArea(0,last_addr*16,128,16); //擦除旧光标
            pointer_addr += 1;
            if(page_flag == 0)
            {
                //第一页 0..3
                if(pointer_addr > 3)
                {
                    //跳转到第二页，定位到第二页起始
                    page_flag = 1;
                    OLED_Clear();
                    app_interface3_clock_secondPage();
                    pointer_addr = 0;
                }
            }
            else
            {
                //第二页 0..2
                if(pointer_addr > 2)
                {
                    //跳转回第一页，定位到第一页起始
                    page_flag = 0;
                    OLED_Clear();
                    app_interface3_clock_firstPage();
                    pointer_addr = 0;
                }
            }
            last_addr = pointer_addr;
            OLED_ReverseArea(0,pointer_addr*16,128,16); //绘制新光标
            OLED_Update();
        }
        else if(app_menu_button.button_confirm_flag == button_press_short_up)   //确认键短按抬起
        {
            OLED_Clear();
            if(page_flag == 0)   //在年月日页，进入对应的设置界面
            {
                app_interface3_clock_firstPage();
                switch (pointer_addr)
                {
                    case 0: /*年设置界面*/
                        OLED_ReverseArea(16,0,32,16);    //光标只覆盖年选项的数字
                        OLED_Update();
                        app_interface3_clock_setYear();
                        break;
                    case 1: /*月设置界面*/
                        OLED_ReverseArea(16,16,16,16);   //光标只覆盖月选项的数字
                        OLED_Update();
                        app_interface3_clock_setMonth();
                        break;
                    case 2: /*日设置界面*/
                        OLED_ReverseArea(16,32,16,16);   //光标只覆盖日选项的数字
                        OLED_Update();
                        app_interface3_clock_setDay();
                        break;
                    case 3: /*时设置界面*/
                        OLED_ReverseArea(16,48,16,16);   //光标只覆盖时选项的数字
                        OLED_Update();
                        app_interface3_clock_setHour();
                        break;
                }
                OLED_Clear();
                return app_menu_third_clock;
            }
            else if(page_flag == 1)  //在时分秒页，进入对应的设置界面
            {
                app_interface3_clock_secondPage();
                switch (pointer_addr)
                {
                    case 0:/*分设置界面*/
                        OLED_ReverseArea(16,0,16,16);   //光标只覆盖分选项的数字
                        OLED_Update();
                        app_interface3_clock_setMinute();
                        break;
                    case 1:/*秒设置界面*/
                        OLED_ReverseArea(16,0,16,16);   //光标只覆盖秒选项的数字
                        OLED_Update();
                        app_interface3_clock_setSec();
                        break;
                    case 2:
                        return app_menu_second_setting;
                }
                OLED_Clear();
                return app_menu_third_clock;
            }
        }
    }
}

volatile menu_address menu_flag = app_menu_first; //起始默认在一级界面
/**
 * @brief 主菜单调度循环，根据 menu_flag 跳转到各级界面并喂狗。
 *
 * 持续运行，调用各界面处理函数并在必要时更新 menu_flag。
 */
void app_menu_UI(void)
{
    test_printf_uart1("\r\nmenu init start");
    while(1)
    {
        drv_wdt_feed();
        switch(menu_flag)
        {
            /*---------------------一级界面-主页---------------------*/
            case app_menu_first:
                menu_flag = app_interface1_main();  //进入 主界面
                break;
            
            /*---------------------二级界面-菜单---------------------*/    
            case app_menu_second_interface:
                menu_flag = app_interface2_menu();   //进入 主界面->菜单
                break;
            
            /*---------------------二级界面-设置---------------------*/
            case app_menu_second_setting:
                menu_flag = app_interface2_setting();
                break;
            
            /*---------------------三级界面-菜单-游戏---------------------*/
            case app_menu_third_game:
                break;
            
            /*---------------------三级界面-菜单-音乐---------------------*/
            case app_menu_third_music:
                /*音乐歌曲内容*/
                menu_flag = app_interface3_music();
                break;
            
            /*---------------------四级界面-菜单-音乐播放界面---------------------*/
            case app_menu_music_application:
                menu_flag = app_music_play();
                break;
            
            /*---------------------三级界面-设置-时钟---------------------*/
            case app_menu_third_clock:
                menu_flag = app_interface3_clock();
                break;
            
            /*---------------------三级界面-设置-音乐设置---------------------*/
            case app_menu_third_music_setting:
                break;
            
            /*---------------------三级界面-设置-系统设置---------------------*/
            case app_menu_third_system_setting:
                break;
            
            default:
                break;
        }
    }
}


