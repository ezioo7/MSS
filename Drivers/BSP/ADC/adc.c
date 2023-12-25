/**
 ****************************************************************************************************
 * @file        adc.c
 * @author      ������
 * @version     V1.0
 * @date        2023-04-23
 * @brief       ADC�ɼ�����
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 * ÿ��ADC��һ���ɼ�����, �������Ϊһ�ű�, ���п����ж���ɼ�ͨ��, �����ظ�, ���� 'ch1, ch2, ch1, ch3'
 * ���������ɨ��ģʽ, ADC�ɼ�ʱ�ᰴ˳��ѱ��е�ͨ���ɼ�һ��;
 * �������������ģʽ, ADC�ɼ����������ϵ�һ��һ�ְ������ɼ�;
 * ��������˼��ģʽ, ����������nbr =2, ���һ�δ���ADCʱת���������ǰ2��, ��һ�δ�����ת��2��, �Դ�����
 *
 ****************************************************************************************************
 * @attention:
 * ���ڶ�ʱ�ɼ�, �¶ȵ�t1�ɼ�һ��, ������t2�ɼ�һ��, ���ʵ��?
 * ADC����ͨ���ɼ����֧��16·, ����ÿ���ɼ�ͨ��������Ϊ������ʱ��, 20us���һ��ͨ��,
 * �ܹ���Ҫ320us���һ�ֲɼ�, ʵ��Ӧ�ø���, �ò���16·
 * ��һ��uint16_t[] ����ɼ�����, ת����DMA���, ���ʹ������+ɨ��+DMA, �����ɼ�������ҪCPU����
 * CPUֻ��Ҫ����ͬʱ���ȡ����Ĳ�ͬλ����
 *
 ****************************************************************************************************
 * @attention
 * PA6(12_6)��ʾPA6����ADC1��ADC2��IN6
 * PA6(12_6) PA7(12_7) PC4(12_14) �������������
 * PD2 PC5(12_15) PD1 PB0(12_8) PD0 PB1(12_9) ��բ�ܷ�����·
 * PD3 PD4 PD5 ��բ�ܷ���
 * PA4(12_4) PA5(12_5) 7.5KW����������
 * PA1(123_1) PC0(123_10) ��չ��ѹ��������
 * PA0(123_0) PC1(123_11) ��չ���¶ȴ�����
 * PC2(123_12) ������չ��, ѹ��������
 * PC3(123_13) ������չ��, �¶ȴ�����
 * ���ֲ���ADC1�����������ADC�ɼ�����, ��ADC1��DMA1�ǹ̶�����
 *
 ****************************************************************************************************
 *
 */
#include "globalE.h"
#include "./BSP/ADC/adc.h"
#include "./SYSTEM/sys/sys.h"

ADC_HandleTypeDef g_adc1_handle = {0};           /* ADC1 ȫ�־�� */
DMA_HandleTypeDef g_dma1_handle = {0};           /* DMA1 ȫ�־�� */
uint16_t g_adc1_buffer[3 * CF_ADC_CH_NUM] = {0}; /* ADC1�ɼ��Ļ���Buffer, ADC1�ɼ��������ݰ���ͨ�����õ�˳��ŵ����ڴ�����, ���֧��16·�ɼ� */

/**
 * @brief       ADC�ɼ�����, Ĭ�ϲ���ADC1����DMA1; ������ͨ����ͨ��˳��, ͨ�����������ú�CF_ADC_CH_NUMȷ��;
 * @param       channels: uint32_t ����, ÿ��Ԫ�ؿ�ȡ 0 ~ 15;
 *  @arg        0 ���� ADC1ͨ��0, 1 ���� ADC1ͨ��1, �Դ�����; �Ƽ�ֱ��ʹ��HAL���ṩ�ĺ� ADC_CHANNEL_0 ~ ADC_CHANNEL_15;
 */
