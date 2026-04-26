#include "stdio.h"
#include "string.h"
#include "stm32f10x.h"                  // Device header
#include "app_display_lrc.h"
#include "app_music_application.h"
#include "app_display_menu.h"
#include "ff.h"
#include "fatfs.h"
#include "oled.h"
#include "OLED_Data.h"
#include "drv_usart.h"
#include "drv_wdt.h"

#define APP_DISPLAY_LRC_DEBUG   0
#if     APP_DISPLAY_LRC_DEBUG
#else
#define test_printf_uart1(...)
#endif

#define LRC_LINES_MAX 80    //最大歌词行数
#define LRC_TEXT_MAX  48    //每行歌词文本长度

extern u8           show_lyrics_flag;   //显示歌词标志位，0表示不显示，1表示显示,默认显示歌词
extern entry_t*     app_music_list;     //全局变量，指向当前音乐列表,文件信息
extern int          app_music_count;    //全局变量，当前音乐列表的文件数

typedef struct{

    u32   time_sec;                //一行歌词的时间标签
    char  text[LRC_TEXT_MAX];      //歌词文本
}lrc_sync_t;

static lrc_sync_t lrc_table[LRC_LINES_MAX];
static u8   lrc_count = 0;            //有效歌词行数
static s8   cur_lrc_idx = -1;         //当前显示歌词索引
static u8   lrc_loaded = 0;           //歌词是否成功加载
static u8   lrc_update_flag = 0;      //歌词内容变化需刷新屏幕
static u8   cur_lrc_text[LRC_TEXT_MAX] = {0}; //当前显示歌词文本


/**
 * @brief 提取MP3文件的主文件名（去除最后的文件后缀）
 * @param filename  输入：完整文件名（如 "枫-周杰伦.mp3"）
 * @param basename  输出缓冲区
 * @param baselen   basename 缓冲区长度（字节）
 */
void extract_filename(const char *filename, char *basename, size_t baselen)
{
    if (basename == NULL || baselen == 0) return;
    strncpy(basename, filename, baselen - 1);
    basename[baselen - 1] = '\0';
    /* 去除最后一个点及其之后的后缀（兼容 .mp3/.MP3 等） */
    char *dot = strrchr(basename, '.');
    if (dot)
    {
        *dot = '\0';
    }
}

/**
 * @brief 解析一行 LRC 内容，提取时间标签（秒）和歌词文本
 * @param line  输入行（例如 "[01:15.00]窗外的麻雀"）
 * @param sec   输出：秒数（成功时有效）
 * @param text  输出：歌词文本（成功时有效）
 * @return 0 成功，1 失败
 */
u8 parse_lrc_line(const char *line, u16 *sec, char *text)
{
    const char *p = line;
    s32   minute = 0;
    s32   second = 0;
    
    while (*p == ' ' || *p == '\t')//跳过前导空白
    {
        p++;
    }
    if (*p != '[')
    {
        return 1;
    }
    p++;
    
    if (sscanf(p, "%d:%d", &minute, &second) != 2)
    {
        return 1;
    }
    
    //跳过时间部分至 ']'
    while (*p && *p != ']')
    {
        p++;
    }
    if (*p != ']')
    {
        return 1;
    }
    p++;
    
    while (*p == ' ' || *p == '\t') //跳过空白
    {
        p++;
    }
    
    if (*p == '\0')     //无歌词文本
    {
        return 1;
    }

    *sec = minute * 60 + second;
    strncpy(text, p, LRC_TEXT_MAX - 1);
    text[LRC_TEXT_MAX - 1] = '\0';
    return 0;
}

/*-----------------------------------------对外接口函数------------------------------------------*/
/**
 * @brief 根据当前播放秒数查找应显示的歌词索引
 * @param current_sec 当前播放秒数
 * @return 索引值，-1 表示无匹配
 */
s8 find_lrc_index(u16 current_sec)
{
    s8 idx = -1;
    for (u8 i = 0; i < lrc_count; i++) {
        if (lrc_table[i].time_sec <= current_sec)
            idx = i;
        else
            break;
    }
    return idx;
}

/**
 * @brief 加载指定 MP3 文件对应的歌词文件
 * @param mp3_filename MP3 文件名（例如 "枫-周杰伦.mp3"）
 *        使用时：app_get_lrc(app_music_list[index].name);
 * @return 0 成功，1 失败
 */
