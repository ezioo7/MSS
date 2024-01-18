/**
 ****************************************************************************************************
 * @file        spi.c
 * @author      ������,wangdexu
 * @version     V1.0
 * @date        2024-01-06
 * @brief       SPI����
 * @license     Copyright (c) 2020-2032, �����г����������޹�˾
 ****************************************************************************************************
 * @attention
 * �����������Ŷ���:
 * PA15 NSS, PB3 CLK, PB4 MISO, PB5 MOSI
 * ��������Ĭ�ϸ���ΪSPI3, ��ӳ��ΪSPI1
 * SPI1������APB2(72MHZ), SPI2��SPI3������APB1(36MHZ); ���ѡ��SPI1����
 * ����NSS���ع̶�ΪPA15, ѡ������IO�ڽԿ�
 ****************************************************************************************************
 * @attention
 * SPI1���ɸ���PA4 5 6 7����
 * PA4 NSS, PA5 SCK, PA6 MISO, PA7 MOSI
 ****************************************************************************************************
 */

#include "spi.h"
#include "globalE.h"
#include "./BSP/GPIO/gpio.h"
SPI_HandleTypeDef g_spi1_handle = {0};
SPI_HandleTypeDef g_spi3_handle = {0};

/**
 * @brief   ��ʼ��SPI1
 * @param   baud_Rate_Prescaler: ʱ���ź�Ԥ��Ƶϵ��, SPI1������APB2����(72MHZ), ���ݴ��豸֧�ֵ����ʱ��Ƶ��ѡ���Ƶϵ��;
 *  @arg    SPI_BAUDRATEPRESCALER_2: �õ�36MHz SPIʱ��, ����W25Q64�Ƽ��ô���;
 * @note    ���ݺ�CF_CF_SPI1_USE_REMAP����ʹ��Ĭ�ϸ������Ż�����ӳ������;
 * @note    REMAP����: PA15 NSS, PB3 CLK, PB4 MISO, PB5 MOSI;
 * @note    Ĭ��AFIO����: PA4 NSS, PA5 SCK, PA6 MISO, PA7 MOSI;
 */
void spi1_init(uint32_t baud_Rate_Prescaler)
{
    g_spi1_handle.Instance = SPI1;
    g_spi1_handle.Init.Mode = SPI_MODE_MASTER;                      /* �����޸�, ��Ϊ���� */
    g_spi1_handle.Init.Direction = SPI_DIRECTION_2LINES;            /* �����޸�, ˫��ȫ˫�� */
    g_spi1_handle.Init.DataSize = SPI_DATASIZE_8BIT;                /* �����޸�, ���ν������ֽ��� */
    g_spi1_handle.Init.CLKPolarity = SPI_POLARITY_LOW;              /* �����޸�, CLK���е͵�ƽ, CPOL = 0 */
    g_spi1_handle.Init.CLKPhase = SPI_PHASE_1EDGE;                  /* �����޸�, ��һ�����ز���, CPHA = 0; ��Polarity�ϲ���� Mode0 */
    g_spi1_handle.Init.NSS = SPI_NSS_SOFT;                          /* �����޸�, ��������������ʱѡ�ô�ģʽ */
    g_spi1_handle.Init.BaudRatePrescaler = baud_Rate_Prescaler;     /* ����ʵ�������ѡ��, ����, W25Q64֧��80MHz, ���Բ���2��Ƶ����, SPI1������APB2(Ƶ��72MHZ), �õ���SPIΪ36HMz */
    g_spi1_handle.Init.FirstBit = SPI_FIRSTBIT_MSB;                 /* �����޸�, ��λ����ʱ�ȷ���λ���ǵ�λ, Ĭ��ѡ���ȷ���λ */
    g_spi1_handle.Init.TIMode = SPI_TIMODE_DISABLE;                 /* �����޸�, TI��˾��SPIЭ�����в�ͬ, F103����֧��TI��˾��SPI */
    g_spi1_handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; /* �����޸�, �Ƿ���CRCУ��, һ�㲻����, ���Ҫ���ӻ��Ƿ�֧��, ����W25Q64��֧�� */
    // g_spi1_handle.Init.CRCPolynomial = 0x1234;                   /* CRCУ���õ��Ķ���ʽ */
    /* Msp�ײ��ʼ�� */
    {
        __HAL_RCC_SPI1_CLK_ENABLE(); /* ��SPI1ʱ�� */
        if (CF_SPI1_USE_REMAP)
        {
            /* PA15Ƭѡ��, PB3 CLK, PB4 MISO, PB5 MOSI */
            gpio_init(GPIOA, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP); /* ����ʹ��SPI_NSS_SOFTģʽ, ���Ա���NSS����û������(����������ģʽ�����������), ����ֱ��˳���ñ���NSS����ȥ���� W25Q64 */
            gpio_init(GPIOB, GPIO_PIN_3, GPIO_MODE_AF_PP, GPIO_NOPULL);
            gpio_init(GPIOB, GPIO_PIN_4, GPIO_MODE_AF_INPUT, GPIO_NOPULL);
            gpio_init(GPIOB, GPIO_PIN_5, GPIO_MODE_AF_PP, GPIO_NOPULL);
            __HAL_AFIO_REMAP_SWJ_NOJTAG();  /* ����SWD, �ر�JTAG*/
            __HAL_AFIO_REMAP_SPI1_ENABLE(); /* ��ӳ��SPI1���� */
        }
        else
        {
            /* PA4 NSS, PA5 SCK, PA6 MISO, PA7 MOSI */
            gpio_init(GPIOA, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL); /* ����ʹ��SPI_NSS_SOFTģʽ, ���Ա���NSS����û������(����������ģʽ�����������), ����ֱ��˳���ñ���NSS����ȥ���� W25Q64 */
            gpio_init(GPIOA, GPIO_PIN_5, GPIO_MODE_AF_PP, GPIO_NOPULL);
            gpio_init(GPIOA, GPIO_PIN_6, GPIO_MODE_AF_INPUT, GPIO_NOPULL);
            gpio_init(GPIOA, GPIO_PIN_7, GPIO_MODE_AF_PP, GPIO_NOPULL);
        }
    }
    HAL_SPI_Init(&g_spi1_handle);
}
