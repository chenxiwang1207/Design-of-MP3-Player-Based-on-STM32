#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <ctype.h>
#include "stm32f10x.h"                  // Device header
#include "drv_usart.h"
#include "com_type.h"
#include "platform_mcu_resource.h"
#include "ff.h"
#include "fatfs.h"
#include "spi_sdcard.h"
#include "sdio_sdcard.h"
#include "drv_wdt.h"
#include "spi_vs1053.h"


#define FATFS_DEBUG 1
#if     FATFS_DEBUG
#else
#define  test_printf_uart1(...)
#endif
static int     entry_count;
static char    receive_data[1024] = {0};   //接收到文件中的数据
static entry_t doc_or_folder[MAX_COUNT];   //存储文件/文件夹信息的数组
FATFS   fs;

/**
 * @brief 获取指定目录下所有文件和文件夹信息
 *        并存放在 entries[max_count]中
 * @param path      目录路径，如 "0:"
 * @param entries   存储文件/文件夹的数组
 * @param max_count 存储获取到的文件/文件夹的最大数量
 * @return 实际获取到的文件/文件夹的总数量
 */
int get_all_information(const char* path, entry_t* entries, int max_count)
{
    DIR dir;
    FILINFO fno;
    FRESULT res;
    int count = 0;

    res = f_opendir(&dir, path);
    if (res != FR_OK) {
        return 0;
    }

    while (count < max_count) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0) break;

        //跳过 "." 和 ".."
        if (fno.fname[0] == '.' && (fno.fname[1] == '\0' || (fno.fname[1] == '.' && fno.fname[2] == '\0'))) 
        {
            continue;
        }

        // 复制名称
        strncpy(entries[count].name, fno.fname, sizeof(entries[count].name) - 1);
        entries[count].name[sizeof(entries[count].name) - 1] = '\0';
        entries[count].is_dir = (fno.fattrib & AM_DIR) ? 1 : 0;

        /* 提取文件后缀并存入 doc_format，目录或无后缀则置空 */
        if (entries[count].is_dir) {
            entries[count].doc_format[0] = '\0';
        } else {
            const char *dot = strrchr(fno.fname, '.');
            if (dot && dot[1] != '\0') {
                strncpy(entries[count].doc_format, dot + 1, sizeof(entries[count].doc_format) - 1);
                entries[count].doc_format[sizeof(entries[count].doc_format) - 1] = '\0';
                /* 转为大写，方便后续比较 */
                for (char *p = entries[count].doc_format; *p; ++p) {
                    *p = (char)toupper((unsigned char)*p);
                }
            } else {
                entries[count].doc_format[0] = '\0';
            }
        }
        count++;
    }
    f_closedir(&dir);
    return count;
}

/* 打印实际读取文件的字节数（十进制）并尝试以字符串方式显示 */
void printf_read_data(UINT len)
{
    /* 以字符串显示（确保以 NUL 结尾） */
    if (len < sizeof(receive_data)) {
        receive_data[len] = 0;
    } 
    else{
        receive_data[sizeof(receive_data) - 1] = 0;
    }
    test_printf_uart1("\r\nread str:%s", (char *)receive_data);
}


/**
 * @brief 获取指定路径下的所有子目录名称（不包含文件）
 *        必须在挂载文件系统后调用，并且路径必须存在且为目录
 *        (此函数已弃用可使用get_all_information来获取)
 * @param path 目录路径，例如 "0:/" 表示SD卡根目录
 */
void list_directories(const char* path)
{
//    FRESULT res;
//    DIR dir;
//    FILINFO fno;

//    //打开目录
//    res = f_opendir(&dir, path);
//    if (res != FR_OK) {
//        test_printf_uart1("\r\nFailed to open directory: %s, error: %d", path, res);
//        return;
//    }
//    test_printf_uart1("\r\nDirectories under %s:", path);

//    //遍历目录项
//    u32 i=0;
//    while(1) {
//        res = f_readdir(&dir, &fno);   //读取下一个目录项
//        if (res != FR_OK || fno.fname[0] == 0) {
//            break;  //错误或目录结束
//        }

//        //判断是否为目录（且不是 "." 和 ".."）
//        if ((fno.fattrib & AM_DIR) && (fno.fname[0] != '.')) {
//            test_printf_uart1("\r\n  %s", fno.fname);
//        }
//    }
//    f_closedir(&dir);
}

