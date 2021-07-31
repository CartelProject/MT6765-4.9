/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK //xen 20160603
    #include <platform/mt_gpio.h>
    //#include "cust_gpio_usage.h"
#else
    //#include <linux/gpio.h>
    //#include <mt-plat/mtk_gpio.h>
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
///////////////////// notice wanghe //////////////////////////////////////////
// 0729:
// LCD_HONGTAO_543_st7710_IVO have hengtiaowen, can not resolve!!!!!!
// added by wanghe 2013-07-31 for esd shanping  FAQ05681

//////////////////////////////////////////////////////////////////////////////

#define LCM_ID_ST7701                                      (0x7701)

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)

#define LCM_PHYSICAL_WIDTH									(0)
#define LCM_PHYSICAL_HEIGHT									(0)

//LCD module explaination					    
//Project		Custom		Factory	 	size		W&H		Glass	degree	data		HWversion
#define LCD_HONGLI_50_ST7701_CMI //xjl 20170116 for ATA test

#define REGFLAG_DELAY             							0XFE // END OF REGISTERS MARKER
#define REGFLAG_END_OF_TABLE      							0x100   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

#ifdef BUILD_LK //xjl 20180531
static LCM_UTIL_FUNCS lcm_util = {0};
#else
static struct LCM_UTIL_FUNCS lcm_util = {0};
#endif

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned int cmd; 
    unsigned char count;
    unsigned char para_list[20];//64]; //xen for less space 20170511
};

//#elif defined (LCD_HONGLI_50_ST7701_CMI) //xjl 20160926 cjc 20161101
#if defined (LCM_ROTATE_180)
	static unsigned char BufLcmInfo[] = "LCD_HONGLI_50_ST7701_CMI_20161101_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_HONGLI_50_ST7701_CMI_20161101";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {
{0x11,1,{0x00}},
{REGFLAG_DELAY, 120, {0}},
{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},
{0xD1,2,{0x11}},
{0x11,1,{0x00}},
{REGFLAG_DELAY, 120, {0}},
{0xFF,5,{0x77,0x01,0x00,0x00,0x10}},
{0xC0,2,{0xE9,0x03}},
{0xC1,2,{0x0D,0x02}},
{0xC2,2,{0x00,0x06}},//31  37 cjc 0x30
{0xB0,16,{0x00,0x07,0x93,0x13,0x19,0x0B,0x0B,0x09,0x08,0x1F,0x08,0x15,0x11,0x0F,0x18,0x17}},
{0xB1,16,{0x00,0x07,0x92,0x12,0x15,0x09,0x08,0x09,0x09,0x1F,0x07,0x15,0x11,0x15,0x18,0x17}},
{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},
{0xB0,1,{0x4D}},
{0xB1,1,{0x3C}},//vcom 47-3C  0x3E//3A
{0xB2,1,{0x02}},//07
{0xB3,1,{0x80}},
{0xB5,1,{0x42}},//47
{0xB7,1,{0x8A}},//8A
{0xB8,1,{0x10}},
{0xB9,1,{0x00}},//cjc add
{0xC1,1,{0x78}},
{0xC2,1,{0x78}},
{0xD0,1,{0x88}},
{0xE0,3,{0x00,0x00,0x02}},
{0xE1,11,{0x02,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x40,0x40}},
{0xE2,13,{0x33,0x33,0x34,0x34,0x62,0x00,0x63,0x00,0x61,0x00,0x64,0x00,0x00}},
{0xE3,4,{0x00,0x00,0x33,0x33}},
{0xE4,2,{0x44,0x44}},
{0xE5,16,{0x04,0x6B,0xC0,0xC0,0x06,0x6B,0xC0,0xC0,0x08,0x6B,0xC0,0xC0,0x0A,0x6B,0xC0,0xC0}},
{0xE6,4,{0x00,0x00,0x33,0x33}},
{0xE7,2,{0x44,0x44}},
{0xE8,16,{0x03,0x6B,0xC0,0xC0,0x05,0x6B,0xC0,0xC0,0x07,0x6B,0xC0,0xC0,0x09,0x6B,0xC0,0xC0}},
{0xEB,7,{0x02,0x00,0x39,0x39,0x88,0x33,0x10}},
{0xEC,2,{0x02,0x00}},
{0xED,16,{0xFF,0x04,0x56,0x7F,0x89,0xF2,0xFF,0x3F,0xF3,0xFF,0x2F,0x98,0xF7,0x65,0x40,0xFF}},
{0xFF,5,{0x77,0x01,0x00,0x00,0x00}},		
{REGFLAG_DELAY, 20, {0}},
{0x35,1,{0x00}},//Display ON
{0x29,1,{0x00}},//Display ON
{REGFLAG_DELAY, 80, {0}},
{REGFLAG_END_OF_TABLE, 0x00, {}}
};


