#include "stdio.h"
#include "stm32f10x.h"                  // Device header
#include "dev_w25q64.h"
#include "drv_spi.h"

typedef enum{
 read_deviceID          = 0,
 read_vendorID          = 1,
 read_status_register_1 = 2,
 write_enable           = 3,
 page_program           = 4,
 sector_erase_4KB       = 5,
 block_erase_32KB       = 6,
 block_erase_64KB       = 7,
 chip_erase             = 8,
 read_data              = 9,
 lowpower_mode          = 10,
 useless_command        = 11,
}W25Q64_Command;

static u8 w25q64_command[12] = {
    0x90,        //read_deviceID  读制造商ID/设备ID    可选的ID读取方式（需搭配地址位，分普通模式 / 双 IO 模式）
    0x9F,        //read_vendorID  读芯片 ID            读取厂商 ID + 内存类型 + 容量信息（最常用的设备 ID 读取指令）
    0x05,        //read_status_register_1  读状态寄存器 1       读取 BUSY、WEL 等状态位（判断芯片是否忙 / 可写）
    0x06,        //write_enable  写使能               所有写 / 擦除操作前必须发送此指令
    0x02,        //page_program  页编程（单字节写）   向指定地址写入 1~256 字节数据
    0x20,        //sector_erase_4KB  扇区擦除（4KB）      擦除指定 4KB 扇区（擦除后数据为 0xFF）
    0x52,        //block_erase_32KB  块擦除（32KB）       擦除指定 32KB 
    0xD8,        //block_erase_64KB  块擦除（64KB）       擦除指定 64KB 块
    0xC7,        //chip_erase  芯片擦除             擦除整个芯片（耗时较长，约数秒）
    0x03,        //read_data  读数据（标准 SPI）   从指定地址读取任意长度数据（最常用的读数据指令）
    0xB9,        //lowpower_mode  掉电模式             进入低功耗模式
    0xFF,        //useless_command
};
static u8 address_byte = 0x00;

u16 w25q64_read_deviceID(void)
{
    u8 data1,data2;
    u16 receive_data;
    drv_spi_write(USER_SPI_3,&w25q64_command[read_deviceID],8);
    drv_spi_write(USER_SPI_3,&address_byte,8);
    drv_spi_write(USER_SPI_3,&address_byte,8);
    drv_spi_write(USER_SPI_3,&address_byte,8);
    drv_spi_xfer(USER_SPI_3,&w25q64_command[useless_command],&data1,8);
    drv_spi_xfer(USER_SPI_3,&w25q64_command[useless_command],&data2,8);
    receive_data = (data1 << 8) | data2;
    return receive_data;
}
/*使能W25Q64读取操作*/
void w25q64_write_enable(void)
{
    drv_spi_write(USER_SPI_3,&w25q64_command[write_enable],8);
}
/*等待W25Q64的busy标志位置位*/
void w25q64_wait_busy(void)
{
    u8 byte = 0;
    do{
        drv_spi_write(USER_SPI_3,&w25q64_command[read_status_register_1],8);
        drv_spi_xfer(USER_SPI_3,&w25q64_command[useless_command],&byte,8);
    }while((byte & 0x01) == 1);
}
/*芯片擦除*/
void w25q64_earse_chip(void)
{
    w25q64_write_enable();
    w25q64_wait_busy();
    
    drv_spi_write(USER_SPI_3,&w25q64_command[chip_erase],8);
    w25q64_wait_busy();
}
/*扇区擦除*/
void w25q64_earse_sector(u32 address)
{
    w25q64_write_enable();
    w25q64_wait_busy();
    
    drv_spi_write(USER_SPI_3,&w25q64_command[sector_erase_4KB],8);
    drv_spi_write(USER_SPI_3,(u8*)(address >> 16),8);
    drv_spi_write(USER_SPI_3,(u8*)(address >> 8),8);
    drv_spi_write(USER_SPI_3,(u8*)(address),8);
    
    w25q64_wait_busy();
}
/*向扇区写入数据*/
void w25q64_write_sector(u32 address,u8* buff,u16 size)
{
    u32 index = 0;
    s32 page_cnt = size / 256;  //页大小256Byte
    while(page_cnt --)
    {
        w25q64_write_enable();
        w25q64_wait_busy();
        
        drv_spi_write(USER_SPI_3,&w25q64_command[page_program],8);
        drv_spi_write(USER_SPI_3,(u8*)(address >> 16),8);
        drv_spi_write(USER_SPI_3,(u8*)(address >> 8),8);
        drv_spi_write(USER_SPI_3,(u8*)(address),8);
        for(u32 i=0;i<256;i++)
        {
            drv_spi_write(USER_SPI_3,&buff[i+index],8);
        }
        w25q64_wait_busy();
        address += 256;
        index   += 256;
    }
}
/*向扇区中读取数据*/
void w25q64_read_sector(u32 address,u8* buff,u16 size)
{
    drv_spi_write(USER_SPI_3,&w25q64_command[read_data],8);
    drv_spi_write(USER_SPI_3,(u8*)(address >> 16),8);
    drv_spi_write(USER_SPI_3,(u8*)(address >> 8),8);
    drv_spi_write(USER_SPI_3,(u8*)(address),8);
    for(u32 i=0;i<size;i++)
    {
        drv_spi_xfer(USER_SPI_3,&w25q64_command[useless_command],&buff[i],8);
    }
}
