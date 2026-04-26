#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include "oled.h"
#include "OLED_Data.h"
#include "Delay.h"
#include "dev_RTC.h"
#include "drv_timer.h"

u8 OLED_GRAM[144][8];

//反显函数
void OLED_ColorTurn(u8 i)
{
    if(i==0)
    {
        OLED_WR_Byte(0xA6,OLED_CMD);//正常显示
    }
    if(i==1)
    {
        OLED_WR_Byte(0xA7,OLED_CMD);//反色显示
    }
}

//屏幕旋转180度
void OLED_DisplayTurn(u8 i)
{
    if(i==0)
        {
            OLED_WR_Byte(0xC8,OLED_CMD);//正常显示
            OLED_WR_Byte(0xA1,OLED_CMD);
        }
    if(i==1)
        {
            OLED_WR_Byte(0xC0,OLED_CMD);//反转显示
            OLED_WR_Byte(0xA0,OLED_CMD);
        }
}

//延时
void IIC_delay(void)
{
//	u8 t=0;
//	while(t--){}
    IWDG_ReloadCounter();
}

//起始信号
void OLED_I2C_Start(void)
{
    OLED_SDA_Set();
    OLED_SCL_Set();
    IIC_delay();
    OLED_SDA_Clr();
    IIC_delay();
    OLED_SCL_Clr();
    IIC_delay();
}

//结束信号
void OLED_I2C_Stop(void)
{
    OLED_SDA_Clr();
    OLED_SCL_Set();
    IIC_delay();
    OLED_SDA_Set();
}

//等待信号响应
void OLED_I2C_WaitAck(void) //测数据信号的电平
{
    OLED_SDA_Set();
    IIC_delay();
    OLED_SCL_Set();
    IIC_delay();
    OLED_SCL_Clr();
    IIC_delay();
}

//写入一个字节
void OLED_Send_Byte(u8 dat)
{
    u8 i;
    for(i=0;i<8;i++)
    {
        if(dat&0x80)//将dat的8位从最高位依次写入
        {
            OLED_SDA_Set();
    }
        else
        {
            OLED_SDA_Clr();
    }
        IIC_delay();
        OLED_SCL_Set();
        IIC_delay();
        OLED_SCL_Clr();//将时钟信号设置为低电平
        dat<<=1;
  }
}

//发送一个字节
//mode:数据/命令标志 0,表示命令;1,表示数据;
void OLED_WR_Byte(u8 dat,u8 mode)
{
    OLED_I2C_Start();
    OLED_Send_Byte(0x78);
    OLED_I2C_WaitAck();
    if(mode){OLED_Send_Byte(0x40);}
  else{OLED_Send_Byte(0x00);}
    OLED_I2C_WaitAck();
    OLED_Send_Byte(dat);
    OLED_I2C_WaitAck();
    OLED_I2C_Stop();
}

//开启OLED显示 
void OLED_DisPlay_On(void)
{
    OLED_WR_Byte(0x8D,OLED_CMD);//电荷泵使能
    OLED_WR_Byte(0x14,OLED_CMD);//开启电荷泵
    OLED_WR_Byte(0xAF,OLED_CMD);//点亮屏幕
}

//关闭OLED显示 
void OLED_DisPlay_Off(void)
{
    OLED_WR_Byte(0x8D,OLED_CMD);//电荷泵使能
    OLED_WR_Byte(0x10,OLED_CMD);//关闭电荷泵
    OLED_WR_Byte(0xAE,OLED_CMD);//关闭屏幕
}

//更新显存到OLED	
void OLED_Update(void)
{
    __disable_irq();
    u8 i,n;
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte(0xb0+i,OLED_CMD); //设置行起始地址
        OLED_WR_Byte(0x02,OLED_CMD);   //设置低列起始地址
        OLED_WR_Byte(0x10,OLED_CMD);   //设置高列起始地址
        
        OLED_I2C_Start();
        OLED_Send_Byte(0x78);
        OLED_I2C_WaitAck();
        OLED_Send_Byte(0x40);
        OLED_I2C_WaitAck();
        for(n=0;n<128;n++)
        {
            OLED_Send_Byte(OLED_GRAM[n][i]);
            OLED_I2C_WaitAck();
        }
        OLED_I2C_Stop();
    }
    __enable_irq();
}


