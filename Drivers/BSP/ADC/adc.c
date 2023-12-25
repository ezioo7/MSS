/**
 ****************************************************************************************************
 * @file        adc.c
 * @author      电气组
 * @version     V1.0
 * @date        2023-04-23
 * @brief       ADC采集程序
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 * 每个ADC有一个采集规则, 可以理解为一张表, 表中可以有多个采集通道, 可以重复, 例如 'ch1, ch2, ch1, ch3'
 * 如果开启了扫描模式, ADC采集时会按顺序把表中的通道采集一遍;
 * 如果开启了连续模式, ADC采集会连续不断地一轮一轮按规则表采集;
 * 如果开启了间断模式, 假设配置了nbr =2, 则第一次触发ADC时转换规则表中前2个, 下一次触发再转换2个, 以此类推
 *
 ****************************************************************************************************
 * @attention:
 * 关于定时采集, 温度等t1采集一次, 电流等t2采集一次, 如何实现?
 * ADC规则通道采集最多支持16路, 假设每个采集通道都设置为最大采样时间, 20us完成一个通道,
 * 总共需要320us完成一轮采集, 实际应该更少, 用不完16路
 * 用一个uint16_t[] 保存采集数据, 转运由DMA完成, 如果使用连续+扫描+DMA, 整个采集任务不需要CPU参与
 * CPU只需要隔不同时间读取数组的不同位即可
 *
 ****************************************************************************************************
 * @attention
 * PA6(12_6)表示PA6用作ADC1和ADC2的IN6
 * PA6(12_6) PA7(12_7) PC4(12_14) 三相电流传感器
 * PD2 PC5(12_15) PD1 PB0(12_8) PD0 PB1(12_9) 晶闸管反馈电路
 * PD3 PD4 PD5 晶闸管反馈
 * PA4(12_4) PA5(12_5) 7.5KW风机电流检测
 * PA1(123_1) PC0(123_10) 扩展卡压力传感器
 * PA0(123_0) PC1(123_11) 扩展卡温度传感器
 * PC2(123_12) 类似扩展卡, 压力传感器
 * PC3(123_13) 类似扩展卡, 温度传感器
 * 发现采用ADC1即可完成所有ADC采集工作, 而ADC1和DMA1是固定搭配
 *
 ****************************************************************************************************
 *
 */
#include "globalE.h"
#include "./BSP/ADC/adc.h"
#include "./SYSTEM/sys/sys.h"

ADC_HandleTypeDef g_adc1_handle = {0};           /* ADC1 全局句柄 */
DMA_HandleTypeDef g_dma1_handle = {0};           /* DMA1 全局句柄 */
uint16_t g_adc1_buffer[3 * CF_ADC_CH_NUM] = {0}; /* ADC1采集的缓存Buffer, ADC1采集到的数据按照通道配置的顺序放到此内存区域, 最多支持16路采集 */

/**
 * @brief       ADC采集配置, 默认采用ADC1搭配DMA1; 可配置通道和通道顺序, 通道总数由配置宏CF_ADC_CH_NUM确定;
 * @param       channels: uint32_t 数组, 每个元素可取 0 ~ 15;
 *  @arg        0 代表 ADC1通道0, 1 代表 ADC1通道1, 以此类推; 推荐直接使用HAL库提供的宏 ADC_CHANNEL_0 ~ ADC_CHANNEL_15;
 */
void adc_dma_enable(const uint32_t *channels)
{
    g_adc1_handle.Instance = ADC1;
    g_adc1_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;       /* 右对齐, 12位转换结果放到16位寄存器中 */
    g_adc1_handle.Init.ContinuousConvMode = ENABLE;           /* 打开连续转换模式, 启动ADC转换后会自动一轮接一轮不停转换 */
    g_adc1_handle.Init.DiscontinuousConvMode = DISABLE;       /* 间断模式, 关闭就好, 不要修改 */
    g_adc1_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START; /* 使用软件触发, 即调用HAL_ADC_Start_DMA函数即可启动ADC转换 */
    g_adc1_handle.Init.NbrOfConversion = CF_ADC_CH_NUM;       /* 规则表中通道的数量*/
    g_adc1_handle.Init.NbrOfDiscConversion = 0;               /* 间断模式, 默认关闭即可, 不要修改*/
    g_adc1_handle.Init.ScanConvMode = ADC_SCAN_ENABLE;        /* 开启扫描模式, 一轮转换多个通道 */

    /* 配置ADC1时钟 */
    __HAL_RCC_ADC1_CLK_ENABLE();
    RCC_PeriphCLKInitTypeDef adc_clk_initer = {0};
    adc_clk_initer.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    adc_clk_initer.AdcClockSelection = RCC_ADCPCLK2_DIV6; /* DIV6 最终得到 12MHz, DIV8 最终得到 9MHz, 只能选这两个之一 */
    HAL_RCCEx_PeriphCLKConfig(&adc_clk_initer);

    /* TODO:使能ADC输入引脚的工作需要应用层做, 并且要在配置ADC之前做好; 配了哪些通道, 就要使能对应的引脚; */

    /* 配置DMA */
    __HAL_RCC_DMA1_CLK_ENABLE();
    /* 查图得ADC1对应DMA1的通道1 */
    g_dma1_handle.Instance = DMA1_Channel1;
    g_dma1_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;              /* DMA 方向, 外设到内存 */
    g_dma1_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;    /* 一次搬运的数据长度, 半字 = 2B */
    g_dma1_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; /* 一次搬运的数据长度, 半字 = 2B */
    g_dma1_handle.Init.MemInc = DMA_MINC_ENABLE;                      /* Mem端地址自增 */
    g_dma1_handle.Init.PeriphInc = DMA_PINC_DISABLE;                  /* P端地址固定 */
    g_dma1_handle.Init.Mode = DMA_CIRCULAR;                           /* 循环模式, 搬完一轮继续下一轮 */
    g_dma1_handle.Init.Priority = DMA_PRIORITY_HIGH;                  /* 该通道的优先级 */
    __HAL_LINKDMA(&g_adc1_handle, DMA_Handle, g_dma1_handle);         /* 双向绑定DMA与ADC的句柄,一定要做这一步 */
    HAL_DMA_Init(&g_dma1_handle);

    HAL_ADC_Init(&g_adc1_handle);

    /* 配置ADC采集通道 */
    ADC_ChannelConfTypeDef ch_conf = {0};
    ch_conf.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
    /* 注意i和i-1 */
    for (uint8_t i = 1; i <= CF_ADC_CH_NUM; i++)
    {
        ch_conf.Channel = channels[i - 1];               /* HAL库 #define ADC_CHANNEL_0   0x00000000U */
        ch_conf.Rank = i;                                /* 某通道在规则表中的位置与在数组channels中的位置一致, 从1开始数 */
        HAL_ADC_ConfigChannel(&g_adc1_handle, &ch_conf); /* HAL_ADC_ConfigChannel并不会修改ch_conf, 所以可以共用一个 */
    }

    HAL_ADCEx_Calibration_Start(&g_adc1_handle);                                     /* 校准ADC */
    HAL_ADC_Start_DMA(&g_adc1_handle, (uint32_t *)g_adc1_buffer, 3 * CF_ADC_CH_NUM); /* 启动ADC1采集*/
}
