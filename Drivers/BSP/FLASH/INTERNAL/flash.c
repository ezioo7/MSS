/**
 ****************************************************************************************************
 * @file        flash.c
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-12-29
 * @brief       Flash驱动
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 * Flash存储分内部和外部FLASH;
 * 内部Flash直接读写, 外部Flash用SPI通信进行读写, 分两套API;
 * 本驱动文件负责内部Flash驱动
 ****************************************************************************************************
 * @attention
 * 内部Flash:
 * 总大小512KB, 由页组成, 一页2KB, 共256页, 写入Code时从低地址开始写, 采用小端模式;
 * 如果要更改多个地址的数据, 将其所在页(多页)擦除, 然后调用write函数进行写操作即可;
 * Flash写一个地址的要求是, 只要该地址上次进行过擦除即可, 并不要求每次写操作之前都紧跟一个擦除操作;
 * 写入的最小单位是2B, 擦除的最小单位是页, 一页2KB, 所有写操作都以2B为基本单位;
 * 相比较w25q64,省去了片选和写使能的操作, 但需要额外添加 volatile 关键字并且进行解锁;
 * 相比较w25q64,同样要求写入结束位置在页尾之前, 如果超出是否回卷暂不知;
 * 写入擦除操作都会导致忙状态, 进行擦除和写操作之前要等待上一个操作完成, 不过HAL库函数内部集成了此操作;
 ****************************************************************************************************
 * @attention
 * 对于参数持久化, 一页 2KB 可以放 512 个 32位数据, 直接选取最后一页作为参数配置页面
 */
#include "./BSP/FLASH/INTERNAL/flash.h"
#include "stdio.h"

/* static functions */
/* 基础的操作, 集成度低, 不暴露给应用层 */
static uint32_t flash_erase_page(uint32_t addr);
static uint32_t flash_write8_addr_withinPage(uint32_t addr, uint8_t *write_Data, uint16_t size);
static uint32_t flash_write16_addr_withinPage(uint32_t addr, uint16_t *write_Data, uint16_t size);
static uint32_t flash_write8_page_withinPage(uint8_t page_Index, uint8_t *write_Data, uint16_t size);

/**
 * @brief   以1B为单位读连续数据;
 * @param   addr: 要读取的地址;
 * @param   read_Data: 将数据读取到此指针指向的地址开始的连续区域;
 * @param   size: 要读取的数据长度, 单位为8bit;
 */
void flash_read_1B(uint32_t addr, uint8_t *read_Data, uint16_t size)
{
    if (size == 0)
    {
        printf("Flash读取失败, 原因: size == 0\r\n");
        return;
    }
    for (uint16_t i = 0; i < size; i++)
    {
        read_Data[i] = *((__IO uint8_t *)(addr + i));
    }
}

/**
 * @brief   以2B为单位读连续数据;
 * @param   addr: 要读取的地址;
 * @param   read_Data: 将数据读取到此指针指向的地址开始的连续区域;
 * @param   size: 要读取的数据长度, 单位为16bit;
 */
void flash_read_2B(uint32_t addr, uint16_t *read_Data, uint16_t size)
{
    if (size == 0)
    {
        printf("Flash读取失败, 原因: size == 0\r\n");
        return;
    }
    for (uint16_t i = 0; i < size; i++)
    {
        read_Data[i] = *((__IO uint16_t *)(addr + 2 * i));
    }
}

/**
 * @brief   以4B为单位读连续数据;
 * @param   addr: 要读取的地址;
 * @param   read_Data: 将数据读取到此指针指向的地址开始的连续区域;
 * @param   size: 要读取的数据长度, 单位为32bit;
 */
void flash_read_4B(uint32_t addr, uint32_t *read_Data, uint16_t size)
{
    if (size == 0)
    {
        printf("Flash读取失败, 原因: size == 0\r\n");
        return;
    }
    for (uint16_t i = 0; i < size; i++)
    {
        read_Data[i] = *((__IO uint32_t *)(addr + 4 * i));
    }
}

/**
 * @brief   以1B为单位读一整页数据;
 * @param   page_Index: 页号, 0 ~ 255;
 * @param   read_Data: 将数据读取到此指针指向的地址开始的连续区域;
 */
