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
 * PD2(J3) PD1(J2) PD0(J1) ��բ�ܷ���, ��������ѹ������ʱΪ�ߵ�ƽ, ������Ϊ�͵�ƽ, PD012��δ������ĳ��Ƭ���������, ����ֻ����GPIO�ж����ֶ�����
 * PE8 ����¶ȳ�85����ź�
 *
 ****************************************************************************************************
 * @TODO:
 * �����ɼ��ı��÷���: ��β���ƽ������ƽ���ٿ���
 * ��U��Ϊ��, PD0����������֮����Ϊһ����������, ��ʵʱ�Դ����������˲ʱֵ��ƽ����, SumU_I += AM_OutU_PhaseI*AM_OutU_PhaseI; SumU_��������++; 
 * ע��ƫ����; 
 * PD0�����ش����ж�, �жϴ��������ȼ���SumU_I/��������, ����effectiveU, Ȼ��SumU_I�Ͳ�����������, ��ʼ��һ�ּ���
 *
 ****************************************************************************************************
 */
#include "globalE.h"
#include "FreeRTOS.h"
#include "task.h"
#include "dataCollect.h"
#include "SystemSafety.h"
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

float AM_EffectiveU_I;    /* ��Ӽ���, ��һ����U�������Чֵ, ��λA */
float AM_EffectiveV_I;    /* ��Ӽ���, ��һ����V�������Чֵ, ��λA */
float AM_EffectiveW_I;    /* ��Ӽ���, ��һ����W�������Чֵ, ��λA */
float SV_FAN_OutU_PhaseI; /* 7.5KW���ȵ���, U·-PA5-ADC5 */
float SV_FAN_OutV_PhaseI; /* 7.5KW���ȵ���, V·-PA4-ADC4 */
float AM_Pressure_A;      /* ��չ��ѹ��������AI2-PA1-ADC1 */
float AM_Pressure_B;      /* ��չ��ѹ��������AI3-PC0-ADC10 */
float AM_Pressure_C;      /* ѹ��������, ����X3-PC2-ADC12 */
float AM_Temp_A;          /* ��չ���¶ȴ�����-PA0-ADC0 */
float AM_Temp_B;          /* ��չ���¶ȴ�����-PC1-ADC11 */
float AM_Temp_C;          /* �¶ȴ�����, ����X3-PC3-ADC13 */
float SV_U_MaxI;          /* ��¼������U��������� */
float SV_V_MaxI;          /* ��¼������V��������� */
float SV_W_MaxI;          /* ��¼������W��������� */

/**
 * @brief   ADC��ʼ��
 */
