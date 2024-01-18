/**
 ****************************************************************************************************
 * @file        globalE.h
 * @author      �����飬zhuowenwei
 * @version     V1.0
 * @date        2024-12-09
 * @brief       ȫ�ֱ���
 * @licence     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 *
 *
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_GlobalE_H
#define __MSS_GlobalE_H

/******************************************************************************/
/*                              �����궨��                                     */
/******************************************************************************/
#define BIT_SET(x, n) (x = x | (0x01 << n))     /*����x�����ĵ�nΪλ1*/
#define BIT_TEST(x, n) ((x & (0x01 << n)) != 0) /*���x�����ĵ�nλ�Ƿ�Ϊ1*/
#define BIT_CLEAR(x, n) (x = x & ~(0x01 << n))  /*����x�����ĵ�nΪλ0*/
#define TRUE 1
#define FALSE 0

typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short int uint16_t;
typedef unsigned char uint8_t;

/******************************************************************************/
/*                               �ɼ���Ϣ                                      */
/******************************************************************************/
extern float CI_DesireVoltage;   /* �����û������������ѹ���� */
extern float SV_DesireVoltage;   /* �������ѹ������ϵͳ���� */
extern float CF_RatedPhI;        /* ����Ķ����,32λ��Ч,��λA,����0.01A */
extern float AM_OutUO_Voltage;   /* ֱ�Ӳɼ�, ���U���ѹƽ��ֵ, PB1, ADC9 */
extern float AM_OutVO_Voltage;   /* ֱ�Ӳɼ�, ���V���ѹƽ��ֵ, PB0, ADC8 */
extern float AM_OutWO_Voltage;   /* ֱ�Ӳɼ�, ���W���ѹƽ��ֵ, PC5, ADC15 */
extern float AM_OutUV_Voltage;   /* ��Ӽ���, ���UV����ѹƽ��ֵ */
extern float AM_OutUW_Voltage;   /* ��Ӽ���, ���UW����ѹƽ��ֵ */
extern float AM_OutWV_Voltage;   /* ��Ӽ���, ���WV����ѹƽ��ֵ */
extern float AM_OutU_PhaseI;     /* ֱ�Ӳɼ�, ���U�����ƽ��ֵ, PA7, ADC7 */
extern float AM_OutV_PhaseI;     /* ֱ�Ӳɼ�, ���V�����ƽ��ֵ, PA6, ADC6 */
extern float AM_OutW_PhaseI;     /* ֱ�Ӳɼ�, ���W�����ƽ��ֵ, PC4, ADC14 */
extern float AM_EffectiveU_I;    /* ��Ӽ���, ��һ����U�������Чֵ, ��λA */
extern float AM_EffectiveV_I;    /* ��Ӽ���, ��һ����V�������Чֵ, ��λA */
extern float AM_EffectiveW_I;    /* ��Ӽ���, ��һ����W�������Чֵ, ��λA */
extern float SV_FAN_OutU_PhaseI; /* ֱ�Ӳɼ�, 7.5KW���ȵ���, U·, PA5, ADC5 */
extern float SV_FAN_OutV_PhaseI; /* ֱ�Ӳɼ�, 7.5KW���ȵ���, V·, PA4, ADC4 */
extern float AM_Pressure_A;      /* ֱ�Ӳɼ�, ��չ��ѹ��������AI2, PA1, ADC1 */
extern float AM_Pressure_B;      /* ֱ�Ӳɼ�, ��չ��ѹ��������AI3, PC0, ADC10 */
extern float AM_Pressure_C;      /* ֱ�Ӳɼ�, X3ѹ��������, PC2, ADC12 */
extern float AM_Temp_A;          /* ֱ�Ӳɼ�, ��չ���¶ȴ�����, PA0, ADC0 */
extern float AM_Temp_B;          /* ֱ�Ӳɼ�, ��չ���¶ȴ�����, PC1, ADC11 */
extern float AM_Temp_C;          /* ֱ�Ӳɼ�, X3�¶ȴ�����, PC3, ADC13 */