void flash_read_page(uint8_t page_Index, uint8_t *read_Data)
{
    if (page_Index > MSS_FLASH_PAGE_NUM - 1)
    {
        printf("Flash读整页失败, 原因: 页号非法\r\n");
        return;
    }
    flash_read_1B(page_Index * MSS_FLASH_PAGE_SIZE_1B, read_Data, MSS_FLASH_PAGE_SIZE_1B);
}

/**
 * @brief   写任意长度的数据到指定位置, 支持跨页;
 * @param   addr: 要写入的地址;
 * @param   write_Data: 将此指针指向的内存地址开始的连续区域中的数据写到Flash中;
 * @param   size: 要写入的数据长度, 单位为 1B;
 * @note    本函数会在实际写入前自动执行擦除操作, 无需手动擦除;
 * @note    要写入的页内的其余位置的数据将被保留;
 */
void flash_write_addr(uint32_t addr, uint8_t *write_Data, uint32_t size)
{
    /**
     * 1.记addr所在页为起始页, 先写满起始页, 记住write_data的偏移量和剩余数据的size;
     * 2.剩下的数据直接调用flash_write_page;
     */
    /* 检查越界 */
    if (size == 0)
    {
        printf("Flash写入失败, 原因: size == 0\r\n");
        return;
    }
    if ((addr + size - 1) > MSS_FLASH_END)
    {
        printf("Flash写入失败, 原因: size过大\r\n");
        return;
    }
    /* 处理起始页 */
    /* 第一页应该写入的数据量 */
    uint16_t first_Size = MSS_FLASH_PAGE_SIZE_1B - addr % MSS_FLASH_PAGE_SIZE_1B;
    flash_write8_addr_withinPage(addr, write_Data, first_Size);
    /* 处理剩余的页 */
    uint8_t first_Page_Index = addr / MSS_FLASH_PAGE_SIZE_1B;
    flash_write_page(first_Page_Index + 1, write_Data + first_Size, size - first_Size);
}

/**
 * @brief   写任意长度的数据到指定页, 支持跨页;
 * @param   first_Page_Index: 要写入的页的页号;
 * @param   write_Data: 将此指针指向的内存地址开始的连续区域中的数据写到Flash中;
 * @param   size: 要写入的数据长度, 单位为 1B;
 * @note    本函数会在实际写入前自动执行擦除操作, 无需手动擦除;
 * @note    要写入的页内的其余位置的数据将被保留;
 */
void flash_write_page(uint8_t first_Page_Index, uint8_t *write_Data, uint32_t size)
{
    /**
     * 1.计算共写多少页;
     * 2.检查最后一页页号是否合法;
     * 3.遍历, 每次读取一整页, 在内存中修改, 修改后写入;
     */
    if (size == 0)
    {
        printf("Flash写入失败, 原因: size == 0\r\n");
        return;
    }
    uint16_t total_Page = (size - 1) / MSS_FLASH_PAGE_SIZE_1B + 1; /* 计算要写的数据总共占多少页 */
    uint8_t end_Page_Index = first_Page_Index + total_Page - 1;
    /* 检查最后一页的页号是否在 0 ~ 255 之内 */
    if (end_Page_Index > MSS_FLASH_PAGE_NUM - 1)
    {
        printf("Flash写入失败, 原因: size过大\r\n");
        return;
    }
    /* 遍历每一页, 写入 */
    uint16_t current_size = MSS_FLASH_PAGE_SIZE_1B;
    for (uint8_t i = 0; i < total_Page; i++)
    {
        /* 最后一页的size要特殊处理 */
        if (i == (total_Page - 1))
        {
            current_size = size - (total_Page - 1) * MSS_FLASH_PAGE_SIZE_1B; /* ALERT: 不能用 current_size = size % MSS_FLASH_PAGE_SIZE_1B 来计算最后一页的size, 如果最后一页恰好要写满, 取模会得到 0 */
        }
        flash_write8_page_withinPage(first_Page_Index + i, write_Data + i * MSS_FLASH_PAGE_SIZE_1B, current_size);
    }
}

/**
 * @brief   写长度小于等于FLASH_PAGE_SIZE的数据到指定位置, 不可跨页;
 * @param   TODO: addr: 要写入的地址;
 * @param   data: 将此指针指向的内存地址开始的连续区域中的数据写到Flash中;
 * @param   size: 要写入的数据长度, 单位为 1B;
 * @note    本函数会在实际写入前自动执行擦除操作, 无需手动擦除;
 * @note    要写入的页内的其余位置的数据将被保留;
 */
