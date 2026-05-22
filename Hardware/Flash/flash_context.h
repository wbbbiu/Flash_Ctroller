#ifndef FLASH_CONTEXT_H
#define FLASH_CONTEXT_H
#include "stm32f4xx.h"
#define CONFIG_START_ADDR 0
#define CONFIG_MAX_ADDR 0x3FFF
#define IDLE_START_ADDR 0X4000
#define IDLE_END_ADDR  0X4255
#define MAGIC_MASK 0x5A5A
#define LOG_START_ADDR 0x4256
#define LOG_MAX_ADDR 0x7FFF
#define LOG_MAX_COUNT 50
typedef enum {
    STORAGE_ID_SYS_CONFIG = 0, // 系统配置
    STORAGE_ID_USER_PREF,      // 用户偏好习惯
    STORAGE_ID_ALARM_DATA,     // 闹钟阵列
    STORAGE_ID_MAX             // 用来统计有多少个ID
  } Storage_Data_ID_e;
typedef struct Flash_Header_t{
    uint16_t magic;
    uint16_t id;
    uint16_t len;
}Flash_Header_t;
#endif 