/**
 * @brief 局部刷新指定矩形区域到 OLED 屏幕
 * @param x 左上角横坐标
 * @param y 左上角纵坐标
 * @param w 区域宽度
 * @param h 区域高度
 *
 * 说明：SSD1306/类似驱动按 page（8 像素高）组织显存。为了降低 I2C (bit-bang)
 * 总线占用并减少与音频 DMA/中断的干扰，这里只把受影响的 page 范围发送到屏幕，
 * 而不是每次都刷新全屏。实现上沿用了 OLED_Update 的列/页设置和发送流程，
 * 但只循环受影响的 page（每个 page 发送 128 字节列数据）
 */
void OLED_UpdateArea(u8 x, u8 y, u8 w, u8 h)
{
    __disable_irq();
    u8 start_page, end_page;
    u8 end_x;
    u8 p, n;

    if (w == 0 || h == 0) return;
    if (x > 127) return;

    end_x = x + w - 1;
    if (end_x > 127) end_x = 127;

    start_page = y / 8;
    end_page = (y + h - 1) / 8;
    if (start_page > 7) return;
    if (end_page > 7) end_page = 7;

    for (p = start_page; p <= end_page; p++)
    {
        OLED_WR_Byte(0xb0 + p, OLED_CMD); // 设置页地址
        OLED_WR_Byte(0x02, OLED_CMD);     // 列起始低位（与 OLED_Update 保持一致）
        OLED_WR_Byte(0x10, OLED_CMD);     // 列起始高位

        OLED_I2C_Start();
        OLED_Send_Byte(0x78);
        OLED_I2C_WaitAck();
        OLED_Send_Byte(0x40);
        OLED_I2C_WaitAck();

        // 为简单可靠性考虑，按列从 0..127 发送本 page 的显存数据（和 OLED_Update 行为一致），
        // 但只对受影响的 page 进行发送，从而把一次全屏 8 page 刷新降为 1~2 page 刷新。
        for (n = 0; n < 128; n++)
        {
            OLED_Send_Byte(OLED_GRAM[n][p]);
            OLED_I2C_WaitAck();
        }
        OLED_I2C_Stop();
    }
    __enable_irq();
}

//清屏函数
void OLED_Clear(void)
{
    u8 i,n;
    for(i=0;i<8;i++)
    {
       for(n=0;n<128;n++)
            {
             OLED_GRAM[n][i]=0;//清除所有数据
            }
  }
    OLED_Update();//更新显示
}

//画点 
//x:0~127
//y:0~63
//t:1 填充 0,清空	
void OLED_DrawPoint(u8 x,u8 y,u8 t)
{
    u8 i,m,n;
    i=y/8;
    m=y%8;
    n=1<<m;
    if(t){OLED_GRAM[x][i]|=n;}
    else
    {
        OLED_GRAM[x][i]=~OLED_GRAM[x][i];
        OLED_GRAM[x][i]|=n;
        OLED_GRAM[x][i]=~OLED_GRAM[x][i];
    }
//	OLED_Update();
}

