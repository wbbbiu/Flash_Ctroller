#include "flash_function.h"
System_Error_t Flash_Config_Write(Spi_Device *spi,uint16_t index,uint8_t *data,uint32_t len){
    System_Error_t error={.all=0};
    if(index>0x7FF){
        error.bits.module=MOD_FLASH;
        error.bits.errno=SECTOR_INDEX_LARGE;
        return error;
    }
      uint32_t addr=index<<12;
     error=Flash_Write_Sector(spi,addr,data,len);
     return error;
}
System_Error_t Flash_Config_Read(Spi_Device *spi,uint16_t index,uint8_t *data,uint32_t len){
    System_Error_t error={.all=0};
    if(index>0x7FF){
        error.bits.module=MOD_FLASH;
        error.bits.errno=SECTOR_INDEX_LARGE;
        return error;
    }
      uint32_t addr=index<<12;

     error=Flash_ReadData(spi,addr,data,len);
     return error;
}