/******************************************************************************/
/*                     UpdateState״̬�仯�������궨��                          */
/******************************************************************************/
extern uint16_t SV_MSSSatus; /* 1-��ʾĬ��״̬*/
#define STATE_STANDY 1       /*1-��ʾStandy״̬��*/
#define STATE_TRANSITION 2   /*2-��ʾTransition״̬*/
#define STATE_RUNNING 3      /*3-��ʾRunning״̬*/
#define STATE_SYSTEMLOCK 10  /*10-SystemLock״̬*/
#define STATE_TEST 11        /*11-TEST״̬*/
extern uint16_t CI_Command;  /*�û��ⲿ��������*/
#define CMD_RUNING 1         /*����ָ��*/
#define CMD_ESTOP 2          /*��ָͣ��*/
#define CMD_PAUSED 3         /*��ָͣ��*/
#define CMD_STOP 4           /*ָֹͣ��*/
#define CMD_TESTING 5        /*����ָ��*/

/******************************************************************************/
/*                      safetyϵͳ��ȫ��ر����ͺ궨��                          */
/******************************************************************************/
extern uint16_t CF_OverPhI_Config; /*ϵͳ���ط�ʱ���趨*/
#define CF_OVERPHI_FACTOR 1.2      /* ���ط�ʱ��ϵ�� */
#define OVERPHI_L0 0               /*0-��ʾ�޷�ʱ���趨*/
#define OVERPHI_L1 1               /*1-��ʾ����1���޷�ʱ���趨��1.05���������ʱ�ޱ���*/
#define OVERPHI_L2 2               /*2-��ʾ����2���޷�ʱ���趨��1.2���������ʱ�ޱ���*/
#define OVERPHI_L3 3               /*3-��ʾ����3���޷�ʱ���趨��1.5���������ʱ�ޱ���*/
#define OVERPHI_L4 4               /*4-��ʾ����4���޷�ʱ���趨��1.8���������ʱ�ޱ���*/
#define OVERPHI_L5 5               /*5-��ʾ����5���޷�ʱ���趨��2���������ʱ�ޱ���*/
#define OVERPHI_L6 6               /*6-��ʾ����6���޷�ʱ���趨��3���������ʱ�ޱ���*/

extern uint32_t SV_Fault1;   /*һ���������ش���ϵͳ��������״̬*/
#define FAT1_NO 0            /*��ʾ�޴���*/
#define FAT1_CANOUT 1        /*CANͨ�ų�ʱ*/
#define FAT1_OVERCURRENT 2   /*�������������س�ʱ����������ط�ʱ��˵��*/
#define FAT1_OUTLOSSPHU 3    /*���ȱ��U*/
#define FAT1_OUTLOSSPHV 4    /*���ȱ��V*/
#define FAT1_OUTLOSSPHW 5    /*���ȱ��W*/
#define FAT1_INTLOSSPHA 6    /*����ȱ��A*/
#define FAT1_INTLOSSPHB 7    /*����ȱ��B*/
#define FAT1_INTLOSSPHC 8    /*����ȱ��C*/
#define FAT1_PHASESEQUENCE 9 /*�����������*/

extern uint32_t SV_Fault2; /*������������ϵͳ���͹�������*/
#define FAT2_NO 0          /*��ʾ�޴���*/
#define FAT2_OVERPHI 1     /*1.05~3�����ش��󣬾���*/

extern uint32_t SV_Fault3; /*�������󣬾���*/

#define SUCCESS_OK 0 /*ϵͳ�޴���*/
#define FAIL_ERR 1   /*ϵͳ����*/

/******************************************************************************/
/*                                ADC�ɼ�����                                  */
/******************************************************************************/
#define CF_DATA_TRANS_TIM MSS_TIM2                /* ��ʱ����ADC�ɼ�������, ����ʹ�õĶ�ʱ�� */
#define CF_DATA_TRANS_TIM_HANDLE g_tim2_handle    /* �����ռ���ʱ����� */
#define CF_ADC_CH_NUM 14                          /* ADC�ɼ���ͨ����Ŀ */
#define CF_HIGH_LEVEL_DATA_TRANS_PERIOD 250       /* ��ʵʱ�����ݵĴ���ʱ����, ��λΪus*/
extern uint16_t g_adc1_buffer[3 * CF_ADC_CH_NUM]; /* ADC1�ɼ��Ļ���Buffer, ADC1�ɼ��������ݰ���ͨ�����õ�˳��ŵ����ڴ�����, 14·�ɼ�, ����������ʷ, ��42������ */

