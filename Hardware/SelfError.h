#ifndef SELF_ERROR_H
#define SELF_ERROR_H
#include "stm32f4xx.h"
typedef enum{
    MOD_ADC=1,
    MOD_PWM,
    MOD_ENV,
    MOD_FLASH,
}Module_ID_t;
typedef enum{
    TIM_OCCUPY,//定时器被占用
    TIM_ERR,//TIM错误不支持TIM
    FREQUENCY_LOW,//频率过高
    RESOLUTION_LARGE//分辨率过高
 }PWM_Errno_t;
 typedef enum{
    WDATA_LEN_LARGE,
    SECTOR_INDEX_LARGE,
    FLASH_CONFUSION,
    FLASH_HEADER_ERR,
 }Flash_Errno_t;
 typedef enum{
   ADC_TIME_OUT//ADC获取超时
 }ADC_Errno_t;
 typedef enum{
    DIV_ZERO//ADC获取超时
  }ENV_Errno_t;
 extern const char *Err_Str[][10];
typedef union {
    uint32_t all;
    struct{
        uint16_t  errno;//错误编号
        uint8_t module;//错误模块
        uint8_t  reserve;//保留位
    }bits;
}System_Error_t;
#endif