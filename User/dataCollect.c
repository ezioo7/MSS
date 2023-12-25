/**
 ****************************************************************************************************
 * @file        dataCollect.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2023-12-14
 * @brief       ���ڴ���ADC�ɼ�������
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 * ����������, һ����ADC�ɼ�ģ���ź�, һ���Ǵ�GPIO�ɼ������ź�, �ֱ�ŵ�adc_init��gpio_input_init��
 *
 ****************************************************************************************************
 * @attention
 * ADC�ɼ����ź���������Щ:
 * PA6(12_6) PA7(12_7) PC4(12_14) �������������, ������?
 * PC5(12_15) PB0(12_8) PB1(12_9) ��բ�ܷ�����·, Ϊ����������ѹ
 * PA4(12_4) PA5(12_5) 7.5KW����������, ����·
 * PA1(123_1) PC0(123_10) ��չ��ѹ��������
 * PA0(123_0) PC1(123_11) ��չ���¶ȴ�����
 * PC2(123_12) ������չ��, ѹ��������
 * PC3(123_13) ������չ��, �¶ȴ�����
 * ���ֲ���ADC1�����������ADC�ɼ�����, ��ADC1��DMA1�ǹ̶�����
 *
 ****************************************************************************************************
 * @attention
 * GPIOֱ�Ӳɼ��������ź�������:
 * PD3(J2) PD4(J1) PD5(J3) PWM���Ĳ��Ļ���
 * PD2(J3) PD1(J2) PD0(J1) ��բ�ܷ���, ��������ѹ������ʱΪ�ߵ�ƽ, ������Ϊ�͵�ƽ
 * PE8 ����¶ȳ�85����ź�
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

/* FIXME:���¶���ŵ�dataCollect����updateState? */
extern float AM_OutUO_Voltage; /* ֱ�Ӳɼ�, ���U���ѹƽ��ֵ, PB1, ADC9 */
extern float AM_OutVO_Voltage; /* ֱ�Ӳɼ�, ���V���ѹƽ��ֵ, PB0, ADC8 */
extern float AM_OutWO_Voltage; /* ֱ�Ӳɼ�, ���W���ѹƽ��ֵ, PC5, ADC15 */
extern float AM_OutUV_Voltage; /* ��Ӽ���, ���UV����ѹƽ��ֵ */
extern float AM_OutUW_Voltage; /* ��Ӽ���, ���UW����ѹƽ��ֵ */
extern float AM_OutWV_Voltage; /* ��Ӽ���, ���WV����ѹƽ��ֵ */
extern float AM_OutU_PhaseI;   /* ֱ�Ӳɼ�, ���U�����ƽ��ֵ, PA7, ADC7 */
extern float AM_OutV_PhaseI;   /* ֱ�Ӳɼ�, ���V�����ƽ��ֵ, PA6, ADC6 */
extern float AM_OutW_PhaseI;   /* ֱ�Ӳɼ�, ���W�����ƽ��ֵ, PC4, ADC14 */

float SV_FAN_OutU_PhaseI; /* 7.5KW���ȵ���, U·-PA5-ADC5 */
float SV_FAN_OutV_PhaseI; /* 7.5KW���ȵ���, V·-PA4-ADC4 */
float AM_Pressure_A;      /* ��չ��ѹ��������AI2-PA1-ADC1 */
float AM_Pressure_B;      /* ��չ��ѹ��������AI3-PC0-ADC10 */
float AM_Pressure_C;      /* ѹ��������, ����X3-PC2-ADC12 */
float AM_Temp_A;          /* ��չ���¶ȴ�����-PA0-ADC0 */
float AM_Temp_B;          /* ��չ���¶ȴ�����-PC1-ADC11 */
float AM_Temp_C;          /* �¶ȴ�����, ����X3-PC3-ADC13 */

/**
 * @brief   ADC��ʼ��
 */
