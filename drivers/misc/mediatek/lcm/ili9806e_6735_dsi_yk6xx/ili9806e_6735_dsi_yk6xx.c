/* Copyright Statement:  PART 2 OLDª©
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */



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

#if 1 // defined (LCM_USE_VIDEO_MODE)  
#define LCM_DSI_CMD_MODE									 0  
#else
#define LCM_DSI_CMD_MODE									 1
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCD_XINGLIANGDA_45_ILI9806E_BOE

  
/////////////////////////////////////////////////////////////////////////

#if defined(LCM_480_854)
	#define FRAME_WIDTH 										(480)
	#define FRAME_HEIGHT										(854)
#elif defined(LCM_480_800)
	#define FRAME_WIDTH  										(480)
	#define FRAME_HEIGHT 										(800)
#else
	#define FRAME_WIDTH 										(480)
	#define FRAME_HEIGHT										(854)
#endif

#define LCM_ID_ILI9806E 										(0x9806)

#define LCM_PHYSICAL_WIDTH									(0)
#define LCM_PHYSICAL_HEIGHT									(0)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFD  // END OF REGISTERS MARKER
//#define DRV_LCM_ONE_LANE
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
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update); MDELAY(10) 
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)


struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[10];//64]; //xen for less space 20170511
};

//#elif defined (LCD_XINGLIANGDA_45_ILI9806E_BOE)   //cty 20140812
#if defined (LCM_ROTATE_180)
	static unsigned char BufLcmInfo[] = "LCD_XINGLIANGDA_45_ILI9806E_BOE_140812_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_XINGLIANGDA_45_ILI9806E_BOE_140812";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xFF,5,{0xFF,0x98,0x06,0x04,0x01}},
	{0x08,1,{0x10}},
	{0x21,1,{0x01}},
	{0x30,1,{0x01}},
	{0x31,1,{0x00}},
	{0x40,1,{0x16}},
	{0x41,1,{0x22}},
	{0x42,1,{0x00}},
	{0x43,1,{0x85}},
	{0x44,1,{0x8B}},
	{0x45,1,{0x1B}},
	{0x50,1,{0x78}},
	{0x51,1,{0x78}},
	{0x52,1,{0x00}},
	{0x53,1,{0x55}},//0x53
	{0x57,1,{0x50}},
	{0x60,1,{0x07}},
	{0x61,1,{0x00}},
	{0x62,1,{0x07}},
	{0x63,1,{0x00}},
	{0xA0,1,{0x00}},
	{0xA1,1,{0x10}},
	{0xA2,1,{0x17}},
	{0xA3,1,{0x0F}},
	{0xA4,1,{0x05}},
	{0xA5,1,{0x0C}},
	{0xA6,1,{0x08}},
	{0xA7,1,{0x06}},
	{0xA8,1,{0x03}},
	{0xA9,1,{0x0A}},
	{0xAA,1,{0x10}},
	{0xAB,1,{0x08}},
	{0xAC,1,{0x0F}},
	{0xAD,1,{0x18}},
	{0xAE,1,{0x11}},
	{0xAF,1,{0x00}},
	{0xC0,1,{0x00}},
	{0xC1,1,{0x10}},
	{0xC2,1,{0x17}},
	{0xC3,1,{0x0F}},
	{0xC4,1,{0x05}},
	{0xC5,1,{0x0A}},
	{0xC6,1,{0x07}},
	{0xC7,1,{0x06}},
	{0xC8,1,{0x03}},
	{0xC9,1,{0x0A}},
	{0xCA,1,{0x12}},
	{0xCB,1,{0x08}},
	{0xCC,1,{0x0F}},
	{0xCD,1,{0x17}},
	{0xCE,1,{0x11}},
	{0xCF,1,{0x00}},
	{0xFF,5,{0xFF,0x98,0x06,0x04,0x06}},
	{0x00,1,{0x20}},
	{0x01,1,{0x05}},
	{0x02,1,{0x00}},
	{0x03,1,{0x00}},
	{0x04,1,{0x01}},
	{0x05,1,{0x01}},
	{0x06,1,{0x88}},
	{0x07,1,{0x04}},
	{0x08,1,{0x01}},
	{0x09,1,{0x90}},
	{0x0A,1,{0x04}},
	{0x0B,1,{0x01}},
	{0x0C,1,{0x01}},
	{0x0D,1,{0x01}},
	{0x0E,1,{0x00}},
	{0x0F,1,{0x00}},
	{0x10,1,{0x55}},
	{0x11,1,{0x50}},
	{0x12,1,{0x01}},
	{0x13,1,{0x0C}},
	{0x14,1,{0x0D}},
	{0x15,1,{0x43}},
	{0x16,1,{0x0B}},
	{0x17,1,{0x00}},
	{0x18,1,{0x00}},
	{0x19,1,{0x00}},
	{0x1A,1,{0x00}},
	{0x1B,1,{0x00}},
	{0x1C,1,{0x00}},
	{0x1D,1,{0x00}},
	{0x20,1,{0x01}},
	{0x21,1,{0x23}},
	{0x22,1,{0x45}},
	{0x23,1,{0x67}},
	{0x24,1,{0x01}},
	{0x25,1,{0x23}},
	{0x26,1,{0x45}},
	{0x27,1,{0x67}},
	{0x30,1,{0x02}},
	{0x31,1,{0x22}},
	{0x32,1,{0x11}},
	{0x33,1,{0xAA}},
	{0x34,1,{0xBB}},
	{0x35,1,{0x66}},
	{0x36,1,{0x00}},
	{0x37,1,{0x22}},
	{0x38,1,{0x22}},
	{0x39,1,{0x22}},
	{0x3A,1,{0x22}},
	{0x3B,1,{0x22}},
	{0x3C,1,{0x22}},
	{0x3D,1,{0x22}},
	{0x3E,1,{0x22}},
	{0x3F,1,{0x22}},
	{0x40,1,{0x22}},
	{0x52,1,{0x10}},
	{0x53,1,{0x10}},
	{0xFF,5,{0xFF,0x98,0x06,0x04,0x07}},
	{0x17,1,{0x22}},
	{0x02,1,{0x77}},
	{0xE1,1,{0x79}},
	{0xFF,5,{0xFF,0x98,0x06,0x04,0x00}},
	{0x11,1,{0x00}},
	{REGFLAG_DELAY,120,{}},
	{0x29,1,{0x00}},
	{REGFLAG_DELAY,10,{}},
	{0x35,1,{0x00}},
	{REGFLAG_DELAY,10,{}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {

	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 10, {}}, //150

	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},
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
		
        unsigned cmd;
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
#endif

	#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
	#else
		params->dsi.mode   =SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
	#endif
	
