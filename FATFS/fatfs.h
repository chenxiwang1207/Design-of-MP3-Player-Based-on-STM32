#ifndef __FATFS_H
#define __FATFS_H

#include "com_type.h"
#include "platform_mcu_resource.h"
#include "ff.h"
#include "fatfs.h"

#define MAX_COUNT   50

typedef struct {
    char name[32];       //文件名或目录名
    char doc_format[8];  //文件格式，如 "MP3", "TXT"，目录或无后缀则为空字符串
    u8   is_dir;         //1: 目录, 0: 文件
} entry_t;

extern FATFS   fs;

char*    get_receive_data(void);
entry_t* get_dir_information(void);
int get_dir_count(void);
int get_all_information(const char* path, entry_t* entries, int max_count);

u8 sd_fatfs_init(void);
void fs_test(void);
void list_directories(const char* path);
s8   fs_enter_dir(char* path,u8 index);

#endif
