/**
 ****************************************************************************************************
 * @file        pwm3TIM.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-01-10
 * @brief       pwm����ģ��,3��ʱ��ʵ�ְ汾
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 * Ԥ��Ƶϵ��ȡ72, ����ʱ��Ƶ��1MHZ, ��ʱʱ�� = 1us; ARR���ֵΪ 65535
 * ��Դȡ50HZ, һ������ 0.02 = 20ms = 20000ʱ��
 * ��λ��60�Ȼ���Ϊʱ�� = 20 / 6 = 3.33ms = 3333ʱ��
 * �ߵ�ƽʱ��ȡ5ms = 5000ʱ��
 * ��ʼ������150�� = 8.33ms = 8333ʱ��
 *
 * FIXME:����������Դ���ڴ��������ܳ���60��, ��-�� < 60�� ; ͨ���Ƚ�arr_Trigger_Delay��arr_Last_Trigger_Delayʵ��
 ****************************************************************************************************
 */
#include "stdlib.h"
#include "pwm3TIM.h"
#include "globalE.h"
#include "./BSP/GPIO/gpio.h"
#include "./BSP/TIM/tim.h"
#include "config.h"

#ifdef CF_PWM_USE_3TIM
#define KP_MAX 0.001           /* ����Kp, ��������ʱ��������Kp���ܳ�����ֵ */
#define KI_MAX 0.000001        /* ����Ki, ��������ʱ��������Ki���ܳ�����ֵ */
#define ARR_PERIOD 20000       /* ����ʱ��, ��λus */
#define ARR_TRIGGER_PULSE 5000 /* �������ʱ��, ��λus */
#define ARR_PHASE_DIFF 3333    /* 60�� �� 50hz ��Ӧ3333us */
#define PSC_PWM 72             /* ��ʱ��Ԥ��Ƶϵ�� */

/* static variables */
/* NOTE:��Ҫ��uint32_t, ���м����Լ���С�Ƚ�ʱ������� */
static int32_t arr_Trigger_Delay = 0;      /* �����Ƕ�Ӧ��ARR */
static int32_t arr_Last_Trigger_Delay = 0; /* ��¼�ϸ����ڲ��õĴ����Ƕ�Ӧ��ARR */
static int32_t arr_Power_Factor = 0;       /* ��ͨ������Ϊ���ع������ئ�, ����Ϊ �� ��Ӧ��ARRֵ */
/* PI�㷨��� */
static double Kp = KP_MAX;
static double Ki = KI_MAX;
static double error = 0;     /* ������� */
static double error_Sum = 0; /* ����ܺ� */
static double error_Max = 0; /* ��������������ľ���ֵ */

/* A��״̬ */
#define STATE_TRIGGER_DELAY 0
#define STATE_CONDUCTING 1
#define STATE_END_WAIT 2
static uint8_t flag_A = STATE_TRIGGER_DELAY; /* 0 = �ӳ�״̬, 1 = ��ͨ״̬, 2 = �ȴ���һ��Դ����״̬ */

/* static functions */
static void param_config(void);
static void new_trigger_delay(void);

void pwm_config(void)
{
    /**
     * 1.���ݲ�������, ����A��TIM��ʼ��ʱ
     * 2.����3��TIM,��ʱʱ��ֱ�ΪA���ʼ��ʱ, 60��, 60��
     * 3.��ʼ��GPIO��
     */
    param_config();                                                                                /* ������ֲ�����ʼֵ */
    tim_update_config(CF_TIM_PWM_PHASE_A, PSC_PWM, arr_Trigger_Delay, CF_PWM_TIM_PRIORITY, FALSE); /* ��ʼARR = A����ʱ */
    tim_update_config(CF_TIM_PWM_PULLUP, PSC_PWM, ARR_PHASE_DIFF, CF_PWM_TIM_PRIORITY, FALSE);     /* �̶�60����� */
    tim_update_config(CF_TIM_PWM_PULLDOWN, PSC_PWM, ARR_PHASE_DIFF, CF_PWM_TIM_PRIORITY, FALSE);   /* �̶�60����� */
    gpio_init(GPIOC, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                              /* ����pwm���IO��, ��������IO���õ͵�ƽ, A�� */
    gpio_init(GPIOD, GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                             /* ����pwm���IO��, ��������IO���õ͵�ƽ, B�� */
    gpio_init(GPIOD, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                             /* ����pwm���IO��, ��������IO���õ͵�ƽ, C�� */
    gpio_init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                              /* ����pwm���IO��, ��������IO���õ͵�ƽ, D�� */
    gpio_init(GPIOD, GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                             /* ����pwm���IO��, ��������IO���õ͵�ƽ, E�� */
    gpio_init(GPIOD, GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                             /* ����pwm���IO��, ��������IO���õ͵�ƽ, F�� */
    gpio_init(GPIOE, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                              /* FIXME: PD15û����, ����PE4���� */
    gpio_init(GPIOB, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                              /* FIXME: debug��, ���һ�����ڿ�ʼ */
}

void pwm_enable(void)
{
    /**
     * 1.����A�������
     */
    tim_enable(&CF_TIM_PWM_PHASE_A_HANDLE);
}
void pwm_disable(void)
{
    __set_PRIMASK(1);                                      /* PRIMASK = 1 ���������п������ж� */
    tim_disable(&CF_TIM_PWM_PHASE_A_HANDLE);               /* ʧ������TIM */
    tim_disable(&CF_TIM_PWM_PULLUP_HANDLE);                /* ʧ������TIM */
    tim_disable(&CF_TIM_PWM_PULLDOWN_HANDLE);              /* ʧ������TIM */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);  /* ����PWM���IO���õ͵�ƽ, PC7-A1�� */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET); /* ����PWM���IO���õ͵�ƽ, PD12-B2�� */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET); /* ����PWM���IO���õ͵�ƽ, PD15-C3�� */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  /* ����PWM���IO���õ͵�ƽ, PC6-D4�� */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET); /* ����PWM���IO���õ͵�ƽ, PD13-E�� */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); /* ����PWM���IO���õ͵�ƽ, PD14-F�� */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);  /* FIXME: debug��, ���ú�ɾ�� */
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);  /* FIXME: PD15û����, ����PE4���� */
    __set_PRIMASK(0);                                      /* ��λPRIMASK */
}

void pwm_phaseA_IT_callback(void)
{
    switch (flag_A)
    {
    /* ״̬Ϊ0ʱ�����ж�, ˵�������Ǽ�ʱ����: ���뵼ͨ״̬; ��Ӧ��������ߵ�ƽ; ����ARRΪ�ߵ�ƽʱ��; ����PULLUP��ʱ��; */
    case STATE_TRIGGER_DELAY:
        flag_A = STATE_CONDUCTING;
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
        tim_set_arr(&CF_TIM_PWM_PHASE_A_HANDLE, ARR_TRIGGER_PULSE);
        tim_enable(&CF_TIM_PWM_PULLUP_HANDLE);
        break;
    /* ״̬Ϊ1ʱ�����ж�, ˵����ͨ��ʱ����: ����ȴ�״̬; ��Ӧ��������͵�ƽ; ����ARRʱ��; ����PULLDOWN��ʱ��; */
    case STATE_CONDUCTING:
        flag_A = STATE_END_WAIT;
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
        tim_set_arr(&CF_TIM_PWM_PHASE_A_HANDLE, ARR_PERIOD - arr_Trigger_Delay - ARR_TRIGGER_PULSE); /* �ȴ� ����ʱ�� - ��������ʱ - ����ʱ�� */
        tim_enable(&CF_TIM_PWM_PULLDOWN_HANDLE);
        break;
    /* ״̬Ϊ2ʱ�����ж�, ˵����һ����Դ��������: �������ڿ�ʼ���; �����µĴ�����; ������ʱ״̬; ����ARRʱ��; */
    case STATE_END_WAIT:
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
        new_trigger_delay();
        flag_A = STATE_TRIGGER_DELAY;
        tim_set_arr(&CF_TIM_PWM_PHASE_A_HANDLE, arr_Trigger_Delay);
        break;
    }
}

void pwm_pullup_IT_callback(void)
{
    static uint8_t pullup_i = 2;
    /**
     * 1.���� 2 ~ 6��ĸߵ�ƽ����
     * 2.��һ��static������¼��ǰ�ǵڼ��ν��뱾�ж�, ���Ϊһ��
     * 3.���ݴ�����ͬ���߲�ͬ������
     */
    switch (pullup_i)
    {
    case 2:
        pullup_i = 3;
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
        break;
    case 3:
        pullup_i = 4;
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        /* FIXME:PD15û����, ����PE4���� */
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
        break;
    case 4:
        pullup_i = 5;
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        break;
    case 5:
        pullup_i = 6;
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        break;
    case 6:
        pullup_i = 2;
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
        tim_disable(&CF_TIM_PWM_PULLUP_HANDLE); /* �ر����߶�ʱ��,�ȴ��´λ��� */
        break;
    }
}

void pwm_pulldown_IT_callback(void)
{
    static uint8_t pulldown_i = 2;
    /**
     * 1.�������� 2 ~ 6��ĸߵ�ƽ����
     * 2.��һ��static������¼��ǰ�ǵڼ��ν��뱾�ж�, ���Ϊһ��
     * 3.���ݴ�����ͬ���Ͳ�ͬ������
     */
    switch (pulldown_i)
    {
    case 2:
        pulldown_i = 3;
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
        break;
    case 3:
        pulldown_i = 4;
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        /* FIXME:PD15û����, ����PE4���� */
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
        break;
    case 4:
        pulldown_i = 5;
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        break;
    case 5:
        pulldown_i = 6;
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
        break;
    case 6:
        pulldown_i = 2;
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
        tim_disable(&CF_TIM_PWM_PULLDOWN_HANDLE); /* �ر����Ͷ�ʱ��,�ȴ��´λ��� */
        break;
    }
}

