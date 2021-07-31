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

#define LCM_ID_NV3051 ((0x3821))

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE									0

#if defined(LCM_720_1440)
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1440)
#elif defined(LCM_720_1498)
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1498)
#elif defined(LCM_640_1280) //xen 20170922
#define FRAME_WIDTH  										(640)
#define FRAME_HEIGHT 										(1280)
#elif defined(LCM_540_1132)
#define FRAME_WIDTH  										(540)
#define FRAME_HEIGHT 										(1132)
#elif defined(LCM_720_1520)
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1520)
#elif defined(LCM_720_1560)
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1560)
#else
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#endif

#define LCM_PHYSICAL_WIDTH									(0)
#define LCM_PHYSICAL_HEIGHT									(0)

#define REGFLAG_DELAY             							0xFFE
#define REGFLAG_END_OF_TABLE      							0x100   // END OF REGISTERS MARKER

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#if defined(YK676_CUSTOMER_KEMI_LT600_HDPLUS)
  #define LCD_QICAI_63_NV3051_HSD_HDPLUS
  #define FOUR_LANE_SUPPORT
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

//static LCM_DSI_MODE_SWITCH_CMD lcm_switch_mode_cmd;
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
    unsigned char para_list[64];
};

#if defined(LCD_QICAI_63_NV3051_HSD_HDPLUS) 
static unsigned char BufLcmInfo[] = "LCD_QICAI_63_NV3051_HSD_HDPLUS_20200321";
static struct LCM_setting_table lcm_initialization_setting[] = {

{0xFF, 1, {0x30}},
{0xFF, 1, {0x52}},
{0xFF, 1, {0x01}},
{0xE3, 1, {0x00}},
{0x0A, 1, {0x01}},
{0x10, 1, {0xa1}},
{0x11, 1, {0x97}},
{0x12, 1, {0xb9}},
{0x13, 1, {0x1a}},
{0x14, 1, {0x06}},
{0x15, 1, {0x43}},
{0x16, 1, {0x43}},
{0x17, 1, {0x52}},
{0x18, 1, {0x00}},
{0x19, 1, {0x33}},
{0x1a, 1, {0x30}},
{0x1b, 1, {0x33}},
{0x1c, 1, {0x51}},
{0x1d, 1, {0x00}},
{0x1e, 1, {0x00}},
//{0x20, 1, {0x90}},
{0x25, 1, {0x0A}},
//{0x20, 1, {0xA0}},
{0x29, 1, {0x05}},
{0x2A, 1, {0xef}},  
{0x30, 1, {0x38}},
{0x31, 1, {0x00}},
{0x32, 1, {0x34}},
{0x38, 1, {0x76}},
//9C
{0x39, 1, {0x81}},
//A7
{0x3A, 1, {0x2d}},

{0x3b, 1, {0x55}},
{0x40, 1, {0x10}},
{0x49, 1, {0x20}},
{0x50, 1, {0x99}},
{0x58, 1, {0x05}},
{0x91, 1, {0x77}},
{0x92, 1, {0x77}},

{0xa0, 1, {0x5F}},
{0xa1, 1, {0x50}},
{0xA4, 1, {0x9c}},
{0xA7, 1, {0x02}},
{0xA8, 1, {0x01}},
{0xA9, 1, {0x01}},
{0xAA, 1, {0xA8}},
{0xAB, 1, {0x28}},
{0xAC, 1, {0x40}},
{0xAD, 1, {0x42}},
{0xAE, 1, {0x42}},
{0xAF, 1, {0x02}},
{0xB0, 1, {0x42}},
{0xB1, 1, {0x26}},
{0xB2, 1, {0x26}},
{0xB3, 1, {0x26}},
{0xB4, 1, {0x22}},
{0xB5, 1, {0x42}},
{0xB6, 1, {0x26}},
{0xB7, 1, {0x42}},
{0xB8, 1, {0x26}},
{0xf0, 1, {0x00}},  
{0xf6, 1, {0xc0}},

{0xFF, 1, {0x30}},
{0xFF, 1, {0x52}},
{0xFF, 1, {0x02}},
{0xB1, 1, {0x11}},
{0xD1, 1, {0x15}},
{0xB4, 1, {0x2F}},
{0xD4, 1, {0x31}},
{0xB2, 1, {0x13}},
{0xD2, 1, {0x15}},
{0xB3, 1, {0x2D}},
{0xD3, 1, {0x33}},
{0xB6, 1, {0x22}},
{0xD6, 1, {0x24}},
{0xB7, 1, {0x3D}},
{0xD7, 1, {0x41}},
{0xC1, 1, {0x08}},
{0xE1, 1, {0x08}},
{0xB8, 1, {0x0D}},
{0xD8, 1, {0x0D}},
{0xB9, 1, {0x04}},
{0xD9, 1, {0x04}},
{0xBD, 1, {0x15}},
{0xDD, 1, {0x15}},
{0xBC, 1, {0x13}},
{0xDC, 1, {0x13}},
{0xBB, 1, {0x11}},
{0xDB, 1, {0x11}},
{0xBA, 1, {0x11}},
{0xDA, 1, {0x11}},
{0xBE, 1, {0x17}},
{0xDE, 1, {0x19}},
{0xBF, 1, {0x0F}},
{0xDF, 1, {0x11}},
{0xC0, 1, {0x16}},
{0xE0, 1, {0x18}},
{0xB5, 1, {0x37}},
{0xD5, 1, {0x34}},
{0xB0, 1, {0x02}},
{0xD0, 1, {0x05}},
{0xFF, 1, {0x30}},
{0xFF, 1, {0x52}},
{0xFF, 1, {0x03}},
{0x05, 1, {0x00}},     
{0x06, 1, {0x00}},    
{0x08, 1, {0x06}},   
{0x09, 1, {0x07}},  
{0x25, 1, {0x32}},     
{0x2A, 1, {0xfa}},   
{0x2B, 1, {0xfb}},  
{0x70, 1, {0x0f}},  
{0x71, 1, {0xc0}},  
{0x30, 1, {0x2A}}, 
{0x31, 1, {0x2A}}, 
{0x32, 1, {0x2A}}, 
{0x33, 1, {0x2A}}, 
{0x34, 1, {0xb1}},  
{0x35, 1, {0x76}},  
{0x36, 1, {0x08}},  
{0x40, 1, {0x07}},  
{0x41, 1, {0x08}},   
{0x42, 1, {0x09}},   
{0x43, 1, {0x0a}},   
{0x45, 1, {0xf4}},  
{0x46, 1, {0xf5}},   
{0x48, 1, {0xf6}},   
{0x49, 1, {0xF7}},  
{0x50, 1, {0x0b}},  
{0x51, 1, {0x0c}},   
{0x52, 1, {0x0d}},   
{0x53, 1, {0x0e}},   
{0x55, 1, {0xf8}},   
{0x56, 1, {0xf9}},   
{0x58, 1, {0xfa}},   
{0x59, 1, {0xfb}},  
{0x80, 1, {0x1f}},  
{0x81, 1, {0x00}},   
{0x82, 1, {0x16}},   
{0x83, 1, {0x15}},  
{0x84, 1, {0x0a}},   
{0x85, 1, {0x0c}},   
{0x86, 1, {0x0e}},   
{0x87, 1, {0x10}},   
{0x88, 1, {0x02}},   
{0x8F, 1, {0x06}},   
{0x96, 1, {0x1f}},   
{0x97, 1, {0x00}},   
{0x98, 1, {0x18}},  
{0x99, 1, {0x17}},  
{0x9A, 1, {0x09}},  
{0x9B, 1, {0x0b}},   
{0x9C, 1, {0x0d}},  
{0x9D, 1, {0x0f}},   
{0x9E, 1, {0x01}},   
{0xA5, 1, {0x05}},   
{0xB0, 1, {0x00}},  
{0xB1, 1, {0x1f}},  
{0xB2, 1, {0x16}},   
{0xB3, 1, {0x15}},  
{0xB4, 1, {0x0f}},   
{0xB5, 1, {0x0d}},  
{0xB6, 1, {0x0b}},   
{0xB7, 1, {0x09}},   
{0xB8, 1, {0x05}},   
{0xBF, 1, {0x01}},   
{0xC6, 1, {0x00}}, 
{0xC7, 1, {0x1f}},   
{0xC8, 1, {0x18}},  
{0xC9, 1, {0x17}},  
{0xCA, 1, {0x10}},   
{0xCB, 1, {0x0e}},   
{0xCC, 1, {0x0c}},  
{0xCD, 1, {0x0a}},   
{0xCE, 1, {0x06}},   
{0xD5, 1, {0x02}},   
{0xFF, 1, {0x30}}, 
{0xFF, 1, {0x52}}, 
{0xFF, 1, {0x00}}, 
{0x36, 1, {0x02}}, 

{0x11, 0, {0x00}},
{REGFLAG_DELAY,120,{}},

{0x29, 0, {0x00}},
{REGFLAG_DELAY,50,{}},

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

	
#ifndef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	params->physical_width = LCM_PHYSICAL_WIDTH/1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
	params->physical_width_um = LCM_PHYSICAL_WIDTH;
	params->physical_height_um = LCM_PHYSICAL_HEIGHT;	
#endif

	params->dsi.mode   = SYNC_EVENT_VDO_MODE;

#ifndef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	params->dsi.switch_mode = CMD_MODE;
	params->dsi.switch_mode_enable = 0;
#endif

	#if defined(FOUR_LANE_SUPPORT)
	params->dsi.LANE_NUM	= LCM_FOUR_LANE;
	#else
	params->dsi.LANE_NUM	= LCM_THREE_LANE;
	#endif

	params->dsi.g_StrLcmInfo = BufLcmInfo;
	
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order     = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq	   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding 	   = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format	       = LCM_DSI_FORMAT_RGB888;
	
	params->dsi.packet_size=256;
	// Video mode setting		
	params->dsi.intermediat_buffer_num = 0;
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
	//params->dsi.vertical_active_line=FRAME_HEIGHT;


    params->dsi.vertical_sync_active				=2;
    params->dsi.vertical_backporch				    =8;
    params->dsi.vertical_frontporch				    = 14;
    params->dsi.vertical_active_line				= FRAME_HEIGHT; 

    params->dsi.horizontal_sync_active			    = 2;//11;
    params->dsi.horizontal_backporch				= 30;//64;20
    params->dsi.horizontal_frontporch				= 24;//64;20
    params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

    // Bit rate calculation
    params->dsi.PLL_CLOCK=215;//234

