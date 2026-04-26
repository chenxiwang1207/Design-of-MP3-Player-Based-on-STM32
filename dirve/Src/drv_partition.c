/*            哦弥陀佛 我佛慈悲  
                   _ooOoo_
                  o8888888o
                  88" . "88
                  (| -_- |)
                  O\  =  /O
               ____/`---'\____
             .'  \\|     |//  `.
            /  \\|||  :  |||//  \
           /  _||||| -:- |||||-  \
           |   | \\\  -  /// |   |
           | \_|  ''\---/''  |   |
           \  .-\__  `-`  ___/-. /
         ___`. .'  /--.--\  `. . __
      ."" '<  `.___\_<|>_/___.'  >'"".
     | | :  `- \`.;`\ _ /`;.`/ - ` : | |
     \  \ `-.   \_ __\ /__ _/   .-` /  /
======`-.____`-.___\_____/___.-`____.-'======
                   `=---='
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
            佛祖保佑       永无BUG^-^
*/
#include <string.h>
#include "stdio.h"
#include "stm32f10x.h"
#include "platform_mcu_resource.h"
#include "platform_partition_config.h"
#include "drv_partition.h"
#include "drv_usart.h"

#define INTERNAL_FLASH_START      MCU_FLASH_INT_BASE         //起始地址
#define INTERNAL_FLASH_END        MCU_FLASH_INT_END_ADDRESS  //终止地址
#define INTERNAL_FLASH_SIZE       MCU_FLASH_INT_SIZE         //内部flash大小
#define INTERNAL_FALSH_PAGE_SIZE  FLASH_PAGE_INT_SIZE        //页大小


//全局分区表指针、用户参数
static drv_partition_table_t *usr_partition_table = NULL; 
static u32 usr_partition_num = 0;
static u8  lock_flag = 0;

/**
* @brief 解锁内部FLASH
* @return 返回执行结果
* @retval DRV_OK    成功
* @retval DRV_ERROR   失败
*/
s32 drv_unlock_clear_errflag(void)
{
	if(lock_flag == 1)		 //上锁状态
    {
        FLASH_Unlock();      //使用解锁内部 flash
        lock_flag = 0;
        // 清除所有标志位（EOP/WRPERR/OPTVERR）
        FLASH_ClearFlag(FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR | FLASH_FLAG_EOP);
        return DRV_OK;
    }
  else return DRV_OK;
}

/**
* @brief  锁上内部FLASH
* @return 返回执行结果
* @retval DRV_OK    成功
* @retval DRV_ERROR   失败
*/
s32 drv_partition_lock(void)
{
    if(lock_flag == 0)	   //解锁状态
    {
        FLASH_Lock();      //锁上内部flash
        lock_flag = 1;
        return DRV_OK;
    }
    else
    {
      return DRV_OK;
    }
}
/**
* @brief  写入数据至内部FLASH
* @param  u32 addr       地址
* @param  u8  *in_buf    写入数据缓冲区
* @param  u32 size       数据长度
* @return 返回执行结果
* @retval DRV_OK      成功
* @retval DRV_ERROR   失败
*/
s32 drv_write_internal_flash(u32 addr,u8* in_buf,u32 size)
{
    if(addr < INTERNAL_FLASH_START || addr+size > INTERNAL_FLASH_START + INTERNAL_FLASH_SIZE
                                                                        || addr + size < addr)
    {
        return DRV_INVAL;
    }
    // 检查字对齐（4字节）
    if(addr % 4 != 0 || size % 4 != 0)
    {
        return DRV_INVAL;
    }

    drv_unlock_clear_errflag();
    u32 timeout = 10000;

    // 等待 FLASH 不忙
    while(FLASH_GetStatus() == FLASH_BUSY)   // SPL 中 FLASH_GetStatus 返回状态
    {
        timeout--;
        if(timeout == 0)
        {
            return DRV_TIMEOUT;
        }
    }
    // 逐字（word）写入
    for(u32 offset = 0; offset < size; offset += 4)
    {
        uint32_t data = *((uint32_t*)(in_buf + offset));
        FLASH_ProgramWord(addr + offset, data);
        // 等待本次写操作完成或超时
        timeout = 10000;
        while(FLASH_GetStatus() == FLASH_BUSY)
        {
            timeout--;
            if(timeout == 0)
            {
                drv_partition_lock();
                return DRV_TIMEOUT;
            }
        }
        // 检查错误标志
        if(FLASH_GetStatus() != FLASH_COMPLETE)
        {
            drv_partition_lock();
            return DRV_ERROR;
        }
    }
    // 清除结束标志（可选）
    FLASH_ClearFlag(FLASH_FLAG_EOP);
    drv_partition_lock();
    return DRV_OK;
}