/******************************************************************************/
/*                                 PWM����                                    */
/******************************************************************************/
#define CF_START_ANGLE 150     /* ��ʼ�ĵ�ͨ��, ��λΪ�� */
#define CI_START_MODE 0        /* �û�����������ģʽ,0=��ѹ����,1=��������,2=ת������ */
#define CI_START_TIME 20       /* �û���������ʱ��,��λΪ�� */
#define CI_MAX_START_I 10      /* ������������, �û����������������, ��λΪ���� */
#define CF_ERROR_MAX_SCALE 0.2 /* ������������ʱPI�㷨����ĵ������ERROR�ı���ϵ��, �������ERROR = CF_MAX_ERROR_SCALE * (PI�㷨��ʼֵ-Ŀ��ֵ) */

#define CF_PWM_USE_3TIM /* ��ѡ CF_PWM_USE_3TIM �� CF_PWM_USE_SOFT, ��Ӧ���ֲ�ͬ��PWM�ײ�ʵ�� */
/* PWMʵ�ַ�ʽ1, ʹ��3����ʱ���Ͳ������ʵ�� */
#define CF_TIM_PWM_PHASE_A MSS_TIM3
#define CF_TIM_PWM_PHASE_A_HANDLE g_tim3_handle
#define CF_TIM_PWM_PULLUP MSS_TIM4
#define CF_TIM_PWM_PULLUP_HANDLE g_tim4_handle
#define CF_TIM_PWM_PULLDOWN MSS_TIM5
#define CF_TIM_PWM_PULLDOWN_HANDLE g_tim5_handle
/* PWMʵ�ַ�ʽ2, ʹ��1����ʱ��������¼���Ϊʱ��, �����ʵ�� */
#define CF_TIM_PWM MSS_TIM6             /* PWM����ʹ�õĶ�ʱ�� */
#define CF_TIM_PWM_HANDLE g_tim6_handle /* PWM����ʹ�õĶ�ʱ����� */

/******************************************************************************/
/*                                 ��������                                    */
/******************************************************************************/
/* �޸��������Ϳ����Զ��޸�printf�ض���ʹ�õĴ���, ע�����Ҫһ�� */
#define CF_USART_PRINTF USART1                /* ָ���ض���printf������ʹ�õĴ��� */
#define CF_USART_PRINTF_HANDLE g_uart1_handle /* ָ��printf�ض���ʹ�õĴ��ھ�� */
#define CF_PRINTF_BUFFER_SIZE 256             /* printf�ض���ʹ�õĻ��������С, ��Ҫ����256 */

/******************************************************************************/
/*                               Modbus����                                   */
/******************************************************************************/
/* �޸��������Ϳ����Զ��޸�FreeModbusʹ�õĴ���, ע�����Ҫһ�� */
#define CF_USART_MB USART2                /* ָ��FreeModbusʹ�õĴ��� */
#define CF_USART_MB_HANDLE g_uart2_handle /* ָ��FreeModbusʹ�õĴ��ھ�� */
/* FIXME: FreeModbusʹ�õĶ�ʱ��, ���������ʱ��������, ���Ը�Ϊʹ�������ʱ�� */
#define CF_TIM_MB MSS_TIM7             /* FreeModbusʹ�õĶ�ʱ�� */
#define CF_TIM_MB_HANDLE g_tim7_handle /* FreeModbusʹ�õĶ�ʱ����� */

/******************************************************************************/
/*                                  SPI����                                   */
/******************************************************************************/
#define CF_SPI1_USE_REMAP 0                /* ָ��SPI1ʹ�õ�������, 1 = ʹ����ӳ������, 0 = ʹ��Ĭ�ϸ������� */
#define CF_W25Q64_SPI SPI1                 /* ָ�������ⲿFLASH-w25q64ʹ�õ�SPI */
#define CF_W25Q64_SPI_HANDLE g_spi1_handle /* ָ�������ⲿFLASH-w25q64ʹ�õ�SPI��� */

/******************************************************************************/
/*                                  W25Q64����                                 */
/******************************************************************************/
#define CF_W25Q64_CS_GPIO GPIOA     /* ָ������W25Q64��ƬѡGPIO */
#define CF_W25Q64_CS_PIN GPIO_PIN_4 /* ָ������W25Q64��Ƭѡ���� */

