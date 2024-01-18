/**
 ****************************************************************************************************
 * @file        w25q64.c
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2024-01-06
 * @brief       外部Flash驱动
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 * Flash存储分内部和外部FLASH;
 * 内部Flash直接读写, 外部Flash用SPI通信进行读写, 分两套API;
 * 本驱动文件负责外部 W25Q64 Flash驱动
 ****************************************************************************************************
 * @attention
 * 外部Flash:
 * 组织结构为 块-扇区-页, 扇区4KB, 页256B; 大小端由用户决定;
 * 写入最小单位是1B, 最大长度是一页; 擦除最小单位是扇区4KB;
 * 数据收发基于 "交换", 通过与W25Q64交换数据来完成发送与接收;
 * 写入和擦除会导致进入忙状态
 ****************************************************************************************************
 * @attention
 * 读和写都采用轮询的方式进行, 为了保证安全性;
 * 开机时, 先将参数读入, 完全读取完成后才能进行下一步的工作, 所以采用轮询方式;
 * 用户保存参数或者关机时 {TODO:关机时要检查参数自上次保存是否有修改(通过设置表示位实现), 如果有, 提示用户选择是否保存新参数} 写入;
 * 写入完成(只是写到了Flash的RAM中)后还要给FLASH固化(RAM写到FLASH单元)的时间, 方法是查询BUSY, BUSY=0后可以关机
 * 电机运行时不允许修改参数保存参数, 所以采用轮询方式写入不会对软启动功能产生影响
 ****************************************************************************************************
 */
#include "./BSP/FLASH/W25Q64/w25q64.h"
#include "./BSP/SPI/spi.h"
#include "stdio.h"

/* static variables */
static uint8_t cmd_Read_Status1[1] = {0x05};
static uint8_t cmd_Write_Enable[1] = {0x06};
static uint8_t cmd_Erase_Sector[1] = {0x20};
static uint8_t cmd_Write[1] = {0x02};
static uint8_t cmd_Read[1] = {0x03};
static uint8_t cmd_Read_ID[1] = {0x9F};

/* static functions */
static void w25q64_wait(void);
static void w25q64_write_enable(void);
static void w25q64_erase_sector(uint32_t addr);
static inline uint32_t get_sectorBase(uint32_t addr);
static inline uint32_t get_pageBase(uint32_t addr);
static inline void set_addr(uint32_t addr, uint8_t *cmd_Arr);

/**
 * @brief   使W25Q64处于可用状态;
 * @note    这里规定W25Q64固定使用SPI1;
 */
void w25q64_init(void)
{
    spi1_init(SPI_BAUDRATEPRESCALER_2);
}

/**
 * @brief   读取W25Q64芯片出厂信息;
 * @param   read_Data: 携带读到的数据;
 * @note    出厂信息共三个字节, 数据部分第一个字节 = 制造商ID, 第二个字节 = 存储类型, 第三个字节 = 芯片容量;
 */
void w25q64_read_ID(uint8_t *read_Data)
{
    printf("W25Q64读取ID\r\n");
    w25q64_wait();                                                                            /* 等待空闲 */
    W25Q64_ENABLE();                                                                          /* 片选 */
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Read_ID, 1, W25Q64_TIMOUT);                   /* 发送读命令 */
    HAL_SPI_TransmitReceive(&CF_W25Q64_SPI_HANDLE, cmd_Read_ID, read_Data, 3, W25Q64_TIMOUT); /* 继续交换3B数据 */
    W25Q64_DISABLE();                                                                         /* 失能芯片 */
}

/**
 * @brief   按地址读任意长度数据;
 * @param   addr: 要读取的地址;
 * @param   read_Data: 将携带读到的数据;
 * @param   len: 读取的长度, 无大小限制;
 */
void w25q64_read(uint32_t addr, uint8_t *read_Data, uint32_t len)
{
    /* 判断地址和长度是否合法 */
    if (addr + len - 1 > W25Q64_END)
    {
        printf("W25Q64读取失败, 原因: 地址或长度非法\r\n");
        return;
    }
    printf("W25Q64读取0x%06X\r\n", addr);
    uint8_t addr_arr[3] = {0};
    set_addr(addr, addr_arr);                                                   /* 填入地址 */
    w25q64_wait();                                                              /* 等待空闲 */
    W25Q64_ENABLE();                                                            /* 片选 */
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Read, 1, W25Q64_TIMOUT);        /* 发送读命令 */
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, addr_arr, 3, W25Q64_TIMOUT);        /* 发送读地址 */
    HAL_SPI_Receive(&CF_W25Q64_SPI_HANDLE, read_Data, len, 10 * W25Q64_TIMOUT); /* 接收数据 */
    W25Q64_DISABLE();                                                           /* 失能芯片 */
}