/**
* @brief  读取内部FLASH数据，数据地址和长度需要字对齐
* @param  u32 addr  地址
* @param  u8  *data 写入数据指针
* @param  u32 size  数据长度
* @return 返回执行结果
* @retval DRV_OK      成功
* @retval DRV_ERROR   失败
*/
s32 drv_read_internal_flash(u32 addr,u8* data,u32 size)
{
    if(addr < INTERNAL_FLASH_START || addr+size > INTERNAL_FLASH_START+INTERNAL_FLASH_SIZE
																	 || addr + size < addr)
    {
        return DRV_INVAL;
    }
    for(u32 i=0;i<size;i++)
    {
        data[i] = *(volatile u8*)(addr + i);
    }
    return DRV_OK;
}

/**
* @brief 擦除内部FLASH页面
* @param[in] start_addr: 起始地址
* @param[in] size      : 擦除数据长度
* @return 返回执行结果
* @retval DRV_OK    擦除成功
* @retval DRV_ERROR 擦除失败
*/
s32 drv_internal_erase_page(u32 addr)
{
    u32 time_out = 100000;
    drv_unlock_clear_errflag();
    // 等待 FLASH 不忙
    while(FLASH_GetStatus() == FLASH_BUSY)
    {
        if(time_out==0)
        {
            return DRV_ERROR;
        }
        time_out--;
    }
    // 擦除页
    FLASH_ErasePage(addr);
    // 等待擦除完成
    time_out = 100000;
    while(FLASH_GetStatus() == FLASH_BUSY)
    {
        if(time_out==0)
        {
            return DRV_ERROR;
        }
        time_out--;
    }
    // 检查是否成功
    if(FLASH_GetStatus() != FLASH_COMPLETE)
    {
        return DRV_ERROR;
    }
    return DRV_OK;
}

/**
* @brief  初始化分区信息，分区范围，FLASH类型等
* @return 返回执行结果
* @retval DRV_OK      成功
* @retval DRV_ERROR   失败
*/
s32 drv_partition_init(const  drv_partition_table_t *user_part_config, u32 user_part_num)
{
    if(user_part_config == NULL || user_part_num == 0)
    {
        return DRV_ERROR;
    }

    //检查分区配置的有效性
    for(u32 i=0;i<user_part_num;i++)
    {
        const drv_partition_table_t *part = &user_part_config[i];

        if(part->partition_num > DRV_PARTITION_MAX)      //检查分区序号是否有效
        {
            return DRV_ERROR;
        }
        if(part->partition_length == 0)                 //检查长度是否有效
        {
            return DRV_ERROR;
        }
        if(part->partition == DRV_PARTITION_FLASH_INT)  //如果是内部FLASH，则还需检查以下内容
        {
            //检查起始地址是否在内部flash范围内
            if(part->partition_start_addr < INTERNAL_FLASH_START || 
               part->partition_start_addr >= INTERNAL_FLASH_START+INTERNAL_FLASH_SIZE)
            {
                return DRV_ERROR;
            }
            //检查结束地址是否越界
            if(part->partition_start_addr + part->partition_length > INTERNAL_FLASH_START+INTERNAL_FLASH_SIZE)
            {
                return DRV_ERROR;
            }
            //检查起始地址是否与页对齐
            if(part->partition_start_addr % INTERNAL_FALSH_PAGE_SIZE != 0)
            {
                return DRV_ERROR;
            }
        }
        if(part->partition_name == NULL)                //检查分区名称是否有效
        {
            return DRV_ERROR;
        }
        if(part->partition_options == 0)                //检查分区属性是否有效
        {
            return DRV_ERROR;
        }
    }
    drv_unlock_clear_errflag();

    //保存分区配置至全局静态变量中
    usr_partition_table = (drv_partition_table_t*) user_part_config;
    usr_partition_num   = user_part_num;
    return DRV_OK;
}

