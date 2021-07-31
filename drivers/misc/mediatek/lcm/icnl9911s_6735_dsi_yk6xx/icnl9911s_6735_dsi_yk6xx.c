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

//#define LCM_ID_ICNL9911AC (0x00FA)//(0x9911)
#define LCM_ID_ICNL9911S (0x9911)

#define LCM_DBG(fmt, arg...)
	//LCM_PRINT ("[LCM-ILI9881-DSI-VDO] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE									0

#if defined(LCM_720_1560)
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1560)

#else
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1520)
#endif


#if defined(YK676_CUSTOMER_CAIFU_BSQ06_HDPLUS)
#define LCM_PHYSICAL_WIDTH									(59000)
#define LCM_PHYSICAL_HEIGHT									(126756)
#else
#define LCM_PHYSICAL_WIDTH									(0)
#define LCM_PHYSICAL_HEIGHT									(0)
#endif


#define REGFLAG_DELAY             							0XFE

#define REGFLAG_END_OF_TABLE      							0x100   // END OF REGISTERS MARKER


#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif


#if defined(YK676_CUSTOMER_CAIFU_BSQ06_HDPLUS)
    #define LCD_ZHONGGUANGDIAN_50_ICNL9911S_INSEL
#elif defined(YK685_CUSTOMER_CAIFU_SS198H_HDPLUS) //xen 20171024
    #define LCD_ZHONGGUANGDIAN_50_ICNL9911AC_INSEL
#else
    #define LCD_ZHONGGUANGDIAN_50_ICNL9911AC_INSEL
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

#ifdef CONFIG_MTK_LCM_5V_IC

extern int set_gpio_lcm_enp_enn(int enable);

#ifdef BUILD_LK
extern int TPS65132_write_byte(kal_uint8 addr, kal_uint8 value);
#else
extern int tps65132_write_bytes(unsigned char addr, unsigned char value);
#endif


#endif

struct LCM_setting_table {
    unsigned int cmd; //zxs 20151127
    unsigned char count;
    unsigned char para_list[64];//64];  //xen for less space 20170511
};

#if defined(LCD_ZHONGGUANGDIAN_50_ICNL9911S_INSEL)
#if defined (LCM_ROTATE_180)
	static unsigned char BufLcmInfo[] = "LCD_ICNL9911S_ZH097_CF1_50_20190926_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_ICNL9911S_ZH097_CF1_50_20190926";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xF0, 2,{0x5A,0x59}},
{0xF1, 2,{0xA5,0xA6}},
{0xB0,30,{0x82,0x81,0x05,0x04,0x87,0x86,0x84,0x85,0x66,0x66,0x33,0x33,0x20,0x01,0x01,0x78,0x01,0x01,0x0F,0x05,0x04,0x03,0x02,0x01,0x02,0x03,0x04,0x00,0x00,0x00}},
{0xB1,29,{0x11,0x51,0x86,0x06,0x01,0x00,0x01,0x7C,0x01,0x01,0x01,0x04,0x08,0x54,0x00,0x00,0x00,0x44,0x40,0x02,0x01,0x40,0x02,0x01,0x40,0x02,0x01,0x40,0x02}},
{0xB2,17,{0x54,0xD4,0x82,0x05,0x40,0x02,0x01,0x40,0x02,0x01,0x05,0x05,0x54,0x0C,0x0C,0x0D,0x0B}},
{0xB3,31,{0x02,0x00,0x00,0x00,0x00,0x26,0x26,0x91,0xA2,0x33,0x44,0x00,0x26,0x00,0x18,0x01,0x02,0x08,0x20,0x30,0x08,0x09,0x44,0x20,0x40,0x20,0x40,0x08,0x09,0x22,0x33}},
{0xB8,24,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xB9, 4,{0x99,0x11,0x01,0xFF}},
{0xBA, 2,{0x3F,0x3F}},
{0xBB,13,{0x01,0x05,0x09,0x11,0x0D,0x19,0x1D,0x15,0x25,0x69,0x00,0x21,0x25}},
{0xBC,14,{0x00,0x00,0x00,0x00,0x02,0x20,0xFF,0x00,0x03,0x33,0x01,0x73,0x44,0x00}},
{0xBD,10,{0xA3,0x22,0x4F,0xCF,0x72,0xA7,0x08,0x04,0xAE,0x95}},
{0xBE,10,{0x65,0x65,0x50,0x32,0x14,0x66,0x43,0x02,0x0E,0x0E}},
{0xBF, 8,{0x07,0x25,0x07,0x25,0x7F,0x00,0x11,0x04}},
{0xC0, 9,{0x10,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0xFF,0x00}},
{0xC1,19,{0xC0,0x0C,0x30,0x7C,0x04,0x0C,0x10,0x04,0x2A,0x18,0x36,0x00,0x07,0x00,0x10,0x00,0xC0,0x00,0xC0}},
{0xC2, 9,{0xFF,0x01,0x10,0x00,0x00,0x00,0x00,0x21,0x43}},
{0xC3,11,{0x06,0x00,0xFF,0x00,0xFF,0x00,0x00,0x81,0x01,0x00,0x00}},
{0xCB, 1,{0x00}},
{0xC5, 1,{0x03}},
{0xB4,28,{0x0A,0x02,0xDC,0x1D,0x00,0x02,0x02,0x02,0x02,0x12,0x10,0x02,0x02,0x0E,0x0C,0x04,0x03,0x03,0x03,0x03,0x03,0x03,0xFF,0xFF,0xF3,0x00,0x00,0x00}},
{0xB5,28,{0x0B,0x02,0xDC,0x1D,0x00,0x02,0x02,0x02,0x02,0x13,0x11,0x02,0x02,0x0F,0x0D,0x05,0x03,0x03,0x03,0x03,0x03,0x03,0xFF,0xFF,0xF3,0x00,0x00,0x00}},
//GAMMA2.0
//{0xC7,22,{0xF7,0xBA,0x92,0x77,0x4A,0x2A,0xFD,0x54,0x23,0xFE,0xD8,0xAF,0x0D,0xE3,0xC8,0x9F,0x85,0x5F,0x1A,0x7E,0xC4,0x00}},
//{0xC8,22,{0xF7,0xBA,0x92,0x77,0x4A,0x2A,0xFD,0x54,0x23,0xFE,0xD8,0xAF,0x0D,0xE3,0xC8,0x9F,0x85,0x5F,0x1A,0x7E,0xC4,0x00}},
//GAMMA2.5
//{0xC7,22,{0xF7,0xAF,0x83,0x65,0x36,0x16,0xE7,0x3E,0x0D,0xE7,0xBF,0x8F,0xE5,0xB5,0x98,0x6F,0x58,0x34,0x1A,0x7E,0xC0,0x00}},
//{0xC8,22,{0xF7,0xAF,0x83,0x65,0x36,0x16,0xE7,0x3E,0x0D,0xE7,0xBF,0x8F,0xE5,0xB5,0x98,0x6F,0x58,0x34,0x1A,0x7E,0xC0,0x00}},
//GAMMA2.2
{0xC7,22,{0xF7,0xBA,0x93,0x76,0x49,0x28,0xF9,0x50,0x1E,0xF6,0xCF,0xA3,0xFD,0xD2,0xB4,0x8B,0x71,0x4C,0x1A,0x7E,0xC0,0x00}},
{0xC8,22,{0xF7,0xBA,0x93,0x76,0x49,0x28,0xF9,0x50,0x1E,0xF6,0xCF,0xA3,0xFD,0xD2,0xB4,0x8B,0x71,0x4C,0x1A,0x7E,0xC0,0x00}},

{0xF1, 2,{0x5A,0x59}},
{0xF0, 2,{0xA5,0xA6}},
{0x35, 1,{0x00}},
{0x11, 0,{0x00}},
{REGFLAG_DELAY, 120,{}},
{0x29, 0,{0x00}},
{REGFLAG_DELAY, 10,{}},
{0x26, 1,{0x01}},	
{REGFLAG_DELAY, 20, {}}, 
{REGFLAG_END_OF_TABLE, 0x00, {}}
};	

#elif defined(LCD_ZHONGGUANGDIAN_50_ICNL9911AC_INSEL) //zxs 20190225
#if defined (LCM_ROTATE_180)
	static unsigned char BufLcmInfo[] = "LCD_ICNL9911AC_ZH097_CF1_50_20190226_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_ICNL9911AC_ZH097_CF1_50_20190226";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xF0,2,{ 0x5A,0x5A}},                                                                              
{0xF1,2,{ 0xA5,0xA5}},                                                                              
{0xC2,1,{ 0x00}},                                                                                   
{0xB0,16,{ 0x21,0x54,0x76,0x54,0x66,0x66,0x33,0x33,0x0c,0x03,0x03,0x8c,0x03,0x03,0x0F,0x00}},       
{0xB1,16,{ 0x11,0xD4,0x02,0x86,0x00,0x01,0x01,0x82,0x01,0x01,0x53,0x00,0x00,0x00,0x00,0x00}},       
{0xB2,16,{ 0x67,0x2A,0x05,0x8A,0x65,0x02,0x08,0x20,0x30,0x91,0x22,0x33,0x44,0x00,0x18,0xA1}},       
{0xB3,16,{ 0x01,0x00,0x00,0x33,0x00,0x26,0x26,0xC0,0x3F,0xAA,0x33,0xC3,0xAA,0x30,0xC3,0xAA}},       
{0xB6,16,{ 0x0a,0x02,0x14,0x15,0x1b,0x02,0x02,0x02,0x02,0x13,0x11,0x02,0x02,0x0F,0x0D,0x05}},       
{0xB4,16,{ 0x0b,0x02,0x14,0x15,0x1b,0x02,0x02,0x02,0x02,0x12,0x10,0x02,0x02,0x0E,0x0C,0x04}},       
{0xB9,2, { 0x1e,0x1E}},                                                                             
{0xBB,16,{ 0x00,0x00,0x00,0x00,0x02,0xFF,0xFC,0x0B,0x13,0x01,0x73,0x44,0x44,0x00,0x00,0x00}},       
{0xBC,10,{ 0x61,0x03,0xff,0xDE,0x72,0xE0,0x2E,0x04,0x88,0x3e}},                                     
{0xBD,16,{ 0x6E,0x0E,0x65,0x65,0x15,0x15,0x50,0x32,0x14,0x66,0x23,0x02,0x00,0x00,0x00,0x00}},       
{0xBE,5, { 0x60,0x60,0x50,0x60,0x77}},                                                              
{0xC1,16,{ 0x70,0x7c,0x0c,0x7c,0x04,0x0C,0x10,0x04,0x2A,0x31,0x00,0x07,0x10,0x10,0x00,0x00}},       
{0xC3,8, { 0x00,0x00,0xFF,0x00,0xFF,0x00,0x00,0x0d}},                                               
{0xC4,8, { 0xB4,0xA3,0xee,0x41,0x04,0x2F,0x00,0x00}},                                               
{0xC5,12,{ 0x07,0x1F,0x42,0x26,0x52,0x44,0x14,0x1A,0x04,0x00,0x0A,0x08}},                           
{0xC6,16,{ 0x81,0x01,0x67,0x01,0x33,0xA0,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},       
//Gamma                                                                                           
{0xC7,16,{0x7C,0x5F,0x4D,0x63,0x50,0x3B,0x2C,0x33,0x20,0x3D,0x3F,0x31,0x4D,0x47,0x55,0x47}},        
{0xC8,5, {0x34,0x5F,0x37,0x26,0x0C}},                                                               
{0xC9,16,{0x7C,0x5F,0x4D,0x63,0x50,0x3B,0x2C,0x33,0x20,0x3D,0x3F,0x31,0x4D,0x47,0x55,0x47}},        
{0xCA,5, {0x34,0x5F,0x37,0x26,0x0C}},                                                               
{0xCB,11,{0x00,0x00,0x00,0x01,0x6C,0x00,0x33,0x00,0x17,0xFF,0xEF}},                                 
{0xF0,2,{0xB4,0x4B}},                                                                               
{0xD0,8,{0x80,0x0D,0xFF,0x0F,0x63,0x2B,0x08,0x08}},                                                 
{0xD2,10,{0x43,0x0C,0x00,0x01,0x80,0x26,0x04,0x00,0x16,0x42}},                                      
                                                                                                    
{0xd5,1,{0x0f}},                                                                                    
{0x35,1,{0x00}},                                                                                    
{0xF0,2,{0xA5,0xA5}},                                                                               
{0xF1,2,{0x5A,0x5A}},                                                                               
	
	


{0x11,0x01,{0x00}},
{REGFLAG_DELAY, 120, {}}, 

{0x29,0x01,{0x00}}, 
{REGFLAG_DELAY, 100, {}}, 
{0x26,1,{0x01}},    
{REGFLAG_END_OF_TABLE, 0x00, {}}
};

#else //yk685
#if defined (LCM_ROTATE_180)
	static unsigned char BufLcmInfo[] = "LCD_ICNL9911AC_ZH097_CF1_50_20190225_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_ICNL9911AC_ZH097_CF1_50_20190225";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xF0,2,{ 0x5A,0x5A}},                                                                              
{0xF1,2,{ 0xA5,0xA5}},                                                                              
{0xC2,1,{ 0x00}},                                                                                   
{0xB0,16,{ 0x21,0x54,0x76,0x54,0x66,0x66,0x33,0x33,0x0c,0x03,0x03,0x8c,0x03,0x03,0x0F,0x00}},       
{0xB1,16,{ 0x11,0xD4,0x02,0x86,0x00,0x01,0x01,0x82,0x01,0x01,0x53,0x00,0x00,0x00,0x00,0x00}},       
{0xB2,16,{ 0x67,0x2A,0x05,0x8A,0x65,0x02,0x08,0x20,0x30,0x91,0x22,0x33,0x44,0x00,0x18,0xA1}},       
{0xB3,16,{ 0x01,0x00,0x00,0x33,0x00,0x26,0x26,0xC0,0x3F,0xAA,0x33,0xC3,0xAA,0x30,0xC3,0xAA}},       
{0xB6,16,{ 0x0a,0x02,0x14,0x15,0x1b,0x02,0x02,0x02,0x02,0x13,0x11,0x02,0x02,0x0F,0x0D,0x05}},       
{0xB4,16,{ 0x0b,0x02,0x14,0x15,0x1b,0x02,0x02,0x02,0x02,0x12,0x10,0x02,0x02,0x0E,0x0C,0x04}},       
{0xB9,2, { 0x1e,0x1E}},                                                                             
{0xBB,16,{ 0x00,0x00,0x00,0x00,0x02,0xFF,0xFC,0x0B,0x13,0x01,0x73,0x44,0x44,0x00,0x00,0x00}},       
{0xBC,10,{ 0x61,0x03,0xff,0xDE,0x72,0xE0,0x2E,0x04,0x88,0x3e}},                                     
{0xBD,16,{ 0x6E,0x0E,0x65,0x65,0x15,0x15,0x50,0x32,0x14,0x66,0x23,0x02,0x00,0x00,0x00,0x00}},       
{0xBE,5, { 0x60,0x60,0x50,0x60,0x77}},                                                              
{0xC1,16,{ 0x70,0x7c,0x0c,0x7c,0x04,0x0C,0x10,0x04,0x2A,0x31,0x00,0x07,0x10,0x10,0x00,0x00}},       
{0xC3,8, { 0x00,0x00,0xFF,0x00,0xFF,0x00,0x00,0x0d}},                                               
{0xC4,8, { 0xB4,0xA3,0xee,0x41,0x04,0x2F,0x00,0x00}},                                               
{0xC5,12,{ 0x07,0x1F,0x42,0x26,0x52,0x44,0x14,0x1A,0x04,0x00,0x0A,0x08}},                           
{0xC6,16,{ 0x81,0x01,0x67,0x01,0x33,0xA0,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},       
//Gamma                                                                                           
{0xC7,16,{0x7C,0x5F,0x4D,0x63,0x50,0x3B,0x2C,0x33,0x20,0x3D,0x3F,0x31,0x4D,0x47,0x55,0x47}},        
{0xC8,5, {0x34,0x5F,0x37,0x26,0x0C}},                                                               
{0xC9,16,{0x7C,0x5F,0x4D,0x63,0x50,0x3B,0x2C,0x33,0x20,0x3D,0x3F,0x31,0x4D,0x47,0x55,0x47}},        
{0xCA,5, {0x34,0x5F,0x37,0x26,0x0C}},                                                               
{0xCB,11,{0x00,0x00,0x00,0x01,0x6C,0x00,0x33,0x00,0x17,0xFF,0xEF}},                                 
{0xF0,2,{0xB4,0x4B}},                                                                               
{0xD0,8,{0x80,0x0D,0xFF,0x0F,0x63,0x2B,0x08,0x08}},                                                 
{0xD2,10,{0x43,0x0C,0x00,0x01,0x80,0x26,0x04,0x00,0x16,0x42}},                                      
                                                                                                    
{0xd5,1,{0x0f}},                                                                                    
{0x35,1,{0x00}},                                                                                    
{0xF0,2,{0xA5,0xA5}},                                                                               
{0xF1,2,{0x5A,0x5A}},                                                                               
	
	


{0x11,0x01,{0x00}},
{REGFLAG_DELAY, 120, {}}, 

{0x29,0x01,{0x00}}, 
{REGFLAG_DELAY, 100, {}}, 
{0x26,1,{0x01}},    
{REGFLAG_END_OF_TABLE, 0x00, {}}
};	
#endif



static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {

	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 30, {}},
	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 150, {}},
	
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


		params->dsi.vertical_sync_active				= 4;	//2;
		params->dsi.vertical_backporch					= 12;	//14;
		params->dsi.vertical_frontporch					= 124;	//16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		
		params->dsi.horizontal_sync_active				= 4;	//2;
		params->dsi.horizontal_backporch				= 28;	//60;	//42;
		params->dsi.horizontal_frontporch				= 28;	//60;	//44;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		params->dsi.PLL_CLOCK = 250;//208;	
		
		
	/*	params->dsi.vertical_sync_active				= 4;	//2;
		params->dsi.vertical_backporch					= 12;	//14;
		params->dsi.vertical_frontporch					= 124;	//16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		
		params->dsi.horizontal_sync_active				= 4;	//2;
		params->dsi.horizontal_backporch				= 74;	//60;	//42;
		params->dsi.horizontal_frontporch				= 74;	//60;	//44;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		params->dsi.PLL_CLOCK = 280;//208;	*/


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
	array[1]=0x068198FF;//1>6
	dsi_set_cmdq(array, 2, 1);
	MDELAY(10); 

	//array[0] = 0x00023700;// set return byte number
	array[0] = 0x00023700;// set return byte number
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xA1, buffer, 2);
	//read_reg_v2(0xA1, buffer+1, 1);

	//zrl 0xA1==9911S 0XFA=9911AC   
	id = buffer[0]<<8 |buffer[1];


	#if defined(BUILD_LK)
		printf("ICNL9911AC 0x%x , 0x%x , 0x%x \n",buffer[0],buffer[1],id);
	#else
		printk("ICNL9911AC 0x%x , 0x%x , 0x%x \n",buffer[0],buffer[1],id);
	#endif
//printk("ICNL9911AC 0x%x , 0x%x , 0x%x \n",buffer[0],buffer[1],id);
	return (id == LCM_ID_ICNL9911S)?1:0;
}

static void lcm_init(void)
{
#ifdef CONFIG_MTK_LCM_5V_IC
	unsigned char cmd = 0x0;
	unsigned char data = 0xFF;
#ifndef CONFIG_FPGA_EARLY_PORTING
	int ret = 0;
#endif
	cmd = 0x00;
	data = 0x0F;
	SET_RESET_PIN(0);
#ifndef BUILD_LK 
	//set_gpio_lcd_enp(1);
	set_gpio_lcm_enp_enn(1);
#endif
#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
#else
	ret = tps65132_write_bytes(cmd, data);
#endif
	cmd = 0x01;
	data = 0x0F;
#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
#else
	ret = tps65132_write_bytes(cmd, data);
#endif
#endif
#if defined(YK736_CUSTOMER_CAIFU9_KS939A_HDPLUS) //zxs 20180717
        SET_RESET_PIN(1);	 
	MDELAY(10); //20
	SET_RESET_PIN(0);
	MDELAY(10);  //zwl//20// 50 
	SET_RESET_PIN(1);
	MDELAY(50);  // 150
#else
	SET_RESET_PIN(1);	 
	MDELAY(10); //20
	SET_RESET_PIN(0);
	MDELAY(20);  // 50 
	SET_RESET_PIN(1);
	MDELAY(50);  // 150
#endif

#if defined(DOUBLE_LCM_BY_IDPIN) //xjl 20161105
    if (get_lcd_id_state()==1)
    {
 #ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
        push_table(lcm_initialization_setting_1, sizeof(lcm_initialization_setting_1) / sizeof(struct LCM_setting_table), 1);
 #else
	push_table(NULL, lcm_initialization_setting_1, sizeof(lcm_initialization_setting_1) / sizeof(struct LCM_setting_table), 1);
 #endif
    }
    else
    {
 #ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
        push_table(lcm_initialization_setting_0, sizeof(lcm_initialization_setting_0) / sizeof(struct LCM_setting_table), 1);
 #else
	push_table(NULL, lcm_initialization_setting_0, sizeof(lcm_initialization_setting_0) / sizeof(struct LCM_setting_table), 1);
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
	LCM_DBG("lcm_suspend");
#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#endif
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
LCM_DRIVER icnl9911s_6735_dsi_lcm_drv_yk6xx = 
#else
struct LCM_DRIVER icnl9911s_6735_dsi_lcm_drv_yk6xx = 
#endif
{
    .name		    = "icnl9911s_67xx_dsi_video", 
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
