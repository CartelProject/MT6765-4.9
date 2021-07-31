/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#ifndef __MTK_CHARGER_INIT_H__
#define __MTK_CHARGER_INIT_H__

#if defined(CONFIG_TERACUBE_2E)
#define BATTERY_CV 4440000
#else
#define BATTERY_CV 4350000 // For high battery 4.4V, use 4200000 otherwise
#endif

#if defined(CONFIG_TERACUBE_2E)
#define V_CHARGER_MAX 11000000 /* 11 V */
#else
#define V_CHARGER_MAX 6500000 /* 6.5 V */
#endif
#define V_CHARGER_MIN 4450000 /* 4.6 V */

#define USB_CHARGER_CURRENT_SUSPEND		0 /* def CONFIG_USB_IF */
#define USB_CHARGER_CURRENT_UNCONFIGURED	70000 /* 70mA */
#define USB_CHARGER_CURRENT_CONFIGURED		500000 /* 500mA */
#ifdef CONFIG_MTK_FAN5405_SUPPORT
#define USB_CHARGER_CURRENT	650000	/*500mA*/
#else
#define USB_CHARGER_CURRENT			500000 /* 500mA */
#endif
/***********************************************************************************
//note by xen 20170817
//if use PSC5425A charger IC,actually current value is:
615mA,821mA,1027mA,1230mA,1436mA,1642mA,1848mA,2052mA  (Rsense=0.033)
//if use PSC5425 charger IC,actually current value is:
600mA,1000mA,1300mA,1600mA,1900mA,2200mA,2410mA,NG  (Rsense=0.033)
495mA,925mA, 1073mA,1320mA,1565mA,1815mA,1980mA,2310  (Rsense=0.040)
 0     1       2     3       4      5      6     7
550mA,650mA, 750mA, 850mA ,1050mA,1150mA,1350mA,1450mA (FAN5405/FAN54015)
***********************************************************************************/
#if defined(YK_AC_CHARGER_CURRENT_1450MA)                      //zwl 20180329
#define AC_CHARGER_CURRENT					1450000   
#elif defined(YK_AC_CHARGER_CURRENT_1350MA)
#define AC_CHARGER_CURRENT					1350000 
#elif defined(YK_AC_CHARGER_CURRENT_1150MA)
#define AC_CHARGER_CURRENT					1150000 
#elif defined(YK670_CONFIG) //zxs 20180807 
#define AC_CHARGER_CURRENT    700000
#elif defined(YK688_CUSTOMER_SK_P6_HDPLUS) //xjl 20190619
#define AC_CHARGER_CURRENT			1550000
#elif defined(CONFIG_TERACUBE_2E)
#define AC_CHARGER_CURRENT			1850000 //2250000
#else
#define AC_CHARGER_CURRENT			2050000
#endif
#define AC_CHARGER_INPUT_CURRENT		3200000
#if defined(YK688_CUSTOMER_SK_P6_HDPLUS) //xjl 20190428
#define NON_STD_AC_CHARGER_CURRENT		600000
#elif defined(YK686_CUSTOMER_YINMAI_S64390_HDPLUS)
#define NON_STD_AC_CHARGER_CURRENT		2050000
#elif defined(CONFIG_TERACUBE_2E)
#define NON_STD_AC_CHARGER_CURRENT		200000 //2050000
#else
#define NON_STD_AC_CHARGER_CURRENT		500000
#endif
#define CHARGING_HOST_CHARGER_CURRENT		650000
#define APPLE_1_0A_CHARGER_CURRENT		650000
#define APPLE_2_1A_CHARGER_CURRENT		800000
#define TA_AC_CHARGING_CURRENT	3000000

/* dynamic mivr */
#define V_CHARGER_MIN_1 4400000 /* 4.4 V */
#define V_CHARGER_MIN_2 4200000 /* 4.2 V */
#define MAX_DMIVR_CHARGER_CURRENT 1400000 /* 1.4 A */

/* sw jeita */
#define JEITA_TEMP_ABOVE_T4_CV	4240000
#define JEITA_TEMP_T3_TO_T4_CV	4240000
#define JEITA_TEMP_T2_TO_T3_CV	4340000
#define JEITA_TEMP_T1_TO_T2_CV	4240000
#define JEITA_TEMP_T0_TO_T1_CV	4040000
#define JEITA_TEMP_BELOW_T0_CV	4040000
#define TEMP_T4_THRES  50
#define TEMP_T4_THRES_MINUS_X_DEGREE 47
#define TEMP_T3_THRES  45
#define TEMP_T3_THRES_MINUS_X_DEGREE 39
#define TEMP_T2_THRES  10
#define TEMP_T2_THRES_PLUS_X_DEGREE 16
#define TEMP_T1_THRES  0
#define TEMP_T1_THRES_PLUS_X_DEGREE 6
#define TEMP_T0_THRES  0
#define TEMP_T0_THRES_PLUS_X_DEGREE  0
#define TEMP_NEG_10_THRES 0

