#ifndef SPI_BASE_H
#define SPI_BASE_H
#include "base.h"
typedef struct Spi_Device{
    Gpio_Base cs;
    Gpio_Base sck;
    Gpio_Base Mosi;
    Gpio_Base Miso;
    SPI_TypeDef *spi;
}Spi_Device;
//初始化这个Spi
void Spi_Create(Spi_Device *Spi,Gpio_Base *cs,Gpio_Base *sck,Gpio_Base *miso,Gpio_Base *mosi,SPI_TypeDef *spi);

uint8_t Spi_RW(Spi_Device *device,uint8_t data);

static inline void Spi_Cs_Low(Spi_Device *spi){
    GPIO_ResetBits(spi->cs.port,spi->cs.pin);
}
static inline void Spi_Cs_Hight(Spi_Device *spi){
    GPIO_SetBits(spi->cs.port,spi->cs.pin);
}
void Spi_Clock_Init(SPI_TypeDef *spi);
#endif