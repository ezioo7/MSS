/**
 ****************************************************************************************************
 * @file        globalE.h
 * @author      电气组，zhuowenwei
 * @version     V1.0
 * @date        2024-12-09
 * @brief       全局变量
 * @licence     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
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
/*                              公共宏定义                                     */
/******************************************************************************/
#define BIT_SET(x, n) (x = x | (0x01 << n))     /*设置x变量的第n为位1*/
#define BIT_TEST(x, n) ((x & (0x01 << n)) != 0) /*检查x变量的第n位是否为1*/
#define BIT_CLEAR(x, n) (x = x & ~(0x01 << n))  /*设置x变量的第n为位0*/
#define TRUE 1
#define FALSE 0

typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned short int  uint16_t;
typedef unsigned char       uint8_t;

/******************************************************************************/
/*                               采集信息                                      */
/******************************************************************************/
extern float CI_DesireVoltage;   /* 接收用户输入期望相电压变量 */
extern float SV_DesireVoltage;   /* 期望相电压变量，系统变量 */
extern float AM_OutUO_Voltage;   /* 直接采集, 输出U相电压平均值, PB1, ADC9 */
extern float AM_OutVO_Voltage;   /* 直接采集, 输出V相电压平均值, PB0, ADC8 */
extern float AM_OutWO_Voltage;   /* 直接采集, 输出W相电压平均值, PC5, ADC15 */
extern float AM_OutUV_Voltage;   /* 间接计算, 输出UV相间电压平均值 */
extern float AM_OutUW_Voltage;   /* 间接计算, 输出UW相间电压平均值 */
extern float AM_OutWV_Voltage;   /* 间接计算, 输出WV相间电压平均值 */
extern float AM_OutU_PhaseI;     /* 直接采集, 输出U相电流平均值, PA7, ADC7 */
extern float AM_OutV_PhaseI;     /* 直接采集, 输出V相电流平均值, PA6, ADC6 */
extern float AM_OutW_PhaseI;     /* 直接采集, 输出W相电流平均值, PC4, ADC14 */
extern float SV_FAN_OutU_PhaseI; /* 直接采集, 7.5KW风扇电流, U路, PA5, ADC5 */
extern float SV_FAN_OutV_PhaseI; /* 直接采集, 7.5KW风扇电流, V路, PA4, ADC4 */
extern float AM_Pressure_A;      /* 直接采集, 扩展卡压力传感器AI2, PA1, ADC1 */
extern float AM_Pressure_B;      /* 直接采集, 扩展卡压力传感器AI3, PC0, ADC10 */
extern float AM_Pressure_C;      /* 直接采集, X3压力传感器, PC2, ADC12 */
extern float AM_Temp_A;          /* 直接采集, 扩展卡温度传感器, PA0, ADC0 */
extern float AM_Temp_B;          /* 直接采集, 扩展卡温度传感器, PC1, ADC11 */
extern float AM_Temp_C;          /* 直接采集, X3温度传感器, PC3, ADC13 */

/******************************************************************************/
/*                     UpdateState状态变化变量及宏定义                          */
/******************************************************************************/
extern uint16_t SV_MSSSatus; /* 1-表示默认状态*/
#define STATE_STANDY 1       /*1-表示Standy状态，*/
#define STATE_TRANSITION 2   /*2-表示Transition状态*/
#define STATE_RUNNING 3      /*3-表示Running状态*/
#define STATE_SYSTEMLOCK 10  /*10-SystemLock状态*/
#define STATE_TEST 11        /*11-TEST状态*/
extern uint16_t CI_Command;  /*用户外部输入命令*/
#define CMD_RUNING 1         /*运行指令*/
#define CMD_ESTOP 2          /*急停指令*/
#define CMD_PAUSED 3         /*暂停指令*/
#define CMD_STOP 4           /*停止指令*/
#define CMD_TESTING 5        /*测试指令*/

/******************************************************************************/
/*                      safety系统安全相关变量和宏定义                          */
/******************************************************************************/
extern uint16_t CF_OverPhI_Config; /*系统过载反时限设定*/
#define OVERPHI_L0 0               /*0-表示无反时限设定*/
#define OVERPHI_L1 1               /*1-表示启用1级无反时限设定，1.05倍额定电流反时限保护*/
#define OVERPHI_L2 2               /*2-表示启用2级无反时限设定，1.2倍额定电流反时限保护*/
#define OVERPHI_L3 3               /*3-表示启用3级无反时限设定，1.5倍额定电流反时限保护*/
#define OVERPHI_L4 4               /*4-表示启用4级无反时限设定，1.8倍额定电流反时限保护*/
#define OVERPHI_L5 5               /*5-表示启用5级无反时限设定，2倍额定电流反时限保护*/
#define OVERPHI_L6 6               /*6-表示启用6级无反时限设定，3倍额定电流反时限保护*/

