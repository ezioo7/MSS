/**
 ****************************************************************************************************
 * @file        pwm3TIM.c
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-01-10
 * @brief       pwm发波模块,3定时器实现版本
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 * 预分频系数取72, 最终时钟频率1MHZ, 计时时基 = 1us; ARR最大值为 65535
 * 电源取50HZ, 一个周期 0.02 = 20ms = 20000时基
 * 相位差60度换算为时间 = 20 / 6 = 3.33ms = 3333时基
 * 高电平时间取5ms = 5000时基
 * 初始触发角150° = 8.33ms = 8333时基
 *
 * FIXME:相邻两个电源周期触发角相差不能超过60°, 旧-新 < 60° ; 通过比较arr_Trigger_Delay与arr_Last_Trigger_Delay实现
 ****************************************************************************************************
 */
#include "stdlib.h"
#include "pwm3TIM.h"
#include "globalE.h"
#include "./BSP/GPIO/gpio.h"
#include "./BSP/TIM/tim.h"
#include "config.h"

#ifdef CF_PWM_USE_3TIM
#define KP_MAX 0.001           /* 最大的Kp, 根据启动时间计算出的Kp不能超过此值 */
#define KI_MAX 0.000001        /* 最大的Ki, 根据启动时间计算出的Ki不能超过此值 */
#define ARR_PERIOD 20000       /* 周期时间, 单位us */
#define ARR_TRIGGER_PULSE 5000 /* 脉冲持续时间, 单位us */
#define ARR_PHASE_DIFF 3333    /* 60° 和 50hz 对应3333us */
#define PSC_PWM 72             /* 定时器预分频系数 */

/* static variables */
/* NOTE:不要用uint32_t, 进行减法以及大小比较时会出问题 */
static int32_t arr_Trigger_Delay = 0;      /* 触发角对应的ARR */
static int32_t arr_Last_Trigger_Delay = 0; /* 记录上个周期采用的触发角对应的ARR */
static int32_t arr_Power_Factor = 0;       /* 导通角下限为负载功率因素φ, 这里为 φ 对应的ARR值 */
/* PI算法相关 */
static double Kp = KP_MAX;
static double Ki = KI_MAX;
static double error = 0;     /* 单次误差 */
static double error_Sum = 0; /* 误差总和 */
static double error_Max = 0; /* 单次允许最大误差的绝对值 */

/* A相状态 */
#define STATE_TRIGGER_DELAY 0
#define STATE_CONDUCTING 1
#define STATE_END_WAIT 2
static uint8_t flag_A = STATE_TRIGGER_DELAY; /* 0 = 延迟状态, 1 = 导通状态, 2 = 等待下一电源周期状态 */

/* static functions */
static void param_config(void);
static void new_trigger_delay(void);

