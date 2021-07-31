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
    #include "cust_gpio_usage.h"
#else
    //#include <linux/gpio.h>
    #include <mt-plat/mtk_gpio.h>
#endif

#define LCM_ID_ILI9881C (0x9881)

//#define LCM_DBG(fmt, arg...) 
	//LCM_PRINT ("[LCM-ILI9881-DSI-VDO] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE									0

#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)

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

#if defined(YK739_CUSTOMER_YKQ_HD720)//
     #define LCD_HONGZHAN_ILI9881C_HD_50_LG
#elif defined(YK568_CUSTOMER_YKQ_HD720)
     #define LCD_HONGLI_ILI9881C_HD_55_CPT
#else
     #define LCD_HONGZHAN_ILI9881C_HD_50_LG
#endif 

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

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
    unsigned char para_list[5];//64]; //xen for less space 20170511
};

#if defined(LCD_HONGZHAN_ILI9881C_HD_50_LG) // zwl
#if defined(CONFIG_MTK_LCM_PHYSICAL_ROTATION_HW)||defined(MTK_LCM_PHYSICAL_ROTATION_HW)
	static unsigned char BufLcmInfo[] = "LCD_HONGZHAN_ILI9881C_HD_50_LG_20161104_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_HONGZHAN_ILI9881C_HD_50_LG_20161104";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xFF,03,{0x98,0x81,0x03}},
//GIP_1
	{0x01,01,{0x00}},
	{0x02,01,{0x00}},
	{0x03,01,{0x73}},
	{0x04,01,{0x03}},
	{0x05,01,{0x00}},
	{0x06,01,{0x06}},
	{0x07,01,{0x06}},
	{0x08,01,{0x00}},
	{0x09,01,{0x18}},
	{0x0a,01,{0x04}},
	{0x0b,01,{0x00}},
	{0x0c,01,{0x02}},
	{0x0d,01,{0x03}},
	{0x0e,01,{0x00}},
	{0x0f,01,{0x25}},
	{0x10,01,{0x25}}, 
	{0x11,01,{0x00}},
	{0x12,01,{0x00}},
	{0x13,01,{0x00}},
	{0x14,01,{0x00}},
	{0x15,01,{0x00}},
	{0x16,01,{0x0C}},
	{0x17,01,{0x00}}, 
	{0x18,01,{0x00}},
	{0x19,01,{0x00}},
	{0x1a,01,{0x00}},
	{0x1b,01,{0x00}},
	{0x1c,01,{0x00}},
	{0x1d,01,{0x00}},
	{0x1e,01,{0xC0}},
	{0x1f,01,{0x80}},  
	{0x20,01,{0x04}},
	{0x21,01,{0x01}},
	{0x22,01,{0x00}},   
	{0x23,01,{0x00}},
	{0x24,01,{0x00}},  
	{0x25,01,{0x00}}, 
	{0x26,01,{0x00}},
	{0x27,01,{0x00}},
	{0x28,01,{0x33}},  
	{0x29,01,{0x03}},
	{0x2a,01,{0x00}},
	{0x2b,01,{0x00}},
	{0x2c,01,{0x00}},
	{0x2d,01,{0x00}},
	{0x2e,01,{0x00}},
	{0x2f,01,{0x00}},
	{0x30,01,{0x00}},
	{0x31,01,{0x00}},
	{0x32,01,{0x00}},
	{0x33,01,{0x00}},
	{0x34,01,{0x04}},
	{0x35,01,{0x00}},
	{0x36,01,{0x00}},
	{0x37,01,{0x00}},
	{0x38,01,{0x3C}},
	{0x39,01,{0x00}},
	{0x3a,01,{0x00}}, 
	{0x3b,01,{0x00}},
	{0x3c,01,{0x00}},
	{0x3d,01,{0x00}},
	{0x3e,01,{0x00}},
	{0x3f,01,{0x00}},
	{0x40,01,{0x00}},
	{0x41,01,{0x00}},
	{0x42,01,{0x00}},
	{0x43,01,{0x00}},
	{0x44,01,{0x00}},
                       