/******************************************************************************/
/*                                  �����̻�����                               */
/******************************************************************************/
/* NOTE:��ͬ�̻�ʵ�ֲ��Զ����ô�СΪ 4KB����������洢����, ��ʼ��ַΪCF_PERSIST_BASE_XXX */
#define CF_PERSIST_BASE_INTERNAL 0x0807F000U /* �ڲ�FLash�洢�־ò����Ŀ�ʼ��ַ */
#define CF_PERSIST_BASE_W25Q64 0x00000000U   /* W25Q64�洢�־ò����Ŀ�ʼ��ַ */
// #define CF_PERSIST_USE_INTERNAL              /* ��ѡ CF_PERSIST_USE_INTERNAL �� CF_PERSIST_USE_W25Q64, ��Ӧ���ֲ�ͬ�Ĳ����̻�ʵ�ַ�ʽ */

/******************************************************************************/
/*                              �ж����ȼ�����                                 */
/******************************************************************************/
/* ע�������Ѿ���ʹ�õ�EXTI��: */
/* EXTI3-PD3, EXTI4-PD4, EXTI5-PD5, ��ͨ���� */
/* EXTI0-PD0, EXTI1-PD1, EXTI2-PD2, ��ѹ�����ᷴ�� */
/* ���ȼ�������0 - 15, ע��ֻ�����ȼ� 5 - 15 ����FreeRTOS���� */
#define CF_PWM_TIM_PRIORITY 0                 /* PWM����ʹ�õĶ�ʱ�����ж����ȼ�, ֱ��Ӱ�쵽�����ľ��� */
#define CF_HIGHLEVEL_DATATRANS_TIM_PRIORITY 1 /* ��ʵʱ�����ݴ������ö�ʱ�����ж����ȼ�, ֱ��Ӱ�쵽���ݲɼ���Ƶ�� */
#define CF_PWM_FEEDBACK_IT_PRIORITY 2         /* ��բ�ܵ�ͨ�Ƿ�����IO�ڵ��ж�*/
#define CF_Voltage_Feedback_IT_Priority 2     /* ��ѹ������ת�������� */
#define CF_PRINTF_IT_PRIORITY 5               /* printf�ض���uart, �����жϷ�ʽ���� */
#define CF_MB_UART_IT_PRIORITY 3              /* FreeModbus�ײ�ʹ�ô���ͨ��, ���ڲ����жϷ�ʽ���ͺͽ���, ���ô����ж����ȼ� */

/**
 * @brief   ��������
 */
typedef struct __MSS_Base_Config
{
    float Power_Voltage;      /* ��Դ��ѹ, ��λ���� */
    uint32_t Power_Frequency; /* ��ԴƵ��, ��λHZ */
    float Power_Factor;       /* ���ع�������, ��λ�Ƕ� */

} MSS_Base_Config;

/**
 * @brief   ����������
 */
typedef struct __MSS_Soft_Start_Config
{
    uint32_t Start_Period; /* ����ʱ��, ��λ�� */
    uint8_t Start_Mode;    /* ����ģʽ, 0 = ��ѹ����, 1 = ��������, 2 = ת�ؿ������� */
} MSS_Soft_Start_Config;

/**
 * @brief   ����ģ�����
 */
typedef struct __MSS_Protect_Config
{
    uint32_t Power_Voltage;
    uint32_t Power_Frequency;
} MSS_Protect_Config;

/**
 * @brief   ͨ�Ų���
 */
typedef struct __MSS_Comm_Config
{
    uint32_t Power_Voltage;
    uint32_t Power_Frequency;
} MSS_Comm_Config;

/**
 * @brief   �û��������
 */
typedef struct __MSS_User_Config
{
    uint32_t Power_Voltage;
    uint32_t Power_Frequency;
} MSS_User_Config;

/**
 * @brief   ���ϼ�¼����
 */
typedef struct __MSS_Log_Config
{
    uint8_t log_level; /* ��־��¼���� */
    uint32_t Power_Frequency;
} MSS_Log_Config;

typedef struct __MSS_Config
{
    MSS_Base_Config Base_Config;
    MSS_Soft_Start_Config Soft_Start_Config;
    MSS_Protect_Config Protect_Config;
    MSS_Comm_Config Comm_Config;
    MSS_User_Config User_Config;
    MSS_Log_Config Log_Config;
} MSS_Config;
extern MSS_Config *mss_Config;

/* ���ߺ���, ��ӡ���� */
void print_arr8(uint8_t *arr, uint16_t len);
void print_arr16(uint16_t *arr, uint16_t len);
void print_arr32(uint32_t *arr, uint16_t len);

#endif