void adc_init(void)
{
    /* PA6(12_6) PA7(12_7) PC4(12_14) ������������� */
    gpio_init(GPIOA, GPIO_PIN_6, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_init(GPIOA, GPIO_PIN_7, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_init(GPIOC, GPIO_PIN_4, GPIO_MODE_ANALOG, GPIO_PULLDOWN);

    /* PC5(12_15) PB0(12_8) PB1(12_9) ��բ�ܷ�����·, Ϊ����������ѹ */
    gpio_init(GPIOC, GPIO_PIN_5, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_init(GPIOB, GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_init(GPIOB, GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PA4(12_4) PA5(12_5) 7.5KW����������, ����· */
    gpio_init(GPIOA, GPIO_PIN_4, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_init(GPIOA, GPIO_PIN_5, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PA1(123_1) PC0(123_10) ��չ��ѹ�������� */
    gpio_init(GPIOA, GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_init(GPIOC, GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PA0(123_0) PC1(123_11) ��չ���¶ȴ����� */
    gpio_init(GPIOA, GPIO_PIN_0, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_init(GPIOC, GPIO_PIN_1, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    /* PC2(123_12) ������չ��, ѹ��������;  PC3(123_13) �¶ȴ����� */
    gpio_init(GPIOC, GPIO_PIN_2, GPIO_MODE_ANALOG, GPIO_PULLDOWN);
    gpio_init(GPIOC, GPIO_PIN_3, GPIO_MODE_ANALOG, GPIO_PULLDOWN);

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
 * @brief   �����ź����ݲɼ�GPIO��������
 */
void gpio_data_input_init(void)
{
    /* ��ѹ�����ᷴ��, ע����������GPIO_MODE_IT_RISING */
    gpio_init(GPIOD, GPIO_PIN_0, GPIO_MODE_IT_RISING, GPIO_PULLDOWN);
    gpio_init(GPIOD, GPIO_PIN_1, GPIO_MODE_IT_RISING, GPIO_PULLDOWN);
    gpio_init(GPIOD, GPIO_PIN_2, GPIO_MODE_IT_RISING, GPIO_PULLDOWN);
    HAL_NVIC_SetPriority(EXTI0_IRQn, CF_Voltage_Feedback_IT_Priority, 0);
    HAL_NVIC_SetPriority(EXTI1_IRQn, CF_Voltage_Feedback_IT_Priority, 0);
    HAL_NVIC_SetPriority(EXTI2_IRQn, CF_Voltage_Feedback_IT_Priority, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
    /* ��բ�ܵ�ͨ����, ��Ӧ���ж���PWM��ʼ����ʱʹ��, ע����������GPIO_MODE_IT_RISING_FALLING */
    gpio_init(GPIOD, GPIO_PIN_3, GPIO_MODE_IT_RISING_FALLING, GPIO_PULLDOWN);
    gpio_init(GPIOD, GPIO_PIN_4, GPIO_MODE_IT_RISING_FALLING, GPIO_PULLDOWN);
    gpio_init(GPIOD, GPIO_PIN_5, GPIO_MODE_IT_RISING_FALLING, GPIO_PULLDOWN);
    /* ����¶ȴ��������� */
    gpio_init(GPIOE, GPIO_PIN_8, GPIO_MODE_INPUT, GPIO_PULLDOWN);
}

/**
 * @brief   ��ʼ����ʵʱ�����ݴ�����
 * @note    CF_HIGH_LEVEL_DATA_TRANS_PERIOD��λΪus, ÿ���ú�ָ����ʱ�����һ�����ݴ���
 */
void highLevel_dataTrans_init(void)
{
    tim_update_config(CF_DATA_TRANS_TIM, 72, CF_HIGH_LEVEL_DATA_TRANS_PERIOD, CF_HIGHLEVEL_DATATRANS_TIM_PRIORITY, TRUE);
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
    /* �ҳ�������˲ʱֵ, ���ڼ��������Чֵ */
    SV_U_MaxI = AM_OutU_PhaseI > SV_U_MaxI ? AM_OutU_PhaseI : SV_U_MaxI;
    SV_V_MaxI = AM_OutV_PhaseI > SV_V_MaxI ? AM_OutV_PhaseI : SV_V_MaxI;
    SV_W_MaxI = AM_OutW_PhaseI > SV_W_MaxI ? AM_OutW_PhaseI : SV_W_MaxI;
    /* FIXME:�������ط�ʱ�޷���,��ϵ��������ģʽ? ���ط�ʱ�޼�� */
    switch (CF_OverPhI_Config)
    {
    case OVERPHI_L6:
        /*6�����ط���ʱ�ж�,3������*/
        AM_OutU_PhaseI > (3 * CF_RatedPhI) ? SV_OVERPHI_L6Cnt++ : SV_OVERPHI_L6Cnt--;
        break;
    case OVERPHI_L5:
        /*5�����ط���ʱ�ж�,2������*/
        AM_OutU_PhaseI > (2 * CF_RatedPhI) ? SV_OVERPHI_L5Cnt++ : SV_OVERPHI_L5Cnt--;
        break;
    case OVERPHI_L4:
        /*4�����ط���ʱ�ж�,1.8������*/
        AM_OutU_PhaseI > (1.8 * CF_RatedPhI) ? SV_OVERPHI_L4Cnt++ : SV_OVERPHI_L4Cnt--;
        break;
    case OVERPHI_L3:
        /*3�����ط���ʱ�ж�,1.5������*/
        AM_OutU_PhaseI > (1.5 * CF_RatedPhI) ? SV_OVERPHI_L3Cnt++ : SV_OVERPHI_L3Cnt--;
        break;
    case OVERPHI_L2:
        /*2�����ط���ʱ�ж�,1.2������*/
        AM_OutU_PhaseI > (1.2 * CF_RatedPhI) ? SV_OVERPHI_L2Cnt++ : SV_OVERPHI_L2Cnt--;
        break;
    case OVERPHI_L1:
        /*1�����ط���ʱ�ж�*/
        AM_OutU_PhaseI > CF_RatedPhI ? SV_OVERPHI_L1Cnt++ : SV_OVERPHI_L1Cnt--;
        break;
    default:
        break;
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
 * @note    PD3(J2) PD4(J1) PD5(J3) PWM���Ĳ��Ļ���
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
