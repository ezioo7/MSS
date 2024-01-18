/**
 ****************************************************************************************************
 * @file        config.c
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-01-08
 * @brief       参数配置模块
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 * 1. 持久化参数可保存在外置FLASH中, 通过SPI通信读写; W25Q64共有2K个扇区, 一个扇区大小为4KB, 可放 1K 个uint32_t参数, 肯定够了;
 * 2. 约定选用最后一个扇区作为参数保存位置, 其余扇区用于日志记录;
 * 3. 需要宏定义的: 参数起始地址
 * 4. 提供参数读取接口, 该接口将第一扇区中所有的参数读入内存一个结构体中
 * 5. 提供参数保存接口, 该接口检查系统当前状态, 如果当前状态允许存储, 则将参数结构体成员按顺序写入FLASH
 ****************************************************************************************************
 * @attention
 * 1. 持久化参数可保存在内部FLASH中, 直接读写; 内部Flash一页 2KB, 可放 512 个 uint32_t, 够用;
 * 2. 约定选用最后一页作为参数保存位置;
 * 3. 需要宏定义的: 参数起始地址
 * 4. 提供参数读取接口, 该接口将第一扇区中所有的参数读入内存一个结构体中
 * 5. 提供参数保存接口, 该接口检查系统当前状态, 如果当前状态允许存储, 则将参数结构体成员按顺序写入FLASH
 ****************************************************************************************************
 */
#include "config.h"
#include "globalE.h"
#include "./BSP/FLASH/W25Q64/w25q64.h"
#include "./BSP/FLASH/INTERNAL/flash.h"

MSS_Config g_Mss_Config = {0};
MSS_Config *mss_Config = &g_Mss_Config;

/**
 * @brief
 */
void load_config_flash(void)
{
    flash_read_1B(MSS_FLASH_PERSIST_BASE, (uint8_t *)mss_Config, sizeof(g_Mss_Config));
}

/**
 * @brief
 */
void store_config_flash(void)
{
    flash_write_addr(MSS_FLASH_PERSIST_BASE, (uint8_t *)mss_Config, sizeof(g_Mss_Config));
}

/**
 * @brief   从W25Q64加载配置到内存
 * @note    在调用此函数之前需要初始化W25Q64对应的SPI
 */
void load_config_w25q64(void)
{
    w25q64_read(W25Q64_PERSIST_BASE, (uint8_t *)mss_Config, sizeof(g_Mss_Config));
}

/**
 * @brief   持久化配置到 W25Q64
 * @note    W25Q64一次写指令最多写256B, 如果参数总量大于此值, 需要分页写入;
 */
void store_config_w25q64(void)
{
    w25q64_write_addr_withinSector(W25Q64_PERSIST_BASE, (uint8_t *)mss_Config, sizeof(g_Mss_Config));
}
