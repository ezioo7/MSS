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
 * 1.规定 40us 为一个节拍, 选取某个定时器作为时基定时器, 每 40us 溢出一次
 * 2.假设电源频率为50Hz, 一个周期 20ms, 500 个节拍
 * 3.相位差60度换算为时间 = 20 / 6 = 3.33ms, 约 83 个节拍
 * 4.高电平时间同样设置为60度, 83个节拍
 * 5.规定U相上桥为a相, T2~T6依次往后
 * 6.a相高电平延时节拍 = (phase_angle / 360) * (20 ms / 40 us) = phase_angle * 500 /360
 * 8.计算a相初相phase_Count, 节拍到达此值后a相给高电平, 同时启动下一相 b计数, b给高电平之前只需要计数60°--phase_Diff即可, 不需要单独再算phase_Count_b
 ****************************************************************************************************
 * @attention
 * 发波顺序为 T1 -> T6 : PC7 PD12 PD15 PC6 PD13 PD14
 * 其中PD15正点原子没引出来, 用PE4代替
 *
 ****************************************************************************************************
 * @attention
 * PE13 14 15分别为风机PWM W V U相
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
#define TARGET_COUNT 0  /* PI算法导通角对应的节拍的目标值,FIXME:应当 = φ */
#define MAX_KP 0.001    /* 最大的Kp, 根据启动时间计算出的Kp不能超过此值 */
#define MAX_KI 0.000001 /* 最大的Ki, 根据启动时间计算出的Ki不能超过此值 */

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

static double phase_Count = 0.0;      /* 导通角对应的节拍个数 */
static double last_phase_Count = 0.0; /* 记录上个周期采用的phase_Count */
static uint16_t firing_Count = 83;    /* 后期根据用户输入的电源频率计算, 高电平维持的时间 */
static uint16_t period_Count = 500;   /* 后期根据用户输入的电源频率计算, 周期时间 */
static uint16_t phase_Diff = 83;      /* 后期根据用户输入的电源频率计算, 相邻两相相位差 */

static uint8_t flag_T1 = 0; /* 0 = 周期开始等待初相phase, 1 = 正在输出高电平, 2 = 正在等待下一周期 */
static uint8_t flag_T2 = 0; /* 0 = 初相phase, 1 = 输出高电平, 2 = 等待下一周期 */
static uint8_t flag_T3 = 0; /* 0 = 初相phase, 1 = 输出高电平, 2 = 等待下一周期 */
static uint8_t flag_T4 = 0; /* 0 = 初相phase, 1 = 输出高电平, 2 = 等待下一周期 */
static uint8_t flag_T5 = 0; /* 0 = 初相phase, 1 = 输出高电平, 2 = 等待下一周期 */
static uint8_t flag_T6 = 0; /* 0 = 初相phase, 1 = 输出高电平, 2 = 等待下一周期 */

static uint16_t count_T1 = 0; /* 用于U相上桥T1节拍, flag = 0时与phase_Count比较, = 1 时与firing_Count比较 */
static uint16_t count_T2 = 0; /* 用于W相下桥T2节拍, flag = 0时与phase_Diff比较, = 1 时与firing_Count比较 */
static uint16_t count_T3 = 0; /* 用于V相上桥T3节拍, flag = 0时与phase_Diff比较, = 1 时与firing_Count比较 */
static uint16_t count_T4 = 0; /* 用于U相下桥T4节拍, flag = 0时与phase_Diff比较, = 1 时与firing_Count比较 */
static uint16_t count_T5 = 0; /* 用于W相上桥T5节拍, flag = 0时与phase_Diff比较, = 1 时与firing_Count比较 */
static uint16_t count_T6 = 0; /* 用于V相下桥T6节拍, flag = 0时与phase_Diff比较, = 1 时与firing_Count比较 */

static uint16_t count_P = 0; /* 周期节拍, P for period, count_P == period_Count时U相上桥T1进入下一周期, 与电源周期保持一致 */

/**
 * @brief   发波GPIO, TIM, 参数初始化
 * @note    CF_START_ANGLE 启动瞬间的导通角, 一般设为150度
 * @note    count_decline 导通角减小的速率, 单位不是度, 是节拍值
 */