static uint32_t flash_write8_addr_withinPage(uint32_t addr, uint8_t *write_Data, uint16_t size)
{
    /**
     * 1.读取所在页数据至内存
     * 2.修改内存
     * 3.擦除所在页
     * 4.写入
     */
    if (size == 0)
    {
        printf("Flash写入失败, 原因: size == 0\r\n");
        return 1;
    }
    /* 判断写入结束位置是否在同一页内 */
    uint32_t page_Base = addr - addr % MSS_FLASH_PAGE_SIZE_1B; /* 页首地址 */
    uint32_t write_End = addr + size - 1;                      /* 写入结束位置 */
    if ((write_End - write_End % MSS_FLASH_PAGE_SIZE_1B) != page_Base)
    {
        printf("Flash写入失败, 原因: 跨页\r\n");
        return 1;
    }

    /* 读取数据 */
    uint8_t buffer[MSS_FLASH_PAGE_SIZE_1B] = {0};
    flash_read_1B(page_Base, buffer, MSS_FLASH_PAGE_SIZE_1B);
    /* 在内存中修改 */
    uint16_t offset = addr - page_Base; /* 写入位置相对于页首的偏移量, 以1B为单位 */
    for (uint16_t i = 0; i < size; i++)
    {
        buffer[offset + i] = write_Data[i];
    }
    /* 擦除 */
    uint32_t page_Error = flash_erase_page(page_Base);

    /**
     * NOTE: 转化为按照2B为基本单位写入;
     * 为什么能这样做? RAM和Flash均采用小端存储, 以保存 uint8_t 0x11, 0x12, 0x10为例, 假设左边表示物理低地址;
     * 在RAM中, 有 | 11 | 12 | 10 | FF(脏数据) |; 转换为 16位指针, 由于RAM采用小端模式, 因此这块地址将被解析为两个 uint16_t 0x1211, 0xFF10;
     * 此时以HALFWORD调用 HAL_FLASH_Program, 将向FLASH写入 0x1211, 0xFF10;
     * FLASH同样采用小端模式, 写入0x1211时, 先写 11, 再写12; 0xFF10同理, 故在FLASH中有 | 11 | 12 | 10 | FF |;
     * 此时再按 8bit 读 FLASH, 读到的就是 0x11, 0x12, 0x10;
     * 但是仍然有一个问题, 如果是奇数个字节, 那么转成 16位指针后, 会多出1B, 加入了一个脏数据, 解决方法是先将FLASH一整页的内容读入内存;
     * 这样多的 1B 就是原本Flash中的内容, 不是脏数据;
     */
    HAL_FLASH_Unlock();
    uint16_t *ptr = (uint16_t *)buffer;
    for (uint16_t i = 0; i < MSS_FLASH_PAGE_SIZE_2B; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, page_Base + 2 * i, ptr[i]);
    }
    HAL_FLASH_Lock();
    return page_Error;
}

/**
 * @brief   写长度小于等于FLASH_PAGE_SIZE的数据到指定位置, 不可跨页;
 * @param   TODO: addr: 要写入的地址, 必须是2B的整数倍
 * @param   data: 将此指针指向的内存地址开始的连续区域中的数据写到Flash中;
 * @param   size: 要写入的数据长度, 单位为 2B;
 * @note    本函数会在实际写入前自动执行擦除操作, 无需手动擦除;
 * @note    要写入的页内的其余位置的数据将被保留;
 */
