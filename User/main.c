/**
 ****************************************************************************************************
 * @file        main.c
 * @author      电气组
 * @version     V1.0
 * @date        2023-04-23
 * @brief       系统主程序
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 *
 *
 *
 ****************************************************************************************************
 */
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/TIM/tim.h"
#include "./BSP/GPIO/gpio.h"
#include "./BSP/ADC/adc.h"
#include "./BSP/UART/uart.h"
#include "./BSP/FLASH/W25Q64/w25q64.h"
#include "./BSP/FLASH/INTERNAL/flash.h"
#include "./BSP/SPI/spi.h"
#include "globalE.h"
#include "control.h"
#include "pwmSoft.h"
#include "pwm3TIM.h"
#include "dataCollect.h"
#include "config.h"
#include "FreeRTOS.h"
#include "taskConfig.h"
#include "task.h"
#include "mb.h"
#include "unity.h"
#include "UpdateState.h"

/* 硬件, 系统初始化函数, 暂时放在main函数中, 后续可能单独做一个文件来放 */
void sys_init(void);
/* 硬件, 外设初始化函数, 暂时放在main函数中, 后续可能单独做一个文件来放 */
void periph_init(void);
/* 软件, 任务初始化函数, 在该函数中创建所有RTOS任务 */
void task_init(void);

/* 任务句柄 */
TaskHandle_t g_start_task_handle;
TaskHandle_t g_led0_task_handle;
TaskHandle_t g_mb_task_handle;
TaskHandle_t g_data_trans_task_handle;
TaskHandle_t g_data_print_task_handle;
TaskHandle_t g_UpdateState_task_handle;

void unity_test(void)
{
    TEST_ASSERT_EQUAL_UINT(6, 6);
    TEST_ASSERT_EQUAL_UINT(5, 5);
    TEST_ASSERT_EQUAL_UINT(4, 4);
}

void w25q64_test_init()
{
    w25q64_init();
    __MSS_SET_CONFIG(Base_Config, Power_Voltage, 3.33);
    __MSS_SET_CONFIG(Base_Config, Power_Frequency, 50);
    __MSS_SET_CONFIG(Soft_Start_Config, Start_Period, 20);
    __MSS_SET_CONFIG(Soft_Start_Config, Start_Mode, 2);
    store_config_w25q64();
}

void w25q64_test()
{
    w25q64_init();
    load_config_w25q64();
    float power_Voltage = __MSS_GET_CONFIG(Base_Config, Power_Voltage);
    printf("%f\r\n", power_Voltage);
    uint8_t start_Mode = __MSS_GET_CONFIG(Soft_Start_Config, Start_Mode);
    printf("%d\r\n", start_Mode);
    __MSS_SET_CONFIG(Base_Config, Power_Voltage, power_Voltage + 1);
    __MSS_SET_CONFIG(Soft_Start_Config, Start_Mode, start_Mode + 2);
    store_config_w25q64();
}

void flash_init()
{
    __MSS_SET_CONFIG(Base_Config, Power_Voltage, 3.33);
    __MSS_SET_CONFIG(Base_Config, Power_Frequency, 50);
    __MSS_SET_CONFIG(Soft_Start_Config, Start_Period, 20);
    __MSS_SET_CONFIG(Soft_Start_Config, Start_Mode, 2);
    store_config_flash();
}

void flash_test()
{
    load_config_flash();
    float power_Voltage = __MSS_GET_CONFIG(Base_Config, Power_Voltage);
    printf("%f\r\n", power_Voltage);
    uint8_t start_Mode = __MSS_GET_CONFIG(Soft_Start_Config, Start_Mode);
    printf("%d\r\n", start_Mode);
    __MSS_SET_CONFIG(Base_Config, Power_Voltage, power_Voltage + 1);
    __MSS_SET_CONFIG(Soft_Start_Config, Start_Mode, start_Mode + 2);
    store_config_flash();
}

