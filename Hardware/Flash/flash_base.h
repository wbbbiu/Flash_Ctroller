#ifndef FLASH_BASE_H
#define FLASH_BASE_H
#include "spi_base.h"
#define WRITE_ENABLE 0X06
#define WRITE_DATA 0X02
#define READ_R1 0X05
#define READ_R2 0X35
#define READ_R3 0X15
#define ERASE_SECTOR 0X20
#define ERASE_BLOCK32 0X52
#define ERASE_BLOCK64 0Xd8
#define ERASE_ALL  0XC7
#define WAIT_BUYS_MASK 0X01
#define READ_MASK 0XFF
#define READ_DATA 0X03
typedef enum{
   PAGE,//页
   SECTOR,//扇区
   BLOCK32,//32kb的块
   BLOCK64,//64KB的快
   ALL
}Mem_Level;
void Flash_init(Spi_Device *device);
System_Error_t Flash_ReadData(Spi_Device *spi,uint32_t addr,uint8_t *data,uint32_t len);
System_Error_t Flash_Write_Sector(Spi_Device *spi,uint32_t addr,uint8_t *data,uint32_t len);
void Flash_Erase_Memory(Spi_Device *spi,uint32_t addr,Mem_Level level);
#endif