static uint32_t flash_write16_addr_withinPage(uint32_t addr, uint16_t *write_Data, uint16_t size)
{
    /**
     * 1.读取所在页数据至内存
     * 2.修改内存
     * 3.擦除所在页
     * 4.写入
     */
    if (size == 0)
    {
        printf("Flash写入失败, 原因: size == 0\r\n");
        return 1;
    }
    /* 判断写入结束位置是否在同一页内 */
    uint32_t page_Base = addr - addr % MSS_FLASH_PAGE_SIZE_1B; /* 页首地址 */
    uint32_t write_End = addr + 2 * size - 1;                  /* 写入结束位置 */
    if ((write_End - write_End % MSS_FLASH_PAGE_SIZE_1B) != page_Base)
    {
        printf("Flash写入失败, 原因: 跨页\r\n");
        return 1;
    }
    /* 判断传入地址是否是2B的整数倍 */
    if (addr % 2 != 0)
    {
        printf("Flash写入失败, 原因: 地址非2B整数倍\r\n");
        return 1;
    }

    /* 读取数据 */
    uint16_t buffer[MSS_FLASH_PAGE_SIZE_2B] = {0};
    flash_read_2B(page_Base, buffer, MSS_FLASH_PAGE_SIZE_2B);
    /* 在内存中修改 */
    uint16_t offset = (addr - page_Base) / 2; /* 写入位置相对于页首的偏移量, 以2B为单位 */
    for (uint16_t i = 0; i < size; i++)
    {
        buffer[offset + i] = write_Data[i];
    }
    /* 擦除 */
    uint32_t page_Error = flash_erase_page(page_Base);
    /* 写入 */
    HAL_FLASH_Unlock();
    for (uint16_t i = 0; i < MSS_FLASH_PAGE_SIZE_2B; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, page_Base + 2 * i, buffer[i]);
    }
    HAL_FLASH_Lock();
    return page_Error;
}

/**
 * @brief   写任意长度小于等于FLASH_PAGE_SIZE的数据到指定页的开始位置
 * @param   page_Index: 要写入的页的页号, 内部 Flash 共256页, 页号0 ~ 255;
 * @param   data: 将此指针指向的内存地址开始的连续区域中的数据写到 Flash 中;
 * @param   size: 要写入的数据长度, 单位为 1B;
 * @note    本函数会在实际写入前自动执行擦除操作, 无需手动擦除;
 * @note    本函数仅支持从页首地址开始写;
 * @note    要写入的页内的其余位置的数据将被保留;
 */
static uint32_t flash_write8_page_withinPage(uint8_t page_Index, uint8_t *write_Data, uint16_t size)
{
    /**
     * 1.读取页号对应页到内存
     * 2.修改内存
     * 3.擦除所在页
     * 4.写入
     */
    /* 判断size和地址是否合法 */
    if (page_Index > MSS_FLASH_PAGE_NUM - 1)
    {
        printf("Flash写入失败, 原因: 页号非法\r\n");
        return 1;
    }
    
    if (size > MSS_FLASH_PAGE_SIZE_1B)
    {
        printf("Flash写入失败, 原因: 跨页\r\n");
        return 1;
    }
    uint32_t page_Base = page_Index * MSS_FLASH_PAGE_SIZE_1B; /* 页首地址 */

    /* 读取数据 */
    uint8_t buffer[MSS_FLASH_PAGE_SIZE_1B] = {0};
    flash_read_1B(page_Base, buffer, MSS_FLASH_PAGE_SIZE_1B);
    /* 在内存中修改 */
    for (uint16_t i = 0; i < size; i++)
    {
        buffer[i] = write_Data[i];
    }
    /* 擦除 */
    uint32_t page_Error = flash_erase_page(page_Base);
    /* 写入 */
    HAL_FLASH_Unlock();
    uint16_t *ptr = (uint16_t *)buffer;
    for (uint16_t i = 0; i < MSS_FLASH_PAGE_SIZE_2B; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, page_Base + 2 * i, ptr[i]);
    }
    HAL_FLASH_Lock();
    return page_Error;
}

/**
 * @brief   擦除指定地址所在的单页
 * @param   addr: 将会擦除改地址所在的页
 */
static uint32_t flash_erase_page(uint32_t addr)
{
    /* 判断 addr 是否合法 */
    if (addr > MSS_FLASH_END)
    {
        printf("Flash擦除失败, 原因: 地址非法\r\n");
        return 1;
    }
    
    HAL_FLASH_Unlock(); /*Flash解锁*/
    uint32_t page_Error = 0;
    FLASH_EraseInitTypeDef erase_handle = {0};                       /* NOTE:不要加static, 多线程会出错 */
    erase_handle.PageAddress = addr - addr % MSS_FLASH_PAGE_SIZE_1B; /* 拿到 addr 所在页的起始地址 */
    erase_handle.NbPages = 1;
    erase_handle.TypeErase = FLASH_TYPEERASE_PAGES;
    HAL_FLASHEx_Erase(&erase_handle, &page_Error); /* 原子代码, 内部通过__HAL_LOCK保证原子性 */
    HAL_FLASH_Lock();                              /* Flash上锁 */
    return page_Error;
}
