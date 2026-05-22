#ifndef FLASH_FUNCTION_H
#define FLASH_FUNCTION_H
#include "flash_base.h"
#include "flash_context.h"
#define CONFIG_FLASH 0
#define LOG_FLASH 1
#define PAGE_SIZE 0X100
#define SECTOR_SIZE 0X1000
#define BLOCK_SIZE 0X10000

typedef struct Flash_Map_t{
    uint16_t log_cur;
    uint16_t config_cur;
    uint16_t config_addrs[STORAGE_ID_MAX];
}Flash_Map_t;
void Flash_Original(Spi_Device *spi);
System_Error_t Flash_Find_Idle(Spi_Device *spi,uint8_t mode,uint32_t *ret);
System_Error_t Flash_Map_Read(Spi_Device *spi,uint32_t *ret);
System_Error_t Flash_Config_Write(Spi_Device *spi,uint16_t index,uint8_t *data,uint32_t len);
System_Error_t Flash_Config_Read(Spi_Device *spi,uint16_t index,uint8_t *data,uint32_t len);
void Flash_Original(Spi_Device *spi);
System_Error_t Flash_Copy_Idle(Spi_Device *spi,uint16_t *idle,uint16_t *start_page);
void Flash_Begin_Config(Spi_Device *spi);
void Flash_Begin_Log(Spi_Device *spi);
System_Error_t Flash_Write_Data(Spi_Device *spi,uint16_t id,uint8_t *data,uint32_t len,uint8_t mode);
System_Error_t Flash_Read_Config(Spi_Device *spi,uint16_t id,uint8_t *data,uint32_t len,uint8_t mode);
//count读几条
System_Error_t Flash_Read_LOG(Spi_Device *spi,uint8_t *data,uint32_t len,uint8_t mode,uint16_t count);
#endif