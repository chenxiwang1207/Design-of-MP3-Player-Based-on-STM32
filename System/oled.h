#ifndef __OLED_H
#define __OLED_H 
#include "sys.h"
#include "stdlib.h"	
#include "platform_mcu_resource.h"

#define _6x8    8     //横坐标大小6 纵坐标大小8
#define _6x12   12    //横坐标大小6 纵坐标大小12
#define _8x16   16    //字符横坐标大小8  纵坐标大小16
#define _16x16  16    //汉字横坐标大小16 纵坐标大小16
#define _12x24  24    //横坐标大小12 纵坐标大小24
#define _24x24  24    //横坐标大小24 纵坐标大小24

/***************根据自己需求更改****************/
#define OLED_SCL_PORT  			GPIOB
#define OLED_SCL_PIN			GPIO_Pin_8
#define OLED_SCL_GPIO_CLK       RCC_APB2Periph_GPIOB
#define OLED_SDA_PORT  			GPIOB
#define OLED_SDA_PIN			GPIO_Pin_9
#define OLED_SDA_GPIO_CLK       RCC_APB2Periph_GPIOB
/*********************END**********************/

#define OLED_SCL_Clr() GPIO_ResetBits(OLED_SCL_PORT,OLED_SCL_PIN)//SCL
#define OLED_SCL_Set() GPIO_SetBits(OLED_SCL_PORT,OLED_SCL_PIN)

#define OLED_SDA_Clr() GPIO_ResetBits(OLED_SDA_PORT,OLED_SDA_PIN)//DIN
#define OLED_SDA_Set() GPIO_SetBits(OLED_SDA_PORT,OLED_SDA_PIN)

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


// ========== 新增函数声明 ==========
void OLED_ClearArea(u8 x, u8 y, u8 w, u8 h);               // 清除指定区域（填充0）
void OLED_ReverseArea(u8 x, u8 y, u8 w, u8 h);             // 指定区域取反
void OLED_Reverse(void);                                   // 全屏取反
void OLED_ShowChineseString(int16_t X, int16_t Y, char *String, uint8_t FontSize);
void OLED_Printf(u8 x, u8 y, u8 size1, u8 mode, const char *fmt, ...);// 格式化输出
void OLED_ShowChinese_Roll(u8 x, u8 y, const char *text, u32 speed_ms); //显示可滚动的中文字符串（超出屏幕宽度时自动循环左移）
void OLED_StopRoll(void);   //停止滚动显示
u16 calc_string_width(const char *text); //计算字符串的总像素宽度（用于滚动显示的列数计算）

// 数字扩展显示
void OLED_ShowSignedNum(u8 x, u8 y, s32 num, u8 len, u8 size1, u8 mode);    //显示有符号整数（带正负号）
void OLED_ShowHexNum(u8 x, u8 y, u32 num, u8 len, u8 size1, u8 mode);       //显示16进制数字
void OLED_ShowBinNum(u8 x, u8 y, u32 num, u8 len, u8 size1, u8 mode);       //显示2进制数字

void OLED_ClearPoint(u8 x,u8 y);    //清除指定点（反色处理）
void OLED_ColorTurn(u8 i);          //设置反色显示,0：正常显示；1：反色显示
void OLED_DisplayTurn(u8 i);        //设置屏幕旋转方向,0：正常显示；1：反转显示（180度旋转）
void OLED_DisPlay_On(void);         //开启OLED显示
void OLED_DisPlay_Off(void);        //关闭OLED显示
void OLED_Update(void); //更新显存内容到OLED屏幕
void OLED_UpdateArea(u8 x, u8 y, u8 w, u8 h);
void OLED_UpdateArea(u8 x, u8 y, u8 w, u8 h);//局部刷新：仅将显存中矩形区域发送到屏幕（减少总线占用，避免在音频播放时造成干扰）
void OLED_Clear(void);  //清屏（将所有像素点设置为0）
void OLED_DrawPoint(u8 x,u8 y,u8 t);    //画点
void OLED_DrawCircle(u8 x,u8 y,u8 r);   //画圆
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode);    //画线
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1,u8 mode);  //在指定位置显示一个字符
void OLED_ShowChar6x8(u8 x,u8 y,u8 chr,u8 mode);        //显示6x8大小的字符（已废弃，建议使用OLED_ShowChar）
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1,u8 mode);       //显示字符串
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode);   //显示数字
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1,u8 mode);       //显示汉字
void OLED_ScrollDisplay(u8 num,u8 space,u8 mode);               //滚动显示汉字（循环左移）
void OLED_ShowPicture(u8 x,u8 y,u8 sizex,u8 sizey,u8 BMP[],u8 mode);    //显示图片（BMP位图）
void OLED_Init(void);   //OLED初始化

