/**
 * @file drv_partition.h
 * @brief flash分区抽象层接口定义 
 * @details flash分区抽象层接口定义，提供flash分区读写擦除等操作接口
 * @version 1.0
 * @date 2026-03-16
 */
#ifndef _DRV_PARTITION_H_
#define _DRV_PARTITION_H_

#include "com_type.h"
#include "platform_mcu_resource.h"
#include "platform_partition_config.h"

#define PAR_OPT_READ_POS  ( 0 ) 
#define PAR_OPT_WRITE_POS ( 1 ) 

#define PAR_OPT_READ_MASK  ( 0x1u << PAR_OPT_READ_POS )
#define PAR_OPT_WRITE_MASK ( 0x1u << PAR_OPT_WRITE_POS )

typedef enum{
    DRV_PAR_READ_DIS=0,     ///< 不可读
    DRV_PAR_READ_EN,        ///< 可读
    DRV_PAR_WRITE_DIS,      ///< 不可写
    DRV_PAR_WRITE_EN,       ///< 可写
}partition_protect_rw_e;

typedef enum 
{
    DRV_PARTITION_ERROR = -1,
    DRV_PARTITION_BOOT,           ///< boot代码区序号
    DRV_PARTITION_CORE,           ///< 核心代码区序号
    DRV_PARTITION_APP,            ///< 应用代码区序号
    DRV_PARTITION_UPDATE_TEMP,    ///< 升级代码区序号
    DRV_PARTITION_RESERVE,        ///< 备份代码区序号
    DRV_PARTITION_UPDATE_FLAG,    ///< 升级标准区序号
    DRV_PARTITION_BREO_FS,        ///< 文件系统区序号
    DRV_PARTITION_FACTORY,        ///< 出厂只读配置区
    DRV_PARTITION_MAX
}partition_num_e;

typedef enum 
{
    DRV_PARTITION_FLASH_INT,       ///< 内部flash
    DRV_PARTITION_FLASH_EXT,       ///< 外部flash
    DRV_PARTITION_RAM,             ///< 内存
    DRV_PARTITION_OWNER_MAX,
}drv_partition_e;

typedef struct {
    partition_num_e partition_num;          ///< 分区序号
    drv_partition_e partition;              ///< 分区是属于内部还是外部FLASH
    u32           partition_start_addr;   ///< 分区起始地址
    u32           partition_length;       ///< 分区长度 
    u32           partition_options;      ///< 分区属性 
    const s8      *partition_name;        ///< 分区名称
}drv_partition_table_t;

/**
* @brief  初始化分区信息，分区范围，FLASH类型等
* @return 返回执行结果
* @retval DRV_OK      成功
* @retval DRV_ERROR   失败
*/
s32 drv_partition_init(const  drv_partition_table_t *user_part_config, u32 user_part_num);

/**
* @brief 获取分区信息
* @param[in] partition_num: 分区序号
* @return 返回分区信息结构体指针
*/
drv_partition_table_t *drv_partition_get_info(partition_num_e partition_num);

/**
* @brief 分区擦除
* @param[in] partition_num : 分区序号
* @param[in] off_set       : 分区地址偏移
* @param[in] size          : 擦除数据长度
* @return 返回执行结果
* @retval DRV_OK    擦除成功
* @retval DRV_ERROR 擦除失败
*/
s32 drv_partition_erase(partition_num_e partition_num, u32 off_set, u32 size);

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
s32 drv_partition_write(partition_num_e partition_num, u32 off_set,void *in_buf, u32 in_buf_len);

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
s32 drv_partition_erase_write(partition_num_e partition_num, u32 off_set,void *in_buf, u32 in_buf_len);

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
s32 drv_partition_read(partition_num_e partition_num, u32 off_set,void *out_buf, u32 out_buf_len);

/**
 * @brief partition特殊命令
 * @param[in] partition_num: 分区序号
 * @param[in] cmd            命令字 
 * @param[in] param          参数
 * @return 返回执行结果
 * @retval DRV_OK       成功
 * @retval <0           失败
 */
s32 drv_partition_ioctl(partition_num_e partition_num, s32 cmd,void* param);

//void partition_test(void);
//void partition_test_all(void);

#endif
