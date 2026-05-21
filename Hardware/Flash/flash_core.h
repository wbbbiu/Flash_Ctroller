#ifndef FLASH_CORE_H
#define FLASH_CORE_H
typedef enum {
    STORAGE_ID_SYS_CONFIG = 0, // 系统配置
    STORAGE_ID_USER_PREF,      // 用户偏好习惯
    STORAGE_ID_ALARM_DATA,     // 闹钟阵列
    STORAGE_ID_MAX             // 用来统计有多少个ID
} Storage_Data_ID_e;

#endif