/* Battery Temperature Protection */
#if defined(HX_HIGH_TEMPERATURE_TEST)
#define MIN_CHARGE_TEMP  -40
#define MIN_CHARGE_TEMP_PLUS_X_DEGREE	-35
#define MAX_CHARGE_TEMP  90
#define MAX_CHARGE_TEMP_MINUS_X_DEGREE	88
#elif defined(YK686_CUSTOMER_YINMAI_S64390_HDPLUS)
#define MIN_CHARGE_TEMP  0
#define MIN_CHARGE_TEMP_PLUS_X_DEGREE	6
#define MAX_CHARGE_TEMP  55
#define MAX_CHARGE_TEMP_MINUS_X_DEGREE	50
#elif defined(TPLINK_CHARGING_FLOW)
#define MIN_CHARGE_TEMP  0 
#define MIN_CHARGE_TEMP_PLUS_X_DEGREE	2
#define MAX_CHARGE_TEMP  55
#define MAX_CHARGE_TEMP_MINUS_X_DEGREE	50
#elif defined(CONFIG_TERACUBE_2E)
#define MIN_CHARGE_TEMP  0
#define MIN_CHARGE_TEMP_PLUS_X_DEGREE	5
#define MAX_CHARGE_TEMP  55
#define MAX_CHARGE_TEMP_MINUS_X_DEGREE	50
#else
#define MIN_CHARGE_TEMP  0
#define MIN_CHARGE_TEMP_PLUS_X_DEGREE	6
#define MAX_CHARGE_TEMP  50
#define MAX_CHARGE_TEMP_MINUS_X_DEGREE	47
#endif

/* pe */
#define PE_ICHG_LEAVE_THRESHOLD 1000000 /* uA */
#define TA_AC_12V_INPUT_CURRENT 3200000
#define TA_AC_9V_INPUT_CURRENT	3200000
#define TA_AC_7V_INPUT_CURRENT	3200000
#define TA_9V_SUPPORT
#define TA_12V_SUPPORT

/* pe2.0 */
#define PE20_ICHG_LEAVE_THRESHOLD 1000000 /* uA */
#define TA_START_BATTERY_SOC	0
#define TA_STOP_BATTERY_SOC	85

/* dual charger */
#define TA_AC_MASTER_CHARGING_CURRENT 1500000
#define TA_AC_SLAVE_CHARGING_CURRENT 1500000
#define SLAVE_MIVR_DIFF 100000

/* slave charger */
#define CHG2_EFF 90

/* cable measurement impedance */
#define CABLE_IMP_THRESHOLD 699
#define VBAT_CABLE_IMP_THRESHOLD 3900000 /* uV */

#if defined(CONFIG_TERACUBE_2E)
#define HIGH_BATTERY_VOLTAGE_SUPPORT
#endif

/* bif */
#if defined(HIGH_BATTERY_VOLTAGE_SUPPORT)
 #if defined(CONFIG_TERACUBE_2E)
  #define BIF_THRESHOLD1 4390000	/* UV */
  #define BIF_THRESHOLD2 4410000	/* UV */
 #else
  #define BIF_THRESHOLD1 4280000 //4250000	/* UV *///xen 20180106
  #define BIF_THRESHOLD2 4320000 //4300000	/* UV *///xen 20180106
 #endif
#else
 #define BIF_THRESHOLD1 4100000	/* UV */
 #define BIF_THRESHOLD2 4120000	/* UV */
#endif
#define BIF_CV_UNDER_THRESHOLD2 4450000	/* UV */
#define BIF_CV BATTERY_CV /* UV */

#if defined(PSC5425_CHARGER_SUPPORT) //modified by zwl for PSC5425 
#define R_SENSE 33
#elif defined(YK670_CONFIG) //zxs 20180807
#define R_SENSE 56 /* mohm */
#elif defined(CONFIG_TERACUBE_2E)
#define R_SENSE 47 /* mohm */
#else
#define R_SENSE 68 //56 /* mohm */ //modified by xen according hardware situation 20180108 
#endif

#define MAX_CHARGING_TIME (12 * 60 * 60) /* 12 hours */

#define DEFAULT_BC12_CHARGER 0 /* MAIN_CHARGER */

/* battery warning */
#define BATTERY_NOTIFY_CASE_0001_VCHARGER
#define BATTERY_NOTIFY_CASE_0002_VBATTEMP

/* pe4 */
#define PE40_MAX_VBUS 11000
#define PE40_MAX_IBUS 3000
#define HIGH_TEMP_TO_LEAVE_PE40 46
#define HIGH_TEMP_TO_ENTER_PE40 39
#define LOW_TEMP_TO_LEAVE_PE40 10
#define LOW_TEMP_TO_ENTER_PE40 16

/* pd */
#define PD_VBUS_UPPER_BOUND 10000000	/* uv */
#define PD_VBUS_LOW_BOUND 5000000	/* uv */
#define PD_ICHG_LEAVE_THRESHOLD 1000000 /* uA */
#define PD_STOP_BATTERY_SOC 80

#define VSYS_WATT 5000000
#define IBUS_ERR 14

#endif /*__MTK_CHARGER_INIT_H__*/
