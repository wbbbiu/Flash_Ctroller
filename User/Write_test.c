// #include "flash_function.h"
// #include "flash_context.h"
// #include "use.h"
// #include <string.h> 

// #define SPI_CS_PORT  GPIOB
// #define SPI_CS_PIN   GPIO_Pin_11
// #define SPI_OUT_PORT GPIOB
// #define SPI_OUT_PIN  GPIO_Pin_5
// #define SPI_SCK_PORT GPIOB
// #define SPI_SCK_PIN  GPIO_Pin_3
// #define SPI_IN_PORT  GPIOB
// #define SPI_IN_PIN   GPIO_Pin_4

// // ==================== 0. 数据结构声明 ====================
// typedef struct SyS_Config {
//     char wifi_name[64];
//     char wifi_password[64];
//     char city[32];
// } SyS_Config;

// typedef struct USER_PREF {
//     uint32_t color;
//     uint32_t count;
// } USER_PREF;

// typedef struct ALARM {
//     char alram[30][15];
//     uint16_t count;
// } ALARM;

// // 💡 全局静态分配，拯救 1KB 栈空间，彻底拒绝内存越界
// static SyS_Config  sys_config;
// static USER_PREF   user_pref;
// static ALARM       alarm_data;
// static Spi_Device  spi1_dev; 

// // 用于接收读取出来的结果变量
// static SyS_Config  read_sys;
// static USER_PREF   read_pref;
// static ALARM       read_alarm;

// int main(void) {
//     user_init();

//     // ==================== 1. 硬件层物理初始化 ====================
//     Gpio_Base CS  = {.port = SPI_CS_PORT,  .pin = SPI_CS_PIN};
//     Gpio_Base OUT = {.port = SPI_OUT_PORT, .pin = SPI_OUT_PIN};
//     Gpio_Base IN  = {.port = SPI_IN_PORT,  .pin = SPI_IN_PIN};
//     Gpio_Base SCK = {.port = SPI_SCK_PORT, .pin = SPI_SCK_PIN};
    
//     Spi_Create(&spi1_dev, &CS, &SCK, &IN, &OUT, SPI1);
//     Flash_init(&spi1_dev);

//     // ==================== 2. 创世格式化（排雷核心） ====================
//     // ⚠️ 导师敲黑板：由于修改了向上取整切页逻辑和二分查找，Flash 内部必须彻底格式化一次！
//     // 【第一步测试】：解开下面这行的注释，编译烧录运行，让 Flash 干净成 0xFFFF。
//     // 【第二步测试】：把下面这行【重新注释掉】，再次编译烧录，断电重启，验证掉电不丢失！
//     Flash_Original(&spi1_dev); 

//     // ==================== 3. 挂载文件系统 (开机二分盲搜+倒序寻址) ====================
//     uint32_t boot_ret = 0; 
//     // 跑完之后，flash_map.config_addrs 映射表将在内存中完美重建
//     Flash_Map_Read(&spi1_dev, &boot_ret); 

//     // ==================== 4. 业务数据赋初值 ====================
//     // A. 160 字节的大对象（跨一页多一点）
//     strcpy(sys_config.city, "hangzhou");
//     strcpy(sys_config.wifi_name, "chengyu");
//     strcpy(sys_config.wifi_password, "123456789");

//     // B. 8 字节的小对象（不跨页）
//     user_pref.color = 123;
//     user_pref.count = 89;

//     // C. 452 字节的超级大对象（跨越两页，最考验向上取整逻辑）
//     memset(&alarm_data, 0, sizeof(ALARM)); 
//     strcpy(alarm_data.alram[0], "week1");
//     strcpy(alarm_data.alram[1], "week2");
//     alarm_data.count = 2; // 正好躺在最后一页的队尾

//     // ==================== 5. 环形队列追加顺序写入 ====================
//     // 你的底层函数会自动为它们打上 Magic=0x5A5A 头部，无情地向后追加排队
//     Flash_Write_Data(&spi1_dev, STORAGE_ID_SYS_CONFIG, (uint8_t*)&sys_config, sizeof(SyS_Config), CONFIG_FLASH);
//     Flash_Write_Data(&spi1_dev, STORAGE_ID_USER_PREF,  (uint8_t*)&user_pref,  sizeof(USER_PREF),  CONFIG_FLASH);
//     Flash_Write_Data(&spi1_dev, STORAGE_ID_ALARM_DATA, (uint8_t*)&alarm_data, sizeof(ALARM),      CONFIG_FLASH);

//     // ==================== 6. 全量读取验证（检验 O(1) 映射寻址） ====================
//     // 清空接收区，防止历史残留欺骗眼睛
//     memset(&read_sys,   0, sizeof(SyS_Config));
//     memset(&read_pref,  0, sizeof(USER_PREF));
//     memset(&read_alarm, 0, sizeof(ALARM));
    
//     // 直接通过 flash_map.config_addrs[ID] 精准定位读取
//     Flash_Read_Config(&spi1_dev, STORAGE_ID_SYS_CONFIG, (uint8_t*)&read_sys,   sizeof(SyS_Config), CONFIG_FLASH);
//     Flash_Read_Config(&spi1_dev, STORAGE_ID_USER_PREF,  (uint8_t*)&read_pref,  sizeof(USER_PREF),  CONFIG_FLASH);
//     Flash_Read_Config(&spi1_dev, STORAGE_ID_ALARM_DATA, (uint8_t*)&read_alarm, sizeof(ALARM),      CONFIG_FLASH);

