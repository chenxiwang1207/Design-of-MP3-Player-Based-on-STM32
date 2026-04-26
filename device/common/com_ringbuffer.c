#include <string.h>
#include "com_type.h"
// #include "com_log.h"
//#include "com_malloc.h"
#include "com_ringbuffer.h"

#define ring_buffer_malloc  com_malloc
#define ring_buffer_free    com_free

#define  ALIGN_DOWN(size, align)      (size-(size%align))
#define  ALIGN_SIZE	                  4

enum ringbuffer_state
{
    RING_BUFFER_EMPTY,
    RING_BUFFER_FULL,
    RING_BUFFER_HALFFULL,
};

/**
 * @brief     获取环形buff状态
 * @param[in] *rb  句柄
 * @return    返回环形buff状态
 * @retval    空
 * @retval    满
 * @retval    非空非满 
 * @remark    无
 */
static enum ringbuffer_state inner_ringbuffer_status(ring_buffer_t *rb)
{
    if (rb->r_index == rb->w_index)
    {
        if (rb->r_mirror == rb->w_mirror)
            return RING_BUFFER_EMPTY;
        else
            return RING_BUFFER_FULL;
    }
    return RING_BUFFER_HALFFULL;
}

/**
 * @brief     初始化静态环形buff句柄
 * @param[inout] *rb  句柄指针
 * @param[in] *pool   环形buff地址
 * @param[in] size    环形buff大小
 * @return    无
 * @retval    无
 * @remark    注意大小不能超过32KB;
 */
