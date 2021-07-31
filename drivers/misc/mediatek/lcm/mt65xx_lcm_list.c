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

#include "mt65xx_lcm_list.h"
#include <lcm_drv.h>
#ifdef BUILD_LK
#include <platform/disp_drv_platform.h>
LCM_DSI_MODE_CON lcm_dsi_mode;
#else
#include <linux/delay.h>
/* #include <mach/mt_gpio.h> */
enum LCM_DSI_MODE_CON lcm_dsi_mode;
#endif


/* used to identify float ID PIN status */
#define LCD_HW_ID_STATUS_LOW      0
#define LCD_HW_ID_STATUS_HIGH     1
#define LCD_HW_ID_STATUS_FLOAT 0x02
#define LCD_HW_ID_STATUS_ERROR  0x03

#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL, fmt)
#else
#define LCD_DEBUG(fmt, args...)  pr_debug("[KERNEL/LCM]"fmt, ##args)
#endif

#ifdef BUILD_LK //xjl 20180531
LCM_DRIVER *lcm_driver_list[] = {
#else
struct LCM_DRIVER *lcm_driver_list[] = {
#endif
#if defined(LCD_ATA_TEST)
	&lcd_ata_test_lcm_drv,
#endif

#if defined(NT35523_6735_DSI_YK6XX)
	&nt35523_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(NT35532_6735_DSI_YK6XX)
	&nt35532_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(NT36526_6735_DSI_YK6XX)
	&nt36526_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(ILI9881D_6735_DSI_YK6XX)
	&ili9881d_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(ILI9882_6735_DSI_YK6XX)
	&ili9882_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(ILI7807D_6735_DSI_YK6XX)
	&ili7807d_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(ILI9881C_6735_DSI_YK6XX)
	&ili9881c_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(ILI7807G_6735_DSI_YK6XX)
	&ili7807g_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(ST7703_6735_DSI_YK6XX)
	&st7703_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(NV3051_6735_DSI_YK6XX)
	&nv3051_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(HX8399C_6735_DSI_YK6XX)
	&hx8399c_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(FT8719_6735_DSI_YK6XX)
	&ft8719_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(OTM1290A_6735_DSI_YK6XX)
	&otm1290a_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(ST7701S_6735_DSI_YK6XX)
	&st7701s_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(ST7701_6735_DSI_YK6XX)
	&st7701_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(ILI9806E_6735_DSI_YK6XX)
	&ili9806e_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(OTM8019A_6735_DSI_YK6XX)
	&otm8019a_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(HX8379A_6735_DSI_YK6XX)
	&hx8379a_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(JD9365_6735_DSI_YK6XX)
	&jd9365_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(JD9366_6735_DSI_YK6XX)
	&jd9366_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(XM92160_6735_DSI_YK6XX)
	&xm92160_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(ICNL9911AC_6735_DSI_YK6XX)
	&icnl9911ac_6735_dsi_lcm_drv_yk6xx,
#endif
#if defined(ICNL9911S_6735_DSI_YK6XX)
	&icnl9911s_6735_dsi_lcm_drv_yk6xx,
#endif

#if defined(ICNL9911SAC_6735_DSI_YK6XX)
	&icnl9911sac_6735_dsi_lcm_drv_yk6xx,
#endif
};

unsigned char lcm_name_list[][128] = {
#if defined(HX8392A_DSI_CMD)
	"hx8392a_dsi_cmd",
#endif

#if defined(S6E3HA3_WQHD_2K_CMD)
	"s6e3ha3_wqhd_2k_cmd",
#endif

#if defined(HX8392A_DSI_VDO)
	"hx8392a_vdo_cmd",
#endif

#if defined(HX8392A_DSI_CMD_FWVGA)
	"hx8392a_dsi_cmd_fwvga",
#endif

#if defined(OTM9608_QHD_DSI_CMD)
	"otm9608a_qhd_dsi_cmd",
#endif

#if defined(OTM9608_QHD_DSI_VDO)
	"otm9608a_qhd_dsi_vdo",
#endif

#if defined(R63417_FHD_DSI_CMD_TRULY_NT50358)
	"r63417_fhd_dsi_cmd_truly_nt50358_drv",
#endif

#if defined(R63417_FHD_DSI_CMD_TRULY_NT50358_QHD)
	"r63417_fhd_dsi_cmd_truly_nt50358_qhd_drv",
#endif

#if defined(R63417_FHD_DSI_VDO_TRULY_NT50358)
	"r63417_fhd_dsi_vdo_truly_nt50358_drv",
#endif

#if defined(R63419_WQHD_TRULY_PHANTOM_2K_CMD_OK)
	"r63419_wqhd_truly_phantom_2k_cmd_ok",
#endif

#if defined(NT35695_FHD_DSI_CMD_TRULY_NT50358)
	"nt35695_fhd_dsi_cmd_truly_nt50358_drv",
#endif

#if defined(S6E3HA3_WQHD_2K_CMD_LANESWAP)
	"s6e3ha3_wqhd_2k_cmd_laneswap_drv",
#endif

#if defined(NT36380_WQHD_VDO_OK)
	"nt36380_wqhd_vdo_lcm_drv",
#endif
#if defined(NT35521_HD_DSI_VDO_TRULY_RT5081)
	"nt35521_hd_dsi_vdo_truly_rt5081_drv",
#endif

#if defined(ILI9881C_HDP_DSI_VDO_ILITEK_RT5081)
	"ili9881c_hdp_dsi_vdo_ilitek_rt5081_drv",
#endif

#if defined(NT35695B_FHD_DSI_VDO_AUO_RT5081_HDP)
	"nt35695B_fhd_dsi_vdo_auo_rt5081_hdp_drv",
#endif
};

#define LCM_COMPILE_ASSERT(condition) \
	LCM_COMPILE_ASSERT_X(condition, __LINE__)
#define LCM_COMPILE_ASSERT_X(condition, line) \
	LCM_COMPILE_ASSERT_XX(condition, line)
#define LCM_COMPILE_ASSERT_XX(condition, line) \
	char assertion_failed_at_line_##line[(condition) ? 1 : -1]

#ifdef BUILD_LK //xjl 20180531
unsigned int lcm_count =
	sizeof(lcm_driver_list) / sizeof(LCM_DRIVER *);
LCM_COMPILE_ASSERT(sizeof(lcm_driver_list) / sizeof(LCM_DRIVER *) != 0);
#else
unsigned int lcm_count =
	sizeof(lcm_driver_list) / sizeof(struct LCM_DRIVER *);
LCM_COMPILE_ASSERT(sizeof(lcm_driver_list) / sizeof(struct LCM_DRIVER *) != 0);
#endif
#if defined(NT35520_HD720_DSI_CMD_TM) | \
	defined(NT35520_HD720_DSI_CMD_BOE) | \
	defined(NT35521_HD720_DSI_VDO_BOE) | \
	defined(NT35521_HD720_DSI_VIDEO_TM)
static unsigned char lcd_id_pins_value = 0xFF;

/*
 * Function:    which_lcd_module_triple
 * Description: read LCD ID PIN status,could identify three status:highlowfloat
 * Input:       none
 * Output:      none
 * Return:      LCD ID1|ID0 value
 * Others:
 */
unsigned char which_lcd_module_triple(void)
{
	unsigned char  high_read0 = 0;
	unsigned char  low_read0 = 0;
	unsigned char  high_read1 = 0;
	unsigned char  low_read1 = 0;
	unsigned char  lcd_id0 = 0;
	unsigned char  lcd_id1 = 0;
	unsigned char  lcd_id = 0;
	/*Solve Coverity scan warning : check return value*/
	unsigned int ret = 0;

	/*only recognise once*/
	if (lcd_id_pins_value != 0xFF)
		return lcd_id_pins_value;

	/*Solve Coverity scan warning : check return value*/
	ret = mt_set_gpio_mode(GPIO_DISP_ID0_PIN, GPIO_MODE_00);
	if (ret != 0)
		pr_debug("[LCM]ID0 mt_set_gpio_mode fail\n");

	ret = mt_set_gpio_dir(GPIO_DISP_ID0_PIN, GPIO_DIR_IN);
	if (ret != 0)
		pr_debug("[LCM]ID0 mt_set_gpio_dir fail\n");

	ret = mt_set_gpio_pull_enable(GPIO_DISP_ID0_PIN, GPIO_PULL_ENABLE);
	if (ret != 0)
		pr_debug("[LCM]ID0 mt_set_gpio_pull_enable fail\n");

	ret = mt_set_gpio_mode(GPIO_DISP_ID1_PIN, GPIO_MODE_00);
	if (ret != 0)
		pr_debug("[LCM]ID1 mt_set_gpio_mode fail\n");

	ret = mt_set_gpio_dir(GPIO_DISP_ID1_PIN, GPIO_DIR_IN);
	if (ret != 0)
		pr_debug("[LCM]ID1 mt_set_gpio_dir fail\n");

	ret = mt_set_gpio_pull_enable(GPIO_DISP_ID1_PIN, GPIO_PULL_ENABLE);
	if (ret != 0)
		pr_debug("[LCM]ID1 mt_set_gpio_pull_enable fail\n");

	/*pull down ID0 ID1 PIN*/
	ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_DOWN);
	if (ret != 0)
		pr_debug("[LCM]ID0 mt_set_gpio_pull_select->Down fail\n");

	ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_DOWN);
	if (ret != 0)
		pr_debug("[LCM]ID1 mt_set_gpio_pull_select->Down fail\n");

	/* delay 100ms , for discharging capacitance*/
	mdelay(100);
	/* get ID0 ID1 status*/
	low_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
	low_read1 = mt_get_gpio_in(GPIO_DISP_ID1_PIN);
	/* pull up ID0 ID1 PIN */
	ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_UP);
	if (ret != 0)
		pr_debug("[LCM]ID0 mt_set_gpio_pull_select->UP fail\n");

	ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_UP);
	if (ret != 0)
		pr_debug("[LCM]ID1 mt_set_gpio_pull_select->UP fail\n");

	/* delay 100ms , for charging capacitance */
	mdelay(100);
	/* get ID0 ID1 status */
	high_read0 = mt_get_gpio_in(GPIO_DISP_ID0_PIN);
	high_read1 = mt_get_gpio_in(GPIO_DISP_ID1_PIN);

	if (low_read0 != high_read0) {
		/*float status , pull down ID0 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,
			GPIO_PULL_DOWN);
		if (ret != 0)
			pr_debug("[LCM]ID0 mt_set_gpio_pull_select->Down fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_FLOAT;
	} else if ((low_read0 == LCD_HW_ID_STATUS_LOW) &&
		(high_read0 == LCD_HW_ID_STATUS_LOW)) {
		/*low status , pull down ID0 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,
			GPIO_PULL_DOWN);
		if (ret != 0)
			pr_debug("[LCM]ID0 mt_set_gpio_pull_select->Down fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_LOW;
	} else if ((low_read0 == LCD_HW_ID_STATUS_HIGH) &&
		(high_read0 == LCD_HW_ID_STATUS_HIGH)) {
		/*high status , pull up ID0 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN, GPIO_PULL_UP);
		if (ret != 0)
			pr_debug("[LCM]ID0 mt_set_gpio_pull_select->UP fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_HIGH;
	} else {
		pr_debug("[LCM] Read LCD_id0 error\n");
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID0_PIN,
			GPIO_PULL_DISABLE);
		if (ret != 0)
			pr_debug("[KERNEL/LCM]ID0 mt_set_gpio_pull_select->Disbale fail\n");

		lcd_id0 = LCD_HW_ID_STATUS_ERROR;
	}


	if (low_read1 != high_read1) {
		/*float status , pull down ID1 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,
			GPIO_PULL_DOWN);
		if (ret != 0)
			pr_debug("[LCM]ID1 mt_set_gpio_pull_select->Down fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_FLOAT;
	} else if ((low_read1 == LCD_HW_ID_STATUS_LOW) &&
		(high_read1 == LCD_HW_ID_STATUS_LOW)) {
		/*low status , pull down ID1 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,
			GPIO_PULL_DOWN);
		if (ret != 0)
			pr_debug("[LCM]ID1 mt_set_gpio_pull_select->Down fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_LOW;
	} else if ((low_read1 == LCD_HW_ID_STATUS_HIGH) &&
		(high_read1 == LCD_HW_ID_STATUS_HIGH)) {
		/*high status , pull up ID1 ,to prevent electric leakage*/
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN, GPIO_PULL_UP);
		if (ret != 0)
			pr_debug("[LCM]ID1 mt_set_gpio_pull_select->UP fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_HIGH;
	} else {

		pr_debug("[LCM] Read LCD_id1 error\n");
		ret = mt_set_gpio_pull_select(GPIO_DISP_ID1_PIN,
			GPIO_PULL_DISABLE);
		if (ret != 0)
			pr_debug("[KERNEL/LCM]ID1 mt_set_gpio_pull_select->Disable fail\n");

		lcd_id1 = LCD_HW_ID_STATUS_ERROR;
	}
#ifdef BUILD_LK
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id0:%d\n", lcd_id0);
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id1:%d\n", lcd_id1);
#else
	pr_debug("[LCM]which_lcd_module_triple,lcd_id0:%d\n", lcd_id0);
	pr_debug("[LCM]which_lcd_module_triple,lcd_id1:%d\n", lcd_id1);
#endif
	lcd_id =  lcd_id0 | (lcd_id1 << 2);

#ifdef BUILD_LK
	dprintf(CRITICAL, "which_lcd_module_triple,lcd_id:%d\n", lcd_id);
#else
	pr_debug("[LCM]which_lcd_module_triple,lcd_id:%d\n", lcd_id);
#endif

	lcd_id_pins_value = lcd_id;
	return lcd_id;
}
#endif
