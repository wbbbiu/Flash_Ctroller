#include "flash_base.h"
static inline void Write_Enable(Spi_Device *spi){
    Spi_Cs_Low(spi);
    Spi_RW(spi,WRITE_ENABLE);
    Spi_Cs_Hight(spi);
}
static inline uint8_t ReadSR1(Spi_Device *spi){
    Spi_Cs_Low(spi);
    Spi_RW(spi,READ_R1);
    uint8_t val=Spi_RW(spi,READ_MASK);
    Spi_Cs_Hight(spi);
    return val;
}

static inline void Write_Addr(Spi_Device *spi,uint32_t addr){
    Spi_RW(spi,(addr>>16)&0xFF);
    Spi_RW(spi,(addr>>8)&0xFF);
    Spi_RW(spi,(addr)&0xFF);
}
static inline void WaitBusy(Spi_Device *spi){
     while(ReadSR1(spi)&WAIT_BUYS_MASK);
}
static inline void erase_memory(Spi_Device *spi,Mem_Level level){
    switch (level)
    {
    case SECTOR:
    Spi_RW(spi,ERASE_SECTOR);
        break;
    case BLOCK32:
        Spi_RW(spi,ERASE_BLOCK32);
            break;
    case BLOCK64:
        Spi_RW(spi,ERASE_BLOCK64);
                break;
    case ALL:
        Spi_RW(spi,ERASE_ALL);
                    break;
    default:
    Spi_RW(spi,ERASE_SECTOR);
        break;
    }
}
void Flash_init(Spi_Device *device){
    Spi_Clock_Init(device->spi);
    GPIO_Clock_Init(device->cs.port);
    GPIO_Clock_Init(device->Mosi.port);
    GPIO_Clock_Init(device->Miso.port);
    GPIO_Clock_Init(device->sck.port);
    GPIO_AFOut_Init(&device->Miso,GPIO_AF_SPI1);
    GPIO_AFOut_Init(&device->Mosi,GPIO_AF_SPI1);
    GPIO_AFOut_Init(&device->sck,GPIO_AF_SPI1);
    GPIO_Out_Init(&device->cs,1);
    
    SPI_InitTypeDef INIT;
    //分配设置
    INIT.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;
    //设置这个边沿
    INIT.SPI_CPHA=SPI_CPHA_1Edge;
    //
    INIT.SPI_CPOL=SPI_CPOL_Low;
    INIT.SPI_CRCPolynomial=0;
    INIT.SPI_DataSize=SPI_DataSize_8b;
    INIT.SPI_Direction=SPI_Direction_2Lines_FullDuplex;
    INIT.SPI_FirstBit=SPI_FirstBit_MSB;
    INIT.SPI_Mode=SPI_Mode_Master;
    INIT.SPI_NSS=SPI_NSS_Soft;
    SPI_Init(device->spi,&INIT);
    SPI_Cmd(device->spi,ENABLE);
}

void Flash_Erase_Memory(Spi_Device *spi,uint32_t addr,Mem_Level level){
    Write_Enable(spi);
    Spi_Cs_Low(spi);
    erase_memory(spi,level);
    if(level!=ALL)
    Write_Addr(spi,addr);
    Spi_Cs_Hight(spi);
    WaitBusy(spi);
}
//进行一个扇区编写
System_Error_t Flash_Write_Sector(Spi_Device *spi,uint32_t addr,uint8_t *data,uint32_t len){
   uint32_t des_addr=addr+len-1;//最终编写到那个地址
   uint8_t in_page_writing=0;//第一次页需要初始化
   System_Error_t error={.all=0};
   if((des_addr&0xFFFFF000)!=(addr&0xFFFFF000)){//如果就是最后这个写到的地址已经不在是这个扇区跨扇区了失败
        error.bits.module=MOD_FLASH;
        error.bits.errno=WDATA_LEN_LARGE;
        return error;
   }
   erase_memory(spi,SECTOR);//擦除扇区
    while(addr<=des_addr){//一直写到这个地址
        if(!in_page_writing){//第一只或者说00全0的页地址是新的需要使能,拉低从新开始
            Write_Enable(spi);
            Spi_Cs_Low(spi);
            Spi_RW(spi, WRITE_DATA);
            Write_Addr(spi,addr);
            in_page_writing=1;
        }
        Spi_RW(spi,*data++);//写入数据
        if(((addr&0xFF)==0xFF)&&in_page_writing){//FF页内地址的最后一个这个写完就是新的页
            Spi_Cs_Hight(spi);//关闭
            WaitBusy(spi);//等待
            in_page_writing=0;
        }
        addr++;
    }
    if(in_page_writing){//最后特判一下如果没有关闭就手动关闭
    Spi_Cs_Hight(spi);
    WaitBusy(spi);
    }
   return error;
}
System_Error_t Flash_ReadData(Spi_Device *spi,uint32_t addr,uint8_t *data,uint32_t len){
    System_Error_t error={.all=0};
    Spi_Cs_Low(spi);
    Spi_RW(spi,READ_DATA);
    Write_Addr(spi,addr);
    for(int i=0;i<(int)len;i++){
        data[i]=Spi_RW(spi,READ_MASK);
    }
    Spi_Cs_Hight(spi);
    return error;
 }