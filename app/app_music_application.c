#include "stdio.h"
#include "string.h"
#include "stm32f10x.h"                  // Device header
#include "app_music_application.h"
#include "app_display_menu.h"
#include "app_display_lrc.h"
#include "drv_spi.h"
#include "ff.h"
#include "fatfs.h"
#include "drv_sys.h"
#include "spi_vs1053.h"
#include "drv_usart.h"
#include "drv_wdt.h"
#include "oled.h"
#include "OLED_Data.h"
#include "app_button.h"
#include "dev_button.h"
#include "dev_button_config.h"
#include "drv_timer.h"
#include "drv_partition.h"
#include "platform_partition_config.h"


#define APP_MUSIC_APPLICATION_DEBUG   0
#if     APP_MUSIC_APPLICATION_DEBUG
#else
#define test_printf_uart1(...)
#endif

#define TEST_PLAY_MP3   0

#define MP3_BUFFER_SIZE  1024
#define x                0
#define y                1

#define ORDER_PLAY       0x0A   //顺序播放
#define RANDOM_PLAY      0x0B   //随机播放
#define CYCLE_PLAY       0x0C   //单曲循环


extern volatile u32 g_page;    //此变量来自“app_display_menu.c”
extern volatile u32 g_pointer; //此变量来自“app_display_menu.c”

static app_button_e app_menu_button;         //全局变量，按键值
entry_t*     app_music_list   = NULL; //全局变量，指向当前音乐列表,文件信息
int          app_music_count  = 0;    //全局变量，当前音乐列表的文件数

static u8           play_song_mode   = ORDER_PLAY;  //播放歌曲模式
u8                  show_lyrics_flag = 1;     //显示歌词标志位，0表示不显示，1表示显示,默认显示歌词
static u8           play_stop_flag   = 0;     //暂停或播放标志位，0表示播放，1表示暂停
static s32          index            = 0;     //当前播放的是第几首歌曲

static volatile u8  timer_inited = 0;    //定时器是否已初始化标志，0表示未初始化，1表示已初始化
static volatile u16 g_song_time  = 0;    //播放一首歌曲的总时长
volatile        u8  g_sec        = 0;    //播放一首歌曲当前的秒数
volatile        u8  g_min        = 0;    //播放一首歌曲当前的分钟
static volatile u8  g_process    = 8;    //播放进度，x坐标（8-120共112次数）

static volatile s8  pointer[2]       = {0,0}; //pointer[x]表示光标横坐标的位置0,1,2...,pointer[y]=0表示光标在上面，pointer[y]=1表示光标在下面,光标默认在退出界面
static volatile s8  last_pointer[2]  = {0,0}; //上次光标的位置
static volatile u8  ui_update_flag   = 0;     //UI更新标志位，0表示不需要更新，1表示需要更新
static volatile u8  time_update_flag = 0;     //时间刷新标志

