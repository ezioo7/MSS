/**
 ****************************************************************************************************
 * @file        dataCollect.c
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-12-14
 * @brief       用于处理ADC采集的数据
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 * 包括两部分, 一个是ADC采集模拟信号, 一个是纯GPIO采集数字信号, 分别放到adc_init和gpio_input_init中
 *
 ****************************************************************************************************
 * @attention
 * ADC采集的信号有以下这些:
 * PA6(12_6) PA7(12_7) PC4(12_14) 三相电流传感器, 物理量?
 * PC5(12_15) PB0(12_8) PB1(12_9) 晶闸管反馈电路, 为电机的输入电压
 * PA4(12_4) PA5(12_5) 7.5KW风机电流检测, 有两路
 * PA1(123_1) PC0(123_10) 扩展卡压力传感器
 * PA0(123_0) PC1(123_11) 扩展卡温度传感器
 * PC2(123_12) 类似扩展卡, 压力传感器
 * PC3(123_13) 类似扩展卡, 温度传感器
 * 发现采用ADC1即可完成所有ADC采集工作, 而ADC1和DMA1是固定搭配
 *
 ****************************************************************************************************
 * @attention
 * GPIO直接采集的数字信号有以下:
 * PD3(J2) PD4(J1) PD5(J3) PWM发的波的回馈
 * PD2(J3) PD1(J2) PD0(J1) 晶闸管反馈, 电机输入电压正半轴时为高电平, 负半轴为低电平
 * PE8 风机温度超85后给信号
 *
 ****************************************************************************************************
 */
#include "globalE.h"
#include "FreeRTOS.h"
#include "task.h"
#include "dataCollect.h"
#include "./BSP/ADC/adc.h"
#include "./BSP/GPIO/gpio.h"
#include "./BSP/TIM/tim.h"

static inline float get_mean(uint8_t index);

/* FIXME:以下定义放到dataCollect还是updateState? */
extern float AM_OutUO_Voltage; /* 直接采集, 输出U相电压平均值, PB1, ADC9 */
extern float AM_OutVO_Voltage; /* 直接采集, 输出V相电压平均值, PB0, ADC8 */
extern float AM_OutWO_Voltage; /* 直接采集, 输出W相电压平均值, PC5, ADC15 */
extern float AM_OutUV_Voltage; /* 间接计算, 输出UV相间电压平均值 */
extern float AM_OutUW_Voltage; /* 间接计算, 输出UW相间电压平均值 */
extern float AM_OutWV_Voltage; /* 间接计算, 输出WV相间电压平均值 */
extern float AM_OutU_PhaseI;   /* 直接采集, 输出U相电流平均值, PA7, ADC7 */
extern float AM_OutV_PhaseI;   /* 直接采集, 输出V相电流平均值, PA6, ADC6 */
extern float AM_OutW_PhaseI;   /* 直接采集, 输出W相电流平均值, PC4, ADC14 */

float SV_FAN_OutU_PhaseI; /* 7.5KW风扇电流, U路-PA5-ADC5 */
float SV_FAN_OutV_PhaseI; /* 7.5KW风扇电流, V路-PA4-ADC4 */
float AM_Pressure_A;      /* 扩展卡压力传感器AI2-PA1-ADC1 */
float AM_Pressure_B;      /* 扩展卡压力传感器AI3-PC0-ADC10 */
float AM_Pressure_C;      /* 压力传感器, 来自X3-PC2-ADC12 */
float AM_Temp_A;          /* 扩展卡温度传感器-PA0-ADC0 */
float AM_Temp_B;          /* 扩展卡温度传感器-PC1-ADC11 */
float AM_Temp_C;          /* 温度传感器, 来自X3-PC3-ADC13 */

/**
 * @brief   ADC初始化
 */