void pwm_config(void)
{
    param_config();                                                      /* 计算各种参数初始值 */
    gpio_init(GPIOC, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);  /* 配置pwm输出IO口, 并将所有IO口置低电平, A相 */
    gpio_init(GPIOD, GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN); /* 配置pwm输出IO口, 并将所有IO口置低电平, B相 */
    gpio_init(GPIOD, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN); /* 配置pwm输出IO口, 并将所有IO口置低电平, C相 */
    gpio_init(GPIOC, GPIO_PIN_6, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);  /* 配置pwm输出IO口, 并将所有IO口置低电平, D相 */
    gpio_init(GPIOD, GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN); /* 配置pwm输出IO口, 并将所有IO口置低电平, E相 */
    gpio_init(GPIOD, GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN); /* 配置pwm输出IO口, 并将所有IO口置低电平, F相 */

    gpio_init(GPIOE, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);        /* FIXME: PD15没引出, 暂用PE4代替 */
    gpio_init(GPIOB, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN);        /* FIXME: debug用, 标记一个周期开始 */
    tim_update_config(CF_TIM_PWM, 72, 40, CF_PWM_TIM_PRIORITY, FALSE); /* 配置定时器, 40us溢出一次 */
}

/**
 * @brief   开始PWM发波
 * @note    注意同时使能接收晶闸管导通角反馈的IO口的中断
 */
void pwm_enable(void)
{
    tim_enable(&CF_TIM_PWM_HANDLE);                      /* 对应TIM开始计数 */
    pwm_feedback_it_enable(CF_PWM_FEEDBACK_IT_PRIORITY); /* 使能接收晶闸管导通角反馈的IO口的中断, 用于相序检测 */
}

/**
 * @brief   停止pwm输出
 * @note    需要保证此函数原子性, 故执行时关中断
 */
void pwm_disable(void)
{
    __set_PRIMASK(1);                                      /* PRIMASK = 1 将屏蔽所有可屏蔽中断 */
    tim_disable(&CF_TIM_PWM_HANDLE);                       /* 失能TIM */
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

/**
 * @brief   定时器中断回调函数
 */
void pwm_update_IT_callback(TIM_HandleTypeDef *tim_handle)
{
    /* 更新节拍值 */
    count_T1++;
    count_T2++;
    count_T3++;
    count_T4++;
    count_T5++;
    count_T6++;
    count_P++;

    /*当节拍计数到达一个周期时, 调用 calculate_new_phase 调整phase_Count */
    if (count_P >= period_Count) /* 如果T1本周期节拍完成 */
    {
        /* TODO:是否需要设置一个标志位标志启动是否完成? 如果启动过程还没结束, 更新phase_Count */
        if (phase_Count != 0)
        {
            calculate_new_phase();
        }
        /* T1进入下一个周期的延时状态 */
        flag_T1 = 0;
        count_T1 = 0;
        count_P = 0;
        /* FIXME: 周期开始翻转PB7作为标志, 仅测试用 */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
    }

    if (flag_T1 == 0 && count_T1 >= phase_Count) /* 如果T1在初相延时状态已经等待了足够的时间 */
    {
        /* 结束延时状态, 对应IO口给高电平 */
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
        /* 节拍清零, 状态转换为 "正在输出高电平" */
        flag_T1 = 1;
        count_T1 = 0;
        /* 通知T2进入延时状态 */
        flag_T2 = 0;
        count_T2 = 0;
    }
    else if (flag_T1 == 1 && count_T1 >= firing_Count) /* 如果T1在高电平状态已经呆了足够的时间 */
    {
        /* 结束高电平输出*/
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
        /* 节拍清零, 状态转换, 进入等待状态, 等待下一个周期开始 */
        flag_T1 = 2;
        count_T1 = 0;
    }

    if (flag_T2 == 0 && count_T2 >= phase_Diff) /* 如果T2在延时状态等待了60° */
    {
        /* 结束延时状态, 对应IO口给高电平 */
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
        /* 节拍清零, 状态转换, 进入高电平状态 */
        flag_T2 = 1;
        count_T2 = 0;
        /* 通知T3进入phase状态 */
        flag_T3 = 0;
        count_T3 = 0;
    }
    else if (flag_T2 == 1 && count_T2 >= firing_Count)
    {
        /* 结束高电平输出*/
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
        /* 节拍清零, 进入等待状态, 等待下一个延时状态 */
        flag_T2 = 2;
        count_T2 = 0;
    }

    if (flag_T3 == 0 && count_T3 >= phase_Diff)
    {
        /* FIXME: PE4暂时代替PD15 */
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_SET);
        /* T3进入高电平状态 */
        flag_T3 = 1;
        count_T3 = 0;
        /* T4进入延时状态 */
        flag_T4 = 0;
        count_T4 = 0;
    }
    else if (flag_T3 == 1 && count_T3 >= firing_Count)
    {
        /* FIXME: PE4暂时代替PD15 */
        // HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
        /* T3结束高电平状态,等待下一周期开始 */
        flag_T3 = 2;
        count_T3 = 0;
    }

    if (flag_T4 == 0 && count_T4 >= phase_Diff)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        /* T4进入高电平状态 */
        flag_T4 = 1;
        count_T4 = 0;
        /* T5进入延时状态 */
        flag_T5 = 0;
        count_T5 = 0;
    }
    else if (flag_T4 == 1 && count_T4 >= firing_Count)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        /* T4结束高电平状态,等待下一周期开始 */
        flag_T4 = 2;
        count_T4 = 0;
    }

    if (flag_T5 == 0 && count_T5 >= phase_Diff) /* T5初相计时结束 */
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
        /* T5进入高电平状态 */
        flag_T5 = 1;
        count_T5 = 0;
        /* T6进入初相延时状态 */
        flag_T6 = 0;
        count_T6 = 0;
    }
    else if (flag_T5 == 1 && count_T5 >= firing_Count)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
        /* T5结束高电平状态,等待下一周期开始 */
        flag_T5 = 2;
        count_T5 = 0;
    }

    if (flag_T6 == 0 && count_T6 >= phase_Diff)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
        /* T6 进入高电平状态 */
        flag_T6 = 1;
        count_T6 = 0;
    }
    else if (flag_T6 == 1 && count_T6 >= firing_Count)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
        /* T6结束高电平状态,等待下一周期开始 */
        flag_T6 = 2;
        count_T6 = 0;
    }
}