//画线
//x1,y1:起点坐标
//x2,y2:结束坐标
void OLED_DrawLine(u8 x1,u8 y1,u8 x2,u8 y2,u8 mode)
{
    u16 t; 
    int xerr=0,yerr=0,delta_x,delta_y,distance;
    int incx,incy,uRow,uCol;
    delta_x=x2-x1; //计算坐标增量 
    delta_y=y2-y1;
    uRow=x1;//画线起点坐标
    uCol=y1;
    if(delta_x>0)incx=1; //设置单步方向 
    else if (delta_x==0)incx=0;//垂直线 
    else {incx=-1;delta_x=-delta_x;}
    if(delta_y>0)incy=1;
    else if (delta_y==0)incy=0;//水平线 
    else {incy=-1;delta_y=-delta_x;}
    if(delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
    else distance=delta_y;
    for(t=0;t<distance+1;t++)
    {
        OLED_DrawPoint(uRow,uCol,mode);//画点
        xerr+=delta_x;
        yerr+=delta_y;
        if(xerr>distance)
        {
            xerr-=distance;
            uRow+=incx;
        }
        if(yerr>distance)
        {
            yerr-=distance;
            uCol+=incy;
        }
    }
//	OLED_Update();
}
//x,y:圆心坐标
//r:圆的半径
void OLED_DrawCircle(u8 x,u8 y,u8 r)
{
    int a, b,num;
    a = 0;
    b = r;
    while(2 * b * b >= r * r)      
    {
        OLED_DrawPoint(x + a, y - b,1);
        OLED_DrawPoint(x - a, y - b,1);
        OLED_DrawPoint(x - a, y + b,1);
        OLED_DrawPoint(x + a, y + b,1);
 
        OLED_DrawPoint(x + b, y + a,1);
        OLED_DrawPoint(x + b, y - a,1);
        OLED_DrawPoint(x - b, y - a,1);
        OLED_DrawPoint(x - b, y + a,1);
        
        a++;
        num = (a * a + b * b) - r*r;//计算画的点离圆心的距离
        if(num > 0)
        {
            b--;
            a--;
        }
    }
//		OLED_Update();
}



//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//size1:选择字体 6x8/6x12/8x16/12x24
//mode:0,反色显示;1,正常显示
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 size1,u8 mode)
{
    u8 i,m,temp,size2,chr1;
    u8 x0=x,y0=y;
    if(size1==8)size2=6;
    else size2=(size1/8+((size1%8)?1:0))*(size1/2);  //得到字体一个字符对应点阵集所占的字节数
    chr1=chr-' ';  //计算偏移后的值
    for(i=0;i<size2;i++)
    {
        if(size1==8)
        {temp=asc2_0806[chr1][i];} //调用0806字体
        else if(size1==12)
        {temp=asc2_1206[chr1][i];} //调用1206字体
        else if(size1==16)
        {temp=asc2_1608[chr1][i];} //调用1608字体
        else if(size1==24)
        {temp=asc2_2412[chr1][i];} //调用2412字体
        else return;
        for(m=0;m<8;m++)
        {
            if(temp&0x01)OLED_DrawPoint(x,y,mode);
            else OLED_DrawPoint(x,y,!mode);
            temp>>=1;
            y++;
        }
        x++;
        if((size1!=8)&&((x-x0)==size1/2))
        {x=x0;y0=y0+8;}
        y=y0;
  }
    OLED_Update();
}


//显示字符串
//x,y:起点坐标  
//size1:字体大小 
//*chr:字符串起始地址 
//mode:0,反色显示;1,正常显示
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 size1,u8 mode)
{
    while((*chr>=' ')&&(*chr<='~'))//判断是不是非法字符!
    {
        OLED_ShowChar(x,y,*chr,size1,mode);
        if(size1==8)x+=6;
        else x+=size1/2;
        chr++;
  }
    OLED_Update();
}

//m^n
u32 OLED_Pow(u8 m,u8 n)
{
    u32 result=1;
    while(n--)
    {
      result*=m;
    }
    return result;
}

//显示数字
//x,y :起点坐标
//num :要显示的数字
//len :数字的位数
//size:字体大小
//mode:0,反色显示;1,正常显示
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size1,u8 mode)
{
    u8 t,temp,m=0;
    if(size1==8)m=2;
    for(t=0;t<len;t++)
    {
        temp=(num/OLED_Pow(10,len-t-1))%10;
            if(temp==0)
            {
                OLED_ShowChar(x+(size1/2+m)*t,y,'0',size1,mode);
      }
            else 
            {
              OLED_ShowChar(x+(size1/2+m)*t,y,temp+'0',size1,mode);
            }
  }
    OLED_Update();
}

//显示汉字
//x,y:起点坐标
//num:汉字对应的序号
//mode:0,反色显示;1,正常显示
void OLED_ShowChinese(u8 x,u8 y,u8 num,u8 size1,u8 mode)
{
    u8 m,temp;
    u8 x0=x,y0=y;
    u16 i,size3=(size1/8+((size1%8)?1:0))*size1;  //得到字体一个字符对应点阵集所占的字节数
    for(i=0;i<size3;i++)
    {
        if(size1==16)
                {temp=Hzk1[num][i];}//调用16*16字体
        else if(size1==24)
                {temp=Hzk2[num][i];}//调用24*24字体
        else if(size1==32)       
                {temp=Hzk3[num][i];}//调用32*32字体
        else if(size1==64)
                {temp=Hzk4[num][i];}//调用64*64字体
        else return;
        for(m=0;m<8;m++)
        {
            if(temp&0x01)OLED_DrawPoint(x,y,mode);
            else OLED_DrawPoint(x,y,!mode);
            temp>>=1;
            y++;
        }
        x++;
        if((x-x0)==size1)
        {x=x0;y0=y0+8;}
        y=y0;
    }
    OLED_Update();
}