/**
* @brief 获取分区信息
* @param[in] partition_num: 分区序号
* @return 返回分区信息结构体指针
*/
drv_partition_table_t *drv_partition_get_info(partition_num_e partition_num)
{
    if(usr_partition_table == NULL || partition_num >= DRV_PARTITION_MAX)
    {
        return NULL;
    }
    /*查找用户配置的实例*/
    for(u32 i=0;i<usr_partition_num;i++)
    {
        if(usr_partition_table[i].partition_num == partition_num)
        {
            return &usr_partition_table[i];
        }
    }
    return NULL;
}

/**
* @brief 分区擦除	
* @param[in] partition_num : 分区序号
* @param[in] off_set       : 分区地址偏移
* @param[in] size          : 擦除数据长度
* @return 返回执行结果
* @retval DRV_OK    擦除成功
* @retval DRV_ERROR 擦除失败
*/
s32 drv_partition_erase(partition_num_e partition_num, u32 off_set, u32 size)
{   
    drv_partition_table_t* partition_info;     //存放分区信息
    u32 start_address = 0;
    u32 sec_size;
    s32 ret = DRV_OK;

    //检查参数是否有效
    if(partition_num>=DRV_PARTITION_MAX || partition_num<=DRV_PARTITION_ERROR || size == 0)
    {
        return DRV_INVAL;
    }
    partition_info = drv_partition_get_info(partition_num);    //获取分区信息存放在partition_info中
    
    //检查获取的分区信息是否有效
    if(partition_info == NULL)
    {
        return DRV_ERROR;
    }
    //检查偏移和擦除数据长度是否有效
    if(off_set + size > partition_info->partition_length)  
    {
        return DRV_ERROR;
    }
    //处理size可能超过分区剩余空间的情况
    u32 erase_size_max = partition_info->partition_length - off_set;
    if(size > erase_size_max)
    {
        size = erase_size_max;
    }
    //检查是否可写
    if((partition_info->partition_options & DRV_PAR_WRITE_DIS) == 0)
    {
        return DRV_ERROR;
    }
    //计算实际擦除地址
    start_address = partition_info->partition_start_addr + off_set;

    //根据分区是属于内部Flash,外部Flash或是内存来设置擦除
    if(partition_info->partition == DRV_PARTITION_RAM)         		//内存分区,使用0xFF填充来模拟擦除
    {
	 	u8 *address = (u8*) start_address;
        for(u32 i=0;i<size;i++)
        {
            address[i] = 0xFF;
        }
        return DRV_OK;
    }
    if(partition_info->partition == DRV_PARTITION_FLASH_INT)
		{   
			sec_size = FLASH_PAGE_INT_SIZE;
		}
    if(partition_info->partition == DRV_PARTITION_FLASH_EXT)
		{   
			sec_size = FLASH_PAGE_EXT_SIZE;
		}
    if (start_address % sec_size != 0 || size % sec_size != 0) 		//检查是否页对齐，擦除需要一页一页擦除，或者是一扇区擦除
    {
		return DRV_ERROR;
    }
    for(u32 i=0; i < size/sec_size;i++ )
    {
        if(partition_info->partition == DRV_PARTITION_FLASH_INT)
        {
            if(drv_internal_erase_page(start_address) != DRV_OK)  
            {
                return DRV_ERROR;
            }
        }
        start_address += sec_size;
    }
    return ret;
}