void OLED_I2C_Start(void);
void OLED_I2C_Stop(void);
void OLED_I2C_WaitAck(void);
void OLED_Send_Byte(u8 dat);
void OLED_WR_Byte(u8 dat,u8 mode);


#if 0
/**
 * @brief 清除指定点（反色处理）
 * @param x 横坐标（0~127）
 * @param y 纵坐标（0~63）
 */
void OLED_ClearPoint(u8 x,u8 y);
/**
 * @brief 设置反色显示
 * @param i 0：正常显示；1：反色显示
 */
void OLED_ColorTurn(u8 i);
/**
 * @brief 设置屏幕旋转方向
 * @param i 0：正常显示；1：反转显示（180度旋转）
 */
void OLED_DisplayTurn(u8 i);
/**
 * @brief I2C起始信号
 */
void OLED_I2C_Start(void);
/**
 * @brief I2C停止信号
 */
void OLED_I2C_Stop(void);
/**
 * @brief I2C等待应答信号
 */
void OLED_I2C_WaitAck(void);
/**
 * @brief 发送一个字节数据
 * @param dat 要发送的字节数据
 */
void OLED_Send_Byte(u8 dat);
/**
 * @brief 向OLED写入一个字节（命令或数据）
 * @param dat 要写入的字节
 * @param mode 0：命令；1：数据
 */
void OLED_WR_Byte(u8 dat,u8 mode);
/**
 * @brief 开启OLED显示
 */
void OLED_DisPlay_On(void);
/**
 * @brief 关闭OLED显示
 */
void OLED_DisPlay_Off(void);
/**
 * @brief 更新显存内容到OLED屏幕（将OLED_GRAM刷新到屏幕）
 */
void OLED_Update(void);
/**
 * @brief 清屏（将所有像素点设置为0）
 */
void OLED_Clear(void);
/**
 * @brief 画点
 * @param x 横坐标（0~127）
 * @param y 纵坐标（0~63）
 * @param t 1：点亮；0：熄灭
 */
void OLED_DrawPoint(u8 x,u8 y,u8 t);
/**
 * @brief 画线
 * @param x1 起点横坐标
 * @param y1 起点纵坐标
 * @param x2 终点横坐标
 * @param y2 终点纵坐标
 * @param mode 1：画线；0：擦除线
 */
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode);
/**
 * @brief 画圆
 * @param x 圆心横坐标
 * @param y 圆心纵坐标
 * @param r 半径
 */
void OLED_DrawCircle(u8 x,u8 y,u8 r);
/**
 * @brief 在指定位置显示一个字符
 * @param x 横坐标
 * @param y 纵坐标
 * @param chr 要显示的字符
 * @param size1 字体大小（6/8/12/16/24）
 * @param mode 0：反色显示；1：正常显示
 */
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1,u8 mode);
/**
 * @brief 显示6x8大小的字符（已废弃，建议使用OLED_ShowChar）
 * @param x 横坐标
 * @param y 纵坐标
 * @param chr 要显示的字符
 * @param mode 0：反色显示；1：正常显示
 */
void OLED_ShowChar6x8(u8 x,u8 y,u8 chr,u8 mode);
/**
 * @brief 显示字符串
 * @param x 起始横坐标
 * @param y 起始纵坐标
 * @param chr 字符串指针
 * @param size1 字体大小（6/8/12/16/24）
 * @param mode 0：反色显示；1：正常显示
 */
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1,u8 mode);
/**
 * @brief 显示数字
 * @param x 起始横坐标
 * @param y 起始纵坐标
 * @param num 要显示的数字
 * @param len 数字位数
 * @param size1 字体大小
 * @param mode 0：反色显示；1：正常显示
 */
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode);
/**
 * @brief 显示汉字
 * @param x 起始横坐标
 * @param y 起始纵坐标
 * @param num 汉字序号（字库中的索引）
 * @param size1 字体大小（16/24/32/64）
 * @param mode 0：反色显示；1：正常显示
 */
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1,u8 mode);

/**
 * @brief 滚动显示汉字（循环左移）
 * @param num 要显示的汉字个数
 * @param space 每一遍显示的间隔（移动次数）
 * @param mode 0：反色显示；1：正常显示
 */
void OLED_ScrollDisplay(u8 num,u8 space,u8 mode);
/**
 * @brief 显示图片（BMP位图）
 * @param x 起始横坐标
 * @param y 起始纵坐标
 * @param sizex 图片宽度
 * @param sizey 图片高度
 * @param BMP 图片数据数组
 * @param mode 0：反色显示；1：正常显示
 */
void OLED_ShowPicture(u8 x,u8 y,u8 sizex,u8 sizey,u8 BMP[],u8 mode);
/**
 * @brief OLED初始化
 */
void OLED_Init(void);
#endif
#endif