#if defined(YK606_CUSTOMER_XINDI_V1_FWVGA_50)||defined(YK610_CUSTOMER_SHENGXIN_V1_FWVGA)//cty for esd
static struct LCM_setting_table lcm_close_ic_power[] = {
	{0x10,1,{0x00}},// Sleep-Out
	{REGFLAG_DELAY, 120, {}},
	{0xFF,6,{0x77,0x01,0x00,0x00,0x11,0x80}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x91}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif

/*
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
    {REGFLAG_DELAY, 100, {}},//wangliangfu change 100 to 50 for dalong 20131008
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};*/

// changed by wanghe 2013-09-07 for resum  shanping
static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x22, 0, {0x00}},
	{REGFLAG_DELAY, 100, {}},

	{0x28, 0, {0x00}},
	{REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_prepare_setting[] = {
#if defined(Q578_CUSTOMER_CAIFU3_TS500_FWVGA_50)||defined(YK630_CUSTOMER_CAIFU_FS510_FWVGA)||defined(YK610_CUSTOMER_MENGCHENG_1_FWVGA)
	{0x01,1,{0x00}},
	{REGFLAG_DELAY, 120, {0}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},
	{0xD1,1,{0x11}}, 
#else
    {0xDA, 1, {0xAA}}, // page 1
#endif
	{REGFLAG_END_OF_TABLE, 0x00, {}}	
};

#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
#else
static void push_table(void *cmdq, struct LCM_setting_table *table,
	unsigned int count, unsigned char force_update)
#endif
{
	unsigned int i;
	
	for(i = 0; i < count; i++) {
		unsigned int cmd; //zxs 20151127
		cmd = table[i].cmd;
		
		switch (cmd) {	
		case REGFLAG_DELAY :
			MDELAY(table[i].count);
			break;
		case REGFLAG_END_OF_TABLE :
			break;
		default:
#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
#else
			dsi_set_cmdq_V22(cmdq, cmd, table[i].count, table[i].para_list, force_update);
#endif
		}
	}
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
static unsigned int lcm_compare_id(void);
#ifdef BUILD_LK //xjl 20180531
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}
#else
static void lcm_set_util_funcs(const struct LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(struct LCM_UTIL_FUNCS));
}
#endif

