/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "kd_imgsensor.h"

#include "regulator/regulator.h"
#include "gpio/gpio.h"
/*#include "mt6306/mt6306.h"*/
#include "mclk/mclk.h"



#include "imgsensor_cfg_table.h"

enum IMGSENSOR_RETURN
	(*hw_open[IMGSENSOR_HW_ID_MAX_NUM])(struct IMGSENSOR_HW_DEVICE **) = {
	imgsensor_hw_regulator_open,
	imgsensor_hw_gpio_open,
	/*imgsensor_hw_mt6306_open,*/
	imgsensor_hw_mclk_open
};
#if defined(YK751_CONFIG)||defined(YK685_CONFIG)||defined(YK179_CONFIG)
#define IMGSENSOR_HW_ID_AVDD_CUSTOM IMGSENSOR_HW_ID_REGULATOR
#else
#define IMGSENSOR_HW_ID_AVDD_CUSTOM IMGSENSOR_HW_ID_GPIO
#endif
struct IMGSENSOR_HW_CFG imgsensor_custom_config[] = {
	{
		IMGSENSOR_SENSOR_IDX_MAIN,
		IMGSENSOR_I2C_DEV_0,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			{IMGSENSOR_HW_ID_AVDD_CUSTOM, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
#if defined(YK682_CONFIG)
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
#else
			{IMGSENSOR_HW_ID_AVDD_CUSTOM, IMGSENSOR_HW_PIN_AVDD},
#endif
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN2,
		IMGSENSOR_I2C_DEV_2,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
#if defined(YK682_CONFIG)
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
#elif defined(YK179_CONFIG)
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
#elif defined(CONFIG_TERACUBE_2E) //xjl 20200423
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
#else
			{IMGSENSOR_HW_ID_AVDD_CUSTOM, IMGSENSOR_HW_PIN_AVDD},
#endif
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
#if defined(YK179_CONFIG)
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_DVDD},
#else
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
#endif
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_SUB2,
		IMGSENSOR_I2C_DEV_1,
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			#if defined(YK179_CONFIG)
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			#else
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			#endif
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},
	{
		IMGSENSOR_SENSOR_IDX_MAIN3,
#if defined(YK179_CONFIG)
		IMGSENSOR_I2C_DEV_0,
#else
		IMGSENSOR_I2C_DEV_1,//cjc yk682
#endif
		{
			{IMGSENSOR_HW_ID_MCLK, IMGSENSOR_HW_PIN_MCLK},
			#if defined(YK179_CONFIG)
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_DVDD},
			#else
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_AVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DOVDD},
			{IMGSENSOR_HW_ID_REGULATOR, IMGSENSOR_HW_PIN_DVDD},
			#endif
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_PDN},
			{IMGSENSOR_HW_ID_GPIO, IMGSENSOR_HW_PIN_RST},
			{IMGSENSOR_HW_ID_NONE, IMGSENSOR_HW_PIN_NONE},
		},
	},

	{IMGSENSOR_SENSOR_IDX_NONE}
};

struct IMGSENSOR_HW_POWER_SEQ platform_power_sequence[] = {
#ifdef MIPI_SWITCH
	{
		PLATFORM_POWER_SEQ_NAME,
		{
			{
				IMGSENSOR_HW_PIN_MIPI_SWITCH_EN,
				IMGSENSOR_HW_PIN_STATE_LEVEL_0,
				0,
				IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH,
				0
			},
			{
				IMGSENSOR_HW_PIN_MIPI_SWITCH_SEL,
				IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH,
				0,
				IMGSENSOR_HW_PIN_STATE_LEVEL_0,
				0
			},
		},
		IMGSENSOR_SENSOR_IDX_SUB,
	},
	{
		PLATFORM_POWER_SEQ_NAME,
		{
			{
				IMGSENSOR_HW_PIN_MIPI_SWITCH_EN,
				IMGSENSOR_HW_PIN_STATE_LEVEL_0,
				0,
				IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH,
				0
			},
			{
				IMGSENSOR_HW_PIN_MIPI_SWITCH_SEL,
				IMGSENSOR_HW_PIN_STATE_LEVEL_0,
				0,
				IMGSENSOR_HW_PIN_STATE_LEVEL_0,
				0
			},
		},
		IMGSENSOR_SENSOR_IDX_MAIN2,
	},
#endif

	{NULL}
};

