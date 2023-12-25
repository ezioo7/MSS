/**
 ****************************************************************************************************
 * @file        pwm.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-14
 * @brief       ����PWM����
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 * ѡȡĳ����ʱ����Ϊʱ����ʱ��, ÿ 40us ����һ��
 * �����ԴƵ��Ϊ50Hz, һ������ 20ms, 500 ������
 * ��λ��60�Ȼ���Ϊʱ�� = 20 / 6 = 3.33ms, Լ83������
 * ȡ�ߵ�ƽʱ�� 4 ms, 100 ������
 * a����ʱ���� = delay_angle/360 * 20 ms / 40 us = delay_angle*500/360
 *
 ****************************************************************************************************
 * @attention
 * ����˳��Ϊ T1 -> T6 : PC7 PD12 PD15 PC6 PD13 PD14
 * ����PD15����ԭ��û������, ��PE4����
 *
 ****************************************************************************************************
 * @attention
 * PE13 14 15�ֱ�Ϊ���PWM W V U��
 */
#include "./SYSTEM/sys/sys.h"
#include "./BSP/GPIO/gpio.h"
#include "./BSP/TIM/tim.h"
#include "globalE.h"
#include "dataCollect.h"
#include "pwm.h"

#define TARGET_COUNT 0

/* static functions*/
static void calculate_new_delay(void);

/* static variables */
static double Kp = 0.001;
static double Ki = 0.000001;
static double error = 0;
static double errorSum = 0;
static double temp = 0;

static double delay_count = 0.0;    /* ��ͨ�Ƕ�Ӧ�ļ������� */
static uint16_t firing_count = 83;  /* ���ڸ����û�����ĵ�ԴƵ�ʼ���, �ߵ�ƽά�ֵ�ʱ�� */
static uint16_t period_count = 500; /* ���ڸ����û�����ĵ�ԴƵ�ʼ���, ����ʱ�� */
static uint16_t phase_diff = 83;    /* ���ڸ����û�����ĵ�ԴƵ�ʼ���, ����������λ�� */

static uint8_t FLAG_T1 = 0; /* 0 = ���ڳ���delay, 1 = ��������ߵ�ƽ, 2 = ���ڵȴ���һ���� */
static uint8_t FLAG_T2 = 0; /* 0 = ����delay, 1 = ����ߵ�ƽ, 2 = �ȴ���һ���� */
static uint8_t FLAG_T3 = 0; /* 0 = ����delay, 1 = ����ߵ�ƽ, 2 = �ȴ���һ���� */
static uint8_t FLAG_T4 = 0; /* 0 = ����delay, 1 = ����ߵ�ƽ, 2 = �ȴ���һ���� */
static uint8_t FLAG_T5 = 0; /* 0 = ����delay, 1 = ����ߵ�ƽ, 2 = �ȴ���һ���� */
static uint8_t FLAG_T6 = 0; /* 0 = ����delay, 1 = ����ߵ�ƽ, 2 = �ȴ���һ���� */

static uint16_t COUNT_T1 = 0; /* ����U������T1����, FLAG = 0ʱ��delay_count�Ƚ�, = 1 ʱ��firing_count�Ƚ� */
static uint16_t COUNT_T2 = 0; /* ����W������T2����, FLAG = 0ʱ��delay_count�Ƚ�, = 1 ʱ��firing_count�Ƚ� */
static uint16_t COUNT_T3 = 0; /* ����V������T3����, FLAG = 0ʱ��delay_count�Ƚ�, = 1 ʱ��firing_count�Ƚ� */
static uint16_t COUNT_T4 = 0; /* ����U������T4����, FLAG = 0ʱ��delay_count�Ƚ�, = 1 ʱ��firing_count�Ƚ� */
static uint16_t COUNT_T5 = 0; /* ����W������T5����, FLAG = 0ʱ��delay_count�Ƚ�, = 1 ʱ��firing_count�Ƚ� */
static uint16_t COUNT_T6 = 0; /* ����V������T6����, FLAG = 0ʱ��delay_count�Ƚ�, = 1 ʱ��firing_count�Ƚ� */

static uint16_t dbug_COUNT_P = 0; /* ���ڼ���, P for period, COUNT_P == period_countʱU������T1������һ���� */

/**
 * @brief   ����GPIO, TIM, ������ʼ��
 * @note    CF_START_ANGLE ����˲��ĵ�ͨ��, һ����Ϊ150��
 * @note    count_decline ��ͨ�Ǽ�С������, ��λ���Ƕ�, �Ǽ���ֵ
 */