/**
 * @brief   按扇区号和页号读取数据;
 * @param   sector_Index: 扇区号, 0 ~ 2K-1;
 * @param   page_Index: 页号, 0 ~ 15;
 * @param   read_Data: 将携带读到的数据;
 * @note    固定读一整页内容;
 */
void w25q64_read_page(uint16_t sector_Index, uint8_t page_Index, uint8_t *read_Data)
{
    if (sector_Index > W25Q64_SECTOR_NUM - 1)
    {
        printf("W25Q64读取失败, 原因: 扇区号非法\r\n");
        return;
    }
    if (page_Index > W25Q64_PAGE_NUM - 1)
    {
        printf("W25Q64读取失败, 原因: 页号非法\r\n");
        return;
    }

    uint32_t read_Addr = sector_Index * W25Q64_SECTOR_SIZE + page_Index * W25Q64_PAGE_SIZE;
    w25q64_read(read_Addr, read_Data, W25Q64_PAGE_SIZE);
}

/**
 * @brief   写任意长度数据到指定位置, 可以跨扇区;
 * @param   addr: 要写入的地址;
 * @param   write_Data: 要写入的数据的首指针;
 * @param   len: 要写入的数据的长度, 单位为B;
 * @note    写入的数据所在的任意扇区的其它数据将被保留, 不会被擦除掉;
 */
void w25q64_write_addr(uint32_t addr, uint8_t *write_Data, uint32_t len)
{
}

/**
 * @brief   写任意长度数据到指定扇区的指定页开始的位置, 可以跨扇区;
 * @param   sector_Index: 扇区号, 0 ~ 2K-1;
 * @param   page_Index: 页号, 0 ~ 15;
 * @param   write_Data: 要写入的数据的首指针;
 * @param   len: 要写入的数据的长度, 单位为 1B;
 * @note    写入的数据所在的任意扇区的其它数据将被保留, 不会被擦除掉;
 * @note    写入起始位置固定为指定页的页首;
 */
void w25q64_write_page(uint16_t sector_Index, uint8_t page_Index, uint8_t *write_Data, uint32_t len)
{
    uint32_t write_Addr = sector_Index * W25Q64_SECTOR_SIZE + page_Index * W25Q64_PAGE_SIZE;
}

/**
 * @brief   写数据到指定位置, 要求写入的起始位置和结束位置位于同一扇区之内;
 * @param   addr: 要写入的地址;
 * @param   write_Data: 要写入的数据的首指针;
 * @param   len: 要写入的数据的长度, 单位为B;
 * @note    会保留扇区内其余位置的数据不变;
 */
void w25q64_write_addr_withinSector(uint32_t addr, uint8_t *write_Data, uint16_t len)
{
    /**
     * 1.安全检查, 处理地址;
     * 2.将写入位置所在扇区读入内存, 在内存中修改;
     * 3.擦除;
     * 4.写入;
     */
    /* 安全检查, 处理地址 */
    uint32_t sector_Base = get_sectorBase(addr);               /* 获取写入开始位置所在扇区的首地址 */
    uint32_t end_Sector_Base = get_sectorBase(addr + len - 1); /* 写入结束位置所在扇区首地址 */
    if (end_Sector_Base != sector_Base)                        /* 如果结束位置所在扇区和开始位置所在扇区不一致, 拒绝写入 */
    {
        printf("W25Q64写入失败, 原因: 跨扇区\r\n");
        return;
    }
    printf("W25Q64写入0x%06X\r\n", addr);

    /* 将写入位置所在扇区读入内存, 在内存中修改 */
    /* MARK: 这里固定将write_Data复制到buffer, 如果write_Data很长, 开销比较大; */
    /* MARK: 解决: 仍然申请 4K 数组, 计算写入开始位置之前的数据长度和结束位置之后的数据长度, 擦除后从buffer把这两部分重新写回; 实现太麻烦 */
    uint8_t buffer[W25Q64_SECTOR_SIZE] = {0};
    w25q64_read(sector_Base, buffer, W25Q64_SECTOR_SIZE);
    uint16_t offset = addr % W25Q64_SECTOR_SIZE; /* 计算写入位置相对于扇区首地址的偏移量 */
    for (uint16_t i = 0; i < len; i++)
    {
        buffer[i + offset] = write_Data[i];
    }

    /* 擦除 */
    w25q64_write_enable();            /* 写使能 */
    w25q64_wait();                    /* 等待芯片空闲 */
    w25q64_erase_sector(sector_Base); /* 擦除 */
    /* NOTE: 擦除后, 需要等待空闲再写入, 如果缺少等待步骤, 在芯片忙时直接发送写命令, 写命令将被忽略 */
    /* ALERT: 不等待将导致读出来就是被擦除后的值, 因为写入指令根本没有被执行 */
    w25q64_wait(); /* 等待芯片空闲 */

    /* 分页写入 */
    uint8_t addr_arr[3] = {0};
    for (uint8_t page_Index = 0; page_Index < W25Q64_PAGE_NUM; page_Index++)
    {
        set_addr(sector_Base + page_Index * W25Q64_PAGE_SIZE, addr_arr);                                                      /* 填入地址 */
        w25q64_wait();                                                                                                        /* 等待芯片空闲 */
        w25q64_write_enable();                                                                                                /* 写使能 */
        w25q64_wait();                                                                                                        /* 等待芯片空闲 */
        W25Q64_ENABLE();                                                                                                      /* 片选 */
        HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Write, 1, W25Q64_TIMOUT);                                                 /* 发送写指令 */
        HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, addr_arr, 3, W25Q64_TIMOUT);                                                  /* 发送地址 */
        HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, buffer + page_Index * W25Q64_PAGE_SIZE, W25Q64_PAGE_SIZE, 5 * W25Q64_TIMOUT); /* 发送写入的内容 */
        W25Q64_DISABLE();                                                                                                     /* 失能芯片 */
    }
}

/**
 * @brief   擦除指定地址所在的4KB扇区;
 * @param   addr: 要擦除的地址;
 */
static void w25q64_erase_sector(uint32_t addr)
{
    uint8_t addr_arr[3] = {0};
    addr = get_sectorBase(addr);                                                 /* 找到 addr所在扇区首地址 */
    set_addr(addr, addr_arr);                                                    /* 填入地址 */
    W25Q64_ENABLE();                                                             /*片选*/
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Erase_Sector, 1, W25Q64_TIMOUT); /* 发送擦除命令 */
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, addr_arr, 3, W25Q64_TIMOUT);         /* 携带地址, 擦除操作将导致忙状态, HAL_SPI_Transmit只关注发送, 忽略接收 */
    W25Q64_DISABLE();                                                            /* 解除片选 */
}

/**
 * @brief   等待w25q64至可用状态, 忙则循环等待;
 * @note    注意使用之前需要调用 W25Q64_ENABLE()使能Flash; 读的是状态寄存器而非FLASH存储单元, 所以FLASH忙的时候也能读;
 */
static void w25q64_wait(void)
{
    uint8_t status1 = 0;
    W25Q64_ENABLE();
    do
    {
        HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Read_Status1, 1, W25Q64_TIMOUT); /* 发送读status1寄存器的命令 */
        HAL_SPI_Receive(&CF_W25Q64_SPI_HANDLE, &status1, 1, W25Q64_TIMOUT);          /* 交换得到status1的值(8bit) */

    } while ((status1 & 0x01) == 0x01);
    W25Q64_DISABLE();
}

/**
 * @brief   写使能
 * @note    注意使用前需要调用 W25Q64_ENABLE()使能Flash并确定芯片空闲
 */
static void w25q64_write_enable(void)
{
    W25Q64_ENABLE();
    HAL_SPI_Transmit(&CF_W25Q64_SPI_HANDLE, cmd_Write_Enable, 1, W25Q64_TIMOUT); /* HAL_SPI_Transmit只关注发送, 忽略接收 */
    W25Q64_DISABLE();
}

/**
 * @brief   计算一个地址所在的扇区的首地址;
 * @note    扇区大小4KB, 16进制表示为 0x1000 B, 故扇区首地址规律为 0x0000, 0x1000, 0x2000...
 */
static inline uint32_t get_sectorBase(uint32_t addr)
{
    return addr - (addr % W25Q64_SECTOR_SIZE);
}

/**
 * @brief   计算一个地址所在页的首地址
 * @note    页大小256B, 16进制表示为 0x100B, 故页首地址规律为 0x000, 0x100, 0x200....
 */
static inline uint32_t get_pageBase(uint32_t addr)
{
    return addr - (addr % W25Q64_PAGE_SIZE);
}

/**
 * @brief   32位地址转换为24位(这其中实际又只有23位有效), 并放到数组的第2,3,4个位置(第一个位置放命令);
 * @param   addr: 要格式化的地址;
 * @param   cmd_Arr: 转换结果放到此数组中;
 * @note    数组低位元素放地址高位;
 */
static inline void set_addr(uint32_t addr, uint8_t *addr_Arr)
{
    addr_Arr[0] = (addr >> 16); /* 取地址高位[22:16] */
    addr_Arr[1] = (addr >> 8);  /* 地址中位[15:8] */
    addr_Arr[2] = (addr >> 0);  /* 地址低位[7:0] */
    // printf("addr[]:%02X, %02X, %02X\r\n", addr_Arr[0], addr_Arr[1], addr_Arr[2]);
}
