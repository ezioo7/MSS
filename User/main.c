/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ������
 * @version     V1.0
 * @date        2023-04-23
 * @brief       ϵͳ������
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
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

/* Ӳ��, ϵͳ��ʼ������, ��ʱ����main������, �������ܵ�����һ���ļ����� */
void sys_init(void);
/* Ӳ��, �����ʼ������, ��ʱ����main������, �������ܵ�����һ���ļ����� */
void periph_init(void);
/* ���, �����ʼ������, �ڸú����д�������RTOS���� */
void task_init(void);

/* ������ */
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

    task_init(); /* ����FreeRTOS����, ��ʼ�������; NOTE:ע��������µĴ��붼����ִ��, �����൱��һ��while��ѭ�� */

    /* Unity���Կ��ʹ�÷���, ���⻹������������unity_around.c�� */
    // UNITY_BEGIN(); /* ���Գ�ʼ�� */
    // RUN_TEST(unity_test);
    // UNITY_END(); /* ��������*/
}

/**
 * @brief   ϵͳ��ʼ������, ��ʱ����main������, �������ܵ�����һ���ļ�����
 */
void sys_init(void)
{
    HAL_Init();                                 /* ��ʼ��HAL�� */
    sys_stm32_clock_init(RCC_PLL_MUL9);         /* ����ʱ�ӣ�72MHz */
    delay_init(72);                             /* ��ʼ����ʱ���� */
    gpio_clk_enable();                          /* ʹ��IO����ʱ�� */
    printf_init(115200, CF_PRINTF_IT_PRIORITY); /* ʹ��printf�ض��� */
}

/**
 * @brief   �����ʼ������, ��ʱ����main������, �������ܵ�����һ���ļ�����
 */
void periph_init(void)
{
    led0_config();              /* LED������*/
    control_input_init();       /* �������������� */
    control_output_init();      /* ������������� */
    adc_init();                 /* ADC �ɼ����� */
    highLevel_dataTrans_init(); /* ���ݴ�������, �������ڴ���ʵʱ�Ըߵ������������ */
    gpio_data_input_init();     /* GPIO �ɼ����� */
    pwm_config();
    pwm_enable();
    // delay_ms(5000);
    // pwm_disable();
}

/* @brief   �����ʼ������, �ڸú����д�������RTOS���� */
void task_init(void)
{
    /* һ��ʼ���������ʱ��δ�����������, �������ٽ���Ҳ���� */
    taskENTER_CRITICAL(); /* �����ٽ���, �����ǹ��ж�(�������ȼ�5-15���жϱ�����), �������sysTick�ж�, Ҳ�ͱ������������  */

    /* ����LED0���� */
    /*xTaskCreate((TaskFunction_t)led0_task,
                (const char *)"led0_task",
                (uint16_t)LED0_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)LED0_TASK_PRIO,
                (TaskHandle_t *)&g_led0_task_handle);*/

    /* ����MB���� */
    xTaskCreate((TaskFunction_t)mb_task,
                (const char *)"mb_task",
                (uint16_t)MB_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)MB_TASK_PRIO,
                (TaskHandle_t *)&g_mb_task_handle);

    /* �����ɼ����ݴ�������, �����Ǵ���ʵʱ��Ҫ�󲻸ߵ������¶� */
    xTaskCreate((TaskFunction_t)lowLevel_dataTrans_task,
                (const char *)"lowLevel_dataTrans_task",
                (uint16_t)DATA_TRANS_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)DATA_TRANS_TASK_PRIO,
                (TaskHandle_t *)&g_data_trans_task_handle);

    /* ����ADC���ݴ�ӡ���� */
    /*xTaskCreate((TaskFunction_t)data_print_task,
                (const char *)"data_print_task",
                (uint16_t)DATA_PRINT_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)DATA_PRINT_TASK_PRIO,
                (TaskHandle_t *)&g_data_print_task_handle);*/

    /* ����UpdateState���� */
    xTaskCreate((TaskFunction_t)UpdateState_task,
                (const char *)"UpdateState_task",
                (uint16_t)UpdateState_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)UpdateState_TASK_PRIO,
                (TaskHandle_t *)&g_UpdateState_task_handle);

    taskEXIT_CRITICAL();   /* �˳��ٽ��� */
    vTaskStartScheduler(); /* ��ʼ������� */
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