/* Legacy design */
struct IMGSENSOR_HW_POWER_SEQ sensor_power_sequence[] = {
#if 0 //xjl 20180607
/*
#if defined(IMX398_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX398_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1100, 0},
			{AFVDD, Vol_2800, 0},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 1},
		},
	},
#endif
#if defined(OV23850_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV23850_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 2},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 5},
		},
	},
#endif
#if defined(IMX386_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX386_MIPI_RAW,
		{
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1100, 0},
			{DOVDD, Vol_1800, 0},
			{AFVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 1},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 5},
		},
	},
#endif
#if defined(IMX386_MIPI_MONO)
	{
		SENSOR_DRVNAME_IMX386_MIPI_MONO,
		{
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1100, 0},
			{DOVDD, Vol_1800, 0},
			{AFVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 1},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 5},
		},
	},
#endif

#if defined(IMX338_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX338_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2500, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1100, 0},
			{AFVDD, Vol_2800, 0},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 1}
		},
	},
#endif
#if defined(S5K4E6_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K4E6_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 1},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2900, 0},
			{DVDD, Vol_1200, 2},
			{AFVDD, Vol_2800, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K3P8SP_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3P8SP_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0},
		},
	},
#endif
#if defined(S5K2T7SP_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K2T7SP_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0},
		},
	},
#endif
#if defined(IMX230_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX230_MIPI_RAW,
		{
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{AVDD, Vol_2500, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1100, 0},
			{AFVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 10}
		},
	},
#endif
#if defined(S5K3M2_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3M2_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K3P3SX_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3P3SX_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K5E2YA_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K5E2YA_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K4ECGX_MIPI_YUV)
	{
		SENSOR_DRVNAME_S5K4ECGX_MIPI_YUV,
		{
			{DVDD, Vol_1200, 1},
			{AVDD, Vol_2800, 1},
			{DOVDD, Vol_1800, 1},
			{AFVDD, Vol_2800, 0},
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(OV16880_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV16880_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 1},
			{RST, Vol_High, 2}
		},
	},
#endif

#if defined(S5K2P7_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K2P7_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1000, 1},
			{DOVDD, Vol_1800, 1},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_Low, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0},
		},
	},
#endif
#if defined(S5K2P8_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K2P8_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(IMX258_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX258_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(IMX258_MIPI_MONO)
	{
		SENSOR_DRVNAME_IMX258_MIPI_MONO,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(IMX377_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX377_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(OV8858_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV8858_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 1},
			{RST, Vol_High, 2}
		},
	},
#endif
#if defined(OV8856_MIPI_RAW)
	{SENSOR_DRVNAME_OV8856_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 2},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 5},
		},
	},
#endif
#if defined(S5K2X8_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K2X8_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(IMX214_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX214_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 1}
		},
	},
#endif
#if defined(IMX214_MIPI_MONO)
	{
		SENSOR_DRVNAME_IMX214_MIPI_MONO,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 1}
		},
	},
#endif
#if defined(S5K3L8_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3L8_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(IMX362_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX362_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 10},
			{DOVDD, Vol_1800, 10},
			{DVDD, Vol_1200, 10},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K2L7_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K2L7_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 3},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(IMX318_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX318_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 10},
			{DOVDD, Vol_1800, 10},
			{DVDD, Vol_1200, 10},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(OV8865_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV8865_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 5},
			{RST, Vol_Low, 5},
			{DOVDD, Vol_1800, 5},
			{AVDD, Vol_2800, 5},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_High, 5},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(IMX219_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX219_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 10},
			{DOVDD, Vol_1800, 10},
			{DVDD, Vol_1000, 10},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K3M3_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3M3_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 2}
		},
	},
#endif
#if defined(OV5670_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV5670_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 5},
			{RST, Vol_Low, 5},
			{DOVDD, Vol_1800, 5},
			{AVDD, Vol_2800, 5},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_High, 5},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(OV5670_MIPI_RAW_2)
	{
		SENSOR_DRVNAME_OV5670_MIPI_RAW_2,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 5},
			{RST, Vol_Low, 5},
			{DOVDD, Vol_1800, 5},
			{AVDD, Vol_2800, 5},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_High, 5},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(OV20880_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV20880_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{RST, Vol_Low, 1},
			{AVDD, Vol_2800, 1},
			{DOVDD, Vol_1800, 1},
			{DVDD, Vol_1100, 1},
			{RST, Vol_High, 5}
		},
	},
#endif
*/
#endif
//added by xen 20170919-begin
//OV:
#if defined(OV16885_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV16885_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 1},
			{RST, Vol_High, 2}
		},
	},
#endif
#if defined(OV13855_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV13855_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(OV9760_MIPI_RAW)
	{
		SENSOR_DRVNAME_OV9760_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1500, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(OV8825_MIPI_RAW)
	{SENSOR_DRVNAME_OV8825_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 1},
			{AVDD, Vol_2800, 1},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 1},
			{RST, Vol_High, 2}
		},
	},
#endif
#if defined(OV8856_MIPI_RAW)
	{SENSOR_DRVNAME_OV8856_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 2},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 5},
		},
	},
