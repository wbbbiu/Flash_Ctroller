#ifndef FLASH_FUNCTION_H
#define FLASH_FUNCTION_H
#include "flash_base.h"
System_Error_t Flash_Config_Write(Spi_Device *spi,uint16_t index,uint8_t *data,uint32_t len);
System_Error_t Flash_Config_Read(Spi_Device *spi,uint16_t index,uint8_t *data,uint32_t len);
#endif