static void param_config(void)
{
    /**
     * 1.A���ʼ������, arr_Trigger_Delay = (CF_START_ANGLE/360) * ARR_PERIOD; NOTE:ע��һ��Ҫд360.0, 150/360����������������ȡ��Ϊ0; Ҳ�����ȳ�ARR_PERIOD;
     * 2.��PI�㷨ϵ����ֵ
     * 3.���㵼ͨ������ �� ��Ӧ��arrֵ
     * 4.����error_Max
     */
    /* ���� A���ʼ������ */
    arr_Trigger_Delay = (CF_START_ANGLE * ARR_PERIOD) / 360.0; /* ����CF_START_ANGLE = 150ʱ, arr_Trigger_Delay = 8333;  */
    /* ��PI�㷨ϵ����ֵ */
    if (CI_START_TIME == 20)
    {
        Kp = KP_MAX;
        Ki = KI_MAX;
    }
    /* FIXME:���㵼ͨ������ �� ��Ӧ��arrֵ, ����PowerFactor��δ��ֵ, ���ù̶�ֵ15����� */
    // arr_Power_Factor = (__MSS_GET_CONFIG(Base_Config, Power_Factor) / 360.0) * ARR_PERIOD;
    arr_Power_Factor = (60 * ARR_PERIOD) / 360.0;
    /* ȷ����ʼ������ > ��ͨ������ */
    if (arr_Trigger_Delay <= arr_Power_Factor)
    {
        /* TODO:��ʾ���󲢽�ֹ���� */
    }
    /* ����error_Max */
    error_Max = CF_ERROR_MAX_SCALE * abs(arr_Power_Factor - arr_Trigger_Delay);
}

static void new_trigger_delay(void)
{
    static double arr_Temp_Trigger_Delay = 0; /* �����µ�arr_Trigger_Delay�ĳ��������� */

    /* NOTE:���޲��ɹ���, ������һ��TIM�жϻ�û������, ��һ���ж�������, ʵ�� arr_Trigger_Delay = 3(arr=2)ʱû������; */
    /* NOTE:����ʵ����Ҳ��������ô�͵Ĺ������� */
    error = arr_Power_Factor - arr_Trigger_Delay;
    if (CI_START_MODE == 0) /* ��ѹ��������ģʽ */
    {
        error_Sum += error;
        arr_Temp_Trigger_Delay = arr_Trigger_Delay + error * Kp + error_Sum * Ki;
        arr_Last_Trigger_Delay = arr_Trigger_Delay;                                                                 /* ��arr_Trigger_Delay�޸�֮ǰ�����¼���� */
        arr_Trigger_Delay = arr_Temp_Trigger_Delay >= arr_Power_Factor ? arr_Temp_Trigger_Delay : arr_Power_Factor; /* ��arr_Trigger_Delay <= arr_Power_Factorʱ, ���������� */
    }
    else if (CI_START_MODE == 1) /* ����ǵ�����������, ��ȡ��ʩ���Ƶ��� */
    {
        /* ��Ҫ����error�Ĵ�С, �����Ƶ�ѹ�������ٶ�, ����Ԥ����������; error_Maxʼ��Ϊ��ֵ */
        if (error > 0)
        {
            error = error > error_Max ? error_Max : error;
        }
        else
        {
            error = 0 - error > error_Max ? 0 - error_Max : error;
        }
        /* �������û�����û����������������, ������������ */
        if (AM_EffectiveU_I <= CI_MAX_START_I)
        {
            error_Sum += error;
            arr_Temp_Trigger_Delay = arr_Trigger_Delay + error * Kp + error_Sum * Ki;
            arr_Last_Trigger_Delay = arr_Trigger_Delay;                                                                 /* ��arr_Trigger_Delay�޸�֮ǰ�����¼���� */
            arr_Trigger_Delay = arr_Temp_Trigger_Delay >= arr_Power_Factor ? arr_Temp_Trigger_Delay : arr_Power_Factor; /* ��arr_Trigger_Delay <= arr_Power_Factorʱ, ���������� */
        }
        else if (AM_EffectiveU_I > 1.1 * CI_MAX_START_I) /* TODO:�����ϵ��1.1�Ƿ�Ҫ��ȡΪ�����õĺ�? */
        {
            arr_Trigger_Delay = arr_Last_Trigger_Delay; /* �������������, �ع�arr_Trigger_Delay, ͬʱ����error������error_Sum, �ԷŻ������������� */
        }
        else
        {
            /* AM_EffectiveU_I > CI_MAX_START_I && AM_EffectiveU_I <= 1.1CI_MAX_START_I */
            /* ���������������֮��, ˵������ֻ���Դ�, arr_Trigger_Delay��arr_Last_Trigger_Delayά�ֲ���, error������error_Sum���� */
        }
    }
}

#endif