   // params->dsi.ssc_disable = 1;
}
static struct LCM_setting_table lcm_prepare_setting[] = {

{0xFF, 1, {0x30}},
{0xFF, 1, {0x52}},
{0xFF, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}	
};

static unsigned int lcm_compare_id(void)
{
	unsigned int array[4];
	unsigned short device_id;
	unsigned char buffer[2];
	
	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(120);
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
	read_reg_v2(0x04, buffer, 2);
	
	device_id = buffer[0]<<8|buffer[1];

	//LCM_PRINT("<<ST7701 ID Read>>%s, device_id=0x%04x,buffer[0]=%x,buffer[1]=%x\n", __func__, device_id,buffer[0],buffer[1]);

	return (device_id == 0x3052)?1:0;
	//return 1;
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
	data = 0x14;//0x0F 5.5v  0x14 6v
	SET_RESET_PIN(0);
#ifndef BUILD_LK 
	//set_gpio_lcd_enp(1);
	set_gpio_lcm_enp_enn(1);
		MDELAY(5);
#endif
#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
#else
	ret = tps65132_write_bytes(cmd, data);
#endif
	cmd = 0x01;
	data = 0x14;//0x0F 5.5v
#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
#else
	ret = tps65132_write_bytes(cmd, data);
#endif
#endif
#ifndef BUILD_LK
	//printk("nv3051:lcm_init lcd-id pin level=%d\n", get_lcd_id_state());
#endif
	SET_RESET_PIN(1);	 
	MDELAY(50); //20); 
	SET_RESET_PIN(0);
	MDELAY(200); //50); 
	SET_RESET_PIN(1);
	MDELAY(150); //150);
#if defined(DOUBLE_LCM_BY_IDPIN)
	if (get_lcd_id_state()==0)
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
	//LCM_DBG("lcm_suspend");
#ifndef BUILD_LK
	//printk("nv3051:lcm_suspend lcd-id pin level=%d\n", get_lcd_id_state());
#endif
#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#endif
	SET_RESET_PIN(1);	 
	MDELAY(10); 
	SET_RESET_PIN(0);
	MDELAY(10); 
	SET_RESET_PIN(1);
	MDELAY(120);
#ifdef CONFIG_MTK_LCM_5V_IC
	//MDELAY(10);
#ifndef BUILD_LK
	set_gpio_lcm_enp_enn(0);
#endif
#endif
}


static void lcm_resume(void)
{
	//LCM_DBG("lcm_resume");
#ifndef BUILD_LK
	//printk("nv3051:lcm_resume lcd-id pin level=%d\n", get_lcd_id_state());
#endif
	lcm_init();

}


// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
#ifdef BUILD_LK //xjl 20180531
LCM_DRIVER nv3051_6735_dsi_lcm_drv_yk6xx = 
#else
struct LCM_DRIVER nv3051_6735_dsi_lcm_drv_yk6xx = 
#endif
{
	.name		    = "nv3051_67xx_dsi_video",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
};
