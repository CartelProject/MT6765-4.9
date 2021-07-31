#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include "lcm_drv.h"

#ifdef BUILD_LK //xen 20160603
    #include <platform/mt_gpio.h>
    #include "cust_gpio_usage.h"
#else
    //#include <linux/gpio.h>
    #include <mt-plat/mtk_gpio.h>
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#if defined(LCM_720_1440)
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1440)
#else
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)
#endif

#define LCM_PHYSICAL_WIDTH									(0)
#define LCM_PHYSICAL_HEIGHT									(0)

#define REGFLAG_DELAY             							0xFE
#define REGFLAG_END_OF_TABLE      							0x100   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0
#define LCM_ID_OTM1290A                                    (0x1290)


#if defined(YK736_CUSTOMER_CAIFU9_KS937_HDPLUS)
#define LCD_HELITAI_57_OTM1290A_IVO
#else
#define LCD_HELITAI_57_OTM1290A_IVO
#endif


// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update) \
	lcm_util.dsi_set_cmdq_V22(cmdq, cmd, count, ppara, force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									    lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				    lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    
       

struct LCM_setting_table {
    unsigned int cmd;
    unsigned char count;
    unsigned char para_list[25];//64]; //xen for less space 20170511
};

#if defined(LCD_HELITAI_57_OTM1290A_IVO)
static unsigned char BufLcmInfo[] = "LCD_HELITAI_57_OTM1290A_IVO_180308";
static struct LCM_setting_table lcm_initialization_setting[] = {	
	{0x00,1, {0x00}}, 
	{0xff,3, {0x12,0x90,0x01}},       //EXTC = 1 
	
	{0x00,1, {0x80}}, 
	{0xff,2, {0x12,0x90}},  //CMD2 enable
	
	{0x00,1, {0x90}}, 
	{0xff,1, {0xb0}}, //0xb0 => mipi 4lane, 0xa0 => mipi 3lane 
	
	//-------------------- panel setting --------------------//
	{0x00,1, {0x80}},
	{0xc0,9, {0x00,0x84,0x00,0x10,0x00,0x10,0x01,0x04,0x03}},  //RTN = 84(18:9)
	
	{0x00,1, {0x90}},           
	{0xC1,1, {0x03}}, //20170711 mclk°£ÀW
	
	{0x00,1, {0xac}}, 
	{0xc1,1, {0x0c}},  //20170711 for 18:9
	
	{0x00,1, {0xc0}}, 
	{0xc0,1, {0x88}},  //for 18:9
	
	{0x00,1, {0xa6}}, 
	{0xc0,6, {0x00,0x60,0x00,0x00,0x00,0x02}},  //20170711 Panel timing setting
	
	{0x00,1, {0xa1}}, 
	{0xb3,1, {0x10}},  //resolution enable    
	
	{0x00,1, {0xb0}}, 
	{0xb3,4, {0x02,0xd0,0x05,0xa0}},  // 720x1440(18:9)           
	
	{0x00,1, {0x81}}, 
	{0xb2,1, {0x21}},  //BGR
	
	{0x00,1, {0xb1}}, 
	{0xc0,1, {0x40}},  //mirror X2
	
	{0x00,1, {0x85}}, 
	{0xb0,1, {0x01}},  //BTA TLPX 4X
	
	{0x00,1, {0x95}}, 
	{0xb0,1, {0x1b}},  //speedup	
	
	{0x00,1, {0x97}}, 
	{0xb0,1, {0x00}},  //MIPI weak pull high/low
	
	{0x00,1, {0x80}}, 
	{0xa4,1, {0x3f}},  //20170711 source floating bias short + PCH GVDD
	
	{0x00,1, {0x81}}, 
	{0xa4,1, {0x02}},

	{0x00,1, {0x80}}, 
	{0xa5,1, {0xb3}},  //PCH level 1/2 GVDD
	
	{0x00,1, {0xa2}}, 
	{0xc0,1, {0x00}},  //PWROFF Blank frame     
	
	{0x00,1, {0x84}}, 
	{0xf5,5, {0x72,0x82,0x79,0x0b,0x92}},  //pump en VGH/VGL
	
	{0x00,1, {0x86}}, 
	{0xa5,1, {0xf0}},  //LVD enable     
	
	{0x00,1, {0xD0}},   //TCON auto dis    20170623    
	{0xC0,4, {0x00,0x00,0x00,0x00}},   
	
	{0x00,1, {0x87}}, 
	{0xc5,2, {0x01,0x44}},  //gate slew rate	     
	
	//-------------------- power setting --------------------//
	{0x00,1, {0x80}},                       //VGH=15V, VGL=-11V, pump ratio:VGH=3x, VGL=-3x
	{0xc5,2, {0xd5,0x23}},      //D[1]:VGH/VGL pump clk stop @ sample hold
	
	{0x00,1, {0x83}},
	{0xC5,1, {0x55}},

	{0x00,1, {0x85}},
	{0xC5,1, {0x12}}, //20170711 Power IC       
	
	{0x00,1, {0x00}}, 
	{0xd8,2, {0x17,0x17}},  // GVDD/NGVDD = 4.8V/-4.8V         
	
	{0x00,1, {0x01}}, 
	{0xd9,1, {0x2f}},  // Vcom    -0.7V
	
	//-------------------- panel timing state control --------------------//
	{0x00,1, {0x80}},       //panel timing state control
	{0xcb,8, {0x14,0x14,0x14,0x14,0x14,0x14,0x14,0x00}},
	
	{0x00,1, {0xa0}},      //panel timing state control
	{0xcb,1, {0x00}},
	
	//-------------------- panel pad mapping control --------------------//  
	{0x00,1, {0x80}},       //panel timing state control
	{0xcc,15,{0x14,0x12,0x10,0x0e,0x0c,0x0a,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00}},
	
	{0x00,1, {0x90}},       //panel timing state control
	{0xcc,15,{0x00,0x00,0x06,0x08,0x00,0x00,0x13,0x11,0x0f,0x0d,0x0b,0x09,0x01,0x01,0x01}},
	
	{0x00,1, {0xa0}},       //panel timing state control
	{0xcc,12,{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x07,0x00,0x00}},
	
	{0x00,1, {0xb0}},       //panel timing state control
	{0xcc,15,{0x05,0x07,0x0d,0x0f,0x09,0x0b,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00}},
	
	{0x00,1, {0xc0}},       //panel timing state control
	{0xcc,15,{0x00,0x00,0x13,0x11,0x00,0x00,0x06,0x08,0x0e,0x10,0x0a,0x0c,0x01,0x01,0x01}},
	
	{0x00,1, {0xd0}},       //panel timing state control
	{0xcc,12,{0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x14,0x12,0x00,0x00}},
	
	//-------------------- panel timing setting --------------------//
	{0x00,1, {0xe0}},      //GOA EQ setting
	{0xcc,5, {0x00,0x00,0x00,0x00,0x00}},
	
	{0x00,1,{0x80}},                       //panel VST setting
	{0xce,6, {0x88,0x00,0x87,0x86,0x85,0x33}},          /////////////
	
	{0x00,1, {0x90}},                       //panel VEND setting
	{0xce,5, {0x84,0x00,0x83,0x82,0x81}},		///////////////////////
	
	{0x00,1, {0xa0}},                       //CLKA setting
	{0xce,11,{0x85,0x81,0x00,0x84,0x00,0x27,0x83,0x01,0x00,0x82,0x02}}, //////////////////
	
	{0x00,1, {0xb0}},                       //CLKB setting
	{0xce,12,{0x81,0x03,0x00,0x00,0x04,0x01,0x05,0x00,0x02,0x06,0x10,0x00}},  
	
	{0x00,1, {0xa1}},
	{0xf3,3, {0x27,0x10,0x00}}, // 8phase F3A1/F3A2/F3A3 setting same as CEA5/CEBA/CEBB
	
	{0x00,1, {0xe0}},                       //GCH setting
	{0xce,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 
	
	{0x00,1, {0xf0}},                        //toggle mode setting
	{0xce,9, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}}, 
	
	{0x00,1, {0x80}},                       //ECLK setting
	{0xcf,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},  //20170711 ECLK shift/chop 2.5us
	
	//-------------------- gamma setting --------------------//
	{0x00,1, {0x00}},                       
	{0xE1,16,{0x32,0x4c,0x5b,0x6c,0x78,0x93,0x8e,0xa3,0x55,0x42,0x4c,0x36,0x25,0x18,0x14,0x12}}, 
	
	{0x00,1, {0x00}},                       
	{0xE2,16,{0x32,0x4c,0x5b,0x6c,0x78,0x93,0x8e,0xa3,0x55,0x42,0x4c,0x36,0x25,0x18,0x14,0x12}}, 

	{0x00,1, {0x92}},	
	{0xF4,3, {0x0A,0xAA,0xAA}},

	{0x00,1, {0x80}},                      //Disable Command 2
	{0xff,2, {0xff,0xff}},
	
	{0x00,1, {0x00}},                     //EXTC=0
	{0xff,3, {0xff,0xff,0xff}},
	{0x11,1, {0x00}},
	{REGFLAG_DELAY, 120, {0x00}},	
	{0x29,1, {0x00}},
	{REGFLAG_DELAY, 10, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}

};
#endif

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	//{0x22, 1, {0x00}},
	//{REGFLAG_DELAY, 100, {}},	
	
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
   
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
	params->dsi.mode  = BURST_VDO_MODE;//SYNC_EVENT_VDO_MODE;//SYNC_PULSE_VDO_MODE; //BURST_VDO_MODE;
	// DSI
#ifndef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	params->dsi.switch_mode = CMD_MODE;
	params->dsi.switch_mode_enable = 0;
#endif
	params->dsi.LANE_NUM				= LCM_FOUR_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;
	
	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;
	
	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;
	
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
	
	params->dsi.g_StrLcmInfo = BufLcmInfo;

	//params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
	params->dsi.vertical_sync_active    = 2;//8; // 4; //10;//8
	params->dsi.vertical_backporch		= 14;//20; // 16; //40;//50 25
	params->dsi.vertical_frontporch 	= 16;//21; //40;//20 25
	params->dsi.vertical_active_line	= FRAME_HEIGHT; 
	
	params->dsi.horizontal_sync_active	= 2;//6;//10;//5
	params->dsi.horizontal_backporch	= 42;//80;//61; // 80; //40;//14
	params->dsi.horizontal_frontporch	= 44;//80;//66; // 80; //40;//14
	//params->dsi.horizontal_blanking_pixel			= 60;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;

	//params->dsi.compatibility_for_nvk = 0;
	params->dsi.PLL_CLOCK = 214;//340; //254;//234;//186;//197;//206; //226;//230;//250;//225;
	//params->dsi.noncont_clock = 1;
	params->dsi.ssc_disable = 1;
#if defined(CONFIG_TERACUBE_2E)  //yx 20161212
	params->dsi.noncont_clock = 1; 
     params->dsi.esd_check_enable =1;
     params->dsi.customization_esd_check_enable =1;
     params->dsi.lcm_esd_check_table[0].cmd =0x0A;
     params->dsi.lcm_esd_check_table[0].count =1;
     params->dsi.lcm_esd_check_table[0].para_list[0] =0x9C;
#endif

}


