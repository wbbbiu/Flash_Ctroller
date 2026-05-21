#include "flash_function.h"

#include "use.h"

#define SPI_CS_PORT GPIOB

#define SPI_CS_PIN GPIO_Pin_11

#define SPI_OUT_PORT GPIOB

#define SPI_OUT_PIN GPIO_Pin_5

#define SPI_SCK_PORT GPIOB

#define SPI_SCK_PIN GPIO_Pin_3

#define SPI_IN_PORT GPIOB

#define SPI_IN_PIN GPIO_Pin_4

typedef enum {

  STORAGE_ID_SYS_CONFIG = 0, // 系统配置

  STORAGE_ID_USER_PREF,      // 用户偏好习惯

  STORAGE_ID_ALARM_DATA,     // 闹钟阵列

  STORAGE_ID_MAX             // 用来统计有多少个ID

} Storage_Data_ID_e;

typedef struct SyS_Config

{

  char wifi_name[64];

  char wifi_password[64];

  char city[32];

}SyS_Config;

typedef struct USER_PREF

{

  uint32_t color;

  uint32_t count;

}USER_PREF;

typedef struct ALARM{

    char alram[30][15];

    uint16_t count;

}ALARM;
#include <string.h> 

// 💡 铁律：超大结构体不要放在 main 函数里面作为局部变量！
// 加上 static，让它们在编译时就分配在全局 RAM 里。
static SyS_Config  sys_config;
static USER_PREF   user_pref;
static ALARM       alarm_data;
static Spi_Device  spi1_dev; // 替代 malloc，直接用全局静态变量

int main(void) {
    user_init();

    // 1. 初始化底层硬件引脚
    Gpio_Base CS  = {.port = GPIOB, .pin = GPIO_Pin_11};
    Gpio_Base OUT = {.port = GPIOB, .pin = GPIO_Pin_5};
    Gpio_Base IN  = {.port = GPIOB, .pin = GPIO_Pin_4};
    Gpio_Base SCK = {.port = GPIOB, .pin = GPIO_Pin_3};
    
    // 直接传地址，再也不用 malloc
    Spi_Create(&spi1_dev, &CS, &SCK, &IN, &OUT, SPI1);
    Flash_init(&spi1_dev);

    // 2. 赋值阶段 (使用标准内存操作)
    strcpy(sys_config.city, "hangzhou");
    strcpy(sys_config.wifi_name, "chengyu");
    strcpy(sys_config.wifi_password, "123456789");

    user_pref.color = 123;
    user_pref.count = 89;

    memset(&alarm_data, 0, sizeof(ALARM)); // 清空
    strcpy(alarm_data.alram[0], "week1");
    strcpy(alarm_data.alram[1], "week2");
    alarm_data.count = 2;

    // 3. 顺序写入 Flash 
    Flash_Config_Write(&spi1_dev, STORAGE_ID_SYS_CONFIG, (uint8_t*)&sys_config, sizeof(SyS_Config));
    Flash_Config_Write(&spi1_dev, STORAGE_ID_USER_PREF,  (uint8_t*)&user_pref,  sizeof(USER_PREF));
    // 💡 这一次，你的 452 字节闹钟写入绝对不会崩了！
    Flash_Config_Write(&spi1_dev, STORAGE_ID_ALARM_DATA, (uint8_t*)&alarm_data, sizeof(ALARM));

    // 4. 验证读取 (ALARM 阵列)
    static ALARM read_alarm; // 同样使用静态变量承接
    memset(&read_alarm, 0, sizeof(ALARM));
    
    Flash_Config_Read(&spi1_dev, STORAGE_ID_ALARM_DATA, (uint8_t*)&read_alarm, sizeof(ALARM));

    // 5. 打印验证
    // 如果屏幕打出了 2 和 week1，说明大对象跨页写入完美成功！
    print_int(0, 0, read_alarm.count);
    print_str(0, 1, read_alarm.alram[0]);

    while(1) {
        // 大循环
    }
}