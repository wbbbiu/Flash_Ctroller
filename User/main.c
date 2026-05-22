#include "flash_function.h"
#include "flash_context.h"
#include "use.h"
#include <string.h> 

#define SPI_CS_PORT  GPIOB
#define SPI_CS_PIN   GPIO_Pin_11
#define SPI_OUT_PORT GPIOB
#define SPI_OUT_PIN  GPIO_Pin_5
#define SPI_SCK_PORT GPIOB
#define SPI_SCK_PIN  GPIO_Pin_3
#define SPI_IN_PORT  GPIOB
#define SPI_IN_PIN   GPIO_Pin_4

// ==================== 0. 数据结构声明 ====================
typedef struct SyS_Config {
    char wifi_name[64];
    char wifi_password[64];
    char city[32];
} SyS_Config;

typedef struct USER_PREF {
    uint32_t color;
    uint32_t count;
} USER_PREF;

typedef struct ALARM {
    char alram[30][15];
    uint16_t count;
} ALARM;

// 💡 全局静态分配，拯救栈内存
static Spi_Device  spi1_dev; 

// 用于接收读取出来的结果变量
static SyS_Config  read_sys;
static USER_PREF   read_pref;
static ALARM       read_alarm;

int main(void) {
    user_init();

    // ==================== 1. 硬件层物理初始化 ====================
    Gpio_Base CS  = {.port = SPI_CS_PORT,  .pin = SPI_CS_PIN};
    Gpio_Base OUT = {.port = SPI_OUT_PORT, .pin = SPI_OUT_PIN};
    Gpio_Base IN  = {.port = SPI_IN_PORT,  .pin = SPI_IN_PIN};
    Gpio_Base SCK = {.port = SPI_SCK_PORT, .pin = SPI_SCK_PIN};
    
    Spi_Create(&spi1_dev, &CS, &SCK, &IN, &OUT, SPI1);
    Flash_init(&spi1_dev);

    // 💥 [已删除] 创世格式化 Flash_Original 彻底不执行，不破坏原本的历史数据

    // ==================== 2. 挂载文件系统 (开机二分盲搜+倒序寻址) ====================
    uint32_t boot_ret = 0; 
    // 💡 核心考验：在完全不进行手动写入的情况下，单片机能不能在物理硅片里把数据找回来！
    Flash_Map_Read(&spi1_dev, &boot_ret); 

    // 💥 [已删除] 所有的业务数据初值、strcpy、memset、以及 Flash_Write_Data 全部砍掉！

    // ==================== 3. 全量读取验证（检验 O(1) 映射寻址） ====================
    // 强制清空接收区，保证干净
    memset(&read_sys,   0, sizeof(SyS_Config));
    memset(&read_pref,  0, sizeof(USER_PREF));
    memset(&read_alarm, 0, sizeof(ALARM));
    
    // 直接调用你的接口去读
    Flash_Read_Config(&spi1_dev, STORAGE_ID_SYS_CONFIG, (uint8_t*)&read_sys,   sizeof(SyS_Config), CONFIG_FLASH);
    Flash_Read_Config(&spi1_dev, STORAGE_ID_USER_PREF,  (uint8_t*)&read_pref,  sizeof(USER_PREF),  CONFIG_FLASH);
    Flash_Read_Config(&spi1_dev, STORAGE_ID_ALARM_DATA, (uint8_t*)&read_alarm, sizeof(ALARM),      CONFIG_FLASH);

    // ==================== 4. 你的专属字符网格坐标打印验证 ====================
    
    // 4.1 验证小对象 USER_PREF (8字节) -> 应该输出 123 和 89
    print_int(0, 0, read_pref.color);
    print_int(5, 0, read_pref.count);
    
    // 4.2 验证大对象 SyS_Config (160字节) -> 应该输出 "hangzhou"
    print_str(0, 2, read_sys.city);
    
    // 4.3 验证超级大对象 ALARM (452字节) -> 应该输出 2 和 "week1"
    print_int(0, 3, read_alarm.count);
    print_str(5, 3, read_alarm.alram[0]);

    while(1) {
        // 大循环，静静等待屏幕亮起奇迹
    }
}