//GIP_2       
	{0x50,01,{0x01}},
	{0x51,01,{0x23}},
	{0x52,01,{0x45}},
	{0x53,01,{0x67}},
	{0x54,01,{0x89}},
	{0x55,01,{0xab}},
	{0x56,01,{0x01}},
	{0x57,01,{0x23}},
	{0x58,01,{0x45}},
	{0x59,01,{0x67}},
	{0x5a,01,{0x89}},
	{0x5b,01,{0xab}},
	{0x5c,01,{0xcd}},
	{0x5d,01,{0xef}},
              
//GIP_3      
	{0x5e,01,{0x11}},
	{0x5f,01,{0x02}},
	{0x60,01,{0x02}},
	{0x61,01,{0x02}},
	{0x62,01,{0x02}},
	{0x63,01,{0x02}},
	{0x64,01,{0x02}},
	{0x65,01,{0x02}},
	{0x66,01,{0x02}},
	{0x67,01,{0x02}},
	{0x68,01,{0x02}},
	{0x69,01,{0x02}},
	{0x6a,01,{0x0C}},
	{0x6b,01,{0x02}},
	{0x6c,01,{0x0F}},
	{0x6d,01,{0x0E}},
	{0x6e,01,{0x0D}},
	{0x6f,01,{0x06}},
	{0x70,01,{0x07}},
	{0x71,01,{0x02}},
	{0x72,01,{0x02}},
	{0x73,01,{0x02}},
	{0x74,01,{0x02}},
	{0x75,01,{0x02}},
	{0x76,01,{0x02}},
	{0x77,01,{0x02}},
	{0x78,01,{0x02}},
	{0x79,01,{0x02}},
	{0x7a,01,{0x02}},
	{0x7b,01,{0x02}},
	{0x7c,01,{0x02}},
	{0x7d,01,{0x02}},
	{0x7e,01,{0x02}},
	{0x7f,01,{0x02}},
	{0x80,01,{0x0C}},
	{0x81,01,{0x02}},
	{0x82,01,{0x0F}},
	{0x83,01,{0x0E}},
	{0x84,01,{0x0D}},
	{0x85,01,{0x06}},
	{0x86,01,{0x07}},
	{0x87,01,{0x02}},
	{0x88,01,{0x02}},
	{0x89,01,{0x02}},
	{0x8A,01,{0x02}},

//CMD_Page 4
	{0xFF,03,{0x98,0x81,0x04}},

	{0x6C,01,{0x15}},
	{0x6E,01,{0x22}},               //di_pwr_reg=0 VGH clamp 15V
	{0x6F,01,{0x33}},              // reg vcl + VGH pumping ratio 3x VGL=-2x
	{0x3A,01,{0xA4}},               
	{0x8D,01,{0x0D}},               //VGL clamp -10V
	{0x87,01,{0xBA}},               
	{0x26,01,{0x76}},
	{0xB2,01,{0xD1}},

//CMD_Page 1
	{0xFF,03,{0x98,0x81,0x01}},
	{0x22,01,{0x0A}},		//BGR, SS
	{0x53,01,{0xC9}},		//VCOM1
	{0x55,01,{0xA7}},		//VCOM2
	{0x50,01,{0x74}},         	//VREG1OUT=4.9V
	{0x51,01,{0x74}},         	//VREG2OUT=-4.9V
	{0x31,01,{0x02}},		//column inversion
	{0x60,01,{0x14}},               //SDT
              
	{0xA0,01,{0x15}},  
	{0xA1,01,{0x23}},                    
	{0xA2,01,{0x30}},                  
	{0xA3,01,{0x12}},                    
	{0xA4,01,{0x16}},                         
	{0xA5,01,{0x29}},                         
	{0xA6,01,{0x1D}},                       
	{0xA7,01,{0x1E}},                     
	{0xA8,01,{0x8A}},                       
	{0xA9,01,{0x1C}},                        
	{0xAA,01,{0x28}},                        
	{0xAB,01,{0x77}},                      
	{0xAC,01,{0x1D}},                          
	{0xAD,01,{0x1C}},                         
	{0xAE,01,{0x4F}},                          
	{0xAF,01,{0x25}},                           
	{0xB0,01,{0x2A}},                         
	{0xB1,01,{0x4C}},                          
	{0xB2,01,{0x5A}},                            
	{0xB3,01,{0x3F}},                        
                            
//VN255 GAMMA    N                                               
	{0xC0,01,{0x05}},		
	{0xC1,01,{0x23}},                      
	{0xC2,01,{0x30}},                  
	{0xC3,01,{0x12}},                         
	{0xC4,01,{0x16}},                        
	{0xC5,01,{0x29}},                       
	{0xC6,01,{0x1D}},                        
	{0xC7,01,{0x1E}},                      
	{0xC8,01,{0x8A}},                         
	{0xC9,01,{0x1C}},                       
	{0xCA,01,{0x28}},                        
	{0xCB,01,{0x78}},                        
	{0xCC,01,{0x1D}},                         
	{0xCD,01,{0x1C}},                         
	{0xCE,01,{0x4F}},                      
	{0xCF,01,{0x25}},                          
	{0xD0,01,{0x2A}},                         
	{0xD1,01,{0x4C}},                          
	{0xD2,01,{0x5A}},                          
	{0xD3,01,{0x3F}},              
                                        
	{0xFF,03,{0x98,0x81,0x00}},
	{0x11,01,{0x00}},
	{REGFLAG_DELAY, 120, {}},  
	{0x29,01,{0x00}},	
	{REGFLAG_DELAY, 10, {}},  
	{REGFLAG_END_OF_TABLE, 0x00, {}} 
};
#elif defined(LCD_HONGLI_ILI9881C_HD_55_CPT) //xjl 20161117
static unsigned char BufLcmInfo[] = "LCD_HONGLI_ILI9881C_HD_55_CPT_20161117";
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xFF,3,{0x98,0x81,0x03}},
	{0x01,1,{0x00}},
	{0x02,1,{0x00}},
	{0x03,1,{0x56}},
	{0x04,1,{0x13}},
	{0x05,1,{0x00}},
	{0x06,1,{0x06}},  
	{0x07,1,{0x01}},
	{0x08,1,{0x00}},
	{0x09,1,{0x30}},
	{0x0A,1,{0x01}},
	{0x0B,1,{0x00}},
	{0x0C,1,{0x30}},
	{0x0D,1,{0x01}},
	{0x0E,1,{0x00}},
	{0x0F,1,{0x18}},
	{0x10,1,{0x18}},
	{0x11,1,{0x00}},
	{0x12,1,{0x00}},
	{0x13,1,{0x00}},  
	{0x14,1,{0x00}},
	{0x15,1,{0x00}},
	{0x16,1,{0x00}}, 
	{0x17,1,{0x00}}, 
	{0x18,1,{0x00}},
	{0x19,1,{0x00}},
	{0x1A,1,{0x00}},
	{0x1B,1,{0x00}},
	{0x1C,1,{0x00}},
	{0x1D,1,{0x00}},
	{0x1E,1,{0x40}},
	{0x1F,1,{0xC0}},
	{0x20,1,{0x02}},
	{0x21,1,{0x05}},
	{0x22,1,{0x02}},
	{0x23,1,{0x00}},
	{0x24,1,{0x86}},
	{0x25,1,{0x85}},
	{0x26,1,{0x00}},
	{0x27,1,{0x00}},
	{0x28,1,{0x3B}},
	{0x29,1,{0x03}},
	{0x2A,1,{0x00}},
	{0x2B,1,{0x00}},
	{0x2C,1,{0x00}},
	{0x2D,1,{0x00}},
	{0x2E,1,{0x00}},
	{0x2F,1,{0x00}},
	{0x30,1,{0x00}},
	{0x31,1,{0x00}},
	{0x32,1,{0x00}},
	{0x33,1,{0x00}},
	{0x34,1,{0x00}},
	{0x35,1,{0x00}},
	{0x36,1,{0x00}},
	{0x37,1,{0x00}},
	{0x38,1,{0x00}},
	{0x39,1,{0x00}},
	{0x3A,1,{0x00}},
	{0x3B,1,{0x00}},
	{0x3C,1,{0x00}},
	{0x3D,1,{0x00}},
	{0x3E,1,{0x00}},
	{0x3F,1,{0x00}},
	{0x40,1,{0x00}},
	{0x41,1,{0x00}},
	{0x42,1,{0x00}},
	{0x43,1,{0x00}},
	{0x44,1,{0x00}},