/**
* @brief 写数据
* @param[in] partition_num : 分区序号
* @param[in] off_set       : 分区地址偏移
* @param[in] in_buf_len    : 写入数据长度
* @param[in] in_buf        : 写入数据缓冲区
* @return 返回执行结果
* @retval DRV_OK      写入成功
* @retval DRV_ERROR   写入失败
*/
s32 drv_partition_write(partition_num_e partition_num, u32 off_set,void *in_buf, u32 in_buf_len)
{
    drv_partition_table_t *partition_info;
    partition_info = drv_partition_get_info(partition_num);
		s32 ret = DRV_OK;
		u32 align_size;
		u8 *in_buff = (u8*) in_buf;
    if(partition_info == NULL)
    {
        return DRV_INVAL;
    }
    if((partition_info->partition_options & PAR_OPT_WRITE_MASK) == 0)   //检查分区属性
    {
        return DRV_INVAL;
    }
    if(partition_info->partition_length < off_set + in_buf_len)         //检查写入数据长度,分区地址偏移是否有效
    {
        return DRV_INVAL;
    }
    u32 start_address = partition_info->partition_start_addr + off_set; //起始地址

    if(partition_info->partition == DRV_PARTITION_RAM)
    {
        memcpy((char *)start_address, in_buff, in_buf_len);
        return DRV_OK;
    }
    if(partition_info->partition == DRV_PARTITION_FLASH_INT)
        align_size = FLASH_PAGE_INT_SIZE;
    else
        align_size = FLASH_PAGE_EXT_SIZE;

    if (start_address % align_size != 0 || in_buf_len % align_size != 0) 
    {
        return DRV_ERROR;
    }

    if(partition_info->partition == DRV_PARTITION_FLASH_INT)
    {
        if(drv_write_internal_flash(start_address, in_buff, in_buf_len))
        {
          ret = DRV_ERROR;
        }
    }
    return ret;
}


/**
* @brief 先擦除再写数据
* @param[in] partition_num : 分区序号
* @param[in] off_set       : 分区地址偏移
* @param[in] in_buf_len    : 写入数据长度
* @param[in] in_buf        : 写入数据缓冲区
* @return 返回执行结果
* @retval DRV_OK     写入成功
* @retval DRV_ERROR  写入失败
*/
s32 drv_partition_erase_write(partition_num_e partition_num, u32 off_set,void *in_buf, u32 in_buf_len)
{
    u16 offset;
    u16 secleft;
    u32 dest;
    u8  *src;
    u8  result = 0;
    u32 sec_size,sec_size_mask;
//    u8  *sec_data;
    drv_partition_table_t *partition_info;

    partition_info = drv_partition_get_info(partition_num);
    if(partition_info == 0)
    {
        return DRV_ERROR;
    }
    if(0 == (partition_info->partition_options & PAR_OPT_WRITE_MASK))
    {
        return DRV_ERROR;
    }
    if(in_buf_len + off_set > partition_info->partition_length)
    {
        return DRV_ERROR;
    }

    if(partition_info->partition == DRV_PARTITION_RAM)
    {
        memcpy((char *)(partition_info->partition_start_addr + off_set), in_buf, in_buf_len);
        return DRV_OK;
    }

    if(partition_info->partition == DRV_PARTITION_FLASH_INT)
        sec_size = FLASH_PAGE_INT_SIZE;
    else
        sec_size = FLASH_PAGE_EXT_SIZE;

    sec_size_mask = sec_size - 1;
    u8 sec_data[FLASH_PAGE_INT_SIZE]={0};
//    sec_data = (u8 *)breo_malloc(sec_size);
//    if(sec_data == 0)
//    {
//        partition_err("malloc fail\n");
//        return -1;
//    }
    dest = partition_info->partition_start_addr + off_set;
    src = (unsigned char *)in_buf;
    offset   = ((u32)dest & sec_size_mask);
    secleft  = sec_size - offset;
    drv_partition_lock();
    /*如果页没有对齐，则先对第一页操作*/
    if (offset != 0)
    {
        dest -= offset;/*dest指针跳转到当前页头*/
        drv_partition_read(partition_num, dest-partition_info->partition_start_addr, (void *)sec_data, sec_size);
        if (in_buf_len > secleft)
        {
            memcpy((u8*)sec_data + offset, src, secleft);
            in_buf_len  -= secleft;
            src  += secleft;
        }
        else
        {
            memcpy((u8*)sec_data + offset, src, in_buf_len);
            in_buf_len=0;
        }
        drv_partition_erase(partition_num, dest-partition_info->partition_start_addr, sec_size);
        drv_partition_write(partition_num, dest-partition_info->partition_start_addr, sec_data, sec_size);

        dest += sec_size;
//      offset = 0;
    }
    while (in_buf_len > 0)
    {
        /*如果存在中间页*/
        if (in_buf_len >= sec_size)
        {
            memcpy((u8*)sec_data, src, sec_size);
            drv_partition_erase(partition_num, dest-partition_info->partition_start_addr, sec_size);
            drv_partition_write(partition_num, dest-partition_info->partition_start_addr, sec_data, sec_size);
            dest += sec_size;
            src  += sec_size;
            in_buf_len  -= sec_size;
        }
        else
        {   /*做最后页操作*/
            drv_partition_read(partition_num, dest-partition_info->partition_start_addr, (void *)sec_data, sec_size);
            memcpy((u8*)sec_data, src, in_buf_len);
            drv_partition_erase(partition_num, dest-partition_info->partition_start_addr, sec_size);
            drv_partition_write(partition_num, dest-partition_info->partition_start_addr, sec_data, sec_size);
            in_buf_len = 0;
        }
    }
    drv_unlock_clear_errflag();
//    breo_free(sec_data);
//    sec_data = NULL;
    if (result == 0)
        return DRV_OK;
    else
        return DRV_ERROR;
}

