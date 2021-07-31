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
#endif

#define LCM_ID_ILI7807G (0x7807)

#define LCM_DBG(fmt, arg...)
	//LCM_PRINT ("[LCM-ILI9881-DSI-VDO] %s (line:%d) :" fmt "\r\n", __func__, __LINE__, ## arg)

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define LCM_DSI_CMD_MODE									0

#define FRAME_WIDTH  										(1080)
#define FRAME_HEIGHT 										(2340)

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

#if defined (LCM_ROTATE_180)
	static unsigned char BufLcmInfo[] = "LCD_ILI7807G_63_LUBIKAN_SHARP_20191207_ROTATE_180";
#else
	static unsigned char BufLcmInfo[] = "LCD_ILI7807G_63_LUBIKAN_SHARP_20191207";
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {
{0xFF,3,{0x78,0x07,0x01}},
{0x00,1,{0x41}},
{0x01,1,{0x50}},
{0x02,1,{0x08}},
{0x03,1,{0x51}},
{0x08,1,{0x81}},
{0x09,1,{0x02}},
{0x0a,1,{0x30 }},
{0x0c,1,{0x08   }},
{0x0e,1,{0x51}},
{0x31,1,{0x07   }},
{0x32,1,{0x07   }},
{0x33,1,{0x07   }},
{0x34,1,{0x07   }},
{0x35,1,{0x07   }},
{0x36,1,{0x02   }},
{0x37,1,{0x30   }},
{0x38,1,{0x2F   }},
{0x39,1,{0x2E   }},
{0x3a,1,{0x36   }},
{0x3b,1,{0x35   }},
{0x3c,1,{0x34   }},
{0x3d,1,{0x07   }},
{0x3e,1,{0x07   }},
{0x3f,1,{0x07   }},
{0x40,1,{0x2C   }},
{0x41,1,{0x28   }},
{0x42,1,{0x13   }},
{0x43,1,{0x11   }},
{0x44,1,{0x07   }},
{0x45,1,{0x07   }},
{0x46,1,{0x07   }},
{0x47,1,{0x07   }},
{0x48,1,{0x09   }},
{0x49,1,{0x07   }},
{0x4a,1,{0x07   }},
{0x4b,1,{0x07   }},
{0x4c,1,{0x07   }},
{0x4d,1,{0x07   }},
{0x4e,1,{0x02   }},
{0x4f,1,{0x30   }},
{0x50,1,{0x2F   }},
{0x51,1,{0x2E   }},
{0x52,1,{0x36   }},
{0x53,1,{0x35   }},
{0x54,1,{0x34   }},
{0x55,1,{0x07   }},
{0x56,1,{0x07   }},
{0x57,1,{0x07   }},
{0x58,1,{0x2C   }},
{0x59,1,{0x28   }},
{0x5a,1,{0x12   }},
{0x5b,1,{0x10   }},
{0x5c,1,{0x07   }},
{0x5d,1,{0x07   }},
{0x5e,1,{0x07   }},
{0x5f,1,{0x07   }},
{0x60,1,{0x08   }},
{0x61,1,{0x07   }},
{0x62,1,{0x07   }},
{0x63,1,{0x07   }},
{0x64,1,{0x07   }},
{0x65,1,{0x07   }},
{0x66,1,{0x02   }},
{0x67,1,{0x30   }},
{0x68,1,{0x2F   }},
{0x69,1,{0x2E   }},
{0x6a,1,{0x36   }},
{0x6b,1,{0x35   }},
{0x6c,1,{0x34   }},
{0x6d,1,{0x07   }},
{0x6e,1,{0x07   }},
{0x6f,1,{0x07   }},
{0x70,1,{0x2C   }},
{0x71,1,{0x28   }},
{0x72,1,{0x10   }},
{0x73,1,{0x12   }},
{0x74,1,{0x07   }},
{0x75,1,{0x07   }},
{0x76,1,{0x07   }},
{0x77,1,{0x07   }},
{0x78,1,{0x08   }},
{0x79,1,{0x07   }},
{0x7a,1,{0x07   }},
{0x7b,1,{0x07   }},
{0x7c,1,{0x07   }},
{0x7d,1,{0x07   }},
{0x7e,1,{0x02   }},
{0x7f,1,{0x30   }},
{0x80,1,{0x2F   }},
{0x81,1,{0x2E   }},
{0x82,1,{0x36   }},
{0x83,1,{0x35   }},
{0x84,1,{0x34   }},
{0x85,1,{0x07   }},
{0x86,1,{0x07   }},
{0x87,1,{0x07   }},
{0x88,1,{0x2C   }},
{0x89,1,{0x28   }},
{0x8a,1,{0x11   }},
{0x8b,1,{0x13   }},
{0x8c,1,{0x07   }},
{0x8d,1,{0x07   }},
{0x8e,1,{0x07   }},
{0x8f,1,{0x07   }},
{0x90,1,{0x09   }},
{0xA7,1,{0x10}},
{0xB2,1,{0x00}},
{0xD1,1,{0x12}},
{0xD3,1,{0x40}},
{0xD4,1,{0x04}},
{0xD8,1,{0x64}},
{0xe6,1,{0x22}},
{0xFF,3,{0x78,0x07,0x02		}},
{0x01,1,{0x35               }},
{0x06,1,{0x70   		}},
{0x07,1,{0x70  		 }},
{0x78,1,{0x26   		}},
{0x79,1,{0x26   		}},
{0x40,1,{0x08   		}},
{0x41,1,{0x00}},
{0x42,1,{0x04   		}},
{0x43,1,{0x17   		}},
{0x47,1,{0x00   		}},
{0x53,1,{0x04   		}},
{0x5F,1,{0x06   		}},
{0xFF,3,{0x78,0x07,0x05		}},
{0x27,1,{0x44}},
{0x28,1,{0x54}},
{0x2B,1,{0x08}},
{0xA0,1,{0x44}},
{0xA2,1,{0x54}},
{0xBC,1,{0x54   }},
{0x03,1,{0x00  }},
{0x04,1,{0x8B  }},
{0x63,1,{0x79  }},
{0x64,1,{0x79  }},
{0x68,1,{0x70  }},
{0x69,1,{0x89  }},
{0x6A,1,{0x40  }},
{0x6B,1,{0x50  }},
{0xFF,3,{0x78,0x07,0x06		}},
{0xD6,1,{0x67 	}},
{0x2E,1,{0x01 	}},
{0xC0,1,{0x91 	}},
{0xC1,1,{0x04 	}},
{0xC3,1,{0x06 	}},
{0x11,1,{0x03 	}},
{0x12,1,{0x10}},
{0x13,1,{0x54 	}},
{0x14,1,{0x41	}},
{0x15,1,{0x00 	}},
{0x16,1,{0x41}},
{0x17,1,{0x47 	}},
{0x18,1,{0x3A 	}},
{0xB4,1,{0xCB      }},
{0xB5,1,{0x15      }},
{0xFF,3,{0x78,0x07,0x07		}},
{0x06,1,{0x90}},
{0xFF,3,{0x78,0x07,0x08}},
{0xE0,40,{0x00,0x00,0x1C,0x49,0x00,0x6B,0x88,0xA0,0x00,0xB5,0xC8,0xD9,0x15,0x0F,0x39,0x78,0x25,0xA7,0xF0,0x2B,0x2A,0x2C,0x65,0xA6,0x3E,0xCE,0x02,0x23,0x3F,0x4F,0x5C,0x6A,0x3F,0x7A,0x8D,0xA2,0x3F,0xBD,0xDF,0xE6}},
{0xE1,40,{0x00,0x00,0x1C,0x49,0x00,0x6B,0x88,0xA0,0x00,0xB5,0xC8,0xD9,0x15,0x0F,0x39,0x78,0x25,0xA7,0xF0,0x2B,0x2A,0x2C,0x65,0xA6,0x3E,0xCE,0x02,0x23,0x3F,0x4F,0x5C,0x6A,0x3F,0x7A,0x8D,0xA2,0x3F,0xBD,0xDF,0xE6}},
{0xFF,3,{0x78,0x07,0x09}},
{0xE0,40,{0x00,0x00,0x1E,0x4D,0x00,0x70,0x8D,0xA5,0x00,0xBB,0xCD,0xDE,0x15,0x14,0x3E,0x7C,0x25,0xAB,0xF4,0x2D,0x2A,0x2F,0x67,0xA7,0x3E,0xCF,0x03,0x25,0x3F,0x50,0x5D,0x6C,0x3F,0x7D,0x8F,0xA5,0x3F,0xC0,0xE1,0xE6}},
{0xE1,40,{0x00,0x00,0x1E,0x4D,0x00,0x70,0x8D,0xA5,0x00,0xBB,0xCD,0xDE,0x15,0x14,0x3E,0x7C,0x25,0xAB,0xF4,0x2D,0x2A,0x2F,0x67,0xA7,0x3E,0xCF,0x03,0x25,0x3F,0x50,0x5D,0x6C,0x3F,0x7D,0x8F,0xA5,0x3F,0xC0,0xE1,0xE6}},
{0xFF,3,{0x78,0x07,0x0A}},
{0xE0,40,{0x00,0x00,0x25,0x59,0x00,0x7D,0x9A,0xB2,0x00,0xC7,0xDA,0xEA,0x15,0x1E,0x47,0x83,0x25,0xB1,0xF7,0x31,0x2A,0x33,0x69,0xA8,0x3E,0xD0,0x04,0x26,0x3F,0x51,0x5E,0x6D,0x3F,0x7D,0x90,0xA5,0x3F,0xC0,0xDF,0xE6}},
{0xE1,40,{0x00,0x00,0x25,0x59,0x00,0x7D,0x9A,0xB2,0x00,0xC7,0xDA,0xEA,0x15,0x1E,0x47,0x83,0x25,0xB1,0xF7,0x31,0x2A,0x33,0x69,0xA8,0x3E,0xD0,0x04,0x26,0x3F,0x51,0x5E,0x6D,0x3F,0x7D,0x90,0xA5,0x3F,0xC0,0xDF,0xE6}},
{0xFF,3,{0x78,0x07,0x0E		}},
{0x00,1,{0xA0}},
{0x00,1,{0xA3   }},
{0x4D,1,{0x5F   }},
{0x41,1,{0x08   }},
{0x43,1,{0xB3   }},
{0x47,1,{0x80}},
{0x49,1,{0xC3 }},
{0xB0,1,{0x31 }},
{0xB1,1,{0x60 }},
{0xB2,1,{0x60 }},
{0xB3,1,{0x00}},
{0xB4,1,{0x33}},
{0x45,1,{0x0A }},
{0x46,1,{0x61}},
{0xBC,1,{0x04}},
{0xBD,1,{0xFC}},
{0xC0,1,{0x34 }},
{0xC6,1,{0x60}},
{0xC7,1,{0x60}},
{0xC8,1,{0x60}},
{0xC9,1,{0x60}},
{0xe0,1,{0x08 }},
{0xe1,1,{0x00}},
{0xe2,1,{0x04}},
{0xe3,1,{0x17}},
{0xe4,1,{0x04}},
{0xe5,1,{0x04}},
{0xe6,1,{0x00}},
{0xFF,3,{0x78,0x07,0x0E		}},
{0x07,1,{0x21	        }},
{0x4B,1,{0x14	        }},
{0xFF,3,{0x78,0x07,0x0C	}},
{0x00,1,{0x19}},
{0x01,1,{0x2A}},
{0x02,1,{0x17}},
{0x03,1,{0x1C}},
{0x04,1,{0x17}},
{0x05,1,{0x19}},
{0x06,1,{0x17}},
{0x07,1,{0x16}},
{0x08,1,{0x18}},
{0x09,1,{0x1E}},
{0x0A,1,{0x19}},
{0x0B,1,{0x29}},
{0x0C,1,{0x17}},
{0x0D,1,{0x18}},
{0x0E,1,{0x19}},
{0x0F,1,{0x27}},
{0x10,1,{0x18}},
{0x11,1,{0x1D}},
{0x12,1,{0x18}},
{0x13,1,{0x20}},
{0x14,1,{0x19}},
{0x15,1,{0x25}},
{0x16,1,{0x17}},
{0x17,1,{0x1A}},
{0x18,1,{0x19}},
{0x19,1,{0x28}},
{0x1A,1,{0x17}},
{0x1B,1,{0x1B}},
{0x1C,1,{0x18}},
{0x1D,1,{0x21}},
{0x1E,1,{0x19}},
{0x1F,1,{0x26}},
{0x20,1,{0x18}},
{0x21,1,{0x24}},
{0x22,1,{0x18}},
{0x23,1,{0x1F}},
{0x24,1,{0x17}},
{0x25,1,{0x17}},
{0x26,1,{0x18}},
{0x27,1,{0x23}},
{0x28,1,{0x18}},
{0x29,1,{0x22}},
//{0xFF,3,{0x78,0x07,0x02	  }},
//{0x36,1,{0x01		}},
{0xFF,3,{0x78,0x07,0x00	  }},
    
{0x11,0x01,{0x00}},
{REGFLAG_DELAY, 120, {}}, 

{0x29,0x01,{0x00}}, 
{REGFLAG_DELAY, 100, {}},   
{REGFLAG_END_OF_TABLE, 0x00, {}}
};	




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


		params->dsi.vertical_sync_active				= 2;	//2;
		params->dsi.vertical_backporch					= 6;	//14;
		params->dsi.vertical_frontporch					= 38;	//16;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 
		
		params->dsi.horizontal_sync_active				= 5;	//2;
		params->dsi.horizontal_backporch				= 53;	//60;	//42;
		params->dsi.horizontal_frontporch				= 53;	//60;	//44;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

		params->dsi.PLL_CLOCK = 550;//208;	


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

	return 1;
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
#ifndef BUILD_LK
extern void tpd_suspend_before_lcm(void);
extern int set_gpio_lcm_enn_enp(int enable);
#endif
static void lcm_suspend(void)
{
	LCM_DBG("lcm_suspend");

#ifdef BUILD_LK //added by xen for softlink lk&kernel lcm driver 20171020
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#else
	tpd_suspend_before_lcm();
	push_table(NULL, lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
#endif
	//SET_RESET_PIN(0);
    //MDELAY(2);
   	//SET_RESET_PIN(1); 
    
#ifdef CONFIG_MTK_LCM_5V_IC
#ifndef BUILD_LK
	MDELAY(10);
	set_gpio_lcm_enn_enp(0);
#endif
#endif
}


static void lcm_resume(void)
{
	LCM_DBG("lcm_resume");
	lcm_init();
	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}



// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
#ifdef BUILD_LK //xjl 20180531
LCM_DRIVER ili7807g_6735_dsi_lcm_drv_yk6xx = 
#else
struct LCM_DRIVER ili7807g_6735_dsi_lcm_drv_yk6xx = 
#endif
{
    .name		    = "ili7807g_67xx_dsi_video", 
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,

};
