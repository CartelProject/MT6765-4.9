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

#define LCM_ID_JD9366 (0x9366)

#define LCM_DBG(fmt, arg...)
	//LCM_PRINT ("[LCM-ILI9881-DSI-VDO] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE									0

#if defined(LCM_720_1440)
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1440)
#elif defined(LCM_640_1280)
#define FRAME_WIDTH  										(640)
#define FRAME_HEIGHT 										(1280)
#else
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#endif

#define LCM_PHYSICAL_WIDTH									(0)
#define LCM_PHYSICAL_HEIGHT									(0)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0x100   // END OF REGISTERS MARKER


#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif


#if defined(YK736_CUSTOMER_KUANGRE_A17_HDPLUS)
 #define LCD_XIAOZHIXIN_JD9366_IPS_57
#elif defined(YK688_CUSTOMER_SK_P6_HDPLUS)
 #define LCD_KALAIDE_JD9366D_XM_IPS_57
#else
 #define LCD_XIAOZHIXIN_JD9366_IPS_57
#endif 


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
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned int cmd; //zxs 20151127
    unsigned char count;
    unsigned char para_list[5];//64];  //xen for less space 20170511
};

#if defined(LCD_XIAOZHIXIN_JD9366_IPS_57)
static unsigned char BufLcmInfo[] = "LCD_XIAOZHIXIN_JD9366_IPS_57_20180320";
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xE0,1,{0x00}},
//--- PASSWORD  ----//
{0xE1,1,{0x93}},
{0xE2,1,{0x66}},
{0xE3,1,{0xF9}},
{0x80,1,{0x03}},
//--- Page1  ----//
{0xE0,1,{0x01}},
{0x00,1,{0x01}},
{0x01,1,{0x2A}},///
{0x03,1,{0x01}},
{0x04,1,{0x1D}},
{0x0A,1,{0x08}},
{0x13,1,{0x00}},
{0x14,1,{0x99}},	
{0x17,1,{0x00}},
{0x18,1,{0xA4}},	
{0x19,1,{0x00}},
{0x1A,1,{0x00}},
{0x1B,1,{0xA4}},  
{0x1C,1,{0x00}},               
{0x1F,1,{0x32}},	
{0x20,1,{0x2F}},	
{0x21,1,{0x19}},	
{0x22,1,{0x0D}},	
{0x23,1,{0x02}},	
{0x24,1,{0xFE}},
{0x26,1,{0xDF}},
{0x35,1,{0x13}},	
{0x37,1,{0x09}},
{0x38,1,{0x04}},	
{0x39,1,{0x01}},	//0C
{0x3A,1,{0x03}},	
{0x3C,1,{0x60}},	
{0x3D,1,{0x18}},	
{0x3E,1,{0x80}},	
{0x3F,1,{0x4E}},
{0x3D,1,{0xFF}},	
{0x3E,1,{0xFF}},	
{0x3F,1,{0xFF}},               
{0x4B,1,{0x04}}, 		

{0x40,1,{0x04}},
{0x41,1,{0xB4}},	
{0x42,1,{0x70}},	
{0x43,1,{0x24}},
{0x44,1,{0x0C}},	
{0x45,1,{0x64}},	
{0x55,1,{0x01}},	//05 3power    //01  2power 
{0x56,1,{0x01}},
{0x57,1,{0x6D}},	
{0x58,1,{0x0A}},	
{0x59,1,{0x8A}},	
{0x5A,1,{0x28}},	
{0x5B,1,{0x23}},
{0x5C,1,{0x15}},	
{0x5D,1,{0x75}},
{0x5E,1,{0x61}},
{0x5F,1,{0x53}},
{0x60,1,{0x47}},
{0x61,1,{0x43}},
{0x62,1,{0x34}},
{0x63,1,{0x39}},
{0x64,1,{0x22}},
{0x65,1,{0x3B}},
{0x66,1,{0x3A}},
{0x67,1,{0x3B}},
{0x68,1,{0x5C}},
{0x69,1,{0x4D}},
{0x6A,1,{0x57}},
{0x6B,1,{0x4B}},
{0x6C,1,{0x4B}},
{0x6D,1,{0x3E}},
{0x6E,1,{0x2E}},
{0x6F,1,{0x1A}},

{0x70,1,{0x75}},
{0x71,1,{0x61}},
{0x72,1,{0x53}},
{0x73,1,{0x47}},
{0x74,1,{0x43}},
{0x75,1,{0x34}},
{0x76,1,{0x39}},
{0x77,1,{0x22}},
{0x78,1,{0x3B}},
{0x79,1,{0x3A}},
{0x7A,1,{0x3B}},
{0x7B,1,{0x5C}},
{0x7C,1,{0x4D}},
{0x7D,1,{0x57}},
{0x7E,1,{0x4B}},
{0x7F,1,{0x4B}},
{0x80,1,{0x3E}},
{0x81,1,{0x2E}},
{0x82,1,{0x1A}},

//Page2
{0xE0,1,{0x02}},
{0x00,1,{0x5D}},
{0x01,1,{0x51}},//ETV1
{0x02,1,{0x5D}},
{0x03,1,{0x5D}},
{0x04,1,{0x5E}},//VGH
{0x05,1,{0x5F}},//VGL
{0x06,1,{0x51}},//ETV1
{0x07,1,{0x41}},//STV1
{0x08,1,{0x45}},//CKV1
{0x09,1,{0x5D}},
{0x0A,1,{0x5D}},
{0x0B,1,{0x4B}},//CKV7
{0x0C,1,{0x49}},//CKV5
{0x0D,1,{0x5D}},
{0x0E,1,{0x5D}},
{0x0F,1,{0x47}},//CKV3
{0x10,1,{0x5F}},//VGL
{0x11,1,{0x5D}},
{0x12,1,{0x5D}},
{0x13,1,{0x5D}},
{0x14,1,{0x5D}},
{0x15,1,{0x5D}},
{0x16,1,{0x5D}},
{0x17,1,{0x50}},//ETV0
{0x18,1,{0x5D}},
{0x19,1,{0x5D}},
{0x1A,1,{0x5E}},//VGH
{0x1B,1,{0x5F}},//VGL
{0x1C,1,{0x50}},//ETV0
{0x1D,1,{0x40}},//STV0
{0x1E,1,{0x44}},//CKV0
{0x1F,1,{0x5D}},
{0x20,1,{0x5D}},
{0x21,1,{0x4A}},//CKV6
{0x22,1,{0x48}},//CKV4
{0x23,1,{0x5D}},
{0x24,1,{0x5D}},
{0x25,1,{0x46}},//CKV2
{0x26,1,{0x5F}},//VGL
{0x27,1,{0x5D}},
{0x28,1,{0x5D}},
{0x29,1,{0x5D}},
{0x2A,1,{0x5D}},
{0x2B,1,{0x5D}},
{0x2C,1,{0x1D}},
{0x2D,1,{0x10}},//ETV0
{0x2E,1,{0x1D}},
{0x2F,1,{0x1D}},
{0x30,1,{0x1F}},//VGL
{0x31,1,{0x1E}},//VGH
{0x32,1,{0x00}},//STV0
{0x33,1,{0x10}},//ETV0
{0x34,1,{0x06}},//CKV2
{0x35,1,{0x1D}},
{0x36,1,{0x1D}},
{0x37,1,{0x08}},//CKV4
{0x38,1,{0x0A}},//CKV6
{0x39,1,{0x1D}},
{0x3A,1,{0x1D}},
{0x3B,1,{0x04}},//CKV0
{0x3C,1,{0x1F}},//VGL
{0x3D,1,{0x1D}},
{0x3E,1,{0x1D}},
{0x3F,1,{0x1D}},
{0x40,1,{0x1D}},
{0x41,1,{0x1D}},                    
{0x42,1,{0x1D}},
{0x43,1,{0x11}},//ETV1
{0x44,1,{0x1D}},
{0x45,1,{0x1D}},
{0x46,1,{0x1F}},//VGL
{0x47,1,{0x1E}},//VGH
{0x48,1,{0x01}},//STV1
{0x49,1,{0x11}},//ETV1
{0x4A,1,{0x07}},//CKV3
{0x4B,1,{0x1D}},
{0x4C,1,{0x1D}},
{0x4D,1,{0x09}},//CKV5
{0x4E,1,{0x0B}},//CKV7
{0x4F,1,{0x1D}},
{0x50,1,{0x1D}},
{0x51,1,{0x05}},//CKV1
{0x52,1,{0x1F}},//VGL
{0x53,1,{0x1D}},
{0x54,1,{0x1D}},
{0x55,1,{0x1D}},
{0x56,1,{0x1D}},
{0x57,1,{0x1D}},
{0x58,1,{0x41}},
{0x59,1,{0x00}},
{0x5A,1,{0x00}},
{0x5B,1,{0x10}},
{0x5C,1,{0x02}},
{0x5D,1,{0x60}},
{0x5E,1,{0x01}},
{0x5F,1,{0x02}},
{0x60,1,{0x40}},
{0x61,1,{0x03}},
{0x62,1,{0x04}},
{0x63,1,{0x14}},//03 13
{0x64,1,{0x1A}},//10
{0x65,1,{0x55}},
{0x66,1,{0xB1}},
{0x67,1,{0x73}},
{0x68,1,{0x04}},
{0x69,1,{0x16}},//03   1//0D 45%   13
{0x6A,1,{0x58}},//56
{0x6B,1,{0x0A}},
{0x6C,1,{0x00}},
{0x6D,1,{0x00}},
{0x6E,1,{0x00}},
{0x6F,1,{0x88}},
{0x70,1,{0x00}},
{0x71,1,{0x00}},
{0x72,1,{0x06}},
{0x73,1,{0x7B}},
{0x74,1,{0x00}},
{0x75,1,{0xBC}},
{0x76,1,{0x00}},
{0x77,1,{0x05}},
{0x78,1,{0x34}},
{0x79,1,{0x00}},
{0x7A,1,{0x00}},
{0x7B,1,{0x00}},
{0x7C,1,{0x00}},
{0x7D,1,{0x03}},
{0x7E,1,{0x7B}},
{0x80,1,{0x06}},

//Page4
{0xE0,1,{0x04}},
{0x00,1,{0x02}},
{0x02,1,{0x23}},
{0x03,1,{0x8F}},
{0x09,1,{0x11}},
{0x0E,1,{0x2A}},
{0x9A,1,{0x01}},
{0x9B,1,{0x05}},
{0xA9,1,{0x01}},
{0xAA,1,{0x68}},
{0xAC,1,{0x11}},
{0xAD,1,{0x1D}},///
{0xAE,1,{0x0A}},
//Page0
{0xE0,1,{0x00}},    
{0xE6,1,{0x02}},
{0xE7,1,{0x06}},

//--- TE----//
{0x35,1,{0x00}},
//Page0
{0xE0,1,{0x00}},
{0x11, 0,   {0x00}},
{REGFLAG_DELAY, 120, {}},
{0x29, 0,   {0x00}},
{REGFLAG_DELAY, 20,  {}},	
{REGFLAG_END_OF_TABLE,0x00,{}}
};
#elif defined(LCD_KALAIDE_JD9366D_XM_IPS_57) //xjl 20190423
static unsigned char BufLcmInfo[] = "LCD_KALAIDE_JD9366D_XM_IPS_57_20190423";
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xE0, 1,{0x00}},

{0xE1, 1,{0x93}},

{0xE2, 1,{0x66}},

{0xE3, 1,{0xF9}},

{0x80, 1,{0x03}},//  0X02:3LANE    0X03:4LANE

{0xE0, 1,{0x01}},

{0x00, 1,{0x01}},

{0x01, 1,{0x29}},

{0x03, 1,{0x01}},

{0x04, 1,{0x29}},

{0x0A, 1,{0x08}},

{0x13, 1,{0x00}},

{0x14, 1,{0x99}},

{0x17, 1,{0x00}},

{0x18, 1,{0xA4}},

{0x19, 1,{0x00}},

{0x1A, 1,{0x00}},

{0x1B, 1,{0xA4}},

{0x1C, 1,{0x00}},

{0x1F, 1,{0x30}},

{0x20, 1,{0x2D}},

{0x21, 1,{0x19}},

{0x22, 1,{0x0D}},

{0x23, 1,{0x02}},

{0x24, 1,{0xFE}},

{0x26, 1,{0xDD}},

{0x35, 1,{0x13}},

{0x37, 1,{0x09}},

{0x38, 1,{0x04}},

{0x39, 1,{0x01}},

{0x3A, 1,{0x03}},

{0x3C, 1,{0x60}},

{0x3D, 1,{0x18}},

{0x3E, 1,{0x80}},

{0x3F, 1,{0x4E}},

{0x3D, 1,{0xFF}},

{0x3E, 1,{0xFF}},

{0x3F, 1,{0xFF}},

{0x4B, 1,{0x04}},

{0x40, 1,{0x04}},

{0x41, 1,{0xB4}},

{0x42, 1,{0x70}},

{0x43, 1,{0x24}},

{0x44, 1,{0x0C}},

{0x45, 1,{0x64}},

{0x55, 1,{0x01}}, //05

{0x56, 1,{0x01}},

{0x57, 1,{0x6D}},

{0x58, 1,{0x0A}},

{0x59, 1,{0x8A}},

{0x5A, 1,{0x28}},

{0x5B, 1,{0x23}},

{0x5C, 1,{0x15}},

{0x5D, 1,{0x75}},

{0x5E, 1,{0x61}},

{0x5F, 1,{0x53}},

{0x60, 1,{0x47}},

{0x61, 1,{0x43}},

{0x62, 1,{0x34}},

{0x63, 1,{0x39}},

{0x64, 1,{0x22}},

{0x65, 1,{0x3B}},

{0x66, 1,{0x3A}},

{0x67, 1,{0x3B}},

{0x68, 1,{0x5C}},

{0x69, 1,{0x4D}},

{0x6A, 1,{0x57}},

{0x6B, 1,{0x4B}},

{0x6C, 1,{0x4B}},

{0x6D, 1,{0x3E}},

{0x6E, 1,{0x2E}},

{0x6F, 1,{0x1A}},

{0x70, 1,{0x75}},

{0x71, 1,{0x61}},

{0x72, 1,{0x53}},

{0x73, 1,{0x47}},

{0x74, 1,{0x43}},

{0x75, 1,{0x34}},

{0x76, 1,{0x39}},

{0x77, 1,{0x22}},

{0x78, 1,{0x3B}},

{0x79, 1,{0x3A}},

{0x7A, 1,{0x3B}},

{0x7B, 1,{0x5C}},

{0x7C, 1,{0x4D}},

{0x7D, 1,{0x57}},

{0x7E, 1,{0x4B}},

{0x7F, 1,{0x4B}},

{0x80, 1,{0x3E}},

{0x81, 1,{0x2E}},

{0x82, 1,{0x1A}},

{0xE0, 1,{0x02}},

{0x00, 1,{0x5D}},

{0x01, 1,{0x51}},

{0x02, 1,{0x5D}},

{0x03, 1,{0x5D}},

{0x04, 1,{0x5E}},

{0x05, 1,{0x5F}},

{0x06, 1,{0x51}},

{0x07, 1,{0x41}},

{0x08, 1,{0x45}},

{0x09, 1,{0x5D}},

{0x0A, 1,{0x5D}},

{0x0B, 1,{0x4B}},

{0x0C, 1,{0x49}},

{0x0D, 1,{0x5D}},

{0x0E, 1,{0x5D}},

{0x0F, 1,{0x47}},

{0x10, 1,{0x5F}},

{0x11, 1,{0x5D}},

{0x12, 1,{0x5D}},

{0x13, 1,{0x5D}},

{0x14, 1,{0x5D}},

{0x15, 1,{0x5D}},

{0x16, 1,{0x5D}},

{0x17, 1,{0x50}},

{0x18, 1,{0x5D}},

{0x19, 1,{0x5D}},

{0x1A, 1,{0x5E}},

{0x1B, 1,{0x5F}},

{0x1C, 1,{0x50}},

{0x1D, 1,{0x40}},

{0x1E, 1,{0x44}},

{0x1F, 1,{0x5D}},

{0x20, 1,{0x5D}},

{0x21, 1,{0x4A}},

{0x22, 1,{0x48}},

{0x23, 1,{0x5D}},

{0x24, 1,{0x5D}},

{0x25, 1,{0x46}},

{0x26, 1,{0x5F}},

{0x27, 1,{0x5D}},

{0x28, 1,{0x5D}},

{0x29, 1,{0x5D}},

{0x2A, 1,{0x5D}},

{0x2B, 1,{0x5D}},

{0x2C, 1,{0x1D}},

{0x2D, 1,{0x10}},

{0x2E, 1,{0x1D}},

{0x2F, 1,{0x1D}},

{0x30, 1,{0x1F}},

{0x31, 1,{0x1E}},

{0x32, 1,{0x00}},

{0x33, 1,{0x10}},

{0x34, 1,{0x06}},

{0x35, 1,{0x1D}},

{0x36, 1,{0x1D}},

{0x37, 1,{0x08}},

{0x38, 1,{0x0A}},

{0x39, 1,{0x1D}},

{0x3A, 1,{0x1D}},

{0x3B, 1,{0x04}},

{0x3C, 1,{0x1F}},

{0x3D, 1,{0x1D}},

{0x3E, 1,{0x1D}},

{0x3F, 1,{0x1D}},