void pwm_config(void)
{
    /**
     * 1.根据参数配置, 计算A相TIM初始计时
     * 2.配置3个TIM,定时时间分别为A相初始计时, 60°, 60°
     * 3.初始化GPIO口
     */
    param_config();                                                                                /* 计算各种参数初始值 */
    tim_update_config(CF_TIM_PWM_PHASE_A, PSC_PWM, arr_Trigger_Delay, CF_PWM_TIM_PRIORITY, FALSE); /* 初始ARR = A相延时 */
    tim_update_config(CF_TIM_PWM_PULLUP, PSC_PWM, ARR_PHASE_DIFF, CF_PWM_TIM_PRIORITY, FALSE);     /* 固定60°溢出 */
    tim_update_config(CF_TIM_PWM_PULLDOWN, PSC_PWM, ARR_PHASE_DIFF, CF_PWM_TIM_PRIORITY, FALSE);   /* 固定60°溢出 */
    gpio_init(GPIOC, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                              /* 配置pwm输出IO口, 并将所有IO口置低电平, A相 */
    gpio_init(GPIOD, GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                             /* 配置pwm输出IO口, 并将所有IO口置低电平, B相 */
    gpio_init(GPIOD, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                             /* 配置pwm输出IO口, 并将所有IO口置低电平, C相 */
    gpio_init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                              /* 配置pwm输出IO口, 并将所有IO口置低电平, D相 */
    gpio_init(GPIOD, GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                             /* 配置pwm输出IO口, 并将所有IO口置低电平, E相 */
    gpio_init(GPIOD, GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                             /* 配置pwm输出IO口, 并将所有IO口置低电平, F相 */
    gpio_init(GPIOE, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                              /* FIXME: PD15没引出, 暂用PE4代替 */
    gpio_init(GPIOB, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);                              /* FIXME: debug用, 标记一个周期开始 */
}

void pwm_enable(void)
{
    /**
     * 1.唤醒A相计数器
     */
    tim_enable(&CF_TIM_PWM_PHASE_A_HANDLE);
}
void pwm_disable(void)
{
    __set_PRIMASK(1);                                      /* PRIMASK = 1 将屏蔽所有可屏蔽中断 */
    tim_disable(&CF_TIM_PWM_PHASE_A_HANDLE);               /* 失能所有TIM */
    tim_disable(&CF_TIM_PWM_PULLUP_HANDLE);                /* 失能所有TIM */
    tim_disable(&CF_TIM_PWM_PULLDOWN_HANDLE);              /* 失能所有TIM */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);  /* 所有PWM输出IO口置低电平, PC7-A1相 */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET); /* 所有PWM输出IO口置低电平, PD12-B2相 */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET); /* 所有PWM输出IO口置低电平, PD15-C3相 */
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);  /* 所有PWM输出IO口置低电平, PC6-D4相 */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET); /* 所有PWM输出IO口置低电平, PD13-E相 */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET); /* 所有PWM输出IO口置低电平, PD14-F相 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);  /* FIXME: debug用, 不用后删掉 */
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);  /* FIXME: PD15没引出, 暂用PE4代替 */
    __set_PRIMASK(0);                                      /* 复位PRIMASK */
}

void pwm_phaseA_IT_callback(void)
{
    switch (flag_A)
    {
    /* 状态为0时触发中断, 说明触发角计时结束: 进入导通状态; 对应引脚输出高电平; 设置ARR为高电平时间; 唤醒PULLUP定时器; */
    case STATE_TRIGGER_DELAY:
        flag_A = STATE_CONDUCTING;
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
        tim_set_arr(&CF_TIM_PWM_PHASE_A_HANDLE, ARR_TRIGGER_PULSE);
        tim_enable(&CF_TIM_PWM_PULLUP_HANDLE);
        break;
    /* 状态为1时触发中断, 说明导通计时结束: 进入等待状态; 对应引脚输出低电平; 设置ARR时间; 唤醒PULLDOWN定时器; */
    case STATE_CONDUCTING:
        flag_A = STATE_END_WAIT;
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
        tim_set_arr(&CF_TIM_PWM_PHASE_A_HANDLE, ARR_PERIOD - arr_Trigger_Delay - ARR_TRIGGER_PULSE); /* 等待 周期时间 - 触发角延时 - 脉冲时间 */
        tim_enable(&CF_TIM_PWM_PULLDOWN_HANDLE);
        break;
    /* 状态为2时触发中断, 说明下一个电源周期来临: 设置周期开始标记; 计算新的触发角; 进入延时状态; 设置ARR时间; */
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
     * 1.负责 2 ~ 6相的高电平脉冲
     * 2.用一个static变量记录当前是第几次进入本中断, 五次为一组
     * 3.根据次数不同拉高不同的引脚
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
        /* FIXME:PD15没引出, 暂用PE4代替 */
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
        tim_disable(&CF_TIM_PWM_PULLUP_HANDLE); /* 关闭拉高定时器,等待下次唤醒 */
        break;
    }
}

void pwm_pulldown_IT_callback(void)
{
    static uint8_t pulldown_i = 2;
    /**
     * 1.负责拉低 2 ~ 6相的高电平脉冲
     * 2.用一个static变量记录当前是第几次进入本中断, 五次为一组
     * 3.根据次数不同拉低不同的引脚
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
        /* FIXME:PD15没引出, 暂用PE4代替 */
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
        tim_disable(&CF_TIM_PWM_PULLDOWN_HANDLE); /* 关闭拉低定时器,等待下次唤醒 */
        break;
    }
}

