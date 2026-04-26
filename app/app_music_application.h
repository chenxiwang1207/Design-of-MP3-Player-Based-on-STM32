#ifndef __APP_MUSIC_APPLICATION_H
#define __APP_MUSIC_APPLICATION_H

#include "com_type.h"
#include "app_display_menu.h"

typedef struct{

    u16 g_song_time;    //歌曲总共时长(s)
    u8  g_sec;          
    u8  g_min;
}song_time_config_t;

u8 compare_filename(const char *filename);
u8 app_play_mp3_song(const char* filename);
void app_play_mp3_init(void);
void app_music_play_UI(void);
menu_address app_music_play(void);
u8 detect_button_state_cb(void);

void clear_song_time(void);
void app_decodetime_cb(void *arg);
void app_show_decodetime_UI(void);
u16 app_get_song_time(const char* filename);
u16 app_get_song_time_g(void);
void app_show_lrc_UI(void);


void test_music_play_UI(void);

#endif
