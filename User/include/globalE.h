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
#ifndef __MSS_GLOBALE_H
#define __MSS_GLOBALE_H

/******************************************************************************/
/*                              �����궨��                                     */
/******************************************************************************/
#define BIT_SET(x, n) (x = x | (0x01 << n))     /*����x�����ĵ�nΪλ1*/
#define BIT_TEST(x, n) ((x & (0x01 << n)) != 0) /*���x�����ĵ�nλ�Ƿ�Ϊ1*/
#define BIT_CLEAR(x, n) (x = x & ~(0x01 << n))  /*����x�����ĵ�nΪλ0*/
#define TRUE 1
#define FALSE 0

typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned short int  uint16_t;
typedef unsigned char       uint8_t;

/******************************************************************************/
/*                               �ɼ���Ϣ                                      */
/******************************************************************************/
extern float CI_DesireVoltage;   /* �����û������������ѹ���� */
extern float SV_DesireVoltage;   /* �������ѹ������ϵͳ���� */
extern float AM_OutUO_Voltage;   /* ֱ�Ӳɼ�, ���U���ѹƽ��ֵ, PB1, ADC9 */
extern float AM_OutVO_Voltage;   /* ֱ�Ӳɼ�, ���V���ѹƽ��ֵ, PB0, ADC8 */
extern float AM_OutWO_Voltage;   /* ֱ�Ӳɼ�, ���W���ѹƽ��ֵ, PC5, ADC15 */
extern float AM_OutUV_Voltage;   /* ��Ӽ���, ���UV����ѹƽ��ֵ */
extern float AM_OutUW_Voltage;   /* ��Ӽ���, ���UW����ѹƽ��ֵ */
extern float AM_OutWV_Voltage;   /* ��Ӽ���, ���WV����ѹƽ��ֵ */
extern float AM_OutU_PhaseI;     /* ֱ�Ӳɼ�, ���U�����ƽ��ֵ, PA7, ADC7 */
extern float AM_OutV_PhaseI;     /* ֱ�Ӳɼ�, ���V�����ƽ��ֵ, PA6, ADC6 */
extern float AM_OutW_PhaseI;     /* ֱ�Ӳɼ�, ���W�����ƽ��ֵ, PC4, ADC14 */
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
#define CF_START_ANGLE 150              /* ��ʼ�ĵ�ͨ��, ������Ϊ 0 - 150 */
#define CF_TIM_PWM MSS_TIM6             /* PWM����ʹ�õĶ�ʱ�� */
#define CF_TIM_PWM_HANDLE g_tim6_handle /* PWM����ʹ�õĶ�ʱ����� */

/******************************************************************************/
/*                                 ��������                                    */
/******************************************************************************/
/* �޸��������Ϳ����Զ��޸�printf�ض���ʹ�õĴ���, ע�����Ҫһ�� */
#define CF_USART_PRINTF USART1                /* ָ���ض���printf������ʹ�õĴ��� */
#define CF_USART_PRINTF_HANDLE g_uart1_handle /* ָ��printf�ض���ʹ�õĴ��ھ�� */
/* printf�ض���ʹ�õĻ��������С, ��Ҫ����256 */
#define CF_PRINTF_BUFFER_SIZE 128

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
/*                              �ж����ȼ�����                                 */
/* ע�������Ѿ���ʹ�õ�EXTI��: EXTI3-PD3, EXTI4-PD4, EXTI5-PD5                  */
/******************************************************************************/
/* ���ȼ�������0 - 15, ע��ֻ�����ȼ� 5 - 15 ����FreeRTOS���� */
#define CF_PWM_TIM_PRIORITY 0                 /* PWM����ʹ�õĶ�ʱ�����ж����ȼ�, ֱ��Ӱ�쵽�����ľ��� */
#define CF_HIGHLEVEL_DATATRANS_TIM_PRIORITY 1 /* ��ʵʱ�����ݴ������ö�ʱ�����ж����ȼ�, ֱ��Ӱ�쵽���ݲɼ���Ƶ�� */
#define CF_PWM_FEEDBACK_IT_PRIORITY 2         /* ��բ�ܵ�ͨ�Ƿ�����IO�ڵ��ж�*/

#endif