void com_ringbuffer_init(ring_buffer_t *rb,u8 *pool,u16 size)
{
    /* initialize read and write index */
    rb->r_mirror = rb->r_index = 0;
    rb->w_mirror = rb->w_index = 0;

    /* set buffer pool and size */
    rb->buffer = pool;
    rb->buffer_len = ALIGN_DOWN(size, ALIGN_SIZE);
}

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
s32 com_ringbuffer_put(ring_buffer_t *rb,const u8 *ptr,u16   length)
{
    u16 size;
    /* whether has enough space */
    size = com_ringbuffer_left_size(rb);

    /* no space */
    if (size == 0)
        return 0;

    /* drop some data */
    if (size < length)
        length = size;

    if (rb->buffer_len - rb->w_index > length)
    {
        /* r_index - w_index = empty space */
        memcpy(&rb->buffer[rb->w_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->w_index += length;
        return length;
    }

    memcpy(&rb->buffer[rb->w_index],&ptr[0],rb->buffer_len - rb->w_index);
    memcpy(&rb->buffer[0],&ptr[rb->buffer_len - rb->w_index],length - (rb->buffer_len - rb->w_index));

    /* we are going into the other side of the mirror */
    rb->w_mirror = ~rb->w_mirror;
    rb->w_index = length - (rb->buffer_len - rb->w_index);

    return length;
}

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
s32 com_ringbuffer_put_force(ring_buffer_t *rb,const u8  *ptr,u16  length)
{
    u16 space_length;
    space_length = com_ringbuffer_left_size(rb);

    if (length > rb->buffer_len)
    {
        ptr = &ptr[length - rb->buffer_len];
        length = rb->buffer_len;
    }

    if (rb->buffer_len - rb->w_index > length)
    {
        memcpy(&rb->buffer[rb->w_index], ptr, length);
        rb->w_index += length;

        if (length > space_length)
            rb->r_index = rb->w_index;

        return length;
    }

    memcpy(&rb->buffer[rb->w_index],&ptr[0],rb->buffer_len - rb->w_index);
    memcpy(&rb->buffer[0],&ptr[rb->buffer_len - rb->w_index],length - (rb->buffer_len - rb->w_index));

    rb->w_mirror = ~rb->w_mirror;
    rb->w_index = length - (rb->buffer_len - rb->w_index);

    if (length > space_length)
    {
        rb->r_mirror = ~rb->r_mirror;
        rb->r_index = rb->w_index;
    }

    return length;
}

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
s32 com_ringbuffer_putchar(ring_buffer_t *rb, const u8 ch)
{
    if (!com_ringbuffer_left_size(rb))
    {
        return 0;
    }
    rb->buffer[rb->w_index] = ch;
    if (rb->w_index == rb->buffer_len-1)
    {
        rb->w_mirror = ~rb->w_mirror;
        rb->w_index = 0;
    }
    else
    {
        rb->w_index++;
    }
    return 1;
}

/**
 * @brief     强制向环形buff写单个字符,写满会覆盖之前写的
 * @param[in] *rb     句柄指针
 * @param[in] ch      字符
 * @return    返回写成功字节数
 * @retval    1  成功
 * @retval	  -1  参数错误
 * @remark    当buff满的情况下会覆盖掉队头的老数据
 */
s32 com_ringbuffer_putchar_force(ring_buffer_t *rb, const u8 ch)
{
    enum ringbuffer_state old_state;
    old_state = inner_ringbuffer_status(rb);
    rb->buffer[rb->w_index] = ch;

    if (rb->w_index == rb->buffer_len-1)
    {
        rb->w_mirror = ~rb->w_mirror;
        rb->w_index = 0;
        if (old_state == RING_BUFFER_FULL)
        {
            rb->r_mirror = ~rb->r_mirror;
            rb->r_index = rb->w_index;
        }
    }
    else
    {
        rb->w_index++;
        if (old_state == RING_BUFFER_FULL)
            rb->r_index = rb->w_index;
    }

    return 1;
}

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
s32 com_ringbuffer_get(ring_buffer_t *rb,u8 *ptr,u16 length)
{
    s32 size;
    size = com_ringbuffer_data_len(rb);
    if (size == 0)
    {
        return 0;
    }

    if (size < length)
    {
        length = size;
    }

    if (rb->buffer_len - rb->r_index > length)
    {
        memcpy(ptr, &rb->buffer[rb->r_index], length);
        rb->r_index += length;
        return length;
    }

    memcpy(&ptr[0],
           &rb->buffer[rb->r_index],
           rb->buffer_len - rb->r_index);
    memcpy(&ptr[rb->buffer_len - rb->r_index],
           &rb->buffer[0],
           length - (rb->buffer_len - rb->r_index));
    rb->r_mirror = ~rb->r_mirror;
    rb->r_index = length - (rb->buffer_len - rb->r_index);

    return length;
}


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
s32 com_ringbuffer_getchar(ring_buffer_t *rb, u8 *ch)
{
    if (!com_ringbuffer_data_len(rb))
        return 0;
    *ch = rb->buffer[rb->r_index];

    if (rb->r_index == rb->buffer_len-1)
    {
        rb->r_mirror = ~rb->r_mirror;
        rb->r_index = 0;
    }
    else
    {
        rb->r_index++;
    }

    return 1;
}

/**
 * @brief     获取环形buff中的数据长度
 * @param[in] *rb  句柄指针
 * @return    返回数据长度
 * @retval    0  空
 * @retval    >0 实际数据长度
 * @remark    无
 */
s32 com_ringbuffer_data_len(ring_buffer_t *rb)
{
    switch (inner_ringbuffer_status(rb))
    {
    case RING_BUFFER_EMPTY:
        return 0;
    case RING_BUFFER_FULL:
        return rb->buffer_len;
    case RING_BUFFER_HALFFULL:
    default:
        if (rb->w_index > rb->r_index)
            return rb->w_index - rb->r_index;
        else
            return rb->buffer_len - (rb->r_index - rb->w_index);
    }
}

/**
 * @brief     获取环形buff的大小
 * @param[in] *rb  句柄指针
 * @return    返回buff大小
 * @retval    >0
 * @retval	   0 参数错误
 * @remark    无
 */
u16 com_ringbuffer_get_size(ring_buffer_t *rb)
{
    return rb->buffer_len;
}

/**
 * @brief     获取环形buff的剩余空间大小
 * @param[in] *rb  句柄指针
 * @return    返回buff剩余空间
 * @retval    >=0
 * @remark    无
 */
u16 com_ringbuffer_left_size(ring_buffer_t *rb)
{
    return rb->buffer_len - com_ringbuffer_data_len(rb);
}

/**
 * @brief     环形buff重置
 * @param[in] *rb  句柄指针
 * @return    无
 * @retval    无
 * @remark    无
 */
void com_ringbuffer_reset(ring_buffer_t *rb)
{
    rb->r_mirror = 0;
    rb->r_index = 0;
    rb->w_mirror = 0;
    rb->w_index = 0;
}

/**
 * @brief     使用动态内存创建环形buff的方式,使用完成之后要销毁掉breo_ringbuffer_destroy
 * @param[in] size  环形buff大小，最大为32K
 * @return    返回句柄指针
 * @retval    NULL 创建失败
 * @retval    !NULL 创建成功
 * @retval	  -1 参数错误
 * @remark    drv_ringbuffer_init和breo_ringbuffer_create这两个接口互斥关系
 */
ring_buffer_t* com_ringbuffer_create(u32 size)
{
    ring_buffer_t *rb = NULL;
//    u8 *pool;
//    size = ALIGN_DOWN(size, ALIGN_SIZE);

//    rb = ring_buffer_malloc(sizeof(ring_buffer_t));
//    if (rb == NULL)
//        goto exit;

//    pool = ring_buffer_malloc(size);
//    if (pool == NULL)
//    {
//        ring_buffer_free(rb);
//        rb = NULL;
//        goto exit;
//    }
//    com_ringbuffer_init(rb, pool, size);
//exit:
    return rb;
}

/**
 * @brief     释放动态申请的句柄
 * @param[in] *rb  描述参数的含义
 * @return    无返回值
 */
void com_ringbuffer_destroy(ring_buffer_t *rb)
{
//    ring_buffer_free(rb->buffer);
//    ring_buffer_free(rb);
}