//num 显示汉字的个数
//space 每一遍显示的间隔
//mode:0,反色显示;1,正常显示
void OLED_ScrollDisplay(u8 num,u8 space,u8 mode)
{
    u8 i,n,t=0,m=0,r;
    while(1)
    {
        if(m==0)
        {
        OLED_ShowChinese(128,24,t,16,mode); //写入一个汉字保存在OLED_GRAM[][]数组中
            t++;
        }
        if(t==num)
            {
                for(r=0;r<16*space;r++)      //显示间隔
                 {
                    for(i=1;i<144;i++)
                        {
                            for(n=0;n<8;n++)
                            {
                                OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
                            }
                        }
           OLED_Update();
                 }
        t=0;
      }
        m++;
        if(m==16){m=0;}
        for(i=1;i<144;i++)   //实现左移
        {
            for(n=0;n<8;n++)
            {
                OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
            }
        }
        OLED_Update();
    }
}

//x,y：起点坐标
//sizex,sizey,图片长宽
//BMP[]：要写入的图片数组
//mode:0,反色显示;1,正常显示
void OLED_ShowPicture(u8 x,u8 y,u8 sizex,u8 sizey,u8 BMP[],u8 mode)
{
    u16 j=0;
    u8 i,n,temp,m;
    u8 x0=x,y0=y;
    sizey=sizey/8+((sizey%8)?1:0);
    for(n=0;n<sizey;n++)
    {
         for(i=0;i<sizex;i++)
         {
                temp=BMP[j];
                j++;
                for(m=0;m<8;m++)
                {
                    if(temp&0x01)OLED_DrawPoint(x,y,mode);
                    else OLED_DrawPoint(x,y,!mode);
                    temp>>=1;
                    y++;
                }
                x++;
                if((x-x0)==sizex)
                {
                    x=x0;
                    y0=y0+8;
                }
                y=y0;
     }
     }
    OLED_Update();
}
/*---------------------------新添加函数---------------------------*/
/**
 * @brief 清除指定矩形区域（将区域内所有点置0）
 * @param x 左上角横坐标
 * @param y 左上角纵坐标
 * @param w 宽度
 * @param h 高度
 */
void OLED_ClearArea(u8 x, u8 y, u8 w, u8 h)
{
    u8 i, j, page, bit, end_x, end_y;
    end_x = x + w;
    end_y = y + h;
    if (end_x > 127) end_x = 127;
    if (end_y > 63)  end_y = 63;

    for (i = x; i < end_x; i++)
    {
        for (j = y; j < end_y; j++)
        {
            page = j / 8;
            bit  = j % 8;
            OLED_GRAM[i][page] &= ~(1 << bit);
        }
    }
}
/**
 * @brief 指定矩形区域取反（点亮的变灭，熄灭的变亮）
 * @param x 左上角横坐标
 * @param y 左上角纵坐标
 * @param w 宽度
 * @param h 高度
 */
void OLED_ReverseArea(u8 x, u8 y, u8 w, u8 h)
{
    u8 i, j, page, bit, end_x, end_y;
    end_x = x + w;
    end_y = y + h;
    if (end_x > 127) end_x = 127;
    if (end_y > 63)  end_y = 63;

    for (i = x; i < end_x; i++)
    {
        for (j = y; j < end_y; j++)
        {
            page = j / 8;
            bit  = j % 8;
            OLED_GRAM[i][page] ^= (1 << bit);
        }
    }
}
/**
 * @brief 全屏取反
 */
void OLED_Reverse(void)
{
    u8 i, j;
    for (i = 0; i < 128; i++)
        for (j = 0; j < 8; j++)
            OLED_GRAM[i][j] ^= 0xFF;
}
/**
 * @brief 显示有符号整数（带正负号）
 */
void OLED_ShowSignedNum(u8 x, u8 y, s32 num, u8 len, u8 size1, u8 mode)
{
    u8 i;
    u32 abs_num;
    if (num < 0)
    {
        OLED_ShowChar(x, y, '-', size1, mode);
        abs_num = -num;
    }
    else
    {
        OLED_ShowChar(x, y, '+', size1, mode);
        abs_num = num;
    }
    for (i = 0; i < len; i++)
    {
        u8 digit = abs_num / OLED_Pow(10, len - i - 1) % 10;
        OLED_ShowChar(x + (i + 1) * size1, y, digit + '0', size1, mode);
    }
}
/**
 * @brief 显示十六进制数
 */
