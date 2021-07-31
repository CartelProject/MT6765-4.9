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
#ifdef BUILD_LK

#else
    #include <linux/string.h>
#endif
#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define LCM_ID_HX8399C                                     0x8399C

/////////////////////////////////////////////////////////////////////////

//set the pixels by pixels difination of LCM_480_854 or LCM_480_800 in ProjectConfig.mk,130309
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1520)


#define LCM_PHYSICAL_WIDTH									(0)
#define LCM_PHYSICAL_HEIGHT									(0)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER


/*--------------------------LCD module explaination begin---------------------------------------*/

//LCD module explaination							//Project		Custom		W&H		Glass	degree	data		HWversion

//LCD_WEIZHOUWEI_588_HX8363B_TM_0516			//yk810		5.88dabao	480*800		TM		180		130516	V0.9
/*--------------------------LCD module explaination end----------------------------------------*/


//set LCM module
#if defined(YK676_CUSTOMER_KEMI_LT600_HDPLUS) // xen 20171223
  #define LCD_HANLONG_HX8399C_INX_62
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
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                                                                                   lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)                                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)  

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
    unsigned char para_list[55]; //64]; //xen for less space 20170511
};


#if defined (LCD_HANLONG_HX8399C_INX_62)
static unsigned char BufLcmInfo[] = "LCD_HANLONG_HX8399C_INX_62";
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xB9,3,{0xff,0x83,0x99}},
{0xBA,2,{0x63,0x23}},
{0xD2,1,{0x88}},
{0xB1,12,{0x00,0x04,0x72,0x92,0x01,0x32,0x33,0x11,0x11,0x4D,0x58,0x06}},
{0xB2,11,{0x00,0x80,0xD0,0x7C,0x05,0x07,0x5A,0x11,0x10,0x00,0x00}},
{0xB6,2,{0x80,0x80}},  //VCOM
{0xB4,44,{0x00,0xFF,0x02,0xD7,0x02,0xD7,0x02,0xD7,0x02,0x00,0x03,0x05,0x00,0x3F,0x03,0x06,0x08,0x21,0x03,0x00,0x00,0x00,0xAC,0x87,0x02,0xA7,0x02,0xA7,0x02,0xA7,0x02,0x00,0x03,0x05,0x00,0x2D,0x03,0x06,0x08,0x00,0x00,0x00,0xAC,0x01}},
{0xD3,39,{0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x32,0x10,0x09,0x00,0x09,0x32,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x0A,0x0A,0x03,0x00,0x00,0x00,0x08,0x60,0x00,0x40,0x00,0x09,0x20,0x01}},
{0xD5,32,{0x18,0x18,0x31,0x31,0x30,0x30,0x2F,0x2F,0x18,0x18,0x19,0x19,0x03,0x03,0x02,0x02,0x01,0x01,0x00,0x00,0x20,0x20,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18}},
{0xD6,32,{0x18,0x18,0x31,0x31,0x30,0x30,0x2F,0x2F,0x18,0x18,0x19,0x19,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x20,0x20,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40}},
{0xD8,16,{0xAA,0xBF,0xF8,0x00,0xAA,0xBF,0xF8,0x00,0xAA,0xBA,0xA8,0x00,0xAA,0xBA,0xA8,0x00}},

//Set_GAMMA 2.0
//{0xE0,54,{0x00,0x1B,0x29,0x24,0x53,0x5D,0x69,0x62,0x68,0x6F,0x75,0x79,0x7C,0x82,0x86,0x8A,0x8E,0x94,0x94,0x9A,0x8C,0x98,0x9A,0x50,0x4D,0x59,0x77,0x00,0x1B,0x29,0x24,0x53,0x5D,0x69,0x62,0x68,0x6F,0x75,0x79,0x7C,0x82,0x86,0x8A,0x8E,0x94,0x94,0x9A,0x8C,0x98,0x9A,0x50,0x4D,0x59,0x77}},
//Set_GAMMA 2.2
{0xE0,54,{0x00,0x1C,0x2C,0x28,0x5B,0x64,0x6F,0x69,0x6F,0x76,0x7B,0x81,0x84,0x89,0x8F,0x91,0x94,0x9C,0x9D,0xA5,0x99,0xA6,0xA9,0x58,0x54,0x60,0x77,0x00,0x1C,0x2C,0x28,0x5B,0x64,0x6F,0x69,0x6F,0x76,0x7B,0x81,0x84,0x89,0x8F,0x91,0x94,0x9C,0x9D,0xA5,0x99,0xA6,0xA9,0x58,0x54,0x60,0x77}},
//Set_GAMMA 2.5
//{0xE0,54,{0x00,0x1F,0x2F,0x2D,0x63,0x6D,0x79,0x73,0x79,0x80,0x84,0x8B,0x8D,0x93,0x99,0x9C,0xA0,0xA8,0xAC,0xB5,0xAB,0xBB,0xBE,0x62,0x5E,0x69,0x77,0x00,0x1F,0x2F,0x2D,0x63,0x6D,0x79,0x73,0x79,0x80,0x84,0x8B,0x8D,0x93,0x99,0x9C,0xA0,0xA8,0xAC,0xB5,0xAB,0xBB,0xBE,0x62,0x5E,0x69,0x77}},

