/**
 ****************************************************************************************************
 * @file        w25q64.h
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2024-01-06
 * @brief       外部Flash驱动
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_W25Q64_H
#define __MSS_W25Q64_H

#include "./SYSTEM/sys/sys.h"
#include "./globalE.h"

#define W25Q64_BASE 0x00000000               /* 整个芯片的起始地址 */
#define W25Q64_END 0x007FFFFF                /* 整个芯片的末尾地址, 总大小0x80-0000, 8M个地址 */
#define W25Q64_SECTOR_SIZE 0x1000            /* 单位为B, 扇区大小4KB */
#define W25Q64_SECTOR_NUM 0x0800             /* 共 2K 个扇区 */
#define W25Q64_PAGE_SIZE 0x0100              /* 单位为B, 页大小256B */
#define W25Q64_PAGE_NUM 0x0010               /* 一个扇区内有 16个页面 */
#define W25Q64_DEFAULT_DATA 0xFF             /* 默认的无意义字节, 读操作时, 主机用此数据交换从机数据 */
#define W25Q64_TIMOUT 1000                   /* 单位ms, 超时时间 */
#define W25Q64_PERSIST_BASE 0x007FF000       /* 用于参数持久化的扇区的首地址 */
#define W25Q64_PERSIST_PAGE0_BASE 0x007FF000 /* 用于参数持久化的第一个页的首地址 */

#define W25Q64_ENABLE() HAL_GPIO_WritePin(CF_W25Q64_CS_GPIO, CF_W25Q64_CS_PIN, GPIO_PIN_RESET) /* 使能W25Q64片选信号 */
#define W25Q64_DISABLE() HAL_GPIO_WritePin(CF_W25Q64_CS_GPIO, CF_W25Q64_CS_PIN, GPIO_PIN_SET)  /* 失能W25Q64片选信号 */

void w25q64_init(void);
void w25q64_read_ID(uint8_t *read_Data);
void w25q64_read(uint32_t addr, uint8_t *read_Data, uint32_t len);
void w25q64_read_page(uint16_t sector_Index, uint8_t page_Index, uint8_t *read_Data);
void w25q64_write_addr_withinSector(uint32_t addr, uint8_t *write_Data, uint16_t len);

/* TODO: 保留, 暂时没必要实现; */
//void w25q64_write_addr(uint32_t addr, uint8_t *write_Data, uint32_t len);
/* TODO: 保留, 暂时没必要实现; */
//void w25q64_write_page(uint16_t sector_Index, uint8_t page_Index, uint8_t *write_Data, uint32_t len);

#endif
