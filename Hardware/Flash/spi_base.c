#include "spi_base.h"
void Spi_Create(Spi_Device *Spi,Gpio_Base *cs,Gpio_Base *sck,Gpio_Base *miso,Gpio_Base *mosi,SPI_TypeDef *spi){
    Spi->cs=*cs;
    Spi->Miso=*miso;
    Spi->Mosi=*mosi;
    Spi->sck=*sck;
    Spi->spi=spi;
}

void Spi_Clock_Init(SPI_TypeDef *spi){
    switch ((uint32_t)spi)
    {
    case (uint32_t)SPI1:
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
        break;
    case (uint32_t)SPI2:
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
    break;
    case (uint32_t)SPI3:
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3,ENABLE);
    break;
    case (uint32_t)SPI4:
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI4,ENABLE);
    break;
    case (uint32_t)SPI5:
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI5,ENABLE);
    break;
    case (uint32_t)SPI6:
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI6,ENABLE);
    break;
    default:
        break;
    }
}

uint8_t Spi_RW(Spi_Device *device,uint8_t data){
    while(SPI_GetFlagStatus(device->spi,SPI_FLAG_TXE)==RESET);
    SPI_SendData(device->spi,data);
    while(SPI_GetFlagStatus(device->spi,SPI_FLAG_RXNE)==RESET);
    uint8_t ret=SPI_ReceiveData(device->spi);
    return ret;
}