void adc_init(void)
{
    /* PA6(12_6) PA7(12_7) PC4(12_14) ������������� */
    gpio_config(GPIOA, GPIO_PIN_6, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOA, GPIO_PIN_7, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOC, GPIO_PIN_4, GPIO_MODE_ANALOG, GPIO_PULLDOWN);

    /* PC5(12_15) PB0(12_8) PB1(12_9) ��բ�ܷ�����·, Ϊ����������ѹ */
    gpio_config(GPIOC, GPIO_PIN_5, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOB, GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOB, GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PA4(12_4) PA5(12_5) 7.5KW����������, ����· */
    gpio_config(GPIOA, GPIO_PIN_4, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOA, GPIO_PIN_5, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PA1(123_1) PC0(123_10) ��չ��ѹ�������� */
    gpio_config(GPIOA, GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOC, GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PA0(123_0) PC1(123_11) ��չ���¶ȴ����� */
    gpio_config(GPIOA, GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOC, GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PC2(123_12) ������չ��, ѹ��������;  PC3(123_13) �¶ȴ����� */
    gpio_config(GPIOC, GPIO_PIN_2, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_config(GPIOC, GPIO_PIN_3, GPIO_MODE_ANALOG, GPIO_PULLDOWN);

    uint32_t channels[14] =
        {
            ADC_CHANNEL_9,   /*U���ѹ*/
            ADC_CHANNEL_8,   /*V���ѹ*/
            ADC_CHANNEL_15,  /*W���ѹ*/
            ADC_CHANNEL_7,   /*U�����*/
            ADC_CHANNEL_6,   /*V�����*/
            ADC_CHANNEL_14,  /*W�����*/
            ADC_CHANNEL_5,   /*���U�����*/
            ADC_CHANNEL_4,   /*���V�����*/
            ADC_CHANNEL_0,   /*��չ���¶ȴ�����A*/
            ADC_CHANNEL_11,  /*��չ���¶ȴ�����B*/
            ADC_CHANNEL_13,  /*X3�¶ȴ�����*/
            ADC_CHANNEL_1,   /*��չ��ѹ��������A*/
            ADC_CHANNEL_10,  /*��չ��ѹ��������B*/
            ADC_CHANNEL_12}; /*X3ѹ��������*/
    adc_dma_enable(channels);
}

/**
 * @brief   �����ź����ݲɼ�GPIO��������, ע�Ⲣδʹ�ܶ�Ӧ���ŵ��ж�
 */
void gpio_data_input_init(void)
{
    /* ��ѹ�����ᷴ�� */
    gpio_config(GPIOD, GPIO_PIN_0, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_1, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_2, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    /* ��բ�ܵ�ͨ����, ��Ӧ���ж���PWM��ʼ����ʱʹ�� */
    gpio_config(GPIOD, GPIO_PIN_3, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_4, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    gpio_config(GPIOD, GPIO_PIN_5, GPIO_MODE_INPUT, GPIO_PULLDOWN);
    /* ����¶ȴ��������� */
    gpio_config(GPIOE, GPIO_PIN_8, GPIO_MODE_INPUT, GPIO_PULLDOWN);
}

/**
 * @brief   ��ʼ����ʵʱ�����ݴ�����
 * @note    CF_HIGH_LEVEL_DATA_TRANS_PERIOD��λΪus, ÿ���ú�ָ����ʱ�����һ�����ݴ���
 */
void highLevel_dataTrans_init(void)
{
    tim_update_enable(CF_DATA_TRANS_TIM, 71, CF_HIGH_LEVEL_DATA_TRANS_PERIOD - 1, CF_HIGHLEVEL_DATATRANS_TIM_PRIORITY, TRUE);
}

/**
 * @brief   ��������, ����ĳһ��������ADCԭʼ����3�β�����ƽ��ֵ
 * FIXME: Ϊʲô����static�ͻᱨ��, δ����get_mean?
 */
static inline float get_mean(uint8_t index)
{
    return (g_adc1_buffer[index] + g_adc1_buffer[index + 14] + g_adc1_buffer[index + 28]) / 3.0;
}

/**
 * @brief   �жϻص�����, ����ADC�ռ�������, ר�Ŵ����ʵʱ�Ե�����
 */
void highLevel_dataTrans_callback(void)
{
    /* ��ԭʼ����ת��Ϊ��ʵ������ */
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
    /* TODO: ���ط�ʱ�޼�� */
    if (1 /* ����̫�� */)
    {
        /* ���ؼ���+1 */
    }
    else
    {
        /* ���ؼ���-1 */
    }
}

/**
 * @brief   ������, ��ADC��ȡ����ԭʼ����ת��Ϊ����ʵ�������������, �������ʵʱ�Ե�����
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
 * @brief   ʹ�ܽ��վ�բ�ܵ�ͨ�Ƿ�����IO�ڵ��ж�
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
 * @brief   ������, ��ʱ��ӡ������ADC����, ��������
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