{0x40, 1,{0x1D}},

{0x41, 1,{0x1D}},

{0x42, 1,{0x1D}},

{0x43, 1,{0x11}},

{0x44, 1,{0x1D}},

{0x45, 1,{0x1D}},

{0x46, 1,{0x1F}},

{0x47, 1,{0x1E}},

{0x48, 1,{0x01}},

{0x49, 1,{0x11}},

{0x4A, 1,{0x07}},

{0x4B, 1,{0x1D}},

{0x4C, 1,{0x1D}},

{0x4D, 1,{0x09}},

{0x4E, 1,{0x0B}},

{0x4F, 1,{0x1D}},

{0x50, 1,{0x1D}},

{0x51, 1,{0x05}},

{0x52, 1,{0x1F}},

{0x53, 1,{0x1D}},

{0x54, 1,{0x1D}},

{0x55, 1,{0x1D}},

{0x56, 1,{0x1D}},

{0x57, 1,{0x1D}},

{0x58, 1,{0x41}},

{0x59, 1,{0x00}},

{0x5A, 1,{0x00}},

{0x5B, 1,{0x10}},

{0x5C, 1,{0x02}},

{0x5D, 1,{0x60}},

{0x5E, 1,{0x01}},

{0x5F, 1,{0x02}},

{0x60, 1,{0x40}},

{0x61, 1,{0x03}},

{0x62, 1,{0x04}},

{0x63, 1,{0x14}},

{0x64, 1,{0x1A}},

{0x65, 1,{0x55}},

{0x66, 1,{0xB1}},

{0x67, 1,{0x73}},

{0x68, 1,{0x04}},

{0x69, 1,{0x16}},

{0x6A, 1,{0x58}},

{0x6B, 1,{0x0A}},

{0x6C, 1,{0x00}},

{0x6D, 1,{0x00}},

{0x6E, 1,{0x00}},

{0x6F, 1,{0x88}},

{0x70, 1,{0x00}},

{0x71, 1,{0x00}},

{0x72, 1,{0x06}},

{0x73, 1,{0x7B}},

{0x74, 1,{0x00}},

{0x75, 1,{0xBC}},

{0x76, 1,{0x00}},

{0x77, 1,{0x05}},

{0x78, 1,{0x34}},

{0x79, 1,{0x00}},

{0x7A, 1,{0x00}},

{0x7B, 1,{0x00}},

{0x7C, 1,{0x00}},

{0x7D, 1,{0x03}},

{0x7E, 1,{0x7B}},

{0x80, 1,{0x06}},

{0xE0, 1,{0x04}},

{0x00, 1,{0x02}},

{0x02, 1,{0x23}},

{0x03, 1,{0x8F}},

{0x09, 1,{0x11}},

{0x0E, 1,{0x2A}},

{0x9A, 1,{0x01}},

{0x9B, 1,{0x05}},

{0xA9, 1,{0x01}},

{0xAA, 1,{0x68}},

{0xAC, 1,{0x19}},

{0xAD, 1,{0x10}},

{0xAE, 1,{0x10}},

{0xE0, 1,{0x00}},

{0xE6, 1,{0x02}},

{0xE7, 1,{0x06}},

{0xE0, 1,{0x00}},

{0x11,1,{0x00}}, ////Sleep Out
{REGFLAG_DELAY,150,{}},

{0x29,1,{0x00}}, ///Display On
{REGFLAG_DELAY,10,{}},	
{REGFLAG_END_OF_TABLE,0x00,{}}
};
#endif


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {

#if defined(LCD_KALAIDE_JD9366D_XM_IPS_57) //xjl 20190423
  	{REGFLAG_DELAY, 50, {}},   //50MS
  	{0xE0, 1, {0X00}},
  	{REGFLAG_DELAY, 50, {}},
  	{0xE7, 1, {0X16}},
  	{REGFLAG_DELAY, 1, {}},
  	{0x28, 1, {0X00}},
	{REGFLAG_DELAY, 50, {}},
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 250, {}},
#else
	// Display off sequence
	{0x22, 0, {0x00}},
	{REGFLAG_DELAY, 100, {}},

	{0x28, 0, {0x00}},
	{REGFLAG_DELAY, 10, {}},

    // Sleep Mode On
	{0x10, 0, {0x00}},
	{REGFLAG_DELAY, 100, {}},
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
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
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

		// enable tearing-free
		//params->dbi.te_mode 				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		//params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE; //BURST_VDO_MODE;
        #endif

        #ifndef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
        	params->physical_width = LCM_PHYSICAL_WIDTH/1000;
        	params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
        	params->physical_width_um = LCM_PHYSICAL_WIDTH;
        	params->physical_height_um = LCM_PHYSICAL_HEIGHT;	

        	params->dsi.switch_mode = CMD_MODE;
        	params->dsi.switch_mode_enable = 0;
        #endif

		params->dsi.g_StrLcmInfo = BufLcmInfo;

		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		params->dsi.word_count=720*3;	

#if defined(LCD_KALAIDE_JD9366D_XM_IPS_57) //xjl 20190423
		params->dsi.vertical_sync_active				= 8;	//2;
		params->dsi.vertical_backporch					= 18;	//14;
		params->dsi.vertical_frontporch					= 20;	//16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		
		params->dsi.horizontal_sync_active				= 8;	//40
		params->dsi.horizontal_backporch				= 30;	//60;
		params->dsi.horizontal_frontporch				= 30;	//60;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		params->dsi.PLL_CLOCK = 230;//208;
#else
		params->dsi.vertical_sync_active				= 8;	//2;
		params->dsi.vertical_backporch					= 18;	//14;
		params->dsi.vertical_frontporch					= 20;	//16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		
		params->dsi.horizontal_sync_active				= 40;	//2;
		params->dsi.horizontal_backporch				= 60;	//60;	//42;
		params->dsi.horizontal_frontporch				= 60;	//60;	//44;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		params->dsi.PLL_CLOCK = 230;//208;	
#endif

#if defined(CONFIG_TERACUBE_2E) //zxs 20160318 //zxs 20151107
        params->dsi.esd_check_enable =1;
        params->dsi.customization_esd_check_enable =1;
        params->dsi.lcm_esd_check_table[0].cmd   =0x0A;
        params->dsi.lcm_esd_check_table[0].count = 1;//1;
        params->dsi.lcm_esd_check_table[0].para_list[0] =0x9C;

        params->dsi.noncont_clock=0;
        params->dsi.clk_lp_per_line_enable=1;
#endif		

}

static unsigned int lcm_compare_id(void)
{
	    int array[4];
    char buffer[5];
    char id_high=0;
    char id_low=0;
    int id=0;
    
    SET_RESET_PIN(1);
    MDELAY(10);
	SET_RESET_PIN(0);
    MDELAY(20);
	SET_RESET_PIN(1);
    MDELAY(50);

    array[0] = 0x00023700;// read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);
    read_reg_v2(0x04,buffer, 2);

    id_high = buffer[0]; 
    id_low = buffer[1];
    id = (id_high<<8)|id_low; 

    #ifdef BUILD_LK
    printf("luke: JD9366 %s %d, id = 0x%08x\n", __func__,__LINE__, id);
    #else

    printk("luke: JD9366 %s %d, id = 0x%08x\n", __func__,__LINE__, id);
    #endif

    return (id == LCM_ID_JD9366) ?1:0;
     
}                                     

static void lcm_init(void)
{
	SET_RESET_PIN(1);	 
	MDELAY(10); //20
	SET_RESET_PIN(0);
	MDELAY(20);  // 50 
	SET_RESET_PIN(1);
	MDELAY(50);  // 150

 #ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
        push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
 #else
	push_table(NULL, lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
 #endif
}


static void lcm_suspend(void)
{
	//LCM_DBG("lcm_suspend");

#if !defined(LCD_KALAIDE_JD9366D_XM_IPS_57) //xjl 20190423
     unsigned int array[1];

     LCM_DBG("lcm_suspend");
     array[0] = 0x00E01500;
     dsi_set_cmdq(array, 1, 1);
     array[0] = 0x16e71500;
     dsi_set_cmdq(array, 1, 1);
#endif

#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#endif
	//SET_RESET_PIN(0);
    //MDELAY(100);
   	//SET_RESET_PIN(1); 
    
}


static void lcm_resume(void)
{
	LCM_DBG("lcm_resume");
	lcm_init();
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

#if (LCM_DSI_CMD_MODE)
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

	dsi_set_cmdq(data_array, 7, 0);

}
#endif

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
#ifdef BUILD_LK //xjl 20180531
LCM_DRIVER jd9366_6735_dsi_lcm_drv_yk6xx = 
#else
struct LCM_DRIVER jd9366_6735_dsi_lcm_drv_yk6xx = 
#endif
{
    .name		    = "jd9366_6735_dsi_video",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
#endif
};