void adc_init(void)
{
    /* PA6(12_6) PA7(12_7) PC4(12_14) 三相电流传感器 */
    gpio_config(GPIOA, GPIO_PIN_6, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOA, GPIO_PIN_7, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOC, GPIO_PIN_4, GPIO_MODE_ANALOG, GPIO_PULLDOWN);

    /* PC5(12_15) PB0(12_8) PB1(12_9) 晶闸管反馈电路, 为电机的输入电压 */
    gpio_config(GPIOC, GPIO_PIN_5, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOB, GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOB, GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PA4(12_4) PA5(12_5) 7.5KW风机电流检测, 有两路 */
    gpio_config(GPIOA, GPIO_PIN_4, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOA, GPIO_PIN_5, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PA1(123_1) PC0(123_10) 扩展卡压力传感器 */
    gpio_config(GPIOA, GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOC, GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PA0(123_0) PC1(123_11) 扩展卡温度传感器 */
    gpio_config(GPIOA, GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOC, GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PC2(123_12) 类似扩展卡, 压力传感器;  PC3(123_13) 温度传感器 */
    gpio_config(GPIOC, GPIO_PIN_2, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOC, GPIO_PIN_3, GPIO_MODE_ANALOG, GPIO_PULLDOWN);

    uint32_t channels[14] =
        {
            ADC_CHANNEL_9,   /*U相电压*/
            ADC_CHANNEL_8,   /*V相电压*/
            ADC_CHANNEL_15,  /*W相电压*/
            ADC_CHANNEL_7,   /*U相电流*/
            ADC_CHANNEL_6,   /*V相电流*/
            ADC_CHANNEL_14,  /*W相电流*/
            ADC_CHANNEL_5,   /*风机U相电流*/
            ADC_CHANNEL_4,   /*风机V相电流*/
            ADC_CHANNEL_0,   /*扩展卡温度传感器A*/
            ADC_CHANNEL_11,  /*扩展卡温度传感器B*/
            ADC_CHANNEL_13,  /*X3温度传感器*/
            ADC_CHANNEL_1,   /*扩展卡压力传感器A*/
            ADC_CHANNEL_10,  /*扩展卡压力传感器B*/
            ADC_CHANNEL_12}; /*X3压力传感器*/
    adc_dma_enable(channels);
}

/**
 * @brief   数字信号数据采集GPIO引脚配置, 注意并未使能对应引脚的中断
 */
void gpio_data_input_init(void)
{
    /* 电压正半轴反馈 */
    gpio_config(GPIOD, GPIO_PIN_0, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_1, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_2, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    /* 晶闸管导通反馈, 对应的中断在PWM开始发波时使能 */
    gpio_config(GPIOD, GPIO_PIN_3, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_4, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_5, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    /* 风机温度传感器报警 */
    gpio_config(GPIOE, GPIO_PIN_8, GPIO_MODE_INPUT, GPIO_PULLDOWN);
}

/**
 * @brief   初始化高实时性数据处理工作
 * @note    CF_HIGH_LEVEL_DATA_TRANS_PERIOD单位为us, 每隔该宏指定的时间进行一次数据处理
 */
void highLevel_dataTrans_init(void)
{
    tim_update_enable(CF_DATA_TRANS_TIM, 71, CF_HIGH_LEVEL_DATA_TRANS_PERIOD - 1, CF_HIGHLEVEL_DATATRANS_TIM_PRIORITY, TRUE);
}

/**
 * @brief   内联函数, 计算某一物理量的ADC原始数据3次采样的平均值
 * FIXME: 为什么不加static就会报错, 未定义get_mean?
 */
static inline float get_mean(uint8_t index)
{
    return (g_adc1_buffer[index] + g_adc1_buffer[index + 14] + g_adc1_buffer[index + 28]) / 3.0;
}

/**
 * @brief   中断回调函数, 处理ADC收集的数据, 专门处理高实时性的数据
 */
void highLevel_dataTrans_callback(void)
{
    /* 从原始数据转化为真实物理量 */
    AM_OutUO_Voltage = get_mean(0);
    AM_OutVO_Voltage = get_mean(1);
    AM_OutWO_Voltage = get_mean(2);
    AM_OutUV_Voltage = 0;
    AM_OutUW_Voltage = 0;
    AM_OutWV_Voltage = 0;
    AM_OutU_PhaseI = get_mean(3);
    AM_OutV_PhaseI = get_mean(4);
    AM_OutW_PhaseI = get_mean(5);
    SV_FAN_OutU_PhaseI = get_mean(6);
    SV_FAN_OutV_PhaseI = get_mean(7);
    /* TODO: 过载反时限检测 */
    if (1 /* 电流太大 */)
    {
        /* 过载计数+1 */
    }
    else
    {
        /* 过载计数-1 */
    }
}

/**
 * @brief   任务函数, 将ADC读取到的原始数据转换为具有实际意义的物理量, 负责处理低实时性的数据
 */
void lowLevel_dataTrans_task(void *pvParameters)
{
    while (1)
    {
        AM_Pressure_A = get_mean(8);
        AM_Pressure_A = get_mean(9);
        AM_Pressure_A = get_mean(10);
        AM_Temp_A = get_mean(11);
        AM_Temp_B = get_mean(12);
        AM_Temp_C = get_mean(13);

        vTaskDelay(300);
    }
}

/**
 * @brief   使能接收晶闸管导通角反馈的IO口的中断
 */
void pwm_feedback_it_enable(uint8_t priority)
{
    HAL_NVIC_SetPriority(EXTI3_IRQn, priority, 0);
    HAL_NVIC_SetPriority(EXTI4_IRQn, priority, 0);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, priority, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/**
 * @brief   任务函数, 定时打印处理后的ADC数据, 仅测试用
 */
void data_print_task(void *pvParameters)
{
    while (1)
    {
        printf("AM_OutUO_Voltage:%f\r\n", AM_OutUO_Voltage);
        printf("AM_OutVO_Voltage:%f\r\n", AM_OutVO_Voltage);
        printf("AM_OutWO_Voltage:%f\r\n\r\n", AM_OutWO_Voltage);
        vTaskDelay(1000);
    }
}
