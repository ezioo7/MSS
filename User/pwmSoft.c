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
 * 1.�涨 40us Ϊһ������, ѡȡĳ����ʱ����Ϊʱ����ʱ��, ÿ 40us ���һ��
 * 2.�����ԴƵ��Ϊ50Hz, һ������ 20ms, 500 ������
 * 3.��λ��60�Ȼ���Ϊʱ�� = 20 / 6 = 3.33ms, Լ 83 ������
 * 4.�ߵ�ƽʱ��ͬ������Ϊ60��, 83������
 * 5.�涨U������Ϊa��, T2~T6��������
 * 6.a��ߵ�ƽ��ʱ���� = (phase_angle / 360) * (20 ms / 40 us) = phase_angle * 500 /360
 * 8.����a�����phase_Count, ���ĵ����ֵ��a����ߵ�ƽ, ͬʱ������һ�� b����, b���ߵ�ƽ֮ǰֻ��Ҫ����60��--phase_Diff����, ����Ҫ��������phase_Count_b
 ****************************************************************************************************
 * @attention
 * ����˳��Ϊ T1 -> T6 : PC7 PD12 PD15 PC6 PD13 PD14
 * ����PD15����ԭ��û������, ��PE4����
 *
 ****************************************************************************************************
 * @attention
 * PE13 14 15�ֱ�Ϊ���PWM W V U��
 *
 ****************************************************************************************************
 */
#include "./SYSTEM/sys/sys.h"
#include "./BSP/GPIO/gpio.h"
#include "./BSP/TIM/tim.h"
#include "globalE.h"
#include "dataCollect.h"
#include "pwmSoft.h"

#ifdef CF_PWM_USE_SOFT
#define TARGET_COUNT 0  /* PI�㷨��ͨ�Ƕ�Ӧ�Ľ��ĵ�Ŀ��ֵ,FIXME:Ӧ�� = �� */
#define MAX_KP 0.001    /* ����Kp, ��������ʱ��������Kp���ܳ�����ֵ */
#define MAX_KI 0.000001 /* ����Ki, ��������ʱ��������Ki���ܳ�����ֵ */

/* static functions*/
static void calculate_new_phase(void);
static void param_config(void);

/* static variables */
static double Kp = MAX_KP;
static double Ki = MAX_KI;
static double error = 0;
static double error_Sum = 0;
static double temp = 0;
static double max_Error = 0;

static double phase_Count = 0.0;      /* ��ͨ�Ƕ�Ӧ�Ľ��ĸ��� */
static double last_phase_Count = 0.0; /* ��¼�ϸ����ڲ��õ�phase_Count */
static uint16_t firing_Count = 83;    /* ���ڸ����û�����ĵ�ԴƵ�ʼ���, �ߵ�ƽά�ֵ�ʱ�� */
static uint16_t period_Count = 500;   /* ���ڸ����û�����ĵ�ԴƵ�ʼ���, ����ʱ�� */
static uint16_t phase_Diff = 83;      /* ���ڸ����û�����ĵ�ԴƵ�ʼ���, ����������λ�� */

static uint8_t flag_T1 = 0; /* 0 = ���ڿ�ʼ�ȴ�����phase, 1 = ��������ߵ�ƽ, 2 = ���ڵȴ���һ���� */
static uint8_t flag_T2 = 0; /* 0 = ����phase, 1 = ����ߵ�ƽ, 2 = �ȴ���һ���� */
static uint8_t flag_T3 = 0; /* 0 = ����phase, 1 = ����ߵ�ƽ, 2 = �ȴ���һ���� */
static uint8_t flag_T4 = 0; /* 0 = ����phase, 1 = ����ߵ�ƽ, 2 = �ȴ���һ���� */
static uint8_t flag_T5 = 0; /* 0 = ����phase, 1 = ����ߵ�ƽ, 2 = �ȴ���һ���� */
static uint8_t flag_T6 = 0; /* 0 = ����phase, 1 = ����ߵ�ƽ, 2 = �ȴ���һ���� */

static uint16_t count_T1 = 0; /* ����U������T1����, flag = 0ʱ��phase_Count�Ƚ�, = 1 ʱ��firing_Count�Ƚ� */
static uint16_t count_T2 = 0; /* ����W������T2����, flag = 0ʱ��phase_Diff�Ƚ�, = 1 ʱ��firing_Count�Ƚ� */
static uint16_t count_T3 = 0; /* ����V������T3����, flag = 0ʱ��phase_Diff�Ƚ�, = 1 ʱ��firing_Count�Ƚ� */
static uint16_t count_T4 = 0; /* ����U������T4����, flag = 0ʱ��phase_Diff�Ƚ�, = 1 ʱ��firing_Count�Ƚ� */
static uint16_t count_T5 = 0; /* ����W������T5����, flag = 0ʱ��phase_Diff�Ƚ�, = 1 ʱ��firing_Count�Ƚ� */
static uint16_t count_T6 = 0; /* ����V������T6����, flag = 0ʱ��phase_Diff�Ƚ�, = 1 ʱ��firing_Count�Ƚ� */

static uint16_t count_P = 0; /* ���ڽ���, P for period, count_P == period_CountʱU������T1������һ����, ���Դ���ڱ���һ�� */

/**
 * @brief   ����GPIO, TIM, ������ʼ��
 * @note    CF_START_ANGLE ����˲��ĵ�ͨ��, һ����Ϊ150��
 * @note    count_decline ��ͨ�Ǽ�С������, ��λ���Ƕ�, �ǽ���ֵ
 */
void pwm_config(void)
{
    param_config();                                                      /* ������ֲ�����ʼֵ */
    gpio_init(GPIOC, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);  /* ����pwm���IO��, ��������IO���õ͵�ƽ, A�� */
    gpio_init(GPIOD, GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN); /* ����pwm���IO��, ��������IO���õ͵�ƽ, B�� */
    gpio_init(GPIOD, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN); /* ����pwm���IO��, ��������IO���õ͵�ƽ, C�� */
    gpio_init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);  /* ����pwm���IO��, ��������IO���õ͵�ƽ, D�� */
    gpio_init(GPIOD, GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN); /* ����pwm���IO��, ��������IO���õ͵�ƽ, E�� */
    gpio_init(GPIOD, GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN); /* ����pwm���IO��, ��������IO���õ͵�ƽ, F�� */

    gpio_init(GPIOE, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);        /* FIXME: PD15û����, ����PE4���� */
    gpio_init(GPIOB, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);        /* FIXME: debug��, ���һ�����ڿ�ʼ */
    tim_update_config(CF_TIM_PWM, 72, 40, CF_PWM_TIM_PRIORITY, FALSE); /* ���ö�ʱ��, 40us���һ�� */
}

/**
 * @brief   ��ʼPWM����
 * @note    ע��ͬʱʹ�ܽ��վ�բ�ܵ�ͨ�Ƿ�����IO�ڵ��ж�
 */
void pwm_enable(void)
{
    tim_enable(&CF_TIM_PWM_HANDLE);                      /* ��ӦTIM��ʼ���� */
    pwm_feedback_it_enable(CF_PWM_FEEDBACK_IT_PRIORITY); /* ʹ�ܽ��վ�բ�ܵ�ͨ�Ƿ�����IO�ڵ��ж�, ���������� */
}

/**
 * @brief   ֹͣpwm���
 * @note    ��Ҫ��֤�˺���ԭ����, ��ִ��ʱ���ж�
 */
void pwm_disable(void)
{
    __set_PRIMASK(1);                                      /* PRIMASK = 1 ���������п������ж� */
    tim_disable(&CF_TIM_PWM_HANDLE);                       /* ʧ��TIM */
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

/**
 * @brief   ��ʱ���жϻص�����
 */
void pwm_update_IT_callback(TIM_HandleTypeDef *tim_handle)
{
    /* ���½���ֵ */
    count_T1++;
    count_T2++;
    count_T3++;
    count_T4++;
    count_T5++;
    count_T6++;
    count_P++;

    /*�����ļ�������һ������ʱ, ���� calculate_new_phase ����phase_Count */
    if (count_P >= period_Count) /* ���T1�����ڽ������ */
    {
        /* TODO:�Ƿ���Ҫ����һ����־λ��־�����Ƿ����? ����������̻�û����, ����phase_Count */
        if (phase_Count != 0)
        {
            calculate_new_phase();
        }
        /* T1������һ�����ڵ���ʱ״̬ */
        flag_T1 = 0;
        count_T1 = 0;
        count_P = 0;
        /* FIXME: ���ڿ�ʼ��תPB7��Ϊ��־, �������� */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
    }

    if (flag_T1 == 0 && count_T1 >= phase_Count) /* ���T1�ڳ�����ʱ״̬�Ѿ��ȴ����㹻��ʱ�� */
    {
        /* ������ʱ״̬, ��ӦIO�ڸ��ߵ�ƽ */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
        /* ��������, ״̬ת��Ϊ "��������ߵ�ƽ" */
        flag_T1 = 1;
        count_T1 = 0;
        /* ֪ͨT2������ʱ״̬ */
        flag_T2 = 0;
        count_T2 = 0;
    }
    else if (flag_T1 == 1 && count_T1 >= firing_Count) /* ���T1�ڸߵ�ƽ״̬�Ѿ������㹻��ʱ�� */
    {
        /* �����ߵ�ƽ���*/
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
        /* ��������, ״̬ת��, ����ȴ�״̬, �ȴ���һ�����ڿ�ʼ */
        flag_T1 = 2;
        count_T1 = 0;
    }

    if (flag_T2 == 0 && count_T2 >= phase_Diff) /* ���T2����ʱ״̬�ȴ���60�� */
    {
        /* ������ʱ״̬, ��ӦIO�ڸ��ߵ�ƽ */
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
        /* ��������, ״̬ת��, ����ߵ�ƽ״̬ */
        flag_T2 = 1;
        count_T2 = 0;
        /* ֪ͨT3����phase״̬ */
        flag_T3 = 0;
        count_T3 = 0;
    }
    else if (flag_T2 == 1 && count_T2 >= firing_Count)
    {
        /* �����ߵ�ƽ���*/
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
        /* ��������, ����ȴ�״̬, �ȴ���һ����ʱ״̬ */
        flag_T2 = 2;
        count_T2 = 0;
    }

    if (flag_T3 == 0 && count_T3 >= phase_Diff)
    {
        /* FIXME: PE4��ʱ����PD15 */
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
        /* T3����ߵ�ƽ״̬ */
        flag_T3 = 1;
        count_T3 = 0;
        /* T4������ʱ״̬ */
        flag_T4 = 0;
        count_T4 = 0;
    }
    else if (flag_T3 == 1 && count_T3 >= firing_Count)
    {
        /* FIXME: PE4��ʱ����PD15 */
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
        /* T3�����ߵ�ƽ״̬,�ȴ���һ���ڿ�ʼ */
        flag_T3 = 2;
        count_T3 = 0;
    }

    if (flag_T4 == 0 && count_T4 >= phase_Diff)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        /* T4����ߵ�ƽ״̬ */
        flag_T4 = 1;
        count_T4 = 0;
        /* T5������ʱ״̬ */
        flag_T5 = 0;
        count_T5 = 0;
    }
    else if (flag_T4 == 1 && count_T4 >= firing_Count)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        /* T4�����ߵ�ƽ״̬,�ȴ���һ���ڿ�ʼ */
        flag_T4 = 2;
        count_T4 = 0;
    }

    if (flag_T5 == 0 && count_T5 >= phase_Diff) /* T5�����ʱ���� */
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        /* T5����ߵ�ƽ״̬ */
        flag_T5 = 1;
        count_T5 = 0;
        /* T6���������ʱ״̬ */
        flag_T6 = 0;
        count_T6 = 0;
    }
    else if (flag_T5 == 1 && count_T5 >= firing_Count)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
        /* T5�����ߵ�ƽ״̬,�ȴ���һ���ڿ�ʼ */
        flag_T5 = 2;
        count_T5 = 0;
    }

    if (flag_T6 == 0 && count_T6 >= phase_Diff)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
        /* T6 ����ߵ�ƽ״̬ */
        flag_T6 = 1;
        count_T6 = 0;
    }
    else if (flag_T6 == 1 && count_T6 >= firing_Count)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
        /* T6�����ߵ�ƽ״̬,�ȴ���һ���ڿ�ʼ */
        flag_T6 = 2;
        count_T6 = 0;
    }
}

