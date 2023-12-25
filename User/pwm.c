/**
 ****************************************************************************************************
 * @file        pwm.c
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-12-14
 * @brief       用于PWM发波
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 * 选取某个定时器作为时基定时器, 每 40us 计数一次
 * 假设电源频率为50Hz, 一个周期 20ms, 500 个计数
 * 相位差60度换算为时间 = 20 / 6 = 3.33ms, 约83个计数
 * 取高电平时间 4 ms, 100 个计数
 * a相延时计数 = delay_angle/360 * 20 ms / 40 us = delay_angle*500/360
 *
 ****************************************************************************************************
 * @attention
 * 发波顺序为 T1 -> T6 : PC7 PD12 PD15 PC6 PD13 PD14
 * 其中PD15正点原子没引出来, 用PE4代替
 *
 ****************************************************************************************************
 * @attention
 * PE13 14 15分别为风机PWM W V U相
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

static double delay_count = 0.0;    /* 导通角对应的计数个数 */
static uint16_t firing_count = 83;  /* 后期根据用户输入的电源频率计算, 高电平维持的时间 */
static uint16_t period_count = 500; /* 后期根据用户输入的电源频率计算, 周期时间 */
static uint16_t phase_diff = 83;    /* 后期根据用户输入的电源频率计算, 相邻两相相位差 */

static uint8_t FLAG_T1 = 0; /* 0 = 正在初相delay, 1 = 正在输出高电平, 2 = 正在等待下一周期 */
static uint8_t FLAG_T2 = 0; /* 0 = 初相delay, 1 = 输出高电平, 2 = 等待下一周期 */
static uint8_t FLAG_T3 = 0; /* 0 = 初相delay, 1 = 输出高电平, 2 = 等待下一周期 */
static uint8_t FLAG_T4 = 0; /* 0 = 初相delay, 1 = 输出高电平, 2 = 等待下一周期 */
static uint8_t FLAG_T5 = 0; /* 0 = 初相delay, 1 = 输出高电平, 2 = 等待下一周期 */
static uint8_t FLAG_T6 = 0; /* 0 = 初相delay, 1 = 输出高电平, 2 = 等待下一周期 */

static uint16_t COUNT_T1 = 0; /* 用于U相上桥T1计数, FLAG = 0时与delay_count比较, = 1 时与firing_count比较 */
static uint16_t COUNT_T2 = 0; /* 用于W相下桥T2计数, FLAG = 0时与delay_count比较, = 1 时与firing_count比较 */
static uint16_t COUNT_T3 = 0; /* 用于V相上桥T3计数, FLAG = 0时与delay_count比较, = 1 时与firing_count比较 */
static uint16_t COUNT_T4 = 0; /* 用于U相下桥T4计数, FLAG = 0时与delay_count比较, = 1 时与firing_count比较 */
static uint16_t COUNT_T5 = 0; /* 用于W相上桥T5计数, FLAG = 0时与delay_count比较, = 1 时与firing_count比较 */
static uint16_t COUNT_T6 = 0; /* 用于V相下桥T6计数, FLAG = 0时与delay_count比较, = 1 时与firing_count比较 */

static uint16_t dbug_COUNT_P = 0; /* 周期计数, P for period, COUNT_P == period_count时U相上桥T1进入下一周期 */

/**
 * @brief   发波GPIO, TIM, 参数初始化
 * @note    CF_START_ANGLE 启动瞬间的导通角, 一般设为150度
 * @note    count_decline 导通角减小的速率, 单位不是度, 是计数值
 */
void pwm_config(void)
{
    delay_count = ((CF_START_ANGLE / 360.0) * 500.0); /* 首相延时计数, 150°对应为208.333  (delay_angle / 360) * (20 ms / 40 us) */
    /* 配置并将所有IO口置低电平 */
    gpio_config(GPIOC, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOC, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* FIXME: PD15没引出, 暂用PE4代替 */
    gpio_config(GPIOE, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* FIXME: debug用, 标记一个周期开始 */
    gpio_config(GPIOB, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);
    /* 配置时钟 */
    tim_update_enable(CF_TIM_PWM, 71, 39, CF_PWM_TIM_PRIORITY, FALSE);
}

/**
 * @brief   开始PWM发波
 * @note    注意同时使能接收晶闸管导通角反馈的IO口的中断
 */
void pwm_enable(void)
{
    __HAL_TIM_SET_COUNTER(&CF_TIM_PWM_HANDLE, 0);        /* 清0计数器 */
    __HAL_TIM_ENABLE(&CF_TIM_PWM_HANDLE);                /* 开始计数 */
    pwm_feedback_it_enable(CF_PWM_FEEDBACK_IT_PRIORITY); /* 能接收晶闸管导通角反馈的IO口的中断, 用于相序检测 */
}

/**
 * @brief   停止pwm输出
 */
void pwm_disable(void)
{
    __HAL_TIM_DISABLE(&CF_TIM_PWM_HANDLE); /* 失能TIM */
    /* 所有PWM输出IO口置低电平 */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
    /* FIXME: debug用 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
}

/**
 * @brief   定时器中断回调函数
 */
void pwm_update_callback(TIM_HandleTypeDef *tim_handle)
{
    /* 更新计数值, 当到达一个周期时, 调用 delay_angle_handle 调整delay_count */
    /* 计数值清0, 开始下一周期计数 */
    COUNT_T1++;
    COUNT_T2++;
    COUNT_T3++;
    COUNT_T4++;
    COUNT_T5++;
    COUNT_T6++;
    dbug_COUNT_P++;

    if (dbug_COUNT_P >= period_count) /* 如果T1本周期计数完成 */
    {
        /* 更新delay_count */
        calculate_new_delay();
        /* T1进入下一个周期的延时状态 */
        FLAG_T1 = 0;
        COUNT_T1 = 0;
        dbug_COUNT_P = 0;
        /* FIXME:周期开始翻转PB7作为标志 */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
    }

    if (FLAG_T1 == 0 && COUNT_T1 >= delay_count) /* 如果T1在延时状态已经等待了足够的时间 */
    {
        /* 结束延时状态, 对应IO口给高电平 */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
        /* 计数清零, 状态转换为 "正在输出高电平" */
        FLAG_T1 = 1;
        COUNT_T1 = 0;
        /* 通知T2进入延时状态 */
        FLAG_T2 = 0;
        COUNT_T2 = 0;
    }
    else if (FLAG_T1 == 1 && COUNT_T1 >= firing_count) /* 如果T1在高电平状态已经呆了足够的时间 */
    {
        /* 结束高电平输出*/
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
        /* 计数清零, 状态转换, 进入等待状态, 等待下一个周期开始 */
        FLAG_T1 = 2;
        COUNT_T1 = 0;
    }

    if (FLAG_T2 == 0 && COUNT_T2 >= phase_diff) /* 如果T2在延时状态等待了60° */
    {
        /* 结束延时状态, 对应IO口给高电平 */
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
        /* 计数清零, 状态转换, 进入高电平状态 */
        FLAG_T2 = 1;
        COUNT_T2 = 0;
        /* 通知T3进入delay状态 */
        FLAG_T3 = 0;
        COUNT_T3 = 0;
    }
    else if (FLAG_T2 == 1 && COUNT_T2 >= firing_count)
    {
        /* 结束高电平输出*/
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
        /* 计数清零, 进入等待状态, 等待下一个延时状态 */
        FLAG_T2 = 2;
        COUNT_T2 = 0;
    }

    if (FLAG_T3 == 0 && COUNT_T3 >= phase_diff)
    {
        /* FIXME: PE4暂时代替PD15 */
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
        FLAG_T3 = 1;
        COUNT_T3 = 0;
        FLAG_T4 = 0;
        COUNT_T4 = 0;
    }
    else if (FLAG_T3 == 1 && COUNT_T3 >= firing_count)
    {
        /* FIXME: PE4暂时代替PD15 */
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