void OLED_ShowHexNum(u8 x, u8 y, u32 num, u8 len, u8 size1, u8 mode)
{
    u8 i, d;
    for (i = 0; i < len; i++)
    {
        d = (num >> (4 * (len - i - 1))) & 0x0F;
        if (d < 10)
            OLED_ShowChar(x + i * size1, y, d + '0', size1, mode);
        else
            OLED_ShowChar(x + i * size1, y, d - 10 + 'A', size1, mode);
    }
}
/**
 * @brief 显示二进制数
 */
void OLED_ShowBinNum(u8 x, u8 y, u32 num, u8 len, u8 size1, u8 mode)
{
    u8 i;
    for (i = 0; i < len; i++)
    {
        u8 bit = (num >> (len - i - 1)) & 0x01;
        OLED_ShowChar(x + i * size1, y, bit + '0', size1, mode);
    }
}

/**
 * @brief 显示中文字符串（支持 ASCII 和 GB2312 汉字混合）
 * @param X 起始横坐标（0~127）
 * @param Y 起始纵坐标（0~63）
 * @param String 要显示的字符串（例如 "你好123"）
 * @param FontSize 字体大小（仅 _16x16 时正常显示 16x16 汉字，其他字体显示 '?'）
 * @note  汉字字模来源于 OLED_CF16x16 数组，需确保字符串中的汉字已在该数组中定义
 *       //调用后不会自动更新屏幕，需要手动调用 OLED_Update()
 */
void OLED_ShowChineseString(int16_t X, int16_t Y, char *String, uint8_t FontSize)
{
    uint16_t i = 0;
    char SingleChar[3];
    uint8_t CharLength;
    int16_t XOffset = 0;
    uint16_t pIndex;
    uint8_t x, y, bit, page, shift;
    uint8_t *hzData;

    while (String[i] != '\0')
    {
        // 提取一个字符
        if ((String[i] & 0x80) == 0x00)   // ASCII
        {
            CharLength = 1;
            SingleChar[0] = String[i++];
            SingleChar[1] = '\0';
        }
        else                              // GB2312 汉字
        {
            CharLength = 2;
            SingleChar[0] = String[i++];
            if (String[i] == '\0') break;
            SingleChar[1] = String[i++];
            SingleChar[2] = '\0';
        }

        if (CharLength == 1)   // 显示 ASCII 字符
        {
            OLED_ShowChar(X + XOffset, Y, SingleChar[0], FontSize, 1);
            XOffset += FontSize;   // 宽度 = 字体大小（例如 8 或 16）
        }
        else   // 显示汉字
        {
            // 在字库中查找
            for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex++)
            {
                if (strcmp(OLED_CF16x16[pIndex].Index, SingleChar) == 0)
                    break;
            }

            if (FontSize == _16x16)   // 只有字体大小为16时才显示16x16汉字
            {
                hzData = (uint8_t*)OLED_CF16x16[pIndex].Data;
                // 直接绘制 16x16 点阵（纵向8点，高位在下）
                for (x = 0; x < 16; x++)          // 列
                {
                    for (y = 0; y < 16; y++)      // 行
                    {
                        page = y / 8;
                        shift = y % 8;
                        bit = (hzData[page * 16 + x] >> shift) & 0x01;
                        if (bit)
                            OLED_DrawPoint(X + XOffset + x, Y + y, 1);
                        else
                            OLED_DrawPoint(X + XOffset + x, Y + y, 0);
                    }
                }
                XOffset += 16;
            }
            else   // 字体不支持汉字，显示 '?'
            {
                OLED_ShowChar(X + XOffset, Y, '?', FontSize, 1);
                XOffset += FontSize;
            }
        }
    }
    //OLED_Update();
}

/**
 * @brief 格式化输出（类似 printf）
 * @param x 横坐标
 * @param y 纵坐标
 * @param size1 字体大小
 * @param mode  是否显示中文,1显示中文，0显示字符
 * @param fmt 格式字符串
 * @param ... 可变参数
 */
void OLED_Printf(u8 x, u8 y, u8 size1, u8 mode, const char *fmt, ...)
{
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    if(mode == 1){
        OLED_ShowChineseString(x,y,(char*)buf,_16x16);
    }
    else{
        OLED_ShowString(x, y, (u8*)buf, size1, 1);
    }
}