/**
 * @brief    进入指定目录
 * @param    path  : 路径
 * @param    index : 目录索引
 * @retval   0, 成功; 其他, 失败代码;
 * @note     index从1开始
 */
s8 fs_enter_dir(char* path,u8 index)
{
//    DIR dir;
//    FILINFO fno = {0};
//    FRESULT res;
//    uint16_t num = 0;

//    res = f_opendir(&dir, (const TCHAR *)path); /* 打开目录 */
//    if (res == FR_OK)
//    {
//        while (1)
//        {
//            fno.fname[0] = 0;
//            res = f_readdir(&dir, &fno); /* 读取目录项 */
//            if (res != FR_OK || fno.fname[0] == 0)
//                break; /* 出错或者目录结束 */
//            else
//                num++;

//            if (num == index)
//                sprintf((char *)path, "%s/%s", path, fno.fname);
//        }
//        f_closedir(&dir); /* 关闭目录 */
//        test_printf_uart1("\r\nopen dir success");
//        return 0;
//    }
//    else
//    {
//        test_printf_uart1("\r\nopen dir fail, res = %d", res);
//        return -1;
//    }
    return 0;
}

/**
 * @brief  获取读取到的文件中的数据
 * @return 读取到的文件中的数据
 */
char* get_receive_data(void)
{
    return receive_data;
}

/**
 * @brief  获取读取到的目录信息
 * @return 读取到的目录信息
 */
entry_t* get_dir_information(void) 
{
    return doc_or_folder;
}

int get_dir_count(void)
{
    return  entry_count;
}

u8 sd_fatfs_init(void)
{
    FRESULT ret;
    
//    FIL     fil;    
//    UINT    rwd_count;
//    FSIZE_t file_size;
    
/**********************挂载SD卡设备**********************/
    ret = f_mount(&fs,"0:",1);
    if(ret != 0){
        test_printf_uart1("\r\nFile system mounting failed, ret=%d", ret); //文件系统挂载失败
    }
    else{
        test_printf_uart1("\r\nFile system mounting success"); //文件系统挂载成功
    }

/*******读取指定路径下的所有子目录名称包含文件/文件夹*******/
    entry_count = get_all_information("0:/音乐",doc_or_folder,MAX_COUNT);
    if (entry_count > 0) 
    {
        test_printf_uart1("\r\nFound %d items in root directory:", entry_count);
        for (int i = 0; i < entry_count; i++)
        {
            if (doc_or_folder[i].is_dir)
            {
                test_printf_uart1("\r\n  %d.%s[DIR]" , i+1, doc_or_folder[i].name);
            }
            else
            {
                test_printf_uart1("\r\n  %d.%s[FILE]", i+1, doc_or_folder[i].name);
            }
        }
    }
    else
    {
        test_printf_uart1("\r\nNo items found or directory open failed.");
    }
    
/**********************打开文件**********************/
//    ret = f_open(&fil, "0:SD_TEST.txt", FA_READ | FA_WRITE | FA_CREATE_ALWAYS);  //FA_CREATE_ALWAYS-总是创建一个新文件，若有重名文件则直接覆盖
//    if(ret){
//        test_printf_uart1("\r\nFailed to open the file, ret=%d", ret); //文件打开失败
//    }
//    else{
//        test_printf_uart1("\r\nSuccessed to open the file"); //文件打开成功
//    }

/**********************读取数据**********************/ /*注意读写指针*/
//    ret = f_open(&fil, "0:/文件/SD_TEST.txt", FA_READ);
//    if (ret == FR_OK) {
//        test_printf_uart1("\r\nFile opened to success before reading");

//        /* 获取文件大小（在打开文件之后）并限制到接收缓冲区大小 */
//        file_size = f_size(&fil);
//        if (file_size == 0) {
//            test_printf_uart1("\r\nFile size is 0, nothing to read");
//        }
//        else{
//            if (file_size > sizeof(receive_data)) {
//                file_size = sizeof(receive_data);
//            }
//            rwd_count = 0;
//            ret = f_read(&fil, receive_data, (UINT)file_size, &rwd_count);
//            if (ret == FR_OK && rwd_count > 0) {
//                test_printf_uart1("\r\nRead data success");
//                printf_read_data(file_size);
//            }
//            else{
//                test_printf_uart1("\r\nRead data failed, ret=%d, read=%u", ret, rwd_count);
//            }
//        }
//        f_close(&fil);  //关闭文件
//    }
//    else{
//        test_printf_uart1("\r\nFile opened to fail before reading, ret=%d", ret);
//    }

/**********************写入数据**********************/ /*注意读写指针*/
//    ret = f_open(&fil, "0:/SD_TEST.txt", FA_WRITE);
//    if (ret == FR_OK) {
//        test_printf_uart1("\r\nFile opened to success before writing");
//        /* 获取文件大小（在打开文件之后）并限制到接收缓冲区大小 */
//        file_size = f_size(&fil);
//        if (file_size == 0) {
//            test_printf_uart1("\r\nFile size is 0, nothing to write");
//        }
//    }
//    rwd_count = 0;
//    u8 write_data[128];
//    for(u32 i=0;i<100;i++)
//    {
//        write_data[i] = i;
//    }
//    ret = f_write(&fil,write_data,sizeof(write_data),&rwd_count);
//    if(ret == 0){
//        test_printf_uart1("\r\nwrite data success");
//    }
//    else{
//        test_printf_uart1("\r\nwrite data failed,ret=%d",ret);
//    }
//    f_sync(&fil);   //同步文件
//    f_close(&fil);  //关闭文件
    return ret;
}