/**
* @brief 读数据
* @param[in]  partition_num : 分区序号
* @param[in]  off_set       : 分区地址偏移
* @param[out] out_buf       : 读取数据缓冲区
* @param[in]  out_buf_len   : 读取数据长度
* @return 返回执行结果
* @retval DRV_OK     读取成功
* @retval DRV_ERROR  读取失败
*/
s32 drv_partition_read(partition_num_e partition_num, u32 off_set,void *out_buf, u32 out_buf_len)
{
    u32 start_addr;
    drv_partition_table_t *partition_info; 

    partition_info = drv_partition_get_info(partition_num);
    if(partition_info == 0)
    {
        return DRV_ERROR;
    }
    if(out_buf_len + off_set > partition_info->partition_length)
    {
        return DRV_ERROR;
    }
    start_addr = partition_info->partition_start_addr + off_set;

    if(partition_info->partition == DRV_PARTITION_RAM)
    {
        memcpy(out_buf, (char *)start_addr, out_buf_len);
        return DRV_OK;
    }
    if(partition_info->partition == DRV_PARTITION_FLASH_INT)
    {
        return drv_read_internal_flash(start_addr,out_buf,out_buf_len);
    }
    return DRV_ERROR;
}

/**
 * @brief partition特殊命令
 * @param[in] partition_num: 分区序号
 * @param[in] cmd            命令字 
 * @param[in] param          参数
 * @return 返回执行结果
 * @retval DRV_OK    擦除成功
 * @retval <0           失败
 */
s32 drv_partition_ioctl(partition_num_e partition_num, s32 cmd,void* param)
{
	return DRV_OK;
}

