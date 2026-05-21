#include "selfDelay.h"
void Delay_us(uint16_t time){
    SysTick->LOAD=168*time;
    SysTick->CTRL=0x5;
    SysTick->VAL=0;
    while (!(SysTick->CTRL&0x10000));
    SysTick->CTRL=0x4;
}
void Delay_ms(uint16_t time){
    while(time--){
        Delay_us(1000);
    }
}
void Delay_s(uint16_t time){
    while (time--)
    {
        Delay_ms(1000);
    }
}