static void param_config(void)
{
    /**
     * 1.A相初始触发角, arr_Trigger_Delay = (CF_START_ANGLE/360) * ARR_PERIOD; NOTE:注意一定要写360.0, 150/360否则按整数计算向下取整为0; 也可以先乘ARR_PERIOD;
     * 2.给PI算法系数赋值
     * 3.计算导通角下限 φ 对应的arr值
     * 4.计算error_Max
     */
    /* 计算 A相初始触发角 */
    arr_Trigger_Delay = (CF_START_ANGLE * ARR_PERIOD) / 360.0; /* 例如CF_START_ANGLE = 150时, arr_Trigger_Delay = 8333;  */
    /* 给PI算法系数赋值 */
    if (CI_START_TIME == 20)
    {
        Kp = KP_MAX;
        Ki = KI_MAX;
    }
    /* FIXME:计算导通角下限 φ 对应的arr值, 这里PowerFactor还未赋值, 先用固定值15度替代 */
    // arr_Power_Factor = (__MSS_GET_CONFIG(Base_Config, Power_Factor) / 360.0) * ARR_PERIOD;
    arr_Power_Factor = (60 * ARR_PERIOD) / 360.0;
    /* 确保初始触发角 > 导通角下限 */
    if (arr_Trigger_Delay <= arr_Power_Factor)
    {
        /* TODO:提示错误并禁止启动 */
    }
    /* 计算error_Max */
    error_Max = CF_ERROR_MAX_SCALE * abs(arr_Power_Factor - arr_Trigger_Delay);
}

static void new_trigger_delay(void)
{
    static double arr_Temp_Trigger_Delay = 0; /* 保存新的arr_Trigger_Delay的初步计算结果 */

    /* NOTE:下限不可过低, 否则上一个TIM中断还没处理完, 下一个中断又来了, 实测 arr_Trigger_Delay = 3(arr=2)时没有问题; */
    /* NOTE:不过实际上也不会有这么低的功率因数 */
    error = arr_Power_Factor - arr_Trigger_Delay;
    if (CI_START_MODE == 0) /* 电压控制启动模式 */
    {
        error_Sum += error;
        arr_Temp_Trigger_Delay = arr_Trigger_Delay + error * Kp + error_Sum * Ki;
        arr_Last_Trigger_Delay = arr_Trigger_Delay;                                                                 /* 在arr_Trigger_Delay修改之前将其记录下来 */
        arr_Trigger_Delay = arr_Temp_Trigger_Delay >= arr_Power_Factor ? arr_Temp_Trigger_Delay : arr_Power_Factor; /* 当arr_Trigger_Delay <= arr_Power_Factor时, 软启动结束 */
    }
    else if (CI_START_MODE == 1) /* 如果是电流控制启动, 采取措施控制电流 */
    {
        /* 需要控制error的大小, 来限制电压增长的速度, 进而预防电流过大; error_Max始终为正值 */
        if (error > 0)
        {
            error = error > error_Max ? error_Max : error;
        }
        else
        {
            error = 0 - error > error_Max ? 0 - error_Max : error;
        }
        /* 如果电流没超过用户配置最大启动电流, 按正常流程走 */
        if (AM_EffectiveU_I <= CI_MAX_START_I)
        {
            error_Sum += error;
            arr_Temp_Trigger_Delay = arr_Trigger_Delay + error * Kp + error_Sum * Ki;
            arr_Last_Trigger_Delay = arr_Trigger_Delay;                                                                 /* 在arr_Trigger_Delay修改之前将其记录下来 */
            arr_Trigger_Delay = arr_Temp_Trigger_Delay >= arr_Power_Factor ? arr_Temp_Trigger_Delay : arr_Power_Factor; /* 当arr_Trigger_Delay <= arr_Power_Factor时, 软启动结束 */
        }
        else if (AM_EffectiveU_I > 1.1 * CI_MAX_START_I) /* TODO:这里的系数1.1是否要提取为可配置的宏? */
        {
            arr_Trigger_Delay = arr_Last_Trigger_Delay; /* 如果电流过大了, 回滚arr_Trigger_Delay, 同时本轮error不计入error_Sum, 以放缓后续电流增长 */
        }
        else
        {
            /* AM_EffectiveU_I > CI_MAX_START_I && AM_EffectiveU_I <= 1.1CI_MAX_START_I */
            /* 如果电流介于两者之间, 说明电流只是略大, arr_Trigger_Delay和arr_Last_Trigger_Delay维持不变, error不计入error_Sum即可 */
        }
    }
}

#endif