u8 app_get_lrc(const char* mp3_filename)
{
    u8    res;
    FIL   file;
    char  basename[64];
    char  filepath[64];
    char  line_buf[LRC_TEXT_MAX];
    u8    line_count = 0;
    
    //初始化状态
    lrc_count = 0;
    lrc_loaded = 0;
    cur_lrc_idx = -1;
    cur_lrc_text[0] = '\0';
    
    extract_filename(mp3_filename, basename, sizeof(basename));
    snprintf(filepath,sizeof(filepath),"0:/音乐歌词/%s-歌词.lrc",basename);
    res = f_open(&file,filepath,FA_READ);
    if(res != 0)
    {
        test_printf_uart1("\r\n打开歌词文件: %s.lrc失败, res=%d", basename, res);
        return 1;
    }
    /***逐行读取并解析***/
     while (line_count < LRC_LINES_MAX && f_gets(line_buf, sizeof(line_buf), &file))
     {
        //去除换行符
        char *p = strchr(line_buf, '\r'); if (p) *p = '\0';
        p = strchr(line_buf, '\n'); if (p) *p = '\0';
        // 去除 UTF-8 BOM（只可能出现在文件开头的第一行）
        if (  line_count == 0 && (unsigned char)line_buf[0] == 0xEF &&  \
             (unsigned char)line_buf[1] == 0xBB && (unsigned char)line_buf[2] == 0xBF)
        {
            size_t l = strlen(line_buf + 3);
            memmove(line_buf, line_buf + 3, l + 1);
        }
        
        u16 sec;
        char text[LRC_TEXT_MAX];
        if (parse_lrc_line(line_buf, &sec, text) == 0)
        {
            lrc_table[line_count].time_sec = sec;
            strcpy(lrc_table[line_count].text, text);
            line_count++;
        }
    }
    f_close(&file);
    
    if (line_count == 0) 
    {
        test_printf_uart1("\r\n[歌词] 无有效歌词行");
        return 1;
    }
    
    lrc_count = line_count;
    lrc_loaded = 1;
    lrc_update_flag = 1; // 请求首次刷新显示已加载的歌词
    test_printf_uart1("\r\n[歌词] 加载成功，共 %d 行", lrc_count);
    return 0;
}

/**
 * @brief 根据当前播放时间更新歌词显示（应在主循环时间刷新分支中调用）
 * @note  仅当歌词内容变化时才会更新显存并置位 lrc_update_flag
 */
u8 lrc_update_display(void)
{
    if (!lrc_loaded || lrc_count == 0)
    {
        return 1; // 无歌词可显示
    }

    #if 1
    u16 current_sec = app_get_song_time_g();
    #else
    u16 current_sec = app_get_song_time(app_music_list[index].name); //获取歌曲总时长（秒）
    #endif
    s8 idx = find_lrc_index(current_sec);

    if (idx < 0)
    {
        //当前时间没有匹配的歌词，若之前有显示则清除
        if (cur_lrc_idx != -1)
        {
            OLED_StopRoll();
            OLED_ClearArea(0, 18, 128, 16);
            lrc_update_flag = 1;
            cur_lrc_idx = -1;
            cur_lrc_text[0] = '\0';
        }
        return 0;
    }
    if (idx != cur_lrc_idx)
    {
        //歌词行发生变化，停止任何滚动，重绘歌词区域
        OLED_StopRoll();
        OLED_ClearArea(0, 18, 128, 16);

        //计算文本像素宽度：ASCII 约 8px，GB2312 汉字 16px
        u16 width = calc_string_width(lrc_table[idx].text);
        if (width > 128)
        {
            //文本超出屏宽，使用 OLED 的滚动显示
            OLED_ShowChinese_Roll(0, 18, lrc_table[idx].text, 100);
        }
        else
        {
            // //居中显示短文本
            // u8 start_x = 0;
            // if (width < 128) start_x = (128 - width) / 2;
            // OLED_ShowChineseString(start_x, 18, lrc_table[idx].text, _16x16);
            // OLED_UpdateArea(0, 18, 128, 16);

            //左对齐显示短文本
            OLED_ShowChineseString(0, 18, lrc_table[idx].text, _16x16);
            OLED_UpdateArea(0, 18, 128, 16);
        }
        //更新当前显示文本和索引
        strncpy((char*)cur_lrc_text, lrc_table[idx].text, LRC_TEXT_MAX - 1);
        cur_lrc_text[LRC_TEXT_MAX - 1] = '\0';
        cur_lrc_idx = idx;
        lrc_update_flag = 1;
    }
    return 0;
}
/**
 * @brief 获取当前歌词显示状态（是否需要刷新屏幕）
 * @return 1 需要刷新，0 不需要
 */
u8 lrc_get_refresh_flag(void)
{
    return lrc_update_flag;
}

/**
 * @brief 清除歌词刷新标志
 */
void lrc_clear_refresh_flag(void)
{
    lrc_update_flag = 0;
}

// /**
//  * @brief 强制刷新当前歌词（用于歌词开关打开时恢复显示）
//  */
// void lrc_force_redraw(void)
// {
//     if (!lrc_loaded || !show_lyrics_flag) return;
//     if (cur_lrc_idx >= 0) {
//         OLED_ClearArea(0,18,128,16);
//         OLED_ShowChinese_Roll(0,18,lrc_table[cur_lrc_idx].text,50);
//         lrc_update_flag = 1;
//     }
// }

/**
 * @brief 强制下一次重绘歌词（用于歌词开关打开时恢复显示）
 */
void lrc_force_redraw(void)
{
    OLED_ClearArea(0,18,128,16);
    cur_lrc_idx = -1; // 使下一次更新认为歌词已变化
    lrc_update_flag = 1;
}

/**
 * @brief 关闭歌词显示（清除歌词区域）
 */
void lrc_hide(void)
{
    OLED_StopRoll();
    OLED_ClearArea(0, 18, 128, 16);
    lrc_update_flag = 1;
}

/**
 * @brief 重置歌词模块状态（切歌时调用）
 */
void lrc_reset(void)
{
    lrc_loaded = 0;
    lrc_count = 0;
    cur_lrc_idx = -1;
    cur_lrc_text[0] = '\0';
    lrc_update_flag = 0;
    OLED_StopRoll();  // 停止可能正在进行的滚动
}
