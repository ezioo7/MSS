/**
 ****************************************************************************************************
 * @file        taskConfig.h
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-12-11
 * @brief       用于配置FreeRTOS任务
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
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
 * START_TASK任务配置
 * 包括: 任务句柄 任务优先级 堆栈大小 任务函数
 */
#define START_TASK_PRIO 1                   /* 任务优先级,可选0-31,越大越优先 */
#define START_STK_SIZE 128                  /* 任务堆栈大小,实际占用该值 乘4 个字节 */
extern void start_task(void *pvParameters); /* 任务函数 */
extern TaskHandle_t g_start_task_handle;    /* 任务句柄 */

/**
 * LED0任务配置
 */
#define LED0_TASK_PRIO 1                   /* 任务优先级 */
#define LED0_STK_SIZE 128                  /* 任务堆栈大小 */
extern void led0_task(void *pvParameters); /* 任务函数*/
extern TaskHandle_t g_led0_task_handle;    /* 任务句柄 */

/**
 * FreeModbus监听任务配置
 */
#define MB_TASK_PRIO 1                   /* 任务优先级 */
#define MB_STK_SIZE 128                  /* 任务堆栈大小 */
extern void mb_task(void *pvParameters); /* 任务函数*/
extern TaskHandle_t g_mb_task_handle;    /* 任务句柄 */

/**
 * 采集数据处理任务
 */
#define DATA_TRANS_TASK_PRIO 1                   /* 任务优先级 */
#define DATA_TRANS_STK_SIZE 128                  /* 任务堆栈大小 */
extern void data_trans_task(void *pvParameters); /* ADC原始数据处理函数 */
extern TaskHandle_t g_adct_task_handle;          /* 任务句柄 */

/**
 * 数据采集打印任务
 */
#define DATA_PRINT_TASK_PRIO 1                   /* 任务优先级 */
#define DATA_PRINT_STK_SIZE 128                  /* 任务堆栈大小 */
extern void data_print_task(void *pvParameters); /* ADC原始数据处理函数 */
extern TaskHandle_t g_data_print_task_handle;    /* 任务句柄 */

/**
 * PWM发波任务
 */
#define PWM_TASK_PRIO 1                   /* 任务优先级 */
#define PWM_STK_SIZE 128                  /* 任务堆栈大小 */
extern void pwm_task(void *pvParameters); /* 任务函数*/
extern TaskHandle_t g_pwm_task_handle;    /* 任务句柄 */

/**
 * 更新状态发波任务
 */
#define UpdateState_TASK_PRIO 1                   /* 任务优先级 */
#define UpdateState_STK_SIZE 128                  /* 任务堆栈大小 */
extern void UpdateState_task(void *pvParameters); /* 任务函数*/
extern TaskHandle_t g_UpdateState_task_handle;    /* 任务句柄 */

#endif
