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
#include "globalE.h"
#include "control.h"
#include "pwm.h"
#include "dataCollect.h"
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
    TEST_ASSERT_EQUAL_UINT(5, 5);
}

int main(void)
{
    sys_init();
    periph_init();
    task_init();

    /* Unity测试框架使用方法, 另外还有两个函数在unity_around.c中 */
    // UNITY_BEGIN();/* 测试初始化 */
    // RUN_TEST(unity_test);
    // UNITY_END(); /* 结束测试*/
}

/**
 * @brief   系统初始化函数, 暂时放在main函数中, 后续可能单独做一个文件来放
 */
void sys_init(void)
{
    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 配置时钟，72MHz */
    delay_init(72);                     /* 初始化延时 */
    gpio_clk_enable();                  /* 使能引脚时钟*/
    printf_init(115200, 5);             /* 使能printf重定向 */
}

/**
 * @brief   外设初始化函数, 暂时放在main函数中, 后续可能单独做一个文件来放
 */
void periph_init(void)
{
    control_input_init();       /* 控制信输入配置 */
    control_output_init();      /* 控制信输出配置 */
    adc_init();                 /* ADC 采集配置 */
    highLevel_dataTrans_init(); /* 数据处理配置, 这里用于处理实时性高的数据例如电流 */
    gpio_data_input_init();     /* GPIO 采集配置 */
    pwm_config();               /* PWM 发波配置 */
    pwm_enable();               /* 使能发波 */
    // delay_ms(5000);
    // pwm_disable();
}

/* @brief   任务初始化函数, 在该函数中创建所有RTOS任务 */
void task_init(void)
{
    /* 一开始创建任务的时候还未开启任务调度, 不进入临界区也可以 */
    taskENTER_CRITICAL(); /* 进入临界区, 本质是关中断(所有优先级5-15的中断被屏蔽), 避免进入sysTick中断, 也就避免了任务调度  */

    /* 创建LED0任务 */
    xTaskCreate((TaskFunction_t)led0_task,
                (const char *)"led0_task",
                (uint16_t)LED0_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)LED0_TASK_PRIO,
                (TaskHandle_t *)&g_led0_task_handle);

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
    xTaskCreate((TaskFunction_t)data_print_task,
                (const char *)"data_print_task",
                (uint16_t)DATA_PRINT_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)DATA_PRINT_TASK_PRIO,
                (TaskHandle_t *)&g_data_print_task_handle);

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
