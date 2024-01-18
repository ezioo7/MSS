/**
 ****************************************************************************************************
 * @file        spi.c
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2024-01-06
 * @brief       SPI驱动
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 * 根据样板引脚定义:
 * PA15 NSS, PB3 CLK, PB4 MISO, PB5 MOSI
 * 以上引脚默认复用为SPI3, 重映射为SPI1
 * SPI1挂载在APB2(72MHZ), SPI2和SPI3挂载在APB1(36MHZ); 因此选择SPI1更好
 * 其中NSS不必固定为PA15, 选用任意IO口皆可
 ****************************************************************************************************
 * @attention
 * SPI1还可复用PA4 5 6 7引脚
 * PA4 NSS, PA5 SCK, PA6 MISO, PA7 MOSI
 ****************************************************************************************************
 */

#include "spi.h"
#include "globalE.h"
#include "./BSP/GPIO/gpio.h"
SPI_HandleTypeDef g_spi1_handle = {0};
SPI_HandleTypeDef g_spi3_handle = {0};

/**
 * @brief   初始化SPI1
 * @param   baud_Rate_Prescaler: 时钟信号预分频系数, SPI1挂载在APB2总线(72MHZ), 根据从设备支持的最大时钟频率选择分频系数;
 *  @arg    SPI_BAUDRATEPRESCALER_2: 得到36MHz SPI时钟, 对于W25Q64推荐用此项;
 * @note    根据宏CF_CF_SPI1_USE_REMAP决定使用默认复用引脚还是重映射引脚;
 * @note    REMAP引脚: PA15 NSS, PB3 CLK, PB4 MISO, PB5 MOSI;
 * @note    默认AFIO引脚: PA4 NSS, PA5 SCK, PA6 MISO, PA7 MOSI;
 */
void spi1_init(uint32_t baud_Rate_Prescaler)
{
    g_spi1_handle.Instance = SPI1;
    g_spi1_handle.Init.Mode = SPI_MODE_MASTER;                      /* 无需修改, 作为主机 */
    g_spi1_handle.Init.Direction = SPI_DIRECTION_2LINES;            /* 无需修改, 双线全双工 */
    g_spi1_handle.Init.DataSize = SPI_DATASIZE_8BIT;                /* 无需修改, 单次交换的字节数 */
    g_spi1_handle.Init.CLKPolarity = SPI_POLARITY_LOW;              /* 无需修改, CLK空闲低电平, CPOL = 0 */
    g_spi1_handle.Init.CLKPhase = SPI_PHASE_1EDGE;                  /* 无需修改, 第一个边沿采样, CPHA = 0; 和Polarity合并组成 Mode0 */
    g_spi1_handle.Init.NSS = SPI_NSS_SOFT;                          /* 无需修改, 单主机控制外设时选用此模式 */
    g_spi1_handle.Init.BaudRatePrescaler = baud_Rate_Prescaler;     /* 根据实际情况及选择, 例如, W25Q64支持80MHz, 所以采用2分频即可, SPI1挂载在APB2(频率72MHZ), 得到的SPI为36HMz */
    g_spi1_handle.Init.FirstBit = SPI_FIRSTBIT_MSB;                 /* 无需修改, 移位交换时先发高位还是低位, 默认选择先发高位 */
    g_spi1_handle.Init.TIMode = SPI_TIMODE_DISABLE;                 /* 无需修改, TI公司的SPI协议略有不同, F103并不支持TI公司的SPI */
    g_spi1_handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE; /* 无需修改, 是否开启CRC校验, 一般不开启, 这个要看从机是否支持, 例如W25Q64不支持 */
    // g_spi1_handle.Init.CRCPolynomial = 0x1234;                   /* CRC校验用到的多项式 */
    /* Msp底层初始化 */
    {
        __HAL_RCC_SPI1_CLK_ENABLE(); /* 打开SPI1时钟 */
        if (CF_SPI1_USE_REMAP)
        {
            /* PA15片选线, PB3 CLK, PB4 MISO, PB5 MOSI */
            gpio_init(GPIOA, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP); /* 由于使用SPI_NSS_SOFT模式, 所以本机NSS引脚没有作用(本机的主从模式仅由软件控制), 这里直接顺便用本机NSS引脚去控制 W25Q64 */
            gpio_init(GPIOB, GPIO_PIN_3, GPIO_MODE_AF_PP, GPIO_NOPULL);
            gpio_init(GPIOB, GPIO_PIN_4, GPIO_MODE_AF_INPUT, GPIO_NOPULL);
            gpio_init(GPIOB, GPIO_PIN_5, GPIO_MODE_AF_PP, GPIO_NOPULL);
            __HAL_AFIO_REMAP_SWJ_NOJTAG();  /* 保留SWD, 关闭JTAG*/
            __HAL_AFIO_REMAP_SPI1_ENABLE(); /* 重映射SPI1引脚 */
        }
        else
        {
            /* PA4 NSS, PA5 SCK, PA6 MISO, PA7 MOSI */
            gpio_init(GPIOA, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL); /* 由于使用SPI_NSS_SOFT模式, 所以本机NSS引脚没有作用(本机的主从模式仅由软件控制), 这里直接顺便用本机NSS引脚去控制 W25Q64 */
            gpio_init(GPIOA, GPIO_PIN_5, GPIO_MODE_AF_PP, GPIO_NOPULL);
            gpio_init(GPIOA, GPIO_PIN_6, GPIO_MODE_AF_INPUT, GPIO_NOPULL);
            gpio_init(GPIOA, GPIO_PIN_7, GPIO_MODE_AF_PP, GPIO_NOPULL);
        }
    }
    HAL_SPI_Init(&g_spi1_handle);
}