/**
 * @brief   �����µĵ�ͨ��
 * @note    �����µ���ʱӦ�÷ŵ��µĵ�Դ���ڿ�ʼʱ��, Ҳ���ǵ�һ�������ʱ״̬ʱ
 * FIXME:   Ŀǰ���ڴ˵�����������if�жϲ�ͬ����ģʽ, ����ȡ��ͬ���Կ��Ƶ�ͨ��, ���������ǲ���Ҫ�滻�ɲ�ͬ�������ص�����
 * TODO:   ��λΪ40us����, ����PI�㷨�ļ��㶼���Խ���Ϊ��λ, Ŀ�����Ϊ0, ������޸�Ϊ��ͨ�ǽǶȻ��ߵ�ѹ
 */
static void calculate_new_phase(void)
{
    error = TARGET_COUNT - phase_Count;
    if (CI_START_MODE == 0) /* ��ѹ��������ģʽ */
    {
        error_Sum += error;
        temp = phase_Count + error * Kp + error_Sum * Ki;
        last_phase_Count = phase_Count;
        phase_Count = temp >= 1 ? temp : 0; /* phase_Count < 1ʱ, ֱ������, ��Ϊ���������� */
    }
    else if (CI_START_MODE == 1) /* ����ǵ�����������, ��ȡ��ʩ���Ƶ��� */
    {
        error = error > max_Error ? max_Error : error; /* ��Ҫ����error�Ĵ�С, �����Ƶ�ѹ�������ٶ�, ����Ԥ���������� */
        if (AM_EffectiveU_I <= CI_MAX_START_I)         /* �������û��, ������������ */
        {
            error_Sum += error;
            temp = phase_Count + error * Kp + error_Sum * Ki;
            last_phase_Count = phase_Count;
            phase_Count = temp >= 1 ? temp : 0; /* phase_Count < 1ʱ, ֱ������, ��Ϊ���������� */
        }
        else if (AM_EffectiveU_I > 1.1 * CI_MAX_START_I) /* TODO:�����ϵ��1.1�Ƿ�Ҫ��ȡΪ�����õĺ�? */
        {
            phase_Count = last_phase_Count; /* �������������, �ع�phase_Count, ͬʱ����error������error_Sum, �ԷŻ������������� */
        }
        else
        {
            /* AM_EffectiveU_I > CI_MAX_START_I && AM_EffectiveU_I <= 1.1CI_MAX_START_I */
            /* ���������������֮��, ˵������ֻ���Դ�, phase_count��last_phase_Countά�ֲ���, error������error_Sum���� */
        }
    }
}

/**
 * @brief �����û����ü����ʼphase_Count, max_Error, Kp, Ki
 */
static void param_config(void)
{
    phase_Count = ((CF_START_ANGLE / 360.0) * 500.0);              /* ������ʱ����, 150���ӦΪ208.333  (phase_angle / 360) * (20 ms / 40 us) */
    max_Error = CF_ERROR_MAX_SCALE * (phase_Count - TARGET_COUNT); /* �����������ĵ���ERROR */
    /* TODO: ��������ʱ������kp��ki */
    if (CI_START_TIME == 20)
    {
        Kp = MAX_KP;
        Kp = MAX_KI;
    }
}

#endif