//GIP_2
	{0x50,1,{0x01}},
	{0x51,1,{0x23}},
	{0x52,1,{0x45}},
	{0x53,1,{0x67}},
	{0x54,1,{0x89}},
	{0x55,1,{0xAB}},
	{0x56,1,{0x01}},
	{0x57,1,{0x23}},
	{0x58,1,{0x45}},
	{0x59,1,{0x67}},
	{0x5A,1,{0x89}},
	{0x5B,1,{0xAB}},
	{0x5C,1,{0xCD}},
	{0x5D,1,{0xEF}},
//GIP_3
	{0x5E,1,{0x11}},
	{0x5F,1,{0x08}},
	{0x60,1,{0x00}},
	{0x61,1,{0x01}},
	{0x62,1,{0x02}},
	{0x63,1,{0x02}},
	{0x64,1,{0x0F}},
	{0x65,1,{0x0E}},
	{0x66,1,{0x0D}},
	{0x67,1,{0x0C}},
	{0x68,1,{0x02}},
	{0x69,1,{0x02}},
	{0x6A,1,{0x02}},
	{0x6B,1,{0x02}},
	{0x6C,1,{0x02}},
	{0x6D,1,{0x02}},
	{0x6E,1,{0x06}},
	{0x6F,1,{0x02}},
	{0x70,1,{0x02}},
	{0x71,1,{0x02}},
	{0x72,1,{0x02}},
	{0x73,1,{0x02}},
	{0x74,1,{0x02}},
	{0x75,1,{0x06}},
	{0x76,1,{0x00}},
	{0x77,1,{0x01}},
	{0x78,1,{0x02}},
	{0x79,1,{0x02}},
	{0x7A,1,{0x0F}},
	{0x7B,1,{0x0E}},
	{0x7C,1,{0x0D}},
	{0x7D,1,{0x0C}},
	{0x7E,1,{0x02}},
	{0x7F,1,{0x02}},
	{0x80,1,{0x02}},
	{0x81,1,{0x02}},
	{0x82,1,{0x02}},
	{0x83,1,{0x02}},
	{0x84,1,{0x08}},
	{0x85,1,{0x02}},
	{0x86,1,{0x02}},
	{0x87,1,{0x02}},
	{0x88,1,{0x02}},
	{0x89,1,{0x02}},
	{0x8A,1,{0x02}},
	{0xFF,3,{0x98,0x81,0x04}},
	{0x6C,1,{0x15}},
	{0x6E,1,{0x2B}},
	{0x6F,1,{0x30}},
	{0x8D,1,{0x15}},
	{0x87,1,{0xBA}},
	{0x26,1,{0x76}},
	{0xB2,1,{0xD1}},
	{0xB5,1,{0x07}},
	{0x35,1,{0x1F}},
	{0x3A,1,{0x24}},
	{0xFF,3,{0x98,0x81,0x01}},
	{0x22,1,{0x38}},
	{0x31,1,{0x00}},		//00 column inversion; 02 dot inversion
	{0x53,1,{0x64}},
	{0x55,1,{0x5f}},
	{0x50,1,{0xB7}},
	{0x51,1,{0xB7}},
	{0x60,1,{0x30}},
	{0x61,1,{0x00}},
	{0x62,1,{0x19}},
	{0x63,1,{0x10}}, 
	{0xA0,1,{0x08}},
	{0xA1,1,{0x1B}},
	{0xA2,1,{0x28}},
	{0xA3,1,{0x0C}},
	{0xA4,1,{0x12}},
	{0xA5,1,{0x24}},
	{0xA6,1,{0x19}},
	{0xA7,1,{0x1d}},
	{0xA8,1,{0x90}},
	{0xA9,1,{0x1C}},
	{0xAA,1,{0x29}},               //VP111        
	{0xAB,1,{0x87}},
	{0xAC,1,{0x1b}},
	{0xAD,1,{0x1a}},
	{0xAE,1,{0x4d}},
	{0xAF,1,{0x22}},
	{0xB0,1,{0x29}},               //VP12         
	{0xB1,1,{0x4B}},
	{0xB2,1,{0x52}},
	{0xB3,1,{0x39}},
	{0xC0,1,{0x08}},
	{0xC1,1,{0x1B}},
	{0xC2,1,{0x28}},
	{0xC3,1,{0x0C}},
	{0xC4,1,{0x12}},
	{0xC5,1,{0x24}},
	{0xC6,1,{0x19}},
	{0xC7,1,{0x1d}},
	{0xC8,1,{0x90}},
	{0xC9,1,{0x1C}},
	{0xCA,1,{0x29}},               //VN111        
	{0xCB,1,{0x87}},
	{0xCC,1,{0x1b}},
	{0xCD,1,{0x1a}},
	{0xCE,1,{0x4d}},
	{0xCF,1,{0x22}},
	{0xD0,1,{0x29}},               //VN12         
	{0xD1,1,{0x4B}},
	{0xD2,1,{0x52}},
	{0xD3,1,{0x39}},
	{0xFF,3,{0x98,0x81,0x00}},
	{0x36,1,{0x02}},
// Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 200, {}},
// Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#endif


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},   //30
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
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}


static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{

		memset(params, 0, sizeof(LCM_PARAMS));
	
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

		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
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
	
#if defined(LCD_HONGZHAN_ILI9881C_HD_50_LG) //cjc 20161019
		params->dsi.vertical_sync_active				= 10;//4;//8;	//2;
		params->dsi.vertical_backporch					= 20;//15; //20; //18;	//14;
		params->dsi.vertical_frontporch					= 20;//16;//20;	//16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		
		params->dsi.horizontal_sync_active				= 40;//10; //40;	//2;
		params->dsi.horizontal_backporch				= 100;//150;//80;//120;	//60;	//42;
		params->dsi.horizontal_frontporch				= 80;//150; //80; //100;	//60;	//44;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		
		params->dsi.PLL_CLOCK = 260; //220;//208;	
#elif defined(LCD_HONGLI_ILI9881C_HD_55_CPT)
		params->dsi.vertical_sync_active				= 4; 
		params->dsi.vertical_backporch					= 13; 
		params->dsi.vertical_frontporch					= 16; 
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 20;
		params->dsi.horizontal_backporch				= 50;
		params->dsi.horizontal_frontporch				= 80; 
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		params->dsi.PLL_CLOCK = 250;
#else		
		params->dsi.vertical_sync_active				= 8;	//2;
		params->dsi.vertical_backporch					= 18;	//14;
		params->dsi.vertical_frontporch					= 20;	//16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		
		params->dsi.horizontal_sync_active				= 40;	//2;
		params->dsi.horizontal_backporch				= 120;	//60;	//42;
		params->dsi.horizontal_frontporch				= 100;	//60;	//44;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		params->dsi.PLL_CLOCK = 220;//208;	

#endif

#if defined(CONFIG_TERACUBE_2E) //zxs 20160318 //zxs 20151107
        params->dsi.esd_check_enable =1;
        params->dsi.customization_esd_check_enable =1;
        params->dsi.lcm_esd_check_table[0].cmd   =0x0A;
        params->dsi.lcm_esd_check_table[0].count =1;
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
	array[1]=0x018198FF;
	dsi_set_cmdq(array, 2, 1);
	MDELAY(10); 

	//array[0] = 0x00023700;// set return byte number
	array[0] = 0x00023700;// set return byte number
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x00, buffer, 1);
	read_reg_v2(0x01, buffer+1, 1);

	//zrl modify for ili9806 read ID,121113       
	id = buffer[0]<<8 |buffer[1];


#if defined(BUILD_LK)
	//printf("ili9881c 0x%x , 0x%x , 0x%x \n",buffer[0],buffer[1],id);
#else
	//printk("ili9881c 0x%x , 0x%x , 0x%x \n",buffer[0],buffer[1],id);
#endif
   //      return 1;
       return (id == LCM_ID_ILI9881C)?1:0;
}                                     

static void lcm_init(void)
{
	SET_RESET_PIN(1);	 
	MDELAY(20); 
	SET_RESET_PIN(0);
	MDELAY(50); 
	SET_RESET_PIN(1);
	MDELAY(150); 

#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}


static void lcm_suspend(void)
{
	//LCM_DBG("lcm_suspend");
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
	//LCM_DBG("lcm_resume");

	lcm_init();

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
LCM_DRIVER ili9881c_6735_dsi_lcm_drv_yk6xx = 
{
    .name		    = "ili9881c_67xx_dsi_video",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	//.esd_check    = lcm_esd_check,
	//.esd_recover  = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
	.update         = lcm_update,
	//.set_backlight= lcm_setbacklight,
	//.set_pwm      = lcm_setpwm,
	//.get_pwm      = lcm_getpwm,
#endif
};