void pwm_config(void)
{
    delay_count = ((CF_START_ANGLE / 360.0) * 500.0); /* ������ʱ����, 150���ӦΪ208.333  (delay_angle / 360) * (20 ms / 40 us) */
    /* ���ò�������IO���õ͵�ƽ */
    gpio_config(GPIOC, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOC, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* FIXME: PD15û����, ����PE4���� */
    gpio_config(GPIOE, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* FIXME: debug��, ���һ�����ڿ�ʼ */
    gpio_config(GPIOB, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* ����ʱ�� */
    tim_update_enable(CF_TIM_PWM, 71, 39, CF_PWM_TIM_PRIORITY, FALSE);
}

/**
 * @brief   ��ʼPWM����
 * @note    ע��ͬʱʹ�ܽ��վ�բ�ܵ�ͨ�Ƿ�����IO�ڵ��ж�
 */
void pwm_enable(void)
{
    __HAL_TIM_SET_COUNTER(&CF_TIM_PWM_HANDLE, 0);        /* ��0������ */
    __HAL_TIM_ENABLE(&CF_TIM_PWM_HANDLE);                /* ��ʼ���� */
    pwm_feedback_it_enable(CF_PWM_FEEDBACK_IT_PRIORITY); /* �ܽ��վ�բ�ܵ�ͨ�Ƿ�����IO�ڵ��ж�, ���������� */
}

/**
 * @brief   ֹͣpwm���
 */
void pwm_disable(void)
{
    __HAL_TIM_DISABLE(&CF_TIM_PWM_HANDLE); /* ʧ��TIM */
    /* ����PWM���IO���õ͵�ƽ */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
    /* FIXME: debug�� */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
}

/**
 * @brief   ��ʱ���жϻص�����
 */
void pwm_update_callback(TIM_HandleTypeDef *tim_handle)
{
    /* ���¼���ֵ, ������һ������ʱ, ���� delay_angle_handle ����delay_count */
    /* ����ֵ��0, ��ʼ��һ���ڼ��� */
    COUNT_T1++;
    COUNT_T2++;
    COUNT_T3++;
    COUNT_T4++;
    COUNT_T5++;
    COUNT_T6++;
    dbug_COUNT_P++;

    if (dbug_COUNT_P >= period_count) /* ���T1�����ڼ������ */
    {
        /* ����delay_count */
        calculate_new_delay();
        /* T1������һ�����ڵ���ʱ״̬ */
        FLAG_T1 = 0;
        COUNT_T1 = 0;
        dbug_COUNT_P = 0;
        /* FIXME:���ڿ�ʼ��תPB7��Ϊ��־ */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
    }

    if (FLAG_T1 == 0 && COUNT_T1 >= delay_count) /* ���T1����ʱ״̬�Ѿ��ȴ����㹻��ʱ�� */
    {
        /* ������ʱ״̬, ��ӦIO�ڸ��ߵ�ƽ */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
        /* ��������, ״̬ת��Ϊ "��������ߵ�ƽ" */
        FLAG_T1 = 1;
        COUNT_T1 = 0;
        /* ֪ͨT2������ʱ״̬ */
        FLAG_T2 = 0;
        COUNT_T2 = 0;
    }
    else if (FLAG_T1 == 1 && COUNT_T1 >= firing_count) /* ���T1�ڸߵ�ƽ״̬�Ѿ������㹻��ʱ�� */
    {
        /* �����ߵ�ƽ���*/
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
        /* ��������, ״̬ת��, ����ȴ�״̬, �ȴ���һ�����ڿ�ʼ */
        FLAG_T1 = 2;
        COUNT_T1 = 0;
    }

    if (FLAG_T2 == 0 && COUNT_T2 >= phase_diff) /* ���T2����ʱ״̬�ȴ���60�� */
    {
        /* ������ʱ״̬, ��ӦIO�ڸ��ߵ�ƽ */
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
        /* ��������, ״̬ת��, ����ߵ�ƽ״̬ */
        FLAG_T2 = 1;
        COUNT_T2 = 0;
        /* ֪ͨT3����delay״̬ */
        FLAG_T3 = 0;
        COUNT_T3 = 0;
    }
    else if (FLAG_T2 == 1 && COUNT_T2 >= firing_count)
    {
        /* �����ߵ�ƽ���*/
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
        /* ��������, ����ȴ�״̬, �ȴ���һ����ʱ״̬ */
        FLAG_T2 = 2;
        COUNT_T2 = 0;
    }

    if (FLAG_T3 == 0 && COUNT_T3 >= phase_diff)
    {
        /* FIXME: PE4��ʱ����PD15 */
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
        FLAG_T3 = 1;
        COUNT_T3 = 0;
        FLAG_T4 = 0;
        COUNT_T4 = 0;
    }
    else if (FLAG_T3 == 1 && COUNT_T3 >= firing_count)
    {
        /* FIXME: PE4��ʱ����PD15 */
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
        FLAG_T3 = 2;
        COUNT_T3 = 0;
    }

    if (FLAG_T4 == 0 && COUNT_T4 >= phase_diff)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        FLAG_T4 = 1;
        COUNT_T4 = 0;
        FLAG_T5 = 0;
        COUNT_T5 = 0;
    }
    else if (FLAG_T4 == 1 && COUNT_T4 >= firing_count)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        FLAG_T4 = 2;
        COUNT_T4 = 0;
    }

    if (FLAG_T5 == 0 && COUNT_T5 >= phase_diff)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        FLAG_T5 = 1;
        COUNT_T5 = 0;
        FLAG_T6 = 0;
        COUNT_T6 = 0;
    }
    else if (FLAG_T5 == 1 && COUNT_T5 >= firing_count)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
        FLAG_T5 = 2;
        COUNT_T5 = 0;
    }

    if (FLAG_T6 == 0 && COUNT_T6 >= phase_diff)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
        FLAG_T6 = 1;
        COUNT_T6 = 0;
    }
    else if (FLAG_T6 == 1 && COUNT_T6 >= firing_count)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
        FLAG_T6 = 2;
        COUNT_T6 = 0;
    }
}

static void calculate_new_delay(void)
{
    if (delay_count != 0)
    {
        error = TARGET_COUNT - delay_count;
        errorSum += error;
        temp = delay_count + error * Kp + errorSum * Ki;
        delay_count = temp >= 1 ? temp : 0;
    }
}