int main(void)
{
    sys_init();
    // periph_init();
    //w25q64_test_init();
    w25q64_test();
    // flash_init();
    //  flash_test();

    task_init(); /* 创建FreeRTOS任务, 开始任务调度; NOTE:注意从这往下的代码都不会执行, 这里相当于一个while死循环 */

    /* Unity测试框架使用方法, 另外还有两个函数在unity_around.c中 */
    // UNITY_BEGIN(); /* 测试初始化 */
    // RUN_TEST(unity_test);
    // UNITY_END(); /* 结束测试*/
}

/**
 * @brief   系统初始化函数, 暂时放在main函数中, 后续可能单独做一个文件来放
 */
void sys_init(void)
{
    HAL_Init();                                 /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9);         /* 配置时钟，72MHz */
    delay_init(72);                             /* 初始化延时函数 */
    gpio_clk_enable();                          /* 使能IO引脚时钟 */
    printf_init(115200, CF_PRINTF_IT_PRIORITY); /* 使能printf重定向 */
}

/**
 * @brief   外设初始化函数, 暂时放在main函数中, 后续可能单独做一个文件来放
 */
void periph_init(void)
{
    led0_config();              /* LED灯配置*/
    control_input_init();       /* 控制信输入配置 */
    control_output_init();      /* 控制信输出配置 */
    adc_init();                 /* ADC 采集配置 */
    highLevel_dataTrans_init(); /* 数据处理配置, 这里用于处理实时性高的数据例如电流 */
    gpio_data_input_init();     /* GPIO 采集配置 */
    pwm_config();
    pwm_enable();
    // delay_ms(5000);
    // pwm_disable();
}

/* @brief   任务初始化函数, 在该函数中创建所有RTOS任务 */
void task_init(void)
{
    /* 一开始创建任务的时候还未开启任务调度, 不进入临界区也可以 */
    taskENTER_CRITICAL(); /* 进入临界区, 本质是关中断(所有优先级5-15的中断被屏蔽), 避免进入sysTick中断, 也就避免了任务调度  */

    /* 创建LED0任务 */
    /*xTaskCreate((TaskFunction_t)led0_task,
                (const char *)"led0_task",
                (uint16_t)LED0_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)LED0_TASK_PRIO,
                (TaskHandle_t *)&g_led0_task_handle);*/

    /* 创建MB任务 */
    xTaskCreate((TaskFunction_t)mb_task,
                (const char *)"mb_task",
                (uint16_t)MB_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)MB_TASK_PRIO,
                (TaskHandle_t *)&g_mb_task_handle);

    /* 创建采集数据处理任务, 这里是处理实时性要求不高的例如温度 */
    xTaskCreate((TaskFunction_t)lowLevel_dataTrans_task,
                (const char *)"lowLevel_dataTrans_task",
                (uint16_t)DATA_TRANS_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)DATA_TRANS_TASK_PRIO,
                (TaskHandle_t *)&g_data_trans_task_handle);

    /* 创建ADC数据打印任务 */
    /*xTaskCreate((TaskFunction_t)data_print_task,
                (const char *)"data_print_task",
                (uint16_t)DATA_PRINT_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)DATA_PRINT_TASK_PRIO,
                (TaskHandle_t *)&g_data_print_task_handle);*/

    /* 创建UpdateState任务 */
    xTaskCreate((TaskFunction_t)UpdateState_task,
                (const char *)"UpdateState_task",
                (uint16_t)UpdateState_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)UpdateState_TASK_PRIO,
                (TaskHandle_t *)&g_UpdateState_task_handle);

    taskEXIT_CRITICAL();   /* 退出临界区 */
    vTaskStartScheduler(); /* 开始任务调度 */
}

void print_arr8(uint8_t *arr, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        printf("%02X, ", arr[i]);
    }
    printf("\r\n");
}

void print_arr16(uint16_t *arr, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        printf("%04X, ", arr[i]);
    }
    printf("\r\n");
}

void print_arr32(uint32_t *arr, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        printf("%08X, ", arr[i]);
    }
    printf("\r\n");
}
