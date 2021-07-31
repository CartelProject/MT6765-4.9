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

#define LCM_ID_ST7701S                                     (0x8802)

#if defined(LCM_480_960)
#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(960)
#else
#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)
#endif
//LCD module explaination					    
//Project		Custom		Factory	 	size		W&H		Glass	degree	data		HWversion

#if defined(YK659_CUSTOMER_ZXT_ZA50_FWVGA)
    #define LCD_SANLONG_45_ST7701S_CTC
#elif defined(YK736_CUSTOMER_YKQ_FWVGA_PLUS) //xen 20171024
    #define LCD_JUTAI_57_ST7701S_CTC
    //#define LCD_SANLONG_45_ST7701S_CTC
#elif defined(YK736_CUSTOMER_KUANGRE_A17_FWVGA_PLUS)//zwl 20171210
    #define LCD_JUTAI_ZHONGGUANGDIAN_57_ST7701S_CTC
    #define DOUBLE_LCM_BY_IDPIN
#elif defined(YK736_CUSTOMER_FULING_E729_HD640)  //just for test 20180129
    #define LCD_HANLONG_50_ST7701S_IVO_IPS
#elif defined(YK737_CUSTOMER_CAIFU9_KS972_FWPLUS)
    //#define LCD_HELITAI_45_ST7701S_CTC_FWPLUS
    #define LCD_HELITAI_DEZHIXIN_45_ST7701S_CTC_FWPLUS
    #define DOUBLE_LCM_BY_IDPIN
#else
    #define LCD_SANLONG_45_ST7701S_CTC
#endif


#define LCM_PHYSICAL_WIDTH									(0)
#define LCM_PHYSICAL_HEIGHT									(0)


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
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned int cmd; 
    unsigned char count;
    unsigned char para_list[20]; //xen for less space 20170511
};


