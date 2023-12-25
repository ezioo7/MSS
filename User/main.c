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
    TEST_ASSERT_EQUAL_UINT(5, 5);
}

int main(void)
{
    sys_init();
    periph_init();
    task_init();

    /* Unity���Կ��ʹ�÷���, ���⻹������������unity_around.c�� */
    // UNITY_BEGIN();/* ���Գ�ʼ�� */
    // RUN_TEST(unity_test);
    // UNITY_END(); /* ��������*/
}

/**
 * @brief   ϵͳ��ʼ������, ��ʱ����main������, �������ܵ�����һ���ļ�����
 */
void sys_init(void)
{
    HAL_Init();                         /* ��ʼ��HAL�� */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* ����ʱ�ӣ�72MHz */
    delay_init(72);                     /* ��ʼ����ʱ */
    gpio_clk_enable();                  /* ʹ������ʱ��*/
    printf_init(115200, 5);             /* ʹ��printf�ض��� */
}

/**
 * @brief   �����ʼ������, ��ʱ����main������, �������ܵ�����һ���ļ�����
 */
void periph_init(void)
{
    control_input_init();       /* �������������� */
    control_output_init();      /* ������������� */
    adc_init();                 /* ADC �ɼ����� */
    highLevel_dataTrans_init(); /* ���ݴ�������, �������ڴ���ʵʱ�Ըߵ������������ */
    gpio_data_input_init();     /* GPIO �ɼ����� */
    pwm_config();               /* PWM �������� */
    pwm_enable();               /* ʹ�ܷ��� */
    // delay_ms(5000);
    // pwm_disable();
}

/* @brief   �����ʼ������, �ڸú����д�������RTOS���� */
void task_init(void)
{
    /* һ��ʼ���������ʱ��δ�����������, �������ٽ���Ҳ���� */
    taskENTER_CRITICAL(); /* �����ٽ���, �����ǹ��ж�(�������ȼ�5-15���жϱ�����), �������sysTick�ж�, Ҳ�ͱ������������  */

    /* ����LED0���� */
    xTaskCreate((TaskFunction_t)led0_task,
                (const char *)"led0_task",
                (uint16_t)LED0_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)LED0_TASK_PRIO,
                (TaskHandle_t *)&g_led0_task_handle);

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
    xTaskCreate((TaskFunction_t)data_print_task,
                (const char *)"data_print_task",
                (uint16_t)DATA_PRINT_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)DATA_PRINT_TASK_PRIO,
                (TaskHandle_t *)&g_data_print_task_handle);

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