extern uint32_t SV_Fault1;   /*一级错误，严重错误，系统进入锁死状态*/
#define FAT1_NO 0            /*表示无错误*/
#define FAT1_CANOUT 1        /*CAN通信超时*/
#define FAT1_OVERCURRENT 2   /*输出三相电流过载超时，具体见过载反时限说明*/
#define FAT1_OUTLOSSPHU 3    /*输出缺相U*/
#define FAT1_OUTLOSSPHV 4    /*输出缺相V*/
#define FAT1_OUTLOSSPHW 5    /*输出缺相W*/
#define FAT1_INTLOSSPHA 6    /*输入缺相A*/
#define FAT1_INTLOSSPHB 7    /*输入缺相B*/
#define FAT1_INTLOSSPHC 8    /*输入缺相C*/
#define FAT1_PHASESEQUENCE 9 /*三相相序错误*/

extern uint32_t SV_Fault2; /*二级错误，引起系统降低功耗运行*/
#define FAT2_NO 0          /*表示无错误*/
#define FAT2_OVERPHI 1     /*1.05~3倍过载错误，警告*/

extern uint32_t SV_Fault3; /*三级错误，警告*/

#define SUCCESS_OK 0 /*系统无错误*/
#define FAIL_ERR 1   /*系统故障*/

/******************************************************************************/
/*                                ADC采集配置                                  */
/******************************************************************************/
#define CF_DATA_TRANS_TIM MSS_TIM2                /* 定时处理ADC采集的数据, 配置使用的定时器 */
#define CF_DATA_TRANS_TIM_HANDLE g_tim2_handle    /* 数据收集定时器句柄 */
#define CF_ADC_CH_NUM 14                          /* ADC采集的通道数目 */
#define CF_HIGH_LEVEL_DATA_TRANS_PERIOD 250       /* 高实时性数据的处理时间间隔, 单位为us*/
extern uint16_t g_adc1_buffer[3 * CF_ADC_CH_NUM]; /* ADC1采集的缓存Buffer, ADC1采集到的数据按照通道配置的顺序放到此内存区域, 14路采集, 保留两份历史, 共42个数据 */

/******************************************************************************/
/*                                 PWM发波                                    */
/******************************************************************************/
#define CF_START_ANGLE 150              /* 初始的导通角, 可设置为 0 - 150 */
#define CF_TIM_PWM MSS_TIM6             /* PWM发波使用的定时器 */
#define CF_TIM_PWM_HANDLE g_tim6_handle /* PWM发波使用的定时器句柄 */

/******************************************************************************/
/*                                 串口配置                                    */
/******************************************************************************/
/* 修改这两个就可以自动修改printf重定向使用的串口, 注意二者要一致 */
#define CF_USART_PRINTF USART1                /* 指定重定向printf函数所使用的串口 */
#define CF_USART_PRINTF_HANDLE g_uart1_handle /* 指定printf重定向使用的串口句柄 */
/* printf重定向使用的缓冲数组大小, 不要超过256 */
#define CF_PRINTF_BUFFER_SIZE 128

/******************************************************************************/
/*                               Modbus配置                                   */
/******************************************************************************/
/* 修改这两个就可以自动修改FreeModbus使用的串口, 注意二者要一致 */
#define CF_USART_MB USART2                /* 指定FreeModbus使用的串口 */
#define CF_USART_MB_HANDLE g_uart2_handle /* 指定FreeModbus使用的串口句柄 */
/* FIXME: FreeModbus使用的定时器, 后续如果定时器不够用, 可以改为使用软件定时器 */
#define CF_TIM_MB MSS_TIM7             /* FreeModbus使用的定时器 */
#define CF_TIM_MB_HANDLE g_tim7_handle /* FreeModbus使用的定时器句柄 */

/******************************************************************************/
/*                              中断优先级配置                                 */
/* 注意现在已经被使用的EXTI有: EXTI3-PD3, EXTI4-PD4, EXTI5-PD5                  */
/******************************************************************************/
/* 优先级可配置0 - 15, 注意只有优先级 5 - 15 的由FreeRTOS管理 */
#define CF_PWM_TIM_PRIORITY 0                 /* PWM发波使用的定时器的中断优先级, 直接影响到发波的精度 */
#define CF_HIGHLEVEL_DATATRANS_TIM_PRIORITY 1 /* 高实时性数据处理所用定时器的中断优先级, 直接影响到数据采集的频率 */
#define CF_PWM_FEEDBACK_IT_PRIORITY 2         /* 晶闸管导通角反馈的IO口的中断*/

#endif