#if defined (LCD_SANLONG_45_ST7701S_CTC) //xjl 20170712
#if defined (LCM_ROTATE_180)
	static unsigned char BufLcmInfo[] = "LCD_SANLONG_45_ST7701S_CTC_20170712_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_SANLONG_45_ST7701S_CTC_20170712";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x11,1,{0x00}},
	{REGFLAG_DELAY, 200, {}},
	{0xff, 5,{0x77,0x01,0x00,0x00,0x10}},
	{0xC0, 2,{0xe9,0x03}},
	{0xC1, 2,{0x0C,0x07}},
	{0xC2, 2,{0x37,0x08}},//31-30
	{0xCC, 1,{0x10}},
	{0xB0,16,{0x40,0x02,0x08,0x11,0x17,0x0B,0x07,0x09,0x09,0x1B,0x07,0x16,0x12,0x0E,0x13,0x0F}},
	{0xB1,16,{0x40,0x01,0x08,0x0F,0x15,0x09,0x04,0x09,0x09,0x1C,0x09,0x15,0x12,0x0E,0x12,0x0F}},
	{0xff, 5,{0x77,0x01,0x00,0x00,0x11}},
	{0xb0, 1,{0x4d}},
	{0xb1, 1,{0x60}},//5e-65-60
	{0xb2, 1,{0x8D}},//07-05-04-8D
	{0xb3, 1,{0x80}},
	{0xb5, 1,{0x4D}},//47-42-4D
	{0xb7, 1,{0xCF}},//85-cf
	{0xB8, 1,{0x33}},//20-33
	{0xB9, 1,{0x10}},
	{0xBB, 1,{0x03}},
	{0xC1, 1,{0x78}},
	{0xC2, 1,{0x78}},
	{0xD0, 1,{0x88}},
	{REGFLAG_DELAY, 100, {}},
	{0xe0, 3,{0x00,0xb4,0x02}},
	{0xe1,11,{0x06,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x20,0x20}},
	{0xe2,13,{0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0xe3, 4,{0x00,0x00,0x33,0x33}},
	{0xe4, 2,{0x44,0x44}},
	{0xe5,16,{0x09,0x67,0xBE,0xA0,0x0B,0x67,0xBE,0xA0,0x05,0x67,0xBE,0xA0,0x07,0x67,0xBE,0xA0}},
	{0xe6, 4,{0x00,0x00,0x33,0x33}},
	{0xe7, 2,{0x44,0x44}},
	{0xe8,16,{0x08,0x67,0xBE,0xA0,0x0A,0x67,0xBE,0xA0,0x04,0x67,0xBE,0xA0,0x06,0x67,0xBE,0xA0}},
	{0xEA,16,{0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00}},
	{0xeb, 7,{0x02,0x02,0x00,0x00,0x00,0x00,0x00}},
	{0xEC, 2,{0x02,0x00}},
	{0xed,16,{0xF5,0x47,0x6F,0x0B,0x8F,0x9F,0xFF,0xFF,0xFF,0xFF,0xF9,0xF8,0xB0,0xF6,0x74,0x5F}},
	{0xEF,12,{0x08,0x08,0x08,0x08,0x08,0x08,0x04,0x04,0x04,0x04,0x04,0x04}},
	{REGFLAG_DELAY, 10, {}},
	{0xFF, 5,{0x77,0x01,0x00,0x00,0x00}},
	{0x29,1,{0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#elif defined(LCD_HELITAI_DEZHIXIN_45_ST7701S_CTC_FWPLUS) //xen 20180326
//dezhixin
static unsigned char BufLcmInfo_0[] = "LCD_DEZHIXIN_45_ST7701S_IVO_FWPLUS_20180314";
static struct LCM_setting_table lcm_initialization_setting_0[] = {
	/**IVO5.0TN 480960+ST7701S*/
	{0x11,1,{0x00}},
	{REGFLAG_DELAY, 200, {}},
	{0xff, 5,{0x77,0x01,0x00,0x00,0x10}},
	{0xC0, 2,{0x77,0x00}},
	{0xC1, 2,{0x12,0x02}},

	{0xC2, 2,{0x20,0x02}},//1Dot inversion
	{0xCC, 1,{0x10}},
	{0xB0,16,{0x00,0x09,0x14,0x13,0x18,0x0A,0x15,0x0A,0x09,0x27,0x06,0x10,0x0E,0xD8,0x1B,0x99}},
	{0xB1,16,{0x00,0x09,0x14,0x13,0x19,0x0C,0x13,0x08,0x09,0x28,0x09,0x17,0x13,0xDB,0x20,0x99}},
	{0xff, 5,{0x77,0x01,0x00,0x00,0x11}},
	{0xb0, 1,{0x60}},
	{0xb1, 1,{0x4F}}, //4B-4F
	{0xb2, 1,{0x89}},
	{0xb3, 1,{0x80}},
	{0xb5, 1,{0x4C}},
	{0xb7, 1,{0x85}},
	{0xB8, 1,{0x20}},
	{0xB9, 1,{0x22}},
	{0xBB, 1,{0x33}},
	{0xC0, 1,{0x09}},
	{0xC1, 1,{0x78}},
	{0xC2, 1,{0x78}},
	{0xD0, 1,{0x88}},
	{REGFLAG_DELAY, 10, {}},
	{0xe0, 3,{0x00,0x00,0x02}},
	{0xe1,11,{0x0a,0x8c,0x0c,0x8c,0x0b,0x8c,0x0d,0x8c,0x00,0x44,0x44}},
	{0xe2,13,{0x33,0x33,0x44,0x44,0xcf,0x8c,0xd1,0x8c,0xd0,0x8c,0xd2,0x8c,0x00}},
	{0xe3, 4,{0x00,0x00,0x33,0x33}},
	{0xe4, 2,{0x44,0x44}},
	{0xe5,16,{0x0c,0xD0,0x8c,0x8e,0x0e,0xD2,0x8c,0x8e,0x10,0xD4,0x8c,0x8e,0x12,0xD6,0x8c,0x8e}},
	{0xe6, 4,{0x00,0x00,0x33,0x33}},
	{0xe7, 2,{0x44,0x44}},
	{0xe8,16,{0x0D,0xD1,0x8c,0x8e,0x0f,0xD3,0x8c,0x8e,0x11,0xD5,0x8c,0x8e,0x13,0xD7,0x8c,0x8e}},	
	{0xeb, 7,{0x02,0x01,0x4E,0x4E,0xee,0x44,0x00}},
	{0xEC, 2,{0x00,0x00}},
	{0xed,16,{0xff,0xf1,0x04,0x56,0x72,0x3F,0xFF,0xFF,0xFF,0xFF,0xF3,0x27,0x65,0x40,0x1f,0xff}},	
  	{0xFF, 5,{0x77,0x01,0x00,0x00,0x13}},  
  	{0xE6, 2,{0x16,0x7c}},
	{0xFF, 5,{0x77,0x01,0x00,0x00,0x00}},
	{0x29,1,{0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

//helitai
static unsigned char BufLcmInfo_1[] = "LCD_HELITAI_45_ST7701S_CTC_FWPLUS_20180309";
static struct LCM_setting_table lcm_initialization_setting_1[] = {
	{0x11,1,{0x00}},
	{REGFLAG_DELAY, 200, {}},
	{0xff, 5,{0x77,0x01,0x00,0x00,0x10}},
	{0xC0, 2,{0x77,0x00}},
	{0xC1, 2,{0x0E,0x0C}},
	{0xC2, 2,{0x01,0x02}}, //0x01
	{0xCC, 1,{0x18}},
	{0xB0,16,{0x00,0x05,0x0c,0x12,0x1a,0x0c,0x07,0x09,0x08,0x1c,0x05,0x12,0x0f,0x0e,0x12,0x14}},
	{0xB1,16,{0x00,0x05,0x0c,0x10,0x14,0x08,0x08,0x09,0x09,0x1f,0x0a,0x17,0x13,0x14,0x1a,0x1d}},

	{0xff, 5,{0x77,0x01,0x00,0x00,0x11}},
	{0xb0, 1,{0x4d}},
	{0xb1, 1,{0x52}}, //3a-4a
	{0xb2, 1,{0x87}},
	{0xb3, 1,{0x80}},
	{0xb5, 1,{0x47}},
	{0xb7, 1,{0x89}},//0x89
	{0xB8, 1,{0x21}},
	{0xB9, 2,{0x00,0x13}},
//	{0xBB, 1,{0x03}},
	{0xC1, 1,{0x78}},
	{0xC2, 1,{0x78}},
	{0xD0, 1,{0x88}},
	{REGFLAG_DELAY, 100, {}},
	{0xe0, 3,{0x00,0xA0,0x02}},
	{0xe1,11,{0x06,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x20,0x20}},
	{0xe2,13,{0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0xe3, 4,{0x00,0x00,0x33,0x33}},
	{0xe4, 2,{0x44,0x44}},
	{0xe5,16,{0x09,0xD4,0xA0,0x8C,0x0B,0xD4,0xA0,0x8C,0x05,0xD4,0xA0,0x8C,0x07,0xD4,0xA0,0x8C}},//0xAA
	{0xe6, 4,{0x00,0x00,0x33,0x33}},
	{0xe7, 2,{0x44,0x44}},
	{0xe8,16,{0x08,0xD4,0xA0,0x8C,0x0A,0xD4,0xA0,0x8C,0x04,0xD4,0xA0,0x8C,0x06,0xD4,0xA0,0x8C}},
	{0xEA,16,{0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00}},
	{0xeb, 7,{0x02,0x02,0x4E,0x4E,0x44,0x00,0x10}},
	{0xEC, 2,{0x02,0x01}},
	{0xed,16,{0x05,0x47,0x61,0xFF,0x8F,0x9F,0xFF,0xFF,0xFF,0xFF,0xF9,0xF8,0xFF,0x16,0x74,0x50}},
	{0xEF,12,{0x08,0x08,0x08,0x08,0x08,0x08,0x04,0x04,0x04,0x04,0x04,0x04}},
	{REGFLAG_DELAY, 10, {}},
	{0xFF, 5,{0x77,0x01,0x00,0x00,0x00}},
	{0x29,1,{0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#elif defined (LCD_HELITAI_45_ST7701S_CTC_FWPLUS) //xjl 20180130
#if defined (LCM_ROTATE_180)
	static unsigned char BufLcmInfo[] = "LCD_HELITAI_45_ST7701S_CTC_FWPLUS_20180130_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_HELITAI_45_ST7701S_CTC_FWPLUS_20180130";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x11,1,{0x00}},
	{REGFLAG_DELAY, 200, {}},
	{0xff, 5,{0x77,0x01,0x00,0x00,0x10}},
	{0xC0, 2,{0x77,0x00}},
	{0xC1, 2,{0x0E,0x0C}},
	{0xC2, 2,{0x01,0x03}},
	{0xCC, 1,{0x18}},
	{0xB0,16,{0x40,0x06,0x8D,0x12,0x19,0x0B,0x0A,0x09,0x08,0x1E,0x08,0x16,0x11,0x0D,0x11,0x19}},
	{0xB1,16,{0x40,0x05,0x8D,0x12,0x15,0x0A,0x08,0x09,0x09,0x1E,0x08,0x15,0x12,0x93,0x18,0x19}},
	{0xff, 5,{0x77,0x01,0x00,0x00,0x11}},
	{0xb0, 1,{0x4d}},
	{0xb1, 1,{0x59}},
	{0xb2, 1,{0x87}},
	{0xb3, 1,{0x80}},
	{0xb5, 1,{0x47}},
	{0xb7, 1,{0x89}},
	{0xB8, 1,{0x21}},
	{0xB9, 2,{0x00,0x13}},
//	{0xBB, 1,{0x03}},
	{0xC1, 1,{0x78}},
	{0xC2, 1,{0x78}},
	{0xD0, 1,{0x88}},
	{REGFLAG_DELAY, 100, {}},
	{0xe0, 3,{0x00,0xA0,0x02}},
	{0xe1,11,{0x06,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x20,0x20}},
	{0xe2,13,{0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0xe3, 4,{0x00,0x00,0x33,0x33}},
	{0xe4, 2,{0x44,0x44}},
	{0xe5,16,{0x09,0xD4,0xAA,0x8C,0x0B,0xD4,0xAA,0x8C,0x05,0xD4,0xAA,0x8C,0x07,0xD4,0xAA,0x8C}},
	{0xe6, 4,{0x00,0x00,0x33,0x33}},
	{0xe7, 2,{0x44,0x44}},
	{0xe8,16,{0x08,0xD4,0xAA,0x8C,0x0A,0xD4,0xAA,0x8C,0x04,0xD4,0xAA,0x8C,0x06,0xD4,0xAA,0x8C}},
	{0xEA,16,{0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00}},
	{0xeb, 7,{0x02,0x02,0x4E,0x4E,0x44,0x00,0x10}},
	{0xEC, 2,{0x02,0x01}},
	{0xed,16,{0x05,0x47,0x61,0xFF,0x8F,0x9F,0xFF,0xFF,0xFF,0xFF,0xF9,0xF8,0xFF,0x16,0x74,0x50}},
	{0xEF,12,{0x08,0x08,0x08,0x08,0x08,0x08,0x04,0x04,0x04,0x04,0x04,0x04}},
	{REGFLAG_DELAY, 10, {}},
	{0xFF, 5,{0x77,0x01,0x00,0x00,0x00}},
	{0x29,1,{0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#elif defined (LCD_JUTAI_57_ST7701S_CTC) //xen 20171024
#if defined (LCM_ROTATE_180)
	static unsigned char BufLcmInfo[] = "LCD_JUTAI_57_ST7701S_CTC_20171024_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_JUTAI_57_ST7701S_CTC_20171024";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {

	{0x01,0,{0x00}},
	{REGFLAG_DELAY, 120, {0}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},
	{0xD1,1,{0x11}},

	{0x11,0,{0x00}},
	{REGFLAG_DELAY, 200, {}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x10}},
	{0xC0,2,{0x77,0x00}},
	{0xC1,2,{0x0E,0x0C}},
	{0xC2,2,{0x07,0x03}},
	{0xCC,1,{0x10}},
	{0xB0,16,{0x40,0x02,0x8D,0x11,0x17,0x0A,0x08,0x09,0x09,0x1B,0x07,0x16,0x12,0x0E,0x15,0x19}},
	{0xB1,16,{0x40,0x02,0x8D,0x11,0x16,0x09,0x08,0x08,0x08,0x1F,0x07,0x15,0x12,0x91,0x15,0x19}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},
	{0xB0,1,{0x4D}},
	{0xB1,1,{0x4D}},
	{0xB2,1,{0x80}},
	{0xB3,1,{0x80}},
	{0xB5,1,{0x40}},
	{0xB7,1,{0x8A}},
	{0xB8,1,{0x23}},
	{0xB9,2,{0x00,0x13}},
	{0xC0,1,{0x09}},
	{0xC1,1,{0x78}},
	{0xC2,1,{0x78}},
	{0xD0,1,{0x88}},
	{0xE0,3,{0x00,0xA0,0x02}},
	{0xE1,11,{0x06,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x20,0x20}},
	{0xE2,13,{0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0xE3,4,{0x00,0x00,0x33,0x33}},
	{0xE4,2,{0x44,0x44}},
	{0xE5,16,{0x09,0xD4,0xAA,0x8C,0x0B,0xD4,0xAA,0x8C,0x05,0xD4,0xAA,0x8C,0x07,0xD4,0xAA,0x8C}},
	{0xE6,4,{0x00,0x00,0x33,0x33}},
	{0xE7,2,{0x44,0x44}},
	{0xE8,16,{0x08,0xD4,0xAA,0x8C,0x0A,0xD4,0xAA,0x8C,0x04,0xD4,0xAA,0x8C,0x06,0xD4,0xAA,0x8C}},
	{0xEA,16,{0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00}},
	{0xEB,7,{0x02,0x02,0x4E,0x4E,0x44,0x00,0x10}},
	{0xEC,2,{0x02,0x01}},
	{0xED,16,{0x05,0x47,0x61,0xFF,0x8F,0x9F,0xFF,0xFF,0xFF,0xFF,0xF9,0xF8,0xFF,0x16,0x74,0x50}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x00}},
	{0x29,0,{0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}

};
#elif defined(LCD_HANLONG_50_ST7701S_IVO_IPS)
static unsigned char BufLcmInfo[] = "LCD_HANLONG_50_ST7701S_IVO_IPS_20180203";
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x01,0,{0x00}},
	{REGFLAG_DELAY, 120, {}},
	{0x11,0,{0x00}},
	{REGFLAG_DELAY, 120, {}},
//---------------------------------------Bank0 Setting-------------------------------------------------//
//------------------------------------Display Control setting----------------------------------------------//
	{0xFF, 5, {0x77,0x01,0x00,0x00,0x10}},
	{0xC0, 2, {0x77,0x00}},
	{0xC1, 2, {0x12,0x02}},
	{0xC2, 2, {0x07,0x02}},
//-------------------------------------Gamma Cluster Setting-------------------------------------------//
	{0xB0, 16, {0x00,0x10,0x1B,0x0F,0x14,0x08,0x0D,0x08,0x08,0x25,0x06,0x15,0x13,0xE6,0x2C,0x11}},
	{0xB1, 16, {0x00,0x10,0x1B,0x0F,0x14,0x08,0x0E,0x08,0x08,0x25,0x04,0x11,0x0F,0x27,0x2C,0x11}},
//---------------------------------------End Gamma Setting----------------------------------------------//
//------------------------------------End Display Control setting----------------------------------------//
//-----------------------------------------Bank0 Setting End---------------------------------------------//
//-------------------------------------------Bank1 Setting---------------------------------------------------//
//-------------------------------- Power Control Registers Initial --------------------------------------//
	{0xFF, 5, {0x77,0x01,0x00,0x00,0x11}},
	{0xB0, 1, {0x5D}},
//-------------------------------------------Vcom Setting---------------------------------------------------//
	{0xB1, 1, {0x16}},
//-----------------------------------------End Vcom Setting-----------------------------------------------//
	{0xB2, 1, {0x87}},
	{0xB3, 1, {0x80}},
	{0xB5, 1, {0x49}},
	{0xB7, 1, {0x89}},
	{0xB8, 1, {0x21}},
	{0xC1, 1, {0x78}},
	{0xC2, 1, {0x78}},
	{0xD0, 1, {0x88}},
//---------------------------------End Power Control Registers Initial -------------------------------//
	{REGFLAG_DELAY, 100, {}},
//---------------------------------------------GIP Setting----------------------------------------------------//
	{0xE0, 3, {0x00,0x00,0x02}},
	{0xE1, 11, {0x0A,0x96,0x0C,0x96,0x0B,0x96,0x0D,0x96,0x00,0x44,0x44}},
	{0xE2, 13, {0x33,0x33,0x44,0x44,0xCF,0x96,0xD1,0x96,0xD0,0x96,0xD2,0x96,0x00}},
	{0xE3, 4, {0x00,0x00,0x33,0x33}},
	{0xE4, 2, {0x44,0x44}},
	{0xE5, 16, {0x0C,0xD0,0x96,0x96,0x0E,0xD2,0x96,0x96,0x10,0xD4,0x96,0x96,0x12,0xD6,0x96,0x96}},
	{0xE6, 4, {0x00,0x00,0x33,0x33}},
	{0xE7, 2, {0x44,0x44}},
	{0xE8, 16, {0x0D,0xD1,0x96,0x96,0x0F,0xD3,0x96,0x96,0x11,0xD5,0x96,0x96,0x13,0xD7,0x96,0x96}},
	{0xEB, 7, {0x02,0x01,0x4E,0x4E,0xEE,0x44,0x00}},
	{0xED, 16, {0xFF,0xF1,0x04,0x56,0x72,0x3F,0xFF,0xFF,0xFF,0xFF,0xF3,0x27,0x65,0x40,0x1F,0xFF}},
//---------------------------------------------End GIP Setting-----------------------------------------------//
//------------------------------ Power Control Registers Initial End-----------------------------------//
//------------------------------------------Bank1 Setting----------------------------------------------------//
	{0xFF, 5, {0x77,0x01,0x00,0x00,0x00}},

	{0x29,0,{0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#elif defined (LCD_JUTAI_ZHONGGUANGDIAN_57_ST7701S_CTC) //zwl 20171221
//jutai_ctc	
static unsigned char BufLcmInfo_0[] = "LCD_JUTAI_57_ST7701S_CTC_A17_20171221";
static struct LCM_setting_table lcm_initialization_setting_0[] = {
	{0x01,0,{0x00}},
	{REGFLAG_DELAY, 120, {0}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},
	{0xD1,1,{0x11}},

	{0x11,0,{0x00}},
	{REGFLAG_DELAY, 200, {}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x10}},
	{0xC0,2,{0x77,0x00}},
	{0xC1,2,{0x0E,0x0C}},
	{0xC2,2,{0x01,0x03}},
	{0xCC,1,{0x10}},
	{0xB0,16,{0x40,0x02,0x8D,0x11,0x17,0x0A,0x08,0x09,0x09,0x1B,0x07,0x16,0x12,0x0E,0x15,0x19}},
	{0xB1,16,{0x40,0x02,0x8D,0x11,0x16,0x09,0x08,0x08,0x08,0x1F,0x07,0x15,0x12,0x91,0x15,0x19}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},
	{0xB0,1,{0x4D}},
	{0xB1,1,{0x57}},
	{0xB2,1,{0x87}},
	{0xB3,1,{0x80}},
	{0xB5,1,{0x47}},
	{0xB7,1,{0x89}},
	{0xB8,1,{0x21}},
	{0xB9,2,{0x00,0x13}},
	{0xC0,1,{0x09}},
	{0xC1,1,{0x78}},
	{0xC2,1,{0x78}},
	{0xD0,1,{0x88}},

	{0xE0,3,{0x00,0xA0,0x02}},
	{0xE1,11,{0x06,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x20,0x20}},
	{0xE2,13,{0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0xE3,4,{0x00,0x00,0x33,0x33}},
	{0xE4,2,{0x44,0x44}},
	{0xE5,16,{0x09,0xD4,0xAA,0x8C,0x0B,0xD4,0xAA,0x8C,0x05,0xD4,0xAA,0x8C,0x07,0xD4,0xAA,0x8C}},

	{0xE6,4,{0x00,0x00,0x33,0x33}},
	{0xE7,2,{0x44,0x44}},
	{0xE8,16,{0x08,0xD4,0xAA,0x8C,0x0A,0xD4,0xAA,0x8C,0x04,0xD4,0xAA,0x8C,0x06,0xD4,0xAA,0x8C}},
	{0xEA,16,{0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00}},
	{0xEB,7,{0x02,0x02,0x4E,0x4E,0x44,0x00,0x10}},

	{0xEC,2,{0x02,0x01}},
	{0xED,16,{0x05,0x47,0x61,0xFF,0x8F,0x9F,0xFF,0xFF,0xFF,0xFF,0xF9,0xF8,0xFF,0x16,0x74,0x50}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x00}},
	{0x29,0,{0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}

};

//zhongguandian_ctc
static unsigned char BufLcmInfo_1[] = "LCD_ZHONGGUANDIAN_57_ST7701S_CTC_20171221";
static struct LCM_setting_table lcm_initialization_setting_1[] = {
		{0x01,0,{0x00}},
	{REGFLAG_DELAY, 120, {0}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},
	{0xD1,1,{0x11}},

	{0x11,0,{0x00}},
	{REGFLAG_DELAY, 200, {}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x10}},
	{0xC0,2,{0x77,0x00}},
	{0xC1,2,{0x0E,0x0C}},
	{0xC2,2,{0x01,0x03}},
	{0xCC,1,{0x10}},
	{0xB0,16,{0x40,0x02,0x8D,0x11,0x17,0x0A,0x08,0x09,0x09,0x1B,0x07,0x16,0x12,0x0E,0x15,0x19}},
	{0xB1,16,{0x40,0x02,0x8D,0x11,0x16,0x09,0x08,0x08,0x08,0x1F,0x07,0x15,0x12,0x91,0x15,0x19}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x11}},
	{0xB0,1,{0x4D}},
	{0xB1,1,{0x57}},
	{0xB2,1,{0x87}},
	{0xB3,1,{0x80}},
	{0xB5,1,{0x47}},
	{0xB7,1,{0x89}},
	{0xB8,1,{0x21}},
	{0xB9,2,{0x00,0x13}},
	{0xC0,1,{0x09}},
	{0xC1,1,{0x78}},
	{0xC2,1,{0x78}},
	{0xD0,1,{0x88}},
	{0xE0,3,{0x00,0xA0,0x02}},
	{0xE1,11,{0x06,0x00,0x00,0x00,0x05,0x00,0x00,0x00,0x00,0x20,0x20}},
	{0xE2,13,{0x00,0x00,0x01,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00}},
	{0xE3,4,{0x00,0x00,0x33,0x33}},
	{0xE4,2,{0x44,0x44}},
	{0xE5,16,{0x09,0xD4,0xAA,0x8C,0x0B,0xD4,0xAA,0x8C,0x05,0xD4,0xAA,0x8C,0x07,0xD4,0xAA,0x8C}},
	{0xE6,4,{0x00,0x00,0x33,0x33}},
	{0xE7,2,{0x44,0x44}},
	{0xE8,16,{0x08,0xD4,0xAA,0x8C,0x0A,0xD4,0xAA,0x8C,0x04,0xD4,0xAA,0x8C,0x06,0xD4,0xAA,0x8C}},
	{0xEA,16,{0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x00}},
	{0xEB,7,{0x02,0x02,0x4E,0x4E,0x44,0x00,0x10}},
	{0xEC,2,{0x02,0x01}},
	{0xED,16,{0x05,0x47,0x61,0xFF,0x8F,0x9F,0xFF,0xFF,0xFF,0xFF,0xF9,0xF8,0xFF,0x16,0x74,0x50}},
	{0xFF,5,{0x77,0x01,0x00,0x00,0x00}},
	{0x29,0,{0x00}},
        {0x21,0,{0x00}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}

};
#endif

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
	//{0x22, 0, {0x00}},
	//{REGFLAG_DELAY, 100, {}},

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

#if defined(DOUBLE_LCM_BY_IDPIN)
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
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;

#ifndef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	params->dsi.switch_mode = CMD_MODE;
	params->dsi.switch_mode_enable = 0;
#endif
	params->dsi.LANE_NUM	= LCM_TWO_LANE;
	
	if (get_lcd_id_state()==0)
	  params->dsi.g_StrLcmInfo = BufLcmInfo_0;
	else
	  params->dsi.g_StrLcmInfo = BufLcmInfo_1;	
	
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order     = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq	   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding 	   = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format	       = LCM_DSI_FORMAT_RGB888;
	
	params->dsi.packet_size=256;
	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
	//params->dsi.vertical_active_line=FRAME_HEIGHT;
	
#if defined(LCD_JUTAI_ZHONGGUANGDIAN_57_ST7701S_CTC)
    if (get_lcd_id_state()==0)
      {
	params->dsi.vertical_sync_active				= 6;
	params->dsi.vertical_backporch					= 18;
	params->dsi.vertical_frontporch					= 18;
	
	params->dsi.horizontal_sync_active				= 20;
	params->dsi.horizontal_backporch				= 100;
	params->dsi.horizontal_frontporch				= 100;
      }
     else
      {
	params->dsi.vertical_sync_active				= 6;
	params->dsi.vertical_backporch					= 18;
	params->dsi.vertical_frontporch					= 18;
	
	params->dsi.horizontal_sync_active				= 20;
	params->dsi.horizontal_backporch				= 100;
	params->dsi.horizontal_frontporch				= 100;
      }
	params->dsi.PLL_CLOCK = 180; //200;
#elif defined(LCD_HELITAI_DEZHIXIN_45_ST7701S_CTC_FWPLUS) //xen 20180326
    if (get_lcd_id_state()==1)
    {//helitai
	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 16;
	params->dsi.vertical_frontporch					= 14;
	
	params->dsi.horizontal_sync_active				= 10;
	params->dsi.horizontal_backporch				= 20;
	params->dsi.horizontal_frontporch				= 16;
    }
    else
    {//dezhixin
	params->dsi.vertical_sync_active				= 4; //8;
	params->dsi.vertical_backporch					= 25; //50;
	params->dsi.vertical_frontporch					= 20; //30;
	
	params->dsi.horizontal_sync_active				= 10; //8;
	params->dsi.horizontal_backporch				= 40;
	params->dsi.horizontal_frontporch				= 30;
    }
	params->dsi.PLL_CLOCK = 200;
#else
	params->dsi.vertical_sync_active				= 4;  //3
	params->dsi.vertical_backporch					= 6;  //20
	params->dsi.vertical_frontporch					= 10;	//20
	
	params->dsi.horizontal_sync_active				= 10;  //10
	params->dsi.horizontal_backporch				= 10; //50
	params->dsi.horizontal_frontporch				= 10; //50
	params->dsi.horizontal_blanking_pixel			= 60;  //60

	params->dsi.PLL_CLOCK = 180; //200;
#endif	
	params->dsi.vertical_active_line				= FRAME_HEIGHT;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	params->dsi.compatibility_for_nvk = 0;	// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's
	
	//params->dsi.PLL_CLOCK = 180; //200;

#if defined(YK606_CUSTOMER_XINDI_V1_FWVGA_50)||defined(YK610_CUSTOMER_SHENGXIN_V1_FWVGA)||defined(Q578_CUSTOMER_CAIFU3_TS500_FWVGA_50)\
	||defined(YK630_CUSTOMER_CAIFU_FS510_FWVGA)
     params->dsi.noncont_clock = 1;   
     params->dsi.esd_check_enable =1;
     params->dsi.customization_esd_check_enable =1;
     params->dsi.lcm_esd_check_table[0].cmd =0x0A;
     params->dsi.lcm_esd_check_table[0].count =1;
     params->dsi.lcm_esd_check_table[0].para_list[0] =0x9C;
#endif
	 
}
#else
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
	params->dsi.mode   = SYNC_PULSE_VDO_MODE;

#ifndef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	params->dsi.switch_mode = CMD_MODE;
	params->dsi.switch_mode_enable = 0;
#endif
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
	params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
	//params->dsi.vertical_active_line=FRAME_HEIGHT;
	
#if defined(LCD_SANLONG_45_ST7701S_CTC)
	params->dsi.vertical_sync_active				= 6;
	params->dsi.vertical_backporch					= 18;
	params->dsi.vertical_frontporch					= 18;
	
	params->dsi.horizontal_sync_active				= 20;
	params->dsi.horizontal_backporch				= 100;
	params->dsi.horizontal_frontporch				= 100;
#elif defined(LCD_HELITAI_45_ST7701S_CTC_FWPLUS) //xjl 20180130
	params->dsi.vertical_sync_active				= 6;
	params->dsi.vertical_backporch					= 18;
	params->dsi.vertical_frontporch					= 18;
	
	params->dsi.horizontal_sync_active				= 20;
	params->dsi.horizontal_backporch				= 100;
	params->dsi.horizontal_frontporch				= 100;
#elif defined(LCD_HANLONG_50_ST7701S_IVO_IPS) //xen 20180202
	params->dsi.vertical_sync_active				= 6;
	params->dsi.vertical_backporch					= 18;
	params->dsi.vertical_frontporch					= 20;//18;
	
	params->dsi.horizontal_sync_active				= 10;//20;
	params->dsi.horizontal_backporch				= 40;//100;
	params->dsi.horizontal_frontporch				= 40;//100;
#elif defined(LCD_JUTAI_57_ST7701S_CTC)
	params->dsi.vertical_sync_active				= 4; //6;
	params->dsi.vertical_backporch					= 20; //18;
	params->dsi.vertical_frontporch					= 18;
	
	params->dsi.horizontal_sync_active				= 10; //20;
	params->dsi.horizontal_backporch				= 20; //100;
	params->dsi.horizontal_frontporch				= 20; //100;
#elif defined(LCD_DZX_50_ST7701S_IVO)	 //ZLH 20170516
	params->dsi.vertical_sync_active				= 4;  //3
	params->dsi.vertical_backporch					= 12;  //20
	params->dsi.vertical_frontporch					= 10;	//20
	
	params->dsi.horizontal_sync_active				= 10;  //10
	params->dsi.horizontal_backporch				= 10; //50
	params->dsi.horizontal_frontporch				= 10; //50
	params->dsi.horizontal_blanking_pixel			= 60;  //60
#else
	params->dsi.vertical_sync_active				= 4;  //3
	params->dsi.vertical_backporch					= 6;  //20
	params->dsi.vertical_frontporch					= 10;	//20
	
	params->dsi.horizontal_sync_active				= 10;  //10
	params->dsi.horizontal_backporch				= 10; //50
	params->dsi.horizontal_frontporch				= 10; //50
	params->dsi.horizontal_blanking_pixel			= 60;  //60
#endif	
	params->dsi.vertical_active_line				= FRAME_HEIGHT;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	params->dsi.compatibility_for_nvk = 0;		// this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's

#if defined(LCD_SANLONG_45_ST7701S_CTC)//||defined(LCD_JUTAI_57_ST7701S_CTC)
	params->dsi.PLL_CLOCK = 200;
#elif defined (LCD_HELITAI_45_ST7701S_CTC_FWPLUS) //xjl 20180130
	params->dsi.PLL_CLOCK = 200;
#else		
	params->dsi.PLL_CLOCK = 180; //200;
#endif

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
#if defined(YK606_CUSTOMER_JIALANDE_1_FWVGA_50) //zlh 20170519
     params->dsi.noncont_clock=1;
     params->dsi.clk_lp_per_line_enable=1; //1=per frame //3=per line
#endif
}
#endif

static void lcm_init(void)
{
	//LCM_PRINT("%s\n", __func__);
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);
#if defined(DOUBLE_LCM_BY_IDPIN)
    if(get_lcd_id_state()==0)
       {
         #ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020	
	   push_table(lcm_initialization_setting_0, sizeof(lcm_initialization_setting_0) / sizeof(struct LCM_setting_table), 1);
         #else
	   push_table(NULL, lcm_initialization_setting_0, sizeof(lcm_initialization_setting_0) / sizeof(struct LCM_setting_table), 1);
         #endif
      }
    else
       {
         #ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020	
	   push_table(lcm_initialization_setting_1, sizeof(lcm_initialization_setting_1) / sizeof(struct LCM_setting_table), 1);
         #else
	   push_table(NULL, lcm_initialization_setting_1, sizeof(lcm_initialization_setting_1) / sizeof(struct LCM_setting_table), 1);
         #endif
       }

#else
        #ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020	
	  push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
        #else
	  push_table(NULL, lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
        #endif
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
	
#if defined(YK606_CUSTOMER_XINDI_V1_FWVGA_50)||defined(YK610_CUSTOMER_SHENGXIN_V1_FWVGA)//cty for esd
	push_table(lcm_close_ic_power, sizeof(lcm_close_ic_power) / sizeof(struct LCM_setting_table), 1);
#endif

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

	//LCM_PRINT("<<ST7701S ID Read>>%s, device_id=0x%04x,buffer[0]=%x,buffer[1]=%x\n", __func__, device_id,buffer[0],buffer[1]);
#ifndef BUILD_LK
	printk("ST7701S ID Read device_id=0x%x", device_id);
#endif
	return (LCM_ID_ST7701S == device_id) ? 1 : 0;
	//return 1;
}


#ifdef BUILD_LK //xjl 20180531
LCM_DRIVER st7701s_6735_dsi_lcm_drv_yk6xx = 
#else
struct LCM_DRIVER st7701s_6735_dsi_lcm_drv_yk6xx = 
#endif
{
    	.name		= "st7701s_67xx_dsi_video", 
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};