/* ---------------------------- 滚动显示控制 ------------------------------------- */
/* 支持 ASCII(8x16) 与 GB2312 汉字(16x16) 混合滚动显示 */
#define OLED_ROLL_MAX_CHARS 64
#define OLED_ROLL_GAP_COLS 8

static const uint8_t *oled_roll_char_data[OLED_ROLL_MAX_CHARS];
static uint8_t oled_roll_char_width[OLED_ROLL_MAX_CHARS];
static uint8_t oled_roll_char_type[OLED_ROLL_MAX_CHARS]; // 0:16x16(汉字), 1:8x16(ASCII)
static uint16_t oled_roll_prefix[OLED_ROLL_MAX_CHARS + 1];
static uint16_t oled_roll_char_count = 0;    // 有效字符数量
static uint16_t oled_roll_total_cols = 0;    // 总列数
static uint16_t oled_roll_offset = 0;        // 当前左移列偏移
static uint8_t oled_roll_y = 0;              // 滚动显示起始纵坐标
static user_timer_e oled_roll_timer = USER_TIMER4; // 默认使用定时器4
static uint8_t oled_roll_running = 0;

//计算字符串的总像素宽度（用于滚动显示的列数计算）
u16 calc_string_width(const char *text)
{
    u16 w = 0;
    const unsigned char *p = (const unsigned char *)text;
    while (*p)
    {
        if (*p & 0x80) { w += 16; p += 2; }
        else { w += 8; p += 1; }
    }
    return w;
}

// 将从 offset 开始的连续 128 列渲染到显存，起点横坐标为 start_x，纵坐标为 Y，高度固定 16
static void draw_string_to_gram(u16 offset, u8 start_x, u8 Y)
{
    u16 col;
    if (oled_roll_total_cols == 0) return;
    for (col = 0; col < 128; col++)
    {
        u16 global_col = offset + col;
        global_col %= oled_roll_total_cols;

        uint8_t page0 = 0, page1 = 0;

        if (global_col < oled_roll_prefix[oled_roll_char_count])
        {
            // 在字符区域内，按前缀表查找字符索引
            uint16_t ci = 0;
            while (ci < oled_roll_char_count && global_col >= oled_roll_prefix[ci+1]) ci++;
            if (ci < oled_roll_char_count)
            {
                uint16_t col_in_char = global_col - oled_roll_prefix[ci];
                const uint8_t *data = oled_roll_char_data[ci];
                if (oled_roll_char_type[ci] == 0) // 16x16 汉字
                {
                    if (col_in_char < 16)
                    {
                        page0 = data[col_in_char];
                        page1 = data[16 + col_in_char];
                    }
                }
                else // ASCII 8x16
                {
                    if (col_in_char < 8)
                    {
                        page0 = data[col_in_char];
                        page1 = data[8 + col_in_char];
                    }
                }
            }
        }

        u8 real_x = start_x + col;
        if (real_x >= 128) continue;

        // 绘制 16 行（2 个 page）
        for (u8 b = 0; b < 8; b++)
        {
            u8 t = (page0 >> b) & 0x01;
            if ((u16)(Y + b) < 64) OLED_DrawPoint(real_x, Y + b, t);
        }
        for (u8 b = 0; b < 8; b++)
        {
            u8 t = (page1 >> b) & 0x01;
            if ((u16)(Y + 8 + b) < 64) OLED_DrawPoint(real_x, Y + 8 + b, t);
        }
    }
    OLED_UpdateArea(start_x, Y, 128, 16);
}

// 定时器回调：左移一列并绘制
static void roll_timer_cb(void *arg)
{
    if (!oled_roll_running) return;
    oled_roll_offset++;
    if (oled_roll_offset >= oled_roll_total_cols) oled_roll_offset = 0;
    draw_string_to_gram(oled_roll_offset, 0, oled_roll_y);
}