void adc_dma_enable(const uint32_t *channels)
{
    g_adc1_handle.Instance = ADC1;
    g_adc1_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;       /* �Ҷ���, 12λת������ŵ�16λ�Ĵ����� */
    g_adc1_handle.Init.ContinuousConvMode = ENABLE;           /* ������ת��ģʽ, ����ADCת������Զ�һ�ֽ�һ�ֲ�ͣת�� */
    g_adc1_handle.Init.DiscontinuousConvMode = DISABLE;       /* ���ģʽ, �رվͺ�, ��Ҫ�޸� */
    g_adc1_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START; /* ʹ���������, ������HAL_ADC_Start_DMA������������ADCת�� */
    g_adc1_handle.Init.NbrOfConversion = CF_ADC_CH_NUM;       /* �������ͨ��������*/
    g_adc1_handle.Init.NbrOfDiscConversion = 0;               /* ���ģʽ, Ĭ�Ϲرռ���, ��Ҫ�޸�*/
    g_adc1_handle.Init.ScanConvMode = ADC_SCAN_ENABLE;        /* ����ɨ��ģʽ, һ��ת�����ͨ�� */

    /* ����ADC1ʱ�� */
    __HAL_RCC_ADC1_CLK_ENABLE();
    RCC_PeriphCLKInitTypeDef adc_clk_initer = {0};
    adc_clk_initer.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    adc_clk_initer.AdcClockSelection = RCC_ADCPCLK2_DIV6; /* DIV6 ���յõ� 12MHz, DIV8 ���յõ� 9MHz, ֻ��ѡ������֮һ */
    HAL_RCCEx_PeriphCLKConfig(&adc_clk_initer);

    /* TODO:ʹ��ADC�������ŵĹ�����ҪӦ�ò���, ����Ҫ������ADC֮ǰ����; ������Щͨ��, ��Ҫʹ�ܶ�Ӧ������; */

    /* ����DMA */
    __HAL_RCC_DMA1_CLK_ENABLE();
    /* ��ͼ��ADC1��ӦDMA1��ͨ��1 */
    g_dma1_handle.Instance = DMA1_Channel1;
    g_dma1_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;              /* DMA ����, ���赽�ڴ� */
    g_dma1_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;    /* һ�ΰ��˵����ݳ���, ���� = 2B */
    g_dma1_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; /* һ�ΰ��˵����ݳ���, ���� = 2B */
    g_dma1_handle.Init.MemInc = DMA_MINC_ENABLE;                      /* Mem�˵�ַ���� */
    g_dma1_handle.Init.PeriphInc = DMA_PINC_DISABLE;                  /* P�˵�ַ�̶� */
    g_dma1_handle.Init.Mode = DMA_CIRCULAR;                           /* ѭ��ģʽ, ����һ�ּ�����һ�� */
    g_dma1_handle.Init.Priority = DMA_PRIORITY_HIGH;                  /* ��ͨ�������ȼ� */
    __HAL_LINKDMA(&g_adc1_handle, DMA_Handle, g_dma1_handle);         /* ˫���DMA��ADC�ľ��,һ��Ҫ����һ�� */
    HAL_DMA_Init(&g_dma1_handle);

    HAL_ADC_Init(&g_adc1_handle);

    /* ����ADC�ɼ�ͨ�� */
    ADC_ChannelConfTypeDef ch_conf = {0};
    ch_conf.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    /* ע��i��i-1 */
    for (uint8_t i = 1; i <= CF_ADC_CH_NUM; i++)
    {
        ch_conf.Channel = channels[i - 1];               /* HAL�� #define ADC_CHANNEL_0   0x00000000U */
        ch_conf.Rank = i;                                /* ĳͨ���ڹ�����е�λ����������channels�е�λ��һ��, ��1��ʼ�� */
        HAL_ADC_ConfigChannel(&g_adc1_handle, &ch_conf); /* HAL_ADC_ConfigChannel�������޸�ch_conf, ���Կ��Թ���һ�� */
    }

    HAL_ADCEx_Calibration_Start(&g_adc1_handle);                                     /* У׼ADC */
    HAL_ADC_Start_DMA(&g_adc1_handle, (uint32_t *)g_adc1_buffer, 3 * CF_ADC_CH_NUM); /* ����ADC1�ɼ�*/
}
