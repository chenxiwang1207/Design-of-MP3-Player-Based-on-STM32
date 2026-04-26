/**
 * @file com_ringbuffer.h
 * @brief 环形缓冲器接口
 * @details 环形缓冲器接口
 * @version 1.0
 * @date 2026-03-11
 * 
 */
#ifndef _COM_RINGBUFFER_H_
#define _COM_RINGBUFFER_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "com_type.h"

/* ring buffer */
typedef struct 
{
    u8 *buffer;
    u16 r_mirror : 1;
    u16 r_index  : 15;
    u16 w_mirror: 1;
    u16 w_index : 15;
    u16 buffer_len;
}ring_buffer_t;

/**
 * @brief     初始化静态环形buff句柄
 * @param[inout] *rb  句柄指针
 * @param[in] *pool   环形buff地址
 * @param[in] size    环形buff大小
 * @return    无
 * @retval    无
 * @remark    注意大小不能超过32KB;
 */
void com_ringbuffer_init(ring_buffer_t *rb, u8 *pool, u16 size);

/**
 * @brief     向环形buff写字符串，写满就不写了
 * @param[in] *rb     句柄指针
 * @param[in] *ptr    写入的字符串指针
 * @param[in] length  写入的字符串长度
 * @return    返回写成功的字节数
 * @retval    >=0
 * @retval	  -1 参数错误
 * @remark    在buff满的情况下,会写失败返回0;
 */
s32 com_ringbuffer_put(ring_buffer_t *rb, const u8 *ptr, u16 length);

/**
 * @brief     强制向环形buff写字符串,写满会覆盖之前写的
 * @param[in] *rb       句柄指针
 * @param[in] *ptr      写入的数据指针
 * @param[in] length    写入的数据长度
 * @return    返回写成功的字节数
 * @retval    >=0
 * @retval	  -1 参数错误
 * @remark    在buff满的情况下,会覆盖掉队头的老数据;
 */
s32 com_ringbuffer_put_force(ring_buffer_t *rb, const u8 *ptr, u16 length);

/**
 * @brief     向环形buff写单个字符，写满就不写了
 * @param[in] *rb   句柄指针
 * @param[in] ch    字符
 * @return    返回写成功字节数
 * @retval    0  失败
 * @retval    1  成功
 * @retval	  -1  参数错误
 * @remark    无
 */
s32 com_ringbuffer_putchar(ring_buffer_t *rb, const u8 ch);

/**
 * @brief     强制向环形buff写单个字符,写满会覆盖之前写的
 * @param[in] *rb     句柄指针
 * @param[in] ch      字符
 * @return    返回写成功字节数
 * @retval    1  成功
 * @retval	  -1  参数错误
 * @remark    当buff满的情况下会覆盖掉队头的老数据
 */
s32 com_ringbuffer_putchar_force(ring_buffer_t *rb, const u8 ch);

/**
 * @brief     从环形buff中读取字符串
 * @param[in]  *rb      句柄指针
 * @param[out] *ptr     字符串指针
 * @param[in]  length   字符串的长度
 * @return    返回实际读取成功的长度
 * @retval    >=0
 * @retval	  -1 参数错误
 * @remark    缓冲区空的情况下返回0
 */
s32 com_ringbuffer_get(ring_buffer_t *rb, u8 *ptr, u16 length);

/**
 * @brief     从环形buff中获取单个字符
 * @param[in]  *rb  句柄指针
 * @param[out] *ch  存放读取到的字符地址
 * @return    返回读取成功的字节数
 * @retval    0 失败
 * @retval    1 成功
 * @retval	  -1 参数错误
 * @remark    无
 */
s32 com_ringbuffer_getchar(ring_buffer_t *rb, u8 *ch);

/**
 * @brief     获取环形buff中的数据长度
 * @param[in] *rb  句柄指针
 * @return    返回数据长度
 * @retval    0  空
 * @retval    >0 实际数据长度
 * @remark    无
 */
s32 com_ringbuffer_data_len(ring_buffer_t *rb);

/**
 * @brief     获取环形buff的大小
 * @param[in] *rb  句柄指针
 * @return    返回buff大小
 * @retval    >0
 * @retval	   0 参数错误
 * @remark    无
 */
u16 com_ringbuffer_get_size(ring_buffer_t *rb);

/**
 * @brief     获取环形buff的剩余空间大小
 * @param[in] *rb  句柄指针
 * @return    返回buff剩余空间
 * @retval    >=0
 * @remark    无
 */
u16 com_ringbuffer_left_size(ring_buffer_t *rb);

/**
 * @brief     环形buff重置
 * @param[in] *rb  句柄指针
 * @return    无
 * @retval    无
 * @remark    无
 */
void com_ringbuffer_reset(ring_buffer_t *rb);

/**
 * @brief     使用动态内存创建环形buff的方式,使用完成之后要销毁掉breo_ringbuffer_destroy
 * @param[in] size  环形buff大小，最大为32K
 * @return    返回句柄指针
 * @retval    NULL 创建失败
 * @retval    !NULL 创建成功
 * @retval	  -1 参数错误
 * @remark    drv_ringbuffer_init和breo_ringbuffer_create这两个接口互斥关系
 */
ring_buffer_t* com_ringbuffer_create(u32 size);

/**
 * @brief     释放动态申请的句柄
 * @param[in] *rb  描述参数的含义
 * @return    无返回值
 */
void com_ringbuffer_destroy(ring_buffer_t *rb);


 #ifdef __cplusplus
 }
 #endif
 
#endif