/**
 * @brief   计算新的导通角
 * @note    计算新的延时应该放到新的电源周期开始时做, 也就是第一相进入延时状态时
 * FIXME:   目前暂在此单个函数中用if判断不同启动模式, 来采取不同策略控制导通角, 后续考虑是不是要替换成不同的三个回调函数
 * TODO:   单位为40us节拍, 现在PI算法的计算都是以节拍为单位, 目标节拍为0, 视情况修改为导通角角度或者电压
 */
static void calculate_new_phase(void)
{
    error = TARGET_COUNT - phase_Count;
    if (CI_START_MODE == 0) /* 电压控制启动模式 */
    {
        error_Sum += error;
        temp = phase_Count + error * Kp + error_Sum * Ki;
        last_phase_Count = phase_Count;
        phase_Count = temp >= 1 ? temp : 0; /* phase_Count < 1时, 直接置零, 认为软启动结束 */
    }
    else if (CI_START_MODE == 1) /* 如果是电流控制启动, 采取措施控制电流 */
    {
        error = error > max_Error ? max_Error : error; /* 需要控制error的大小, 来限制电压增长的速度, 进而预防电流过大 */
        if (AM_EffectiveU_I <= CI_MAX_START_I)         /* 如果电流没超, 按正常流程走 */
        {
            error_Sum += error;
            temp = phase_Count + error * Kp + error_Sum * Ki;
            last_phase_Count = phase_Count;
            phase_Count = temp >= 1 ? temp : 0; /* phase_Count < 1时, 直接置零, 认为软启动结束 */
        }
        else if (AM_EffectiveU_I > 1.1 * CI_MAX_START_I) /* TODO:这里的系数1.1是否要提取为可配置的宏? */
        {
            phase_Count = last_phase_Count; /* 如果电流过大了, 回滚phase_Count, 同时本轮error不计入error_Sum, 以放缓后续电流增长 */
        }
        else
        {
            /* AM_EffectiveU_I > CI_MAX_START_I && AM_EffectiveU_I <= 1.1CI_MAX_START_I */
            /* 如果电流介于两者之间, 说明电流只是略大, phase_count和last_phase_Count维持不变, error不计入error_Sum即可 */
        }
    }
}

/**
 * @brief 根据用户配置计算初始phase_Count, max_Error, Kp, Ki
 */
static void param_config(void)
{
    phase_Count = ((CF_START_ANGLE / 360.0) * 500.0);              /* 首相延时节拍, 150°对应为208.333  (phase_angle / 360) * (20 ms / 40 us) */
    max_Error = CF_ERROR_MAX_SCALE * (phase_Count - TARGET_COUNT); /* 计算最大允许的单次ERROR */
    /* TODO: 根据启动时间配置kp和ki */
    if (CI_START_TIME == 20)
    {
        Kp = MAX_KP;
        Kp = MAX_KI;
    }
}

#endif
