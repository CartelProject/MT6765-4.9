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

#define LCM_ID_HX8379A                                     0x8379A

/////////////////////////////////////////////////////////////////////////

//set the pixels by pixels difination of LCM_480_854 or LCM_480_800 in ProjectConfig.mk,130309
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

#define LCM_PHYSICAL_WIDTH									(0)
#define LCM_PHYSICAL_HEIGHT									(0)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER


/*--------------------------LCD module explaination begin---------------------------------------*/

//LCD module explaination							//Project		Custom		W&H		Glass	degree	data		HWversion

//LCD_WEIZHOUWEI_588_HX8363B_TM_0516			//yk810		5.88dabao	480*800		TM		180		130516	V0.9
/*--------------------------LCD module explaination end----------------------------------------*/


//set LCM module
#if defined(YK568_CUSTOMER_SHUANGHOU_WVGA) // xen 20171223
  #define LCD_DEZHIXIN_HX8379A_BOE_31
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
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                                                                                   lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)                                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)  

       

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[50]; //64]; //xen for less space 20170511
};


#if defined (LCD_DEZHIXIN_HX8379A_BOE_31)
#if defined (LCM_ROTATE_180)
	static unsigned char BufLcmInfo[] = "LCD_KELAI_45_HX8379C_BOE_0528_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_KELAI_45_HX8379C_BOE_0528";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xB9,3 ,{0xFF,0x83,0x79}},
	{0xB1,16,{0x44,0x18,0x18,0x31,0x51,0x50,0xD0,0xD8,0x58,0x80,0x38,0x38,0xF8,0x33,0x32,0x22}},
	{0xB2,9 ,{0x80,0x3C,0x0A,0x03,0x70,0x50,0x11,0x42,0x1D}},
	{0xB4,10,{0x02,0x7C,0x02,0x7C,0x02,0x7C,0x22,0x86,0x23,0x86}},
	{0xC7,4 ,{0x00,0x00,0x00,0xC0}},
	{0xCC,1 ,{0x02}},
	{0xD2,1 ,{0x77}},
	{0xD3,37,{0x00,0x07,0x00,0x00,0x00,0x08,0x08,0x32,0x10,0x01,0x00,0x01,0x03,0x72,0x03,0x72,0x00,0x08,0x00,0x08,
	          0x33,0x33,0x05,0x05,0x37,0x05,0x05,0x37,0x08,0x00,0x00,0x00,0x0A,0x00,0x01,0x01,0x0F}},
	{0xD5,34,{0x18,0x18,0x18,0x18,0x18,0x18,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00,0x18,0x18,0x21,0x20,0x18,0x18,
	          0x19,0x19,0x23,0x22,0x38,0x38,0x78,0x78,0x18,0x18,0x18,0x18,0x00,0x00}},
	{0xD6,32,{0x18,0x18,0x18,0x18,0x18,0x18,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x18,0x18,0x22,0x23,0x19,0x19,
	          0x18,0x18,0x20,0x21,0x38,0x38,0x38,0x38,0x18,0x18,0x18,0x18}},
	{0xE0,42,{0x00,0x05,0x08,0x20,0x24,0x3F,0x17,0x38,0x09,0x0A,0x0C,0x17,0x0F,0x11,0x13,0x12,0x14,0x0A,0x15,0x16,
	          0x18,0x00,0x05,0x08,0x20,0x24,0x3F,0x17,0x38,0x09,0x0A,0x0C,0x17,0x0F,0x11,0x13,0x12,0x14,0x0A,0x15,0x16,0x18}},
	//{0xE0,42,{0x00,0x07,0x0B,0x28,0x2F,0x3F,0x1B,0x3D,0x08,0x0A,0x0C,0x17,0x0F,0x11,0x13,0x12,0x14,0x0A,0x15,0x16,
	          //0x18,0x00,0x07,0x0B,0x28,0x2F,0x3F,0x1B,0x3D,0x08,0x0A,0x0C,0x17,0x0F,0x11,0x13,0x12,0x14,0x0A,0x15,0x16,0x18}},
	{0xB6,2 ,{0x5E,0x5E}},		  
				  
	{0x11,1,{0x00}},		// Sleep-Out
	{REGFLAG_DELAY, 150,  {}},	
	{0x29,1,{0x00}},
	{REGFLAG_DELAY, 50, {} }, 
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};
#else
#if defined (LCM_ROTATE_180)
	static unsigned char BufLcmInfo[] = "LCD_KELAI_45_HX8379C_BOE_DEF_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_KELAI_45_HX8379C_BOE_DEF";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {
	{0xB9,3,{0xFF,0x83,0x79}},

	{0xB1,20,{0x64,0x12,0x12,0x31,0x91,0x90,0xD0,0xdC,0xc8,0xc7,
	          0x38,0x38,0xF8,0x44,0x44,0x44,0x00,0x80,0x30,0x00}},
	          
	{0xB2,9,{0x80,0xFE,0x0B,0x04,0x30,0x50,0x11,0x42,0x1D}},

	{0xB4,10,{0x00,0x80,0x00,0x86,0x00,0x86,0x08,0x86,0x08,0x86}},
	{0xD3,29,{0x00,0x00,0x00,0x00,0x00,0x06,0x06,0x32,0x10,0x06,
	          0x00,0x06,0x03,0x70,0x03,0x70,0x00,0x08,0x00,0x08,
	          0x11,0x11,0x06,0x06,0x13,0x06,0x06,0x13,0x09}},
	          
	{0xCC,1,{0x02}},

	{0xD5,32,{0x18,0x18,0x19,0x19,0x01,0x00,0x03,0x02,0x21,0x20,
	          0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
	          0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18}},
	          
	{0xD6,32,{0x18,0x18,0x18,0x18,0x18,0x18,0x26,0x27,0x24,0x25,
	          0x02,0x03,0x00,0x01,0x06,0x07,0x04,0x05,0x22,0x23,
	          0x20,0x21,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18}},
	      
	{0xE0,42,{0x00,0x00,0x00,0x1B,0x19,0x3F,0x13,0x31,0x0A,0x0C,
	          0x0C,0x18,0x0D,0x10,0x17,0x12,0x14,0x08,0x13,0x14,
	          0x18,0x00,0x00,0x00,0x1A,0x1C,0x3F,0x12,0x31,0x0A,
	          0x0B,0x0C,0x18,0x10,0x14,0x13,0x14,0x15,0x08,0x13,0x15,0x1A}},	
	 
	{0xB6,2,{0x45,0x45}},         
	          
	{0x11,1,{0x00}},		// Sleep-Out
	{REGFLAG_DELAY, 150,  {}},  
	{0x29,1,{0x00}},
	{REGFLAG_DELAY, 50, {} }, 
		 
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
	
#ifndef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	params->physical_width = LCM_PHYSICAL_WIDTH/1000;
	params->physical_height = LCM_PHYSICAL_HEIGHT/1000;
	params->physical_width_um = LCM_PHYSICAL_WIDTH;
	params->physical_height_um = LCM_PHYSICAL_HEIGHT;	
#endif
		params->dsi.mode   = SYNC_PULSE_VDO_MODE; // BURST_VDO_MODE;

		params->dsi.g_StrLcmInfo = BufLcmInfo;

		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
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

		params->dsi.vertical_sync_active				= 4;  // 3;
		params->dsi.vertical_backporch 				= 8; //7; // 10; is important if up or down black line
		params->dsi.vertical_frontporch 				= 16; // 16 20;
		params->dsi.vertical_active_line				= FRAME_HEIGHT;
    
		params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				= 80;//50;
		params->dsi.horizontal_frontporch				= 80;//50;
		//params->dsi.horizontal_blanking_pixel				= 60;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		params->dsi.PLL_CLOCK = 149;//200;  // 210
}


static void lcm_init(void)
{
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

	return (LCM_ID_HX8379A==id)?1:0;  

}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hx8379a_6735_dsi_lcm_drv_yk6xx = 
{

	.name		= "hx8379a_67xx_dsi_video",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};