#define PARTITION_TEST 0
#if     PARTITION_TEST
#define partition_debug_test(fmt, arg...) com_printf("[par]<%s><%d>" fmt, __func__, __LINE__, ##arg)
 // 定义测试分区表
 const drv_partition_table_t test_table[] =
 {
    {DRV_PARTITION_BOOT,       DRV_PARTITION_FLASH_INT,BOOT_FIRMWARE_ADDR,BOOT_FIRMWARE_SIZE,DRV_PAR_READ_EN|DRV_PAR_WRITE_DIS,"boot",},
    {DRV_PARTITION_CORE,       DRV_PARTITION_FLASH_INT,CORE_FIRMWARE_ADDR,CORE_FIRMWARE_SIZE,DRV_PAR_READ_EN|DRV_PAR_WRITE_DIS,"core"},
    {DRV_PARTITION_UPDATE_TEMP,DRV_PARTITION_FLASH_INT,UPDATE_TEMP_ADDR,  UPDATE_TEMP_SIZE,  DRV_PAR_READ_EN|DRV_PAR_WRITE_EN ,"updata"},
    {DRV_PARTITION_UPDATE_FLAG,DRV_PARTITION_FLASH_INT,UPDATA_FLAG_ADDR,  UPDATE_FLAG_SIZE,  DRV_PAR_READ_EN|DRV_PAR_WRITE_EN ,"updata_flag"},
 };
 
void partition_test_all(void)
{
    s32  ret;
    char str_buf[64];
    u32  str_len;
    
    // 测试开始
    str_len = snprintf(str_buf, sizeof(str_buf), "=== Partition Test Start ===\r\n");
    drv_uart_write(USER_UART_1, str_buf, str_len);
    
    /* 初始化分区 */
    ret = drv_partition_init(test_table, sizeof(test_table)/sizeof(test_table[0]));
    str_len = snprintf(str_buf, sizeof(str_buf), "Init: %s\r\n", (ret == DRV_OK) ? "OK" : "FAIL");
    drv_uart_write(USER_UART_1, str_buf, str_len);
    if(ret != DRV_OK) return;
    
    /* 测试UPDATE_TEMP分区 */
    u8 test_data[64], read_data[64];
    u32 i, errors = 0;
    
    // 生成测试数据
    for(i = 0; i < 64; i++)
    {
        test_data[i] = (i * 5 + 13) & 0xFF;
    }   
		// 先擦除后写入
		ret = drv_partition_erase_write(DRV_PARTITION_UPDATE_TEMP, 0, test_data, 64);
		str_len = snprintf(str_buf, sizeof(str_buf), "Write: %s\r\n", (ret == DRV_OK) ? "OK" : "FAIL");
		drv_uart_write(USER_UART_1, str_buf, str_len);

		// 读取
		memset(read_data, 0, 64);
		ret = drv_partition_read(DRV_PARTITION_UPDATE_TEMP, 0, read_data, 64);
		str_len = snprintf(str_buf, sizeof(str_buf), "Read: %s\r\n", (ret == 0) ? "OK" : "FAIL");
		drv_uart_write(USER_UART_1, str_buf, str_len);

		// 验证
		for(i =(i = 0; i < 64; i++)
		{
				if(read_data[i] != test_data[i]) errors++;
		}
		str_len = snprintf(str_buf, sizeof(str_buf), "Verify: %s\r\n", (errors == 0) ? "OK" : "FAIL");
		drv_uart_write(USER_UART_1, str_buf, str_len);

		/* 测试UPDATE_FLAG分区 */
		u8 flag_data[16], flag_read[16];

		for(i = 0; i < 16; i++)
		{
				flag_data[i] = (i * 3 + 7) & 0xFF;
		}
		ret = drv_partition_erase_write(DRV_PARTITION_UPDATE_FLAG, 0, flag_data, 16);
		str_len = snprintf(str_buf, sizeof(str_buf), "Flag Write: %s\r\n", (ret == DRV_OK) ? "OK" : "FAIL");
		drv_uart_write(USER_UART_1, str_buf, str_len);

		ret = drv_partition_read(DRV_PARTITION_UPDATE_FLAG, 0, flag_read, 16);
		str_len = snprintf(str_buf, sizeof(str_buf), "Flag Read: %s\r\n", (ret == 0) ? "OK" : "FAIL");
		drv_uart_write(USER_UART_1, str_buf, str_len);

		/* 测试完成 */
		str_len = snprintf(str_buf, sizeof(str_buf), "=== Partition Test End ===\r\n\r\n");
		drv_uart_write(USER_UART_1, str_buf, str_len);
}
#endif
