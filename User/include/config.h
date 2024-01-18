/**
 ****************************************************************************************************
 * @file        config.h
 * @author      电气组,wangdexu
 * @version     V1.0
 * @date        2023-01-08
 * @brief       参数配置模块
 * @license     Copyright (c) 2020-2032, 安徽中车瑞达电气有限公司
 ****************************************************************************************************
 * @attention
 *
 *
 ****************************************************************************************************
 */
#ifndef __MSS_Config_H
#define __MSS_Config_H

#include "stdint.h"



/* 结构体a.b     . 和 -> 后都不能加括号, a.(b) 和 a -> (b)都报错 */
/* MARK: 测试通过 */
#define __MSS_GET_CONFIG(__CONFIG_GROUP__, __CONFIG_ITEM__) ((mss_Config->__CONFIG_GROUP__).__CONFIG_ITEM__)
/* MARK: 测试通过 */
#define __MSS_SET_CONFIG(__CONFIG_GROUP__, __CONFIG_ITEM__, __CONFIG_VALUE__) ((mss_Config->__CONFIG_GROUP__).__CONFIG_ITEM__ = (__CONFIG_VALUE__))


/*TODO:待测*/
void load_config_flash(void);
/*TODO:待测*/
void store_config_flash(void);
/*TODO:待测*/
void load_config_w25q64(void);
/*TODO:待测*/
void store_config_w25q64(void);

#endif