#define FATFS_TEST  0
#if     FATFS_TEST

/*------------------测试读写-----------------*/
void fs_test(void)
{
    static FATFS fs;
    static FIL fil;
    static BYTE work[FF_MIN_SS];

    /* 初始化 SD 卡硬件（确保底层驱动就绪） */
    s8 sd_ret = MSD_Init();
    if (sd_ret != MSD_RESPONSE_NO_ERROR)
    {
        test_printf_uart1("\r\nSD card initialization failed, ret=%d", sd_ret); //文件初始化失败
        return;
    }
    else
    {
        test_printf_uart1("\r\nSD card initialization success, ret=%d", sd_ret); //文件初始化成功
    }

    // 挂载并初始化
    FRESULT res = f_mount(&fs, "0:", 1);
    if (res == FR_NO_FILESYSTEM) {
        test_printf_uart1("\r\nFile system not found, f_mount res=%d", res); //文件系统未找到
        res = f_mkfs("0:", 0, work, sizeof(work));
        if (res) {
            test_printf_uart1("\r\nFile system formatting failed, res=%d", res); //文件系统格式化失败
            return ;
        } else {
            test_printf_uart1("\r\nFile system formatting success, res=%d", res); //文件系统格式化成功
            res = f_mount(&fs, "0:", 1);
            if (res) {
                test_printf_uart1("\r\nFile system mounting failed after format, res=%d", res); //文件系统挂载失败
                return;
            }
        }
    } else if (res) {
        test_printf_uart1("\r\nFile system mounting failed, res=%d", res); //文件系统挂载失败
    } else {
        test_printf_uart1("\r\nFile system mounting success, res=%d", res); //文件系统挂载成功
    }

    /* 打开并写入文件测试 */
    test_printf_uart1("\r\nFile system mounting success."); //文件系统挂载成功
    res = f_open(&fil, "0:/a.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        test_printf_uart1("\r\nFailed to open the file, res=%d", res); //打开文件失败
        goto error;
    }
    test_printf_uart1("\r\nSuccessed to open the file, res=%d", res); //打开文件成功

    static uint16_t test_write_buf[128];
    for (int i = 0; i < 128; i++) {
        test_write_buf[i] = (uint16_t)i;
    }

    UINT bw = 0;
    res = f_write(&fil, (void *)test_write_buf, sizeof(test_write_buf), &bw);
    if (res != FR_OK || bw != sizeof(test_write_buf)) {
        test_printf_uart1("\r\nFailed to write the file! res=%d, bw=%d", res, bw); //写入文件失败
        f_close(&fil);
        goto error;
    }
    f_sync(&fil);
    f_close(&fil);
    test_printf_uart1("\r\nFile writing and closing were successful. res=%d", res); //写入并关闭文件成功

    /* 重新打开并读取验证 */
    res = f_open(&fil, "0:/a.txt", FA_READ);
    if (res != FR_OK) {
        test_printf_uart1("\r\nFailed to open the file for reading, res=%d", res); //打开文件读取失败
        goto error;
    }
    test_printf_uart1("\r\nSuccessed to open the file for reading, res=%d", res); //打开文件读取成功

    static uint16_t test_read_buf[128];
    bw = 0;
    res = f_read(&fil, (void *)test_read_buf, sizeof(test_read_buf), &bw);
    if (res != FR_OK || bw != sizeof(test_read_buf)) {
        test_printf_uart1("\r\nFailed to read the file! res=%d, bw=%d", res, bw); //读文件失败
        f_close(&fil);
        goto error;
    }

    /* 打印读取到的内容 */
    {
        char print_buf[64];
        for (int i = 0; i < 128; i++) {
            test_printf_uart1("ReadData[%d]=%u,", i, test_read_buf[i]); //打印每个数据
        }
    }

    /* 比较数据 */
    if (memcmp(test_read_buf, test_write_buf, sizeof(test_read_buf)) != 0) {
        test_printf_uart1("\r\nData reading and writing failed, and the contents are inconsistent."); //数据读写失败，内容不一致
        f_close(&fil);
        goto error;
    }

    f_close(&fil);
    test_printf_uart1("\r\nReading and Writing Test Passed"); //读写测试通过

    // 创建并删除多个文件测试（在子目录下，避免根目录项限制）
    {
        static char path[64];
        // 先创建子目录 testdir（忽略已存在错误）
        res = f_mkdir("0:/testdir");
        if (res != FR_OK && res != FR_EXIST) {
            test_printf_uart1("\r\nmkdir testdir failed, res=%d", res);
            // 目录创建失败则后续操作无法继续，但可以继续执行其他测试
        } else {
            test_printf_uart1("\r\nmkdir testdir success (or already exists)");
        }

        // 创建子目录 dir1（同样处理已存在情况）
        res = f_mkdir("0:/dir1");
        if (res != FR_OK && res != FR_EXIST) {
            test_printf_uart1("\r\nmkdir dir1 failed, res=%d", res);
            // 如果 dir1 创建失败，后续文件操作都会失败，这里可提前跳转，但为了示例继续
        } else {
            test_printf_uart1("\r\nmkdir dir1 success (or already exists)");
        }

        //在 dir1 下创建文件
        for (int i = 0; i < 2; i++) {
            sprintf(path, "0:/dir1/file_txt%d.txt", i);
            res = f_open(&fil, path, FA_CREATE_ALWAYS | FA_WRITE);
            if (res != FR_OK) {
                test_printf_uart1("\r\nFailed to create the file %s, res=%d", path, res);
                continue;
            }
            f_close(&fil);
            test_printf_uart1("\r\nSuccessed to create the file %s", path);
        }

        //删除创建的文件
        for (int i = 0; i < 2; i++) {
            sprintf(path, "0:/dir1/file_txt%d.txt", i);
            res = f_unlink(path);
            if (res != FR_OK) {
                test_printf_uart1("\r\nFailed to delete the file %s, res=%d", path, res);
            } else {
                test_printf_uart1("\r\nSuccessed to delete the file %s", path);
            }
        }
        test_printf_uart1("\r\nFile creation and deletion test completed"); //文件创建与删除测试完成
    }
cleanup:
    f_mount(0, "0:", 0);
    return;

error:
    f_mount(0, "0:", 0);
    return;
}
#endif