#ifdef BUILD_LK //xjl 20180531
static void lcm_get_params(LCM_PARAMS *params)
#else
static void lcm_get_params(struct LCM_PARAMS *params)
#endif
{
#ifdef BUILD_LK //xjl 20180531
	memset(params, 0, sizeof(LCM_PARAMS));
#else
	memset(params, 0, sizeof(struct LCM_PARAMS));
#endif
	
	params->type   = LCM_TYPE_DSI; 
	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;
	
#ifndef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	params->physical_width = LCM_PHYSICAL_WIDTH/1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
	params->physical_width_um = LCM_PHYSICAL_WIDTH;
	params->physical_height_um = LCM_PHYSICAL_HEIGHT;

	params->dsi.switch_mode = CMD_MODE;
	params->dsi.switch_mode_enable = 0;	
#endif

	params->dsi.mode   = SYNC_PULSE_VDO_MODE;
	params->dsi.LANE_NUM	= LCM_TWO_LANE;
	
	params->dsi.g_StrLcmInfo = BufLcmInfo;
	
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order     = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq	   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding 	   = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format	       = LCM_DSI_FORMAT_RGB888;
	
	params->dsi.packet_size=256;
	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	//params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
	//params->dsi.vertical_active_line=FRAME_HEIGHT;

	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 17;
	params->dsi.vertical_frontporch					= 12;
	
	params->dsi.horizontal_sync_active				= 10;
	params->dsi.horizontal_backporch				= 60;
	params->dsi.horizontal_frontporch				= 80;

	params->dsi.vertical_active_line				= FRAME_HEIGHT;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	params->dsi.compatibility_for_nvk = 0;	// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's
	
	params->dsi.PLL_CLOCK = 170; //200;

#if defined(YK606_CUSTOMER_XINDI_V1_FWVGA_50)||defined(YK610_CUSTOMER_SHENGXIN_V1_FWVGA)||defined(Q578_CUSTOMER_CAIFU3_TS500_FWVGA_50)\
	||defined(YK630_CUSTOMER_CAIFU_FS510_FWVGA)
     params->dsi.noncont_clock = 1;   //esd   yx 20160826
     params->dsi.esd_check_enable =1;
     params->dsi.customization_esd_check_enable =1;
     params->dsi.lcm_esd_check_table[0].cmd =0x0A;
     params->dsi.lcm_esd_check_table[0].count =1;
     params->dsi.lcm_esd_check_table[0].para_list[0] =0x9C;
	 
	 //params->dsi.lcm_esd_check_table[1].cmd =0x04;
     //params->dsi.lcm_esd_check_table[1].count =3;
	 //params->dsi.lcm_esd_check_table[1].para_list[0] =0x55;
	 //params->dsi.lcm_esd_check_table[1].para_list[1] =0x55;
	 //params->dsi.lcm_esd_check_table[1].para_list[2] =0x55;
#endif

}


static void lcm_init(void)
{
	//LCM_PRINT("%s\n", __func__);
	SET_RESET_PIN(1);
	MDELAY(1);
	SET_RESET_PIN(0);
	MDELAY(30);
	SET_RESET_PIN(1);
	MDELAY(120);
	
#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}

static void lcm_suspend(void)
{
	//LCM_PRINT("%s\n", __func__);

#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#endif
	//SET_RESET_PIN(0);
	MDELAY(50);
}

static void lcm_resume(void)
{
	//LCM_PRINT("%s\n", __func__);
	
	lcm_init();
}

static unsigned int lcm_compare_id(void)
{
	unsigned int array[4];
	unsigned short device_id;
	unsigned char buffer[2];
	
	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);
	
#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_prepare_setting, sizeof(lcm_prepare_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_prepare_setting, sizeof(lcm_prepare_setting) / sizeof(struct LCM_setting_table), 1);
#endif
    //*************Enable CMD2 Page1  *******************//
	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xA1, buffer, 2);
	
	device_id = buffer[0]<<8|buffer[1];

	//LCM_PRINT("<<ST7701 ID Read>>%s, device_id=0x%04x,buffer[0]=%x,buffer[1]=%x\n", __func__, device_id,buffer[0],buffer[1]);

	return (LCM_ID_ST7701 == device_id) ? 1 : 0;
}


#ifdef BUILD_LK //xjl 20180531
LCM_DRIVER st7701_6735_dsi_lcm_drv_yk6xx = 
#else
struct LCM_DRIVER st7701_6735_dsi_lcm_drv_yk6xx = 
#endif
{
    .name			= "st7701_67xx_dsi_video", 
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};