static void lcm_init(void)
{
	SET_RESET_PIN(1);
	MDELAY(5);
	SET_RESET_PIN(0);
	MDELAY(120);
	SET_RESET_PIN(1);
	MDELAY(50);//150

#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020	
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}


static void lcm_suspend(void)
{
#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#else
	push_table(NULL, lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#endif
}


static void lcm_resume(void)
{

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
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]= 0x00053902;
    data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
    data_array[2]= (y1_LSB);
    dsi_set_cmdq(data_array, 3, 1);
    
    data_array[0]= 0x002c3909;
    dsi_set_cmdq(data_array, 1, 0);

}
#endif

static unsigned int lcm_compare_id(void)
{
        unsigned int id=0;//,id2=0;
        unsigned char buffer[4]={0};//,buffer2[2];
        unsigned int array[2]={0};  

        SET_RESET_PIN(1);
        SET_RESET_PIN(0);
        MDELAY(25);
        SET_RESET_PIN(1);
        MDELAY(50);

        //array[0]=0x00043902;
        //array[1]=0x011980ff;
        //dsi_set_cmdq(array, 2, 1); //{0xff, 3 ,{0x80,0x19,0x01}}, // Command2 Enable
       
        //array[0]=0x80001500;
        //dsi_set_cmdq(array, 1, 1); //{0x00, 1 ,{0x80}},
       
        //array[0]=0x00033902;
        //array[1]=0x001980ff;
        //dsi_set_cmdq(array, 2, 1); //{0xff, 2 ,{0x80,0x19}}, // Orise Mode Enable
  
        array[0] = 0x00043700;// set return byte number
        dsi_set_cmdq(array, 1, 1);
        read_reg_v2(0xA1, buffer, 4); // Read Register 0xA1 : 0x80,0x09/0x01,0x8B (OTM8009A/OTM8018B);

        id = buffer[2]<<8 |buffer[3];
        return (id==LCM_ID_OTM1290A)?1:0;

}

LCM_DRIVER otm1290a_6735_dsi_lcm_drv_yk6xx = 
{
    .name	        = "otm1290a_67xx_dsi_video",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
#if (LCM_DSI_CMD_MODE)
    .set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
    .compare_id    = lcm_compare_id,
};