#ifndef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	params->dsi.switch_mode = CMD_MODE;
	params->dsi.switch_mode_enable = 0;
#endif

	params->dsi.g_StrLcmInfo = BufLcmInfo;
	// DSI
	/* Command mode setting */
	//1 Three lane or Four lane
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
	params->dsi.packet_size=256;

	// Video mode setting		
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	
	params->dsi.vertical_sync_active				= 4;// 3    2
	params->dsi.vertical_backporch					= 16;// 20   1
	params->dsi.vertical_frontporch					= 20; // 1  12
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 10;// 50  2
	params->dsi.horizontal_backporch				= 80;
	params->dsi.horizontal_frontporch				= 80;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	//params->dsi.LPX=8; 

	// Bit rate calculation
	params->dsi.PLL_CLOCK = 200;

	params->dsi.clk_lp_per_line_enable = 0;
	
	#if defined(LCD_ZHONGGUANGDIAN_45_ILI9806E_CTC_TN)||defined(CONFIG_TERACUBE_2E)
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;//1// 0
	params->dsi.lcm_esd_check_table[0].cmd          = 0x0A;//0x7F
	params->dsi.lcm_esd_check_table[0].count        = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;//0x01
	#endif

}


static void lcm_init(void)
{
 
	SET_RESET_PIN(1);	
	MDELAY(1); 
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);
#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);//
#else
	push_table(NULL, lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}


static void lcm_suspend(void)
{

    //LCM_PRINT("ili9806e:lcm_suspend\n");
#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#endif
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(20);

}

static void lcm_resume(void)
{

    //LCM_PRINT("ili9806e:lcm_resume\n");

	lcm_init();

}

/*
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{

	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(&data_array, 7, 0);

}*/


// ---------------------------------------------------------------------------
//  Get LCM ID Information
// ---------------------------------------------------------------------------
//extern void DSI_clk_HS_mode(char enter);
static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[3];
	unsigned int array[16];  

	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);	

	array[0]=0x00063902;
	array[1]=0x0698FFFF;
	array[2]=0x00000104;
	dsi_set_cmdq(array, 3, 1);
	MDELAY(10); 

	array[0] = 0x00023700;// set return byte number
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x00, buffer, 1);
	read_reg_v2(0x01, buffer+1, 1);
    
	id = buffer[0]<<8 |buffer[1];

	//LCM_PRINT("ili9806e:LCM_ID_ILI9806E=0x%x \n",id);

	return (id == LCM_ID_ILI9806E)?1:0;

} 


// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
#ifdef BUILD_LK //xjl 20180531
LCM_DRIVER ili9806e_6735_dsi_lcm_drv_yk6xx = 
#else
struct LCM_DRIVER ili9806e_6735_dsi_lcm_drv_yk6xx = 
#endif
{

#if LCM_DSI_CMD_MODE
    .name             = "ili9806e_67xx_dsi_cmd",  
#else
    .name             = "ili9806e_67xx_dsi_video",
#endif    

    .set_util_funcs   = lcm_set_util_funcs,
    .get_params       = lcm_get_params,
    .init             = lcm_init,
    .suspend          = lcm_suspend,
    .resume           = lcm_resume,
    .compare_id       = lcm_compare_id,
#if LCM_DSI_CMD_MODE
	.update           = lcm_update,
	//.set_backlight  = lcm_setbacklight,
	//.set_pwm        = lcm_setpwm,
	//.get_pwm        = lcm_getpwm,
	//.esd_check   	  = lcm_esd_check,
    //.esd_recover    = lcm_esd_recover,
	
#endif

};