// 启动循环左移显示（支持 GB2312 汉字 + ASCII）
// x: 起始横坐标（建议传 0）
// y: 起始纵坐标（0~48），显示高度为 16
// text: GB2312 编码字符串（可混合 ASCII）
// speed_ms: 每次移动的时间间隔（毫秒）
void OLED_ShowChinese_Roll(u8 x, u8 y, const char *text, u32 speed_ms)
{
    // 停止已有滚动
    OLED_StopRoll();
    if (text == NULL) return;

    // 解析字符串，提取每个字符并在字库中索引
    const unsigned char *p = (const unsigned char *)text;
    oled_roll_char_count = 0;
    while (*p && oled_roll_char_count < OLED_ROLL_MAX_CHARS)
    {
        if (*p & 0x80)
        {
            // GB2312 两字节汉字
            char key[3];
            key[0] = p[0]; key[1] = p[1]; key[2] = '\0';

            const ChineseCell_t *cell = NULL;
            for (uint16_t pi = 0; strcmp(OLED_CF16x16[pi].Index, "") != 0; pi++)
            {
                if (strcmp(OLED_CF16x16[pi].Index, key) == 0)
                {
                    cell = &OLED_CF16x16[pi];
                    break;
                }
            }
            // 若未找到，使用末尾的默认图形
            if (cell == NULL)
            {
                uint16_t last = 0;
                while (strcmp(OLED_CF16x16[last].Index, "") != 0) last++;
                cell = &OLED_CF16x16[last];
            }

            oled_roll_char_data[oled_roll_char_count] = (const uint8_t *)cell->Data;
            oled_roll_char_width[oled_roll_char_count] = 16;
            oled_roll_char_type[oled_roll_char_count] = 0;
            oled_roll_char_count++;
            p += 2;
        }
        else
        {
            // ASCII 单字节，使用 asc2_1608（8x16）
            unsigned char c = *p;
            const uint8_t *adata = NULL;
            if (c >= ' ' && c <= '~')
            {
                adata = (const uint8_t *)asc2_1608[c - ' '];
            }
            else
            {
                adata = (const uint8_t *)asc2_1608[0]; // 非打印字符显示空格
            }

            oled_roll_char_data[oled_roll_char_count] = adata;
            oled_roll_char_width[oled_roll_char_count] = 8;
            oled_roll_char_type[oled_roll_char_count] = 1;
            oled_roll_char_count++;
            p += 1;
        }
    }

    if (oled_roll_char_count == 0) return;

    // 计算前缀表与总列数
    oled_roll_prefix[0] = 0;
    for (uint16_t i = 0; i < oled_roll_char_count; i++)
    {
        oled_roll_prefix[i+1] = oled_roll_prefix[i] + oled_roll_char_width[i];
    }
    oled_roll_total_cols = oled_roll_prefix[oled_roll_char_count] + OLED_ROLL_GAP_COLS;

    oled_roll_y = y;
    oled_roll_offset = 0;
    oled_roll_running = 1;

    draw_string_to_gram(0, x, y);//首帧渲染
    drv_timer_init(oled_roll_timer, speed_ms, roll_timer_cb);//启动定时器
}

// 停止滚动显示
void OLED_StopRoll(void)
{
    if (!oled_roll_running) return;
    drv_timer_stop(oled_roll_timer);
    drv_timer_close(oled_roll_timer);
    oled_roll_running = 0;
}

/*-------------------------------------------------------------------------------------*/

//OLED的初始化
void OLED_Init(void)
{
    MyRTC_Init();
    
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(OLED_SCL_GPIO_CLK|OLED_SDA_GPIO_CLK, ENABLE);	 //使能A端口时钟
    GPIO_InitStructure.GPIO_Pin = OLED_SCL_PIN;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
    GPIO_Init(OLED_SCL_PORT, &GPIO_InitStructure);	  //初始化PA0,1
    GPIO_SetBits(OLED_SCL_PORT,OLED_SCL_PIN);
    
    GPIO_InitStructure.GPIO_Pin = OLED_SDA_PIN;	
    GPIO_Init(OLED_SDA_PORT, &GPIO_InitStructure);	  //初始化PA0,1
    GPIO_SetBits(OLED_SDA_PORT,OLED_SDA_PIN);

    Delay_ms(200);
    
    OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
    OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
    OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
    OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
    OLED_WR_Byte(0xCF,OLED_CMD);// Set SEG Output Current Brightness
    OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
    OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
    OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
    OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
    OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
    OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    OLED_WR_Byte(0x00,OLED_CMD);//-not offset
    OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
    OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
    OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
    OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
    OLED_WR_Byte(0x12,OLED_CMD);
    OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
    OLED_WR_Byte(0x30,OLED_CMD);//Set VCOM Deselect Level
    OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
    OLED_WR_Byte(0x02,OLED_CMD);//
    OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
    OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
    OLED_Clear();
    OLED_WR_Byte(0xAF,OLED_CMD);
}

