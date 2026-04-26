#ifndef __SPI_VS1053_H
#define __SPI_VS1053_H

#include "com_type.h"
#include "platform_mcu_resource.h"

/* VS10XX ¶ÁÐ´ÃüÁî */
#define VS_WRITE_COMMAND    0x02
#define VS_READ_COMMAND     0x03

/* VS10XX ¼Ä´æÆ÷¶¨Òå */
#define SPI_MODE            0x00
#define SPI_STATUS          0x01
#define SPI_BASS            0x02
#define SPI_CLOCKF          0x03
#define SPI_DECODE_TIME     0x04
#define SPI_AUDATA          0x05
#define SPI_WRAM            0x06
#define SPI_WRAMADDR        0x07
#define SPI_HDAT0           0x08
#define SPI_HDAT1           0x09

#define SPI_AIADDR          0x0a
#define SPI_VOL             0x0b
#define SPI_AICTRL0         0x0c
#define SPI_AICTRL1         0x0d
#define SPI_AICTRL2         0x0e
#define SPI_AICTRL3         0x0f
#define SM_DIFF             0x01
#define SM_JUMP             0x02
#define SM_RESET            0x04
#define SM_OUTOFWAV         0x08
#define SM_PDOWN            0x10
#define SM_TESTS            0x20
#define SM_STREAM           0x40
#define SM_PLUSV            0x80
#define SM_DACT             0x100
#define SM_SDIORD           0x200
#define SM_SDISHARE         0x400
#define SM_SDINEW           0x800
#define SM_ADPCM            0x1000
#define SM_ADPCM_HP         0x2000

#define I2S_CONFIG          0XC040
#define GPIO_DDR            0XC017
#define GPIO_IDATA          0XC018
#define GPIO_ODATA          0XC019

typedef struct{

    u8 left_volume;
    u8 right_volume;
}vs1053_volume_t;




void vs1053_w_XCS(u8 value);
void vs1053_w_XDCS(u8 value);
void vs1053_w_XRESET(u8 value);
u8 vs1053_r_DREQ(void);

static void delay_ms(u32 ms);
void vs1053_spi_init(void);
void vs1053_spi_highSpeed(void);
u8 spi1_readwriteByte(u8 TxData);

void vs1053_hard_XRESET(void);
void vs1053_soft_XRESET(void);

void vs1053_write_regsiter(u8 reg_addr,u16 data);
u16 vs1053_read_regsiter(u8 reg_addr);

u8 vs1053_write_data(u8* buf);
void vs1053_set_volume(u8 left,u8 right);


void vs1053_set_speaker(u8 sw);
void vs1053_write_ram(u16 addr,u16 data);
u16 vs1053_read_ram(u16 address);

void vs1053_reset_decode_time(void);
void vs1053_set_speed(u8 speed);
void vs1053_restart_play(void);

void vs1053_set_all(void);
void vs1053_sin_test(void);
#endif