#endif
#if defined(OV8865_MIPI_RAW)
	{SENSOR_DRVNAME_OV8865_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 2},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 5},
		},
	},
#endif

/*GC*/
#if defined(GC13023_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC13023_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC8024_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC8024_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC8024F_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC8024F_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC8034_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC8034_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC5035_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC5035_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC8054_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC8054_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC5025_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC5025_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC5025F_MIPI_RAW) //xen 20171221
		{
			SENSOR_DRVNAME_GC5025F_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC5025FSEC_MIPI_RAW) //xen 20171221
		{
			SENSOR_DRVNAME_GC5025FSEC_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(OV4689_MIPI_RAW)
		{
			SENSOR_DRVNAME_OV4689_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif

#if defined(GC2385_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC2385_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC2375H_MIPI_RAW) 
		{
			SENSOR_DRVNAME_GC2375H_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC02M1_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC02M1_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(OV4689_MIPI_RAW)
		{
			SENSOR_DRVNAME_OV4689_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif

#if defined(GC2385F_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC2385F_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC2145_MIPI_YUV)
		{
			SENSOR_DRVNAME_GC2145_MIPI_YUV,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1500, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC2145_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC2145_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC2146_MIPI_YUV)
		{
			SENSOR_DRVNAME_GC2146_MIPI_YUV,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1500, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC0310_MIPI_YUV)
		{
			SENSOR_DRVNAME_GC0310_MIPI_YUV,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1500, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC0310_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC0310_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1500, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC030A_MIPI_RAW)
		{
			SENSOR_DRVNAME_GC030A_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1500, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(GC033A_MIPI_RAW) //this chip has no RST pin
		{
			SENSOR_DRVNAME_GC033A_MIPI_RAW,
			{
				//{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 5},
				//{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1500, 5},
				{AVDD, Vol_2800, 5},
				{SensorMCLK, Vol_High, 5},
				{PDN, Vol_Low, 5},
				//{RST, Vol_High, 5},
				{PDN, Vol_High, 5},
				{PDN, Vol_Low, 5}
			},
		},
#endif

/*A*/
#if defined(A5142_MIPI_RAW) //xen 20171113
		{
			SENSOR_DRVNAME_A5142_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif

/*AR*/
#if defined(AR1335_MIPI_RAW) //xen 20171219
		{
			SENSOR_DRVNAME_AR1335_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif

/*HI*/
#if defined(HI1336_MIPI_RAW)
	{
		SENSOR_DRVNAME_HI1336_MIPI_RAW,
		{
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{AVDD, Vol_2800, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1100, 0},
			{AFVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 10}
		},
	},
#endif
#if defined(HI1336F_MIPI_RAW)
	{
		SENSOR_DRVNAME_HI1336F_MIPI_RAW,
		{
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{AVDD, Vol_2800, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1200, 0},
			//{AFVDD, Vol_2800, 1},
			{SensorMCLK, Vol_High, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 10}
		},
	},
#endif
#if defined(HI846_MIPI_RAW) //xen 20171023, same with SP8409
		{
			SENSOR_DRVNAME_HI846_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(HI846_2LANE_MIPI_RAW) 
		{
			SENSOR_DRVNAME_HI846_2LANE_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(HI843B_MIPI_RAW) //zwl 20180306
		{
			SENSOR_DRVNAME_HI843B_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif

#if defined(HI556_MIPI_RAW)  //same with SP5509
		{
			SENSOR_DRVNAME_HI556_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(HI556F_MIPI_RAW)  //same with SP5509,front camera
		{
			SENSOR_DRVNAME_HI556F_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(HI544_MIPI_RAW)  //same with SP5509
		{
			SENSOR_DRVNAME_HI544_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{DVDD, Vol_1200, 5},
				{AVDD, Vol_2800, 5},
				{AFVDD, Vol_2800, 1},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
//Samsung:
#if defined(S5K3P3SX_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3P3SX_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_Low, 4},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 1},
			{RST, Vol_High, 0}
		},
	},
#endif
#if defined(S5K3P9_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K3P9_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 4},
			{RST, Vol_Low, 1},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1000, 0},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0},
		},
	},
#endif
#if defined(S5K3L8_MIPI_RAW)
		{
			SENSOR_DRVNAME_S5K3L8_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1200, 5},
				{AFVDD, Vol_2800, 1},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(S5K3L6_MIPI_RAW)
		{
			SENSOR_DRVNAME_S5K3L6_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1200, 5},
				{AFVDD, Vol_2800, 1},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(S5K3L6SEC_MIPI_RAW)
		{
			SENSOR_DRVNAME_S5K3L6SEC_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1200, 5},
				{AFVDD, Vol_2800, 1},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(S5K4H7_MIPI_RAW)
	{
		SENSOR_DRVNAME_S5K4H7_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 0}
		},
	},
#endif
//IMX
#if defined(IMX135_MIPI_RAW)
		{
			SENSOR_DRVNAME_IMX135_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1200, 5},
				{AFVDD, Vol_2800, 1},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(IMX134_MIPI_RAW)
		{
			SENSOR_DRVNAME_IMX134_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1200, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(IMX134_2LANE_MIPI_RAW)
		{
			SENSOR_DRVNAME_IMX134_2LANE_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1200, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(IMX214_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX214_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 5},
			{RST, Vol_Low, 5},
			{AVDD, Vol_2800, 5},
			{DOVDD, Vol_1800, 5},
			{DVDD, Vol_1200, 5},
			{AFVDD, Vol_2800, 5},
			{PDN, Vol_High, 5},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(IMX286_MIPI_RAW)
	{
		SENSOR_DRVNAME_IMX286_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1100, 0},
			{AFVDD, Vol_2800, 0},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 1},
		},
	},
#endif
#if defined(IMX286_MIPI_MONO)
	{
		SENSOR_DRVNAME_IMX286_MIPI_MONO,
		{
			{SensorMCLK, Vol_High, 0},
			{AVDD, Vol_2800, 0},
			{DOVDD, Vol_1800, 0},
			{DVDD, Vol_1100, 0},
			{AFVDD, Vol_2800, 0},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 1},
		},
	},
#endif
#if defined(SP2609F_MIPI_RAW)
		{
			SENSOR_DRVNAME_SP2609F_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1500, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(SP5508_MIPI_RAW)
		{
			SENSOR_DRVNAME_SP5508_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1200, 5},
				{AFVDD, Vol_2800, 1},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(SP250A_MIPI_RAW)
		{
			SENSOR_DRVNAME_SP250AMIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1500, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(SP250AF_MIPI_RAW)
		{
			SENSOR_DRVNAME_SP250AF_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1500, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(SP2506_MIPI_RAW)
		{
			SENSOR_DRVNAME_SP2506_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_Low, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1500, 5},
				{PDN, Vol_High, 5},
				{RST, Vol_High, 5}
			},
		},
#endif
#if defined(SP0A08_MIPI_RAW)
		{
			SENSOR_DRVNAME_SP0A08_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1500, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif

#if defined(SP0821_MIPI_RAW)
		{
			SENSOR_DRVNAME_SP0821_MIPI_RAW,
			{
				{SensorMCLK, Vol_High, 0},
				{PDN, Vol_High, 1},
				{RST, Vol_Low, 10},
				{DOVDD, Vol_1800, 5},
				{AVDD, Vol_2800, 5},
				{DVDD, Vol_1500, 5},
				{PDN, Vol_Low, 5},
				{RST, Vol_High, 5}
			},
		},
#endif

//added by xen 20170919-end
#if 0 //modified by xen 20171113
#if defined(GC2365_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC2365_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_High, 1},
			{RST, Vol_Low, 10},
			{DOVDD, Vol_1800, 5},
			{DVDD, Vol_1200, 5},
			{AVDD, Vol_2800, 5},
			{PDN, Vol_Low, 5},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(GC2366_MIPI_RAW)
	{
		SENSOR_DRVNAME_GC2366_MIPI_RAW,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_High, 1},
			{RST, Vol_Low, 10},
			{DOVDD, Vol_1800, 5},
			{DVDD, Vol_1200, 5},
			{AVDD, Vol_2800, 5},
			{PDN, Vol_Low, 5},
			{RST, Vol_High, 5}
		},
	},
#endif
	/*Test*/
#if defined(OV13870_MIPI_RAW_5MP)
	{
		SENSOR_DRVNAME_OV13870_MIPI_RAW_5MP,
		{
			{SensorMCLK, Vol_High, 0},
			{PDN, Vol_Low, 0},
			{RST, Vol_Low, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 1},
			{PDN, Vol_High, 0},
			{RST, Vol_High, 5}
		},
	},
#endif
#if defined(OV8856_MIPI_RAW_5MP)
	{SENSOR_DRVNAME_OV8856_MIPI_RAW_5MP,
		{
			{SensorMCLK, Vol_High, 0},
			{DOVDD, Vol_1800, 0},
			{AVDD, Vol_2800, 0},
			{DVDD, Vol_1200, 0},
			{AFVDD, Vol_2800, 2},
			{PDN, Vol_Low, 0},
			{PDN, Vol_High, 0},
			{RST, Vol_Low, 0},
			{RST, Vol_High, 5},
		},
	},
#endif
#endif
	/* add new sensor before this line */
	{NULL,},
};