{0xBD,2,{0x01}},
{0xD8,18,{0xBF,0xEF,0xF8,0x00,0xBF,0xEF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0xBD,1,{0x02}},
{0xD8,8,{0xBF,0xEF,0xF8,0x00,0xBF,0xEF,0xF8,0x00}},
{0xBD,1,{0x00}},
{0xCC,1,{0x08}},
{0xC6,2,{0xFF,0xF9}},
{0xB9,3,{0x00,0x00,0x00}},

{0x11,1,{0x00}},                  
{REGFLAG_DELAY,120,{}},	          
{0x29,1,{0x00}},                  
{REGFLAG_DELAY,50,{}},            
{REGFLAG_END_OF_TABLE, 0x00, {}}  
};

#endif

/*
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 20, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};*/


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 100, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 120, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_compare_id_setting[] = {
        // Display off sequence
	{0x00,1,{0x00}},
	{0xB9,  3,      {0xFF, 0x83,0x79}},
	{REGFLAG_DELAY, 10, {}},

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
		params->dsi.mode   = SYNC_PULSE_VDO_MODE; // BURST_VDO_MODE;

#ifndef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	params->dsi.switch_mode = CMD_MODE;
	params->dsi.switch_mode_enable = 0;
#endif
		params->dsi.g_StrLcmInfo = BufLcmInfo;

		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
    		params->dsi.packet_size=256;
		// Video mode setting		
		params->dsi.intermediat_buffer_num = 2;
    
		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.vertical_sync_active				= 3;//2;
	params->dsi.vertical_backporch					= 16;
	params->dsi.vertical_frontporch					= 10;  // rom Q driver
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 20;	
	params->dsi.horizontal_backporch				= 60;//20;
	params->dsi.horizontal_frontporch				= 60;//40;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		
    params->dsi.PLL_CLOCK = 240; //this value must be in MTK suggested table
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
	data = 0x0f;//0x0F 5.5v  0x14 6v
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
	data = 0x0f;//0x0F 5.5v
#ifdef BUILD_LK
	ret = TPS65132_write_byte(cmd, data);
#else
	ret = tps65132_write_bytes(cmd, data);
#endif
#endif
	SET_RESET_PIN(1);
  	MDELAY(20);
	SET_RESET_PIN(0);
	//MDELAY(1);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(50);

#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);//
#else
	push_table(NULL, lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}


static unsigned int lcm_compare_id(void);
static void lcm_suspend(void)
{

#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#endif
	SET_RESET_PIN(0);
#ifdef CONFIG_MTK_LCM_5V_IC
	//MDELAY(10);
#ifndef BUILD_LK
	set_gpio_lcm_enp_enn(0);
#endif
#endif
}


static void lcm_resume(void)
{
	lcm_init();
//	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


static unsigned int lcm_compare_id(void)  // wanghe 2014-05-07 id should fix???
{
	unsigned int id=0;
	unsigned char buffer[3]={0};
	unsigned int array[16];
	
	return 1;
	SET_RESET_PIN(1);
	MDELAY(25);
	SET_RESET_PIN(0);
	MDELAY(25);
	SET_RESET_PIN(1);
	MDELAY(150);

#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_compare_id_setting, sizeof(lcm_compare_id_setting) / sizeof(struct LCM_setting_table), 1);
#endif

	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xDA, &buffer[0], 1);//0x83
		
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xDB, &buffer[1], 1);//0x79
		
	array[0] = 0x00023700;
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xDC, &buffer[2], 1);//0x0c 

	id=(buffer[0]<<12)|(buffer[1]<<4)|buffer[2];

//	LCM_PRINT("HX8379c sensor ID= 0x%x",id);

	return (LCM_ID_HX8399C==id)?1:0;  

}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
#ifdef BUILD_LK //xjl 20180531
LCM_DRIVER hx8399c_6735_dsi_lcm_drv_yk6xx = 
#else
struct LCM_DRIVER hx8399c_6735_dsi_lcm_drv_yk6xx = 
#endif
{

	.name		= "hx8399c_67xx_dsi_video",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};