//     // ==================== 7. 屏幕多维度打印验证 ====================
//     // 💡 修复：拉开垂直 Y 坐标轴（以 16 像素为一行的间距），防止字符乱码堆叠
    
//     // 7.1 验证小对象 USER_PREF (8字节) -> 应该输出 123 和 89
//     print_int(0, 0,  read_pref.color);
//     print_int(5, 0, read_pref.count);
    
//     // 7.2 验证大对象 SyS_Config (160字节) -> 应该输出 "hangzhou"
//     print_str(0, 2, read_sys.city);
    
//     // 7.3 验证超级大对象 ALARM (452字节，跨多页边界) -> 应该输出 2 和 "week1"
//     print_int(0, 3,  read_alarm.count);
//     print_str(5, 3, read_alarm.alram[0]);

//     while(1) {
//         // 进入主循环或低功耗状态
//     }
// }













// #include "flash_function.h"
// #include "flash_context.h"
// #include "use.h"
// #include <string.h> 

// #define SPI_CS_PORT  GPIOB
// #define SPI_CS_PIN   GPIO_Pin_11
// #define SPI_OUT_PORT GPIOB
// #define SPI_OUT_PIN  GPIO_Pin_5
// #define SPI_SCK_PORT GPIOB
// #define SPI_SCK_PIN  GPIO_Pin_3
// #define SPI_IN_PORT  GPIOB
// #define SPI_IN_PIN   GPIO_Pin_4

// // ==================== 0. 数据结构声明 ====================
// typedef struct SyS_Config {
//     char wifi_name[64];
//     char wifi_password[64];
//     char city[32];
// } SyS_Config;

// typedef struct USER_PREF {
//     uint32_t color;
//     uint32_t count;
// } USER_PREF;

// typedef struct ALARM {
//     char alram[30][15];
//     uint16_t count;
// } ALARM;

// // 💡 全局静态分配，拯救栈内存
// static Spi_Device  spi1_dev; 

// // 用于接收读取出来的结果变量
// static SyS_Config  read_sys;
// static USER_PREF   read_pref;
// static ALARM       read_alarm;

// int main(void) {
//     user_init();

//     // ==================== 1. 硬件层物理初始化 ====================
//     Gpio_Base CS  = {.port = SPI_CS_PORT,  .pin = SPI_CS_PIN};
//     Gpio_Base OUT = {.port = SPI_OUT_PORT, .pin = SPI_OUT_PIN};
//     Gpio_Base IN  = {.port = SPI_IN_PORT,  .pin = SPI_IN_PIN};
//     Gpio_Base SCK = {.port = SPI_SCK_PORT, .pin = SPI_SCK_PIN};
    
//     Spi_Create(&spi1_dev, &CS, &SCK, &IN, &OUT, SPI1);
//     Flash_init(&spi1_dev);

//     // 💥 [已删除] 创世格式化 Flash_Original 彻底不执行，不破坏原本的历史数据

//     // ==================== 2. 挂载文件系统 (开机二分盲搜+倒序寻址) ====================
//     uint32_t boot_ret = 0; 
//     // 💡 核心考验：在完全不进行手动写入的情况下，单片机能不能在物理硅片里把数据找回来！
//     Flash_Map_Read(&spi1_dev, &boot_ret); 

//     // 💥 [已删除] 所有的业务数据初值、strcpy、memset、以及 Flash_Write_Data 全部砍掉！

//     // ==================== 3. 全量读取验证（检验 O(1) 映射寻址） ====================
//     // 强制清空接收区，保证干净
//     memset(&read_sys,   0, sizeof(SyS_Config));
//     memset(&read_pref,  0, sizeof(USER_PREF));
//     memset(&read_alarm, 0, sizeof(ALARM));
    
//     // 直接调用你的接口去读
//     Flash_Read_Config(&spi1_dev, STORAGE_ID_SYS_CONFIG, (uint8_t*)&read_sys,   sizeof(SyS_Config), CONFIG_FLASH);
//     Flash_Read_Config(&spi1_dev, STORAGE_ID_USER_PREF,  (uint8_t*)&read_pref,  sizeof(USER_PREF),  CONFIG_FLASH);
//     Flash_Read_Config(&spi1_dev, STORAGE_ID_ALARM_DATA, (uint8_t*)&read_alarm, sizeof(ALARM),      CONFIG_FLASH);

//     // ==================== 4. 你的专属字符网格坐标打印验证 ====================
    
//     // 4.1 验证小对象 USER_PREF (8字节) -> 应该输出 123 和 89
//     print_int(0, 0, read_pref.color);
//     print_int(5, 0, read_pref.count);
    
//     // 4.2 验证大对象 SyS_Config (160字节) -> 应该输出 "hangzhou"
//     print_str(0, 2, read_sys.city);
    
//     // 4.3 验证超级大对象 ALARM (452字节) -> 应该输出 2 和 "week1"
//     print_int(0, 3, read_alarm.count);
//     print_str(5, 3, read_alarm.alram[0]);

//     while(1) {
//         // 大循环，静静等待屏幕亮起奇迹
//     }
// }