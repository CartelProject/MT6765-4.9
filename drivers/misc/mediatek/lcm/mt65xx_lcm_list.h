/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __MT65XX_LCM_LIST_H__
#define __MT65XX_LCM_LIST_H__

#include <lcm_drv.h>

//new added lcm drivers by xen 20170918
#ifdef BUILD_LK //xjl 20180531
extern LCM_DRIVER st7703_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER nv3051_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER ft8719_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER nt35523_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER nt35532_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER nt36526_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER ili9881d_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER ili9882_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER ili9881c_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER otm1290a_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER jd9365_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER jd9366_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER xm92160_6735_dsi_lcm_drv_yk6xx;

extern LCM_DRIVER icnl9911ac_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER icnl9911s_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER icnl9911sac_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER ili7807d_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER hx8399c_6735_dsi_lcm_drv_yk6xx;
//FWVGA:
extern LCM_DRIVER lcd_ata_test_lcm_drv;
extern LCM_DRIVER st7701s_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER st7701_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER ili9806e_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER otm8019a_6735_dsi_lcm_drv_yk6xx;
extern LCM_DRIVER hx8379a_6735_dsi_lcm_drv_yk6xx;
//FHD
extern LCM_DRIVER ili7807g_6735_dsi_lcm_drv_yk6xx;
#else
//HD720:
extern struct LCM_DRIVER st7703_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER nv3051_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER ft8719_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER nt35523_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER nt35532_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER nt36526_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER ili9881d_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER ili9882_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER ili9881c_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER otm1290a_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER jd9365_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER jd9366_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER xm92160_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER icnl9911ac_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER icnl9911s_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER icnl9911sac_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER ili7807d_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER hx8399c_6735_dsi_lcm_drv_yk6xx;
//FWVGA:
extern struct LCM_DRIVER lcd_ata_test_lcm_drv;
extern struct LCM_DRIVER st7701s_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER st7701_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER ili9806e_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER otm8019a_6735_dsi_lcm_drv_yk6xx;
extern struct LCM_DRIVER hx8379a_6735_dsi_lcm_drv_yk6xx;
//FHD
extern struct LCM_DRIVER ili7807g_6735_dsi_lcm_drv_yk6xx;
#endif

#ifdef BUILD_LK
extern void mdelay(unsigned long msec);
#endif

#endif