//对两个按键状态进行比较，0为按键相同，1为按键状态不相同
u8 button_compare(const app_button_e *button_a, const app_button_e *button_b)
{
    if((button_a->button_up_flag      == button_b->button_up_flag)      &&
       (button_a->button_down_flag    == button_b->button_down_flag)    &&
       (button_a->button_confirm_flag == button_b->button_confirm_flag) &&
       (button_a->button_left_flag    == button_b->button_left_flag)    &&
       (button_a->button_right_flag   == button_b->button_right_flag)
    )
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

// 检测按键状态回调函数，若发生改变则做出处理
// 返回值说明：
//   0xFF：未产生需要上层处理的按键事件（例如只是光标移动）
//   0x0A：在退出处按下确认键，需退出播放界面
//   0：  上一首
//   1：  歌词开关切换
//   2：  暂停/播放切换
//   3：  播放模式切换
//   4：  下一首
u8 detect_button_state_cb(void)
{
    app_button_e temp_button = app_menu_button;
    app_menu_button = App_Get_Button_Value();
    
    if(button_compare(&temp_button, &app_menu_button) == 0)
        return 0xFF;

    // 辅助变量：记录旧光标位置（用于清除高亮）
    s8 old_x = last_pointer[x];
    s8 old_y = last_pointer[y];

    /* ---------- 上按键短按抬起 ---------- */
    if(app_menu_button.button_up_flag == button_press_short_up)
    {
        // 1. 清除旧高亮
        OLED_ReverseArea(last_pointer[x]*28, last_pointer[y]*48, 16, 16);
        
        if(pointer[y] == 1)          // 下排 → 上排退出处
        {
            pointer[y] = 0;
            pointer[x] = 0;
        }
        else                         // 上排 → 下排第一个图标（上一首）
        {
            pointer[y] = 1;
            pointer[x] = 0;
        }
        // 2. 保存当前光标至 last_pointer，然后更新 pointer
        last_pointer[x] = pointer[x];
        last_pointer[y] = pointer[y];
        
        // 3. 点亮新高亮
        OLED_ReverseArea(last_pointer[x]*28, last_pointer[y]*48, 16, 16);
        ui_update_flag = 1;
        test_printf_uart1("\r\n光标位置[%d,%d]", pointer[x], pointer[y]);
    }
    /* ---------- 下按键短按抬起 ---------- */
    else if(app_menu_button.button_down_flag == button_press_short_up)
    {
        OLED_ReverseArea(last_pointer[x]*28, last_pointer[y]*48, 16, 16);

        if(pointer[y] == 0)          // 上排 → 下排第一个图标
        {
            pointer[y] = 1;
            pointer[x] = 0;
        }
        else                         // 下排 → 上排退出处
        {
            pointer[y] = 0;
            pointer[x] = 0;
        }
        last_pointer[x] = pointer[x];
        last_pointer[y] = pointer[y];
        OLED_ReverseArea(last_pointer[x]*28, last_pointer[y]*48, 16, 16);
        ui_update_flag = 1;
        test_printf_uart1("\r\n光标位置[%d,%d]", pointer[x], pointer[y]);
    }
    /* ---------- 左按键短按抬起 ---------- */
    else if(app_menu_button.button_left_flag == button_press_short_up)
    {
        if(pointer[y] == 1) // 仅在下排有效
        {
            OLED_ReverseArea(last_pointer[x]*28, last_pointer[y]*48, 16, 16);

            if(pointer[x] > 0)
                pointer[x]--;
            else
                pointer[x] = 4;      // 左边界循环至最右
            
            last_pointer[x] = pointer[x];
            last_pointer[y] = pointer[y];
            
            OLED_ReverseArea(last_pointer[x]*28, last_pointer[y]*48, 16, 16);
            ui_update_flag = 1;
            test_printf_uart1("\r\n光标位置[%d,%d]", pointer[x], pointer[y]);
        }
    }
    /* ---------- 右按键短按抬起 ---------- */
    else if(app_menu_button.button_right_flag == button_press_short_up)
    {
        if(pointer[y] == 1)
        {
            OLED_ReverseArea(last_pointer[x]*28, last_pointer[y]*48, 16, 16);
            
            if(pointer[x] < 4)
                pointer[x]++;
            else
                pointer[x] = 0;      // 右边界循环至最左
            
            last_pointer[x] = pointer[x];
            last_pointer[y] = pointer[y];
            
            OLED_ReverseArea(last_pointer[x]*28, last_pointer[y]*48, 16, 16);
            ui_update_flag = 1;
            test_printf_uart1("\r\n光标位置[%d,%d]", pointer[x], pointer[y]);
        }
    }
    /* ---------- 确认按键短按抬起 ---------- */
    else if(app_menu_button.button_confirm_flag == button_press_short_up)
    {
        // 退出图标
        if(pointer[x] == 0 && pointer[y] == 0)
        {
            drv_timer_stop(USER_TIMER2);
            drv_timer_close(USER_TIMER2);
            drv_timer_stop(USER_TIMER3);
            drv_timer_close(USER_TIMER3);
            OLED_StopRoll();
            timer_inited = 0;   //清除定时器初始化标志
            test_printf_uart1("\r\n退出音乐播放界面");
            return 0x0A;
        }
        
        if(pointer[y] == 1)
        {
            switch(pointer[x])
            {
                case 0: // 上一首
                    drv_timer_stop(USER_TIMER2);
                    drv_timer_close(USER_TIMER2);
                    drv_timer_stop(USER_TIMER3);
                    drv_timer_close(USER_TIMER3);
                    OLED_StopRoll();
                    drv_timer_stop(USER_TIMER4);
                    drv_timer_close(USER_TIMER4);
                    timer_inited = 0;
                    test_printf_uart1("\r\n上一首");
                    //此处理放在app_music_play()函数中
                    return 0;
                    
                case 1: //歌词开关
                    if(show_lyrics_flag == 0)
                    {
                        show_lyrics_flag = 1;
                        lrc_force_redraw();
                        OLED_Printf(28,48,_16x16,1,"词");
                        OLED_ReverseArea(28, 48, 16, 16);
                        test_printf_uart1("\r\n开启歌词");
                    }
                    else if(show_lyrics_flag == 1)
                    {
                        show_lyrics_flag = 0;
                        lrc_hide(); //关闭歌词显示
                        lrc_clear_refresh_flag();
                        OLED_ShowPicture(28,48,16,16,(u8*)close_lrc,1);
                        OLED_ReverseArea(28, 48, 16, 16);
                        test_printf_uart1("\r\n关闭歌词");
                    }
                    ui_update_flag = 1;
                    return 1;
                    
                case 2: //暂停/播放
                    play_stop_flag = !play_stop_flag;
                    if(play_stop_flag)
                    {
                        drv_timer_stop(USER_TIMER2);
                        drv_timer_stop(USER_TIMER3);
                        drv_timer_stop(USER_TIMER4);
                        OLED_ShowPicture(56, 48, 16, 16, (u8*)stop_play, 1);
                        test_printf_uart1("\r\n暂停播放");
                    }
                    else
                    {
                        drv_timer_start(USER_TIMER2);
                        drv_timer_start(USER_TIMER3);
                        drv_timer_start(USER_TIMER4);
                        OLED_ShowPicture(56, 48, 16, 16, (u8*)play_button, 1);
                        test_printf_uart1("\r\n播放歌曲");
                    }
                    //重新高亮该区域（因为 ShowPicture 覆盖了反色）
                    OLED_ReverseArea(56, 48, 16, 16);
                    ui_update_flag = 1;
                    return 2;
                    
                case 3: //播放模式切换
                    if(play_song_mode == ORDER_PLAY)
                    {
                        play_song_mode = RANDOM_PLAY;
                        OLED_ShowPicture(84, 48, 16, 16, (u8*)random, 1);
                        test_printf_uart1("\r\n切换为随机播放");
                    }
                    else if(play_song_mode == RANDOM_PLAY)
                    {
                        play_song_mode = CYCLE_PLAY;
                        OLED_ShowPicture(84, 48, 16, 16, (u8*)cycle, 1);
                        test_printf_uart1("\r\n切换为单曲循环");
                    }
                    else
                    {
                        play_song_mode = ORDER_PLAY;
                        OLED_ShowPicture(84, 48, 16, 16, (u8*)order, 1);
                        test_printf_uart1("\r\n切换为顺序播放");
                    }
                    OLED_ReverseArea(84, 48, 16, 16);
                    ui_update_flag = 1;
                    return 3;
                    
                case 4: // 下一首
                    drv_timer_stop(USER_TIMER2);
                    drv_timer_close(USER_TIMER2);
                    drv_timer_stop(USER_TIMER3);
                    drv_timer_close(USER_TIMER3);
                    OLED_StopRoll();
                    drv_timer_stop(USER_TIMER4);
                    drv_timer_close(USER_TIMER4);
                    timer_inited = 0;
                    test_printf_uart1("\r\n下一首");
                    //此处理放在 app_music_play()函数中
                    return 4;
            }
        }
    }
    //如果只是光标移动，可在此处立即刷新受影响的行
    if(ui_update_flag)
    {
        OLED_Update(); 
        ui_update_flag = 0; 
    }
    return 0xFF;
}

//比较输入的文件名和SD卡的音乐文件名
//若相同则放回0，否则返回1
u8 compare_filename(const char *filename)
{
    for(u32 i=0;i<app_music_count;i++)
    {
       if(strcmp(app_music_list[i].name,filename) == 0)
       {
           return 0;
       }
    }
    return 1;
}
/**
 * @brief       播放一曲MP3音乐
 * @param       filename : MP3文件路径
 */
u8 app_play_mp3_song(const char* filename)
{
    FIL  file;
    u8   res;
    static u8   mp3_buffer[MP3_BUFFER_SIZE];      //VS1053的MP3数据缓冲区
		#if APP_MUSIC_APPLICATION_DEBUG
    u32  file_size;
		#endif
    u32  off_set;
    UINT rwd = 0;
    
    // vs1053_sin_test(); //正弦测试
    // vs1053_set_all();  //设置VS1053的寄存器，进入解码模式，设置时钟频率、音频数据格式等
    
    if(compare_filename(filename) != 0) //对文件名进行判断若没找到此文件，则返回
    {
        test_printf_uart1("\r\n未找到此文件,请检查文件名是否错误");
        return 0xFF;
    }
    vs1053_restart_play();  //重启播放
    
    char filepath[64];
    snprintf(filepath,sizeof(filepath),"0:/音乐/%s",filename);
    res = f_open(&file,filepath,FA_READ);

    if(res == 0)
    {
        test_printf_uart1("\r\n打开音乐文件: %s 成功", filename);
    }
    else
    {
        test_printf_uart1("\r\n打开音乐文件: %s 失败, res=%d", filename, res);
        return 0xFF;
    }
		
		#if APP_MUSIC_APPLICATION_DEBUG
    file_size = f_size(&file);
    test_printf_uart1("\r\n文件大小: %d bytes",file_size);
    #endif
		
    while(1)
    {
        off_set=0;
        res =  f_read(&file,mp3_buffer,sizeof(mp3_buffer),&rwd);
        do{
            if(play_stop_flag == 0)
            {
                vs1053_write_data(mp3_buffer + off_set);
                off_set += 32;
            }
            
            u8 ret = detect_button_state_cb();
            if(ret == 0 || ret == 4)
            {
                test_printf_uart1("\r\n按键返回值：%d",ret);
                return ret;
            }
            if(ret == 0x0A)
            {
                return 0xFF;
            }
            drv_wdt_feed();
        }while(off_set < MP3_BUFFER_SIZE);

        if (show_lyrics_flag)
        {
            lrc_update_display(); //更新歌词显示（仅在歌词开启时）
        }
        /* 在音频播放循环里只做局部刷新，减少 I2C（bit-bang）总线占用，降低
         * 与音频 DMA/中断冲突概率，避免屏幕花屏。这里只刷新可能变化的行（
         * 退出/控制行通常在 y=0 或 y=48），每次刷新 1~2 page，代价远小于整屏刷新
         */
        if(ui_update_flag == 1)
        {
            u8 y_pos_a = (last_pointer[y] == 0) ? 0 : 48;
            u8 y_pos_b = (pointer[y] == 0) ? 0 : 48;
            OLED_UpdateArea(0, y_pos_a, 128, 16);
            if(y_pos_b != y_pos_a)
            {
                OLED_UpdateArea(0, y_pos_b, 128, 16);
            }
            ui_update_flag = 0;
        }
        if(time_update_flag == 1)
        {
            OLED_Printf(2, 36, _6x8, 0, "%d:%02d", g_min, g_sec);
            OLED_DrawLine(8,44,g_process,44,1);
            OLED_UpdateArea(2,16,128,34);
            time_update_flag = 0;
        }
        if(lrc_get_refresh_flag() == 1)
        {
            // OLED_UpdateArea(0,18,128,16);
            lrc_clear_refresh_flag();
        }
        drv_wdt_feed();
        if(rwd != MP3_BUFFER_SIZE || res != 0) //读取完成
        {
            break;
        }
    }
    test_printf_uart1("\r\nMP3文件播放完成");
    f_close(&file);
    return 0x0F;
}

void app_play_mp3_init(void)
{
    app_music_list  = get_dir_information(); //获取目录信息
    app_music_count = get_dir_count();       //获取目录数量

    vs1053_spi_init();
    vs1053_sin_test(); //正弦测试
    vs1053_set_all();  //设置VS1053的寄存器，进入解码模式，设置时钟频率、音频数据格式等
    
    test_printf_uart1("\r\nVS1053 初始化成功");
}

void app_music_calculate_index(void)
{
    if(g_page == 0)
    {
        index = g_pointer - 1;
    }
    else if(g_page > 0)
    {
        index = 3 + g_pointer + (4 * (g_page-1));
    }
    if(index >= app_music_count)
    {
        test_printf_uart1("\r\n文件错误");
    }
}

//读取flash中保存的歌曲播放模式，并在OLED上显示对应的图标
void song_mode_flash_Handle(void)
{
    u8 read_flashData = 0;
    
    drv_partition_read(DRV_PARTITION_UPDATE_TEMP, 0, &read_flashData, 1);
    test_printf_uart1("\r\nread song mode flag:0x%02X", read_flashData);
    if(read_flashData == ORDER_PLAY)        //顺序播放 -> 显示 order
        OLED_ShowPicture(84, 48, 16, 16, (u8*)order, 1);
    else if(read_flashData == RANDOM_PLAY)  //随机播放 -> 显示 random
        OLED_ShowPicture(84, 48, 16, 16, (u8*)random, 1);
    else                                    //单曲循环 -> 显示 cycle
        OLED_ShowPicture(84, 48, 16, 16, (u8*)cycle, 1);
    play_song_mode = read_flashData;
}

void app_music_play_UI(void)
{
    pointer[x] = 0;
    pointer[y] = 0;
    last_pointer[x] = pointer[x];
    last_pointer[y] = pointer[y];
    OLED_Clear();
    OLED_ShowPicture(0,0,16,16,(u8*)Return,1);			//显示返回键
    OLED_Printf(17,0,_16x16,1,"%s",app_music_list[index].name);		//显示歌曲信息
    OLED_ShowPicture(0,48,16,16,(u8*)last_song,1);	//显示上一首键
    if (show_lyrics_flag)
    {
        OLED_Printf(28,48,_16x16,1,"词");
        app_show_lrc_UI();  //显示歌词
    }
    else
    {
        lrc_hide();
        OLED_ShowPicture(28,48,16,16,(u8*)close_lrc,1);	//显示关闭歌词图标
    }
    OLED_ShowPicture(56,48,16,16,(u8*)play_button,1);		//显示播放/暂停歌曲键
    song_mode_flash_Handle();
    OLED_ShowPicture(112,48,16,16,(u8*)next_song,1);		//显示下一首键
    OLED_ReverseArea(0,0,16,16);  //起始光标在退出处
    app_show_decodetime_UI();			//显示歌曲当前播放时间和进度条
    OLED_Update();
}

menu_address app_music_play(void)
{
		#if APP_MUSIC_APPLICATION_DEBUG
		u8 res;
		#endif
    u8 ret;
    static u8 play_complete_flag = 0;   //mp3歌曲正常播放完成标志位，0x0F表示歌曲正常播放完成，其他则为用户手动切换歌曲
    app_play_mp3_init();
    app_music_calculate_index();

    while(1)
    {
        app_music_play_UI();
        clear_song_time();   //清零当前播放时间 (g_min = 0; g_sec = 0)
        test_printf_uart1("\r\n当前播放的歌曲:%s, index=%d",app_music_list[index].name,index);
        ret = app_play_mp3_song(app_music_list[index].name);
        play_complete_flag = ret;

			  #if APP_MUSIC_APPLICATION_DEBUG
        res = drv_partition_erase_write(DRV_PARTITION_UPDATE_TEMP,0,&play_song_mode,1);
        test_printf_uart1("\r\n擦写返回值：%d",res);
				#endif

        if(play_complete_flag != 0x0F)  //未播放完成切换音乐,按照播放模式来播放音乐
        {
            switch(ret)
            {
                case 0:     //点击上一首
                    if(play_song_mode == ORDER_PLAY)
                    {
                        index = index - 1;
                        if(index < 0) index = app_music_count - 1;
                    }
                    else if(play_song_mode == RANDOM_PLAY)
                    {
                        index = rand() % app_music_count;
                    }
                    else
                    {
                        index = index;
                    }
                    break;
                
                case 4:     //点击下一首
                    if(play_song_mode == ORDER_PLAY)
                    {
                        index = index + 1;
                        if(index >= app_music_count) index = 0;
                    }
                    else if(play_song_mode == RANDOM_PLAY)
                    {
                        index = rand() % app_music_count;
                    }
                    else
                    {
                        index = index;
                    }
                    break;

                case 0xFF:   //播放歌曲时出现读取SD卡错误，或文件不存在/损坏，或点击退出
                    return app_menu_third_music;
                default:
                    break;
            }
        }
        else if(play_complete_flag == 0x0F) //播放完成则按播放模式来切换音乐
        {
            drv_timer_stop(USER_TIMER2);
            drv_timer_close(USER_TIMER2);
            drv_timer_stop(USER_TIMER3);
            drv_timer_close(USER_TIMER3);
            drv_timer_stop(USER_TIMER4);
            drv_timer_close(USER_TIMER4);
            timer_inited = 0;
            
            switch(play_song_mode)
            {
                case ORDER_PLAY:        //顺序播放
                    index++;
                    if(index >= app_music_count){ index = 0; }
                    test_printf_uart1("\r\n下一首歌曲:%s,index:%d",app_music_list[index].name,index);
                    break;
                    
                case RANDOM_PLAY:       //随机播放
                    index = rand() % app_music_count;
                    test_printf_uart1("\r\n下一首歌曲:%s,index:%d",app_music_list[index].name,index);
                    break;
                
                case CYCLE_PLAY:        //单曲循环
                    index = index;
                    test_printf_uart1("\r\n下一首歌曲:%s,index:%d",app_music_list[index].name,index);
                    break;
            }
        }
    }
}

void clear_song_time(void)
{
    g_min = 0;
    g_sec = 0;
    g_process = 8;
}


/**
 * @brief       得到当前音频播放的时间
 * @param       无
 * @retval      解码时长(单位: s)
 */
u16 app_get_song_time(const char* filename)
{
    FIL  file;
    u8   res;
    u32  file_size  = 0;
    u32  total_time = 0;
    
    char filepath[64];
    snprintf(filepath,sizeof(filepath),"0:/音乐/%s",filename);
    res = f_open(&file,filepath,FA_READ);
    if(res != 0)
    {
        test_printf_uart1("\r\n打开音乐文件: %s 失败, res=%d", filename, res);
    }
    file_size  = f_size(&file);
    if(file_size > 7000000)     //高品质音乐文件，码率320kbps
    {
        total_time = (file_size * 8) / 320000;
    }
    else                        //普通品质音乐文件，码率128kbps
    {
        total_time = (file_size * 8) / 128000;
    }
    test_printf_uart1("\r\n歌曲总时长:%ds",total_time);
    f_close(&file);

    return total_time;
}

u16 app_get_song_time_g(void)
{
    test_printf_uart1("\r\n当前歌曲总时长: %d秒", g_min*60 + g_sec);
    return (g_min*60 + g_sec);
}

/**
 * @brief       得到当前多少ms调整一次进度条
 * @param       无
 * @retval      Xms
 */
static u32 app_get_process_ms(const char* filename)
{
    u32 process_ms = 0;   
    
    u16 total_time = app_get_song_time(filename);
    process_ms = (total_time * 1000) / 112; //每次调整进度对应的毫秒数
//    test_printf_uart1("\r\n计算得到每次调整进度对应的时间: %dms", process_ms);
    return process_ms;
}

void app_decodetime_cb(void *arg)
{
    //累计秒数
    g_sec++;
    if (g_sec >= 60)
    {
        g_sec = 0;
        g_min++;
    }
    //检查是否已播放到歌曲总长（g_song_time 由主函数设置）
    if ((g_min * 60 + g_sec) >= g_song_time)
    {
        g_min = 0;
        g_sec = 0;
    }
    time_update_flag = 1;
}

void app_process_cb(void *arg)
{
    g_process += 1; //每次增加1，根据实际需要调整增量和定时器间隔
    if (g_process > 120)
    {
        g_process = 8;
    }
    time_update_flag = 1;
}

//显示解码时间和进度条
void app_show_decodetime_UI(void)
{
    u8   min,sec;
    u32  process_ms;
    
    g_song_time = app_get_song_time(app_music_list[index].name);  //获取当前歌曲总时长（秒）
    process_ms  = app_get_process_ms(app_music_list[index].name); //获取多少ms调整一次进度条
    test_printf_uart1("\r\n读取每次调整进度对应的时间: %dms", process_ms);

    min = g_song_time / 60;
    sec = g_song_time - (min * 60);

    //绘制静态部分：总时间（右侧）和进度条底线
    OLED_Printf(100, 36, _6x8, 0, "%d:%02d", min, sec);
    OLED_DrawLine(8, 44, 9, 44, 1);
    OLED_Printf(2, 36, _6x8, 0, "0:00");
    
    //仅第一次进入音乐播放界面时初始化定时器
    if (timer_inited == 0)
    {
        drv_timer_init(USER_TIMER2,1000,app_decodetime_cb);
        drv_timer_init(USER_TIMER3,process_ms,app_process_cb);
        timer_inited = 1;
    }
}

//显示歌词界面
void app_show_lrc_UI(void)
{
    u8 res;

    lrc_reset();     //重置歌词模块状态
    #if TEST_PLAY_MP3
    app_get_lrc("枫-周杰伦.mp3");
    #else
    test_printf_uart1("\r\n正在加载歌曲%s的歌词...", app_music_list[index].name);
    res = app_get_lrc(app_music_list[index].name);    //解析歌词文件，获取歌词行数和内容
    if(res != 0)
    {
        test_printf_uart1("\r\n加载指定MP3音乐歌词错误,res=%d",res);
        return;
    }
    #endif
    lrc_update_display(); //更新歌词显示
}

#if  TEST_PLAY_MP3
void test_music_play_UI(void)
{
    app_play_mp3_init();
    app_music_calculate_index();
    app_music_play_UI();
    app_play_mp3_song("枫-周杰伦.mp3");
}
#endif
