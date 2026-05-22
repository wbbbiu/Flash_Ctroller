#ifndef _SPI_BASE_H_
#define _SPI_BASE_H_
#include "stm32f4xx.h"
#include "SelfError.h"
#include <stdlib.h>
//当前时间片计数
extern volatile uint32_t g_sys_tick_ms;
extern volatile uint8_t sys_tick_flag;//心跳初始化
//TIM占用的Bitmap,因为有的TIM已经占用就不能再使用了,第n位=1代表第n位启用
extern volatile uint32_t G_TIM_Usage_Bitmap;
/*
TIM_Ret 定时器初始化之后的返回值用于告诉你这个定时器的编号和这个定时器在那个总线上
*/
typedef struct TIM_Ret{
    uint8_t id:6;//定时器编号比如TIM1就是1
    uint8_t aph:2;//挂载在那个总线上1代表APB1,42MHZ,转为定时器84,2代表APB2,84MHZ,转为定时器168MHZ
}TIM_Ret;
/*
用来存储GPIO信息
*/
typedef struct Gpio_Base
{
    GPIO_TypeDef *port;//gpio端口
    uint16_t pin;//gpio的pin
}Gpio_Base;
//初始化GPIO时钟
void GPIO_Clock_Init(GPIO_TypeDef *gpio);
//初始化定时器时钟 返回定时器信息
TIM_Ret TIM_Clock_Init(TIM_TypeDef *tim);
//心跳初始化
void Sys_Tick_Init(void);
//当前心跳计数值
static inline  uint32_t get_sys_tick(){
    return g_sys_tick_ms;
}
/*
配置中断使能
channel ：中断通道
sub ：响应优先级
pre:抢占优先级
*/
void NVIC_Configuration(uint8_t channel,uint8_t sub,uint8_t pre);
//配置GPIO输出模式,mode决定默认输出,0低,1高
void GPIO_Out_Init(Gpio_Base *gpio,uint16_t mode);
//配置GPIO复用输出,af复用功能
void GPIO_AFOut_Init(Gpio_Base *gpio,uint8_t af);
void u16_merge_sort(uint16_t *a,int l,int r);
#endif