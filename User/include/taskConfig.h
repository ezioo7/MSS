/**
 ****************************************************************************************************
 * @file        taskConfig.h
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-11
 * @brief       ��������FreeRTOS����
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_TaskConfig_H
#define __MSS_TaskConfig_H

#include "FreeRTOS.h"
#include "task.h"

/**
 * START_TASK��������
 * ����: ������ �������ȼ� ��ջ��С ������
 */
#define START_TASK_PRIO 1                   /* �������ȼ�,��ѡ0-31,Խ��Խ���� */
#define START_STK_SIZE 128                  /* �����ջ��С,ʵ��ռ�ø�ֵ ��4 ���ֽ� */
extern void start_task(void *pvParameters); /* ������ */
extern TaskHandle_t g_start_task_handle;    /* ������ */

/**
 * LED0��������
 */
#define LED0_TASK_PRIO 1                   /* �������ȼ� */
#define LED0_STK_SIZE 128                  /* �����ջ��С */
extern void led0_task(void *pvParameters); /* ������*/
extern TaskHandle_t g_led0_task_handle;    /* ������ */

/**
 * FreeModbus������������
 */
#define MB_TASK_PRIO 1                   /* �������ȼ� */
#define MB_STK_SIZE 128                  /* �����ջ��С */
extern void mb_task(void *pvParameters); /* ������*/
extern TaskHandle_t g_mb_task_handle;    /* ������ */

/**
 * �ɼ����ݴ�������
 */
#define DATA_TRANS_TASK_PRIO 1                   /* �������ȼ� */
#define DATA_TRANS_STK_SIZE 128                  /* �����ջ��С */
extern void data_trans_task(void *pvParameters); /* ADCԭʼ���ݴ����� */
extern TaskHandle_t g_adct_task_handle;          /* ������ */

/**
 * ���ݲɼ���ӡ����
 */
#define DATA_PRINT_TASK_PRIO 1                   /* �������ȼ� */
#define DATA_PRINT_STK_SIZE 128                  /* �����ջ��С */
extern void data_print_task(void *pvParameters); /* ADCԭʼ���ݴ����� */
extern TaskHandle_t g_data_print_task_handle;    /* ������ */

/**
 * PWM��������
 */
#define PWM_TASK_PRIO 1                   /* �������ȼ� */
#define PWM_STK_SIZE 128                  /* �����ջ��С */
extern void pwm_task(void *pvParameters); /* ������*/
extern TaskHandle_t g_pwm_task_handle;    /* ������ */

/**
 * ����״̬��������
 */
#define UpdateState_TASK_PRIO 1                   /* �������ȼ� */
#define UpdateState_STK_SIZE 128                  /* �����ջ��С */
extern void UpdateState_task(void *pvParameters); /* ������*/
extern TaskHandle_t g_UpdateState_task_handle;    /* ������ */

#endif
