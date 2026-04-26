#ifndef __OLED_DATA_H
#define __OLED_DATA_H

#include <stdint.h>

/*字符集定义*/
/*以下两个宏定义只可解除其中一个的注释*/
//#define OLED_CHARSET_UTF8			//定义字符集为UTF8
#define OLED_CHARSET_GB2312		//定义字符集为GB2312

/*字模基本单元*/
typedef struct 
{
	
#ifdef OLED_CHARSET_UTF8			//定义字符集为UTF8
	char Index[5];					//汉字索引，空间为5字节
#endif
	
#ifdef OLED_CHARSET_GB2312			//定义字符集为GB2312
	char Index[3];					//汉字索引，空间为3字节
#endif
	
	uint8_t Data[32];				//字模数据
} ChineseCell_t;

/*ASCII字模数据声明*/
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F6x8[][6];
extern const uint8_t OLED_F12x24[][36];

extern const unsigned char asc2_0806[][6];
//12*12 ASCII字符集点阵
extern const unsigned char asc2_1206[95][12];
//16*16 ASCII字符集点阵
extern const unsigned char asc2_1608[][16];
//24*24 ASICII字符集点阵
extern const unsigned char asc2_2412[][36];


/*汉字字模数据声明*/
extern const ChineseCell_t OLED_CF16x16[];

extern const unsigned char Hzk1[][32];
extern const unsigned char Hzk2[][72];
extern const unsigned char Hzk3[][128];
extern const unsigned char Hzk4[][512];


/*图像数据声明*/
extern const uint8_t Diode [];
extern const uint8_t Return[];
extern const uint8_t charge[];
extern const uint8_t success[];
extern const uint8_t play_button[];
extern const uint8_t stop_play[];
extern const uint8_t last_song[];
extern const uint8_t next_song[];
extern const uint8_t order[];
extern const uint8_t random[];
extern const uint8_t cycle[];
extern const uint8_t close_lrc[];
/*按照上面的格式，在这个位置加入新的图像数据声明*/
//...
#endif
