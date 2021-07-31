/*
 * Copyright (C) 2017 MediaTek Inc.
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
#include "imgsensor_cfg_table.h"
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/init.h>
#include <linux/types.h>

#undef CONFIG_MTK_SMI_EXT
#ifdef CONFIG_MTK_SMI_EXT
#include "mmdvfs_mgr.h"
#endif
#ifdef CONFIG_OF
/* device tree */
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#endif

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#endif

#ifdef CONFIG_MTK_CCU
#include "ccu_inc.h"
#endif

#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"
#include "kd_imgsensor_errcode.h"


#include "imgsensor_sensor_list.h"
#include "imgsensor_hw.h"
#include "imgsensor_i2c.h"
#include "imgsensor_proc.h"
#include "imgsensor_clk.h"
#include "imgsensor.h"

#define PDAF_DATA_SIZE 4096

#ifdef CONFIG_MTK_SMI_EXT
static int current_mmsys_clk = MMSYS_CLK_MEDIUM;
#endif

/* Test Only!! Open this define for temperature meter UT */
/* Temperature workqueue */
/* #define CONFIG_CAM_TEMPERATURE_WORKQUEUE */
#ifdef CONFIG_CAM_TEMPERATURE_WORKQUEUE
static void cam_temperature_report_wq_routine(struct work_struct *);
	struct delayed_work cam_temperature_wq;
#endif

#define FEATURE_CONTROL_MAX_DATA_SIZE 128000

struct platform_device *gpimgsensor_hw_platform_device;
struct device *gimgsensor_device;
/* 81 is used for V4L driver */
static struct cdev *gpimgsensor_cdev;
static struct class *gpimgsensor_class;

static DEFINE_MUTEX(gimgsensor_mutex);

struct IMGSENSOR  gimgsensor;
struct IMGSENSOR *pgimgsensor = &gimgsensor;



DEFINE_MUTEX(pinctrl_mutex);

#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //timer design
#include <linux/timer.h>
struct timer_list main2_detect_timer; //xen 20170306
struct work_struct main2_detect_work; //xen 20170308
struct workqueue_struct *main2_detect_workqueue;
//#define MAIN2_DETECT_TIMEOUT   (HZ/10) //(HZ/2)	/*0.1 seconds*/ //modified by xen 20170401
#define MAIN2_DETECT_TIMEOUT   (HZ/8) //(HZ/2)	/*0.125 seconds*/

extern int kdCISModulePowerOnMain2(bool On);

kal_uint16 bMain2PoweronStatus=0; //xen 20170328
kal_uint16 bMain2_timer_status=0;

#if 1
extern int iReadRegI2C6(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C6(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
#else
extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
#endif
extern void kpd_detect_shelter_near_handler(void);//report key
extern void kpd_detect_shelter_far_handler(void);

kal_uint16 main2_light_pre=0x0;
kal_uint16 main2_light_current=0;
kal_uint16 pre_shutter = 0xFFFF;
kal_uint16 current_shutter = 0;
kal_uint16 pre_gain = 0xFFFF;
kal_uint16 isNotification = 0;  //whether current main2 light level turns dark,if yes,value is 1,else value is 0 
kal_uint16 isNotification_pre = 0;   //whether main2 light level of last time turns dark,if yes,value is 1,else value is 0 
kal_uint16 needNotifyMMI_near = 0;  //near to main2 camera
kal_uint16 needNotifyMMI_far = 0;   //far from main2 camera
kal_uint16 notifyTimes = 0;  //0 means no near

kal_uint32 main2_sensor_id=0xFFFF; 
kal_uint16 mDarkness_threshold = 0; //shelter light level theshold
kal_uint16 mShelterNear_threshold = 0;
kal_uint16 mShelterFar_threshold = 0;

kal_uint16 gFirstTimeAfterPreview = 0; //get-rid-of the first light level 
kal_uint16 gMain2LightLevel[5]={0};

kal_uint16 current_gain = 0xFFFF;
kal_uint16 iSum = 0;
kal_uint16 main2_light_pre_pre = 0x0;

void main_sensor_set_shutter(kal_uint16 shutter);
kal_uint16 main_sensor_get_shutter(void);
void main_sensor_set_gain(kal_uint16 gain);
kal_uint16 main_sensor_get_gain(void);

kal_uint16 detect_main2_light(void);
void main2_cam_detect_init(void);
void main2_cam_boot_detect(void);

void main2_detect_work_callback(struct work_struct *work);
void main2_detect_timeout(unsigned long a);
#endif

/************************************************************************
 * Profiling
 ************************************************************************/
#define IMGSENSOR_PROF 1
#if IMGSENSOR_PROF
void IMGSENSOR_PROFILE_INIT(struct timeval *ptv)
{
	do_gettimeofday(ptv);
}

void IMGSENSOR_PROFILE(struct timeval *ptv, char *tag)
{
	struct timeval tv;
	unsigned long  time_interval;

	do_gettimeofday(&tv);
	time_interval =
	    (tv.tv_sec - ptv->tv_sec) * 1000000 + (tv.tv_usec - ptv->tv_usec);

	pr_info("[%s]Profile = %lu us\n", tag, time_interval);
}

#else
void IMGSENSOR_PROFILE_INIT(struct timeval *ptv) {}
void IMGSENSOR_PROFILE(struct timeval *ptv, char *tag) {}
#endif

/************************************************************************
 * sensor function adapter
 ************************************************************************/
#define IMGSENSOR_FUNCTION_ENTRY()    /*pr_info("[%s]:E\n",__FUNCTION__)*/
#define IMGSENSOR_FUNCTION_EXIT()     /*pr_info("[%s]:X\n",__FUNCTION__)*/
struct IMGSENSOR_SENSOR *
imgsensor_sensor_get_inst(enum IMGSENSOR_SENSOR_IDX idx)
{
	if (idx < IMGSENSOR_SENSOR_IDX_MIN_NUM ||
	    idx >= IMGSENSOR_SENSOR_IDX_MAX_NUM)
		return NULL;
	else
		return &pgimgsensor->sensor[idx];
}

static void
imgsensor_mutex_init(struct IMGSENSOR_SENSOR_INST *psensor_inst)
{
	mutex_init(&psensor_inst->sensor_mutex);
}

static void imgsensor_mutex_lock(struct IMGSENSOR_SENSOR_INST *psensor_inst)
{
#ifdef IMGSENSOR_LEGACY_COMPAT
#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) 
	struct IMGSENSOR_SENSOR      *psensor_main2 = imgsensor_sensor_get_inst(IMGSENSOR_SENSOR_IDX_MAIN2);
	struct IMGSENSOR_SENSOR_INST *psensor_inst_main2 = &psensor_main2->inst;
#endif
	
	printk("<xjl>imgsensor_mutex_lock enter\n");

	if (psensor_inst->status.arch) {
		mutex_lock(&psensor_inst->sensor_mutex);
	} else {
		mutex_lock(&gimgsensor_mutex);
		imgsensor_i2c_set_device(&psensor_inst->i2c_cfg);
		printk("<xjl>imgsensor_mutex_lock sensor_idx=%d\n", psensor_inst->sensor_idx);
        #if defined(_MAIN2_CAM_SHELTER_DESIGN2_) 
        if (psensor_inst->sensor_idx==IMGSENSOR_SENSOR_IDX_MAIN) 
        { 
			printk("<xjl>imgsensor_mutex_lock set_device_main2\n");
            imgsensor_i2c_set_device_main2(&psensor_inst_main2->i2c_cfg);
        }
        #endif
	}
#else
	mutex_lock(&psensor_inst->sensor_mutex);
#endif
}

static void imgsensor_mutex_unlock(struct IMGSENSOR_SENSOR_INST *psensor_inst)
{
#ifdef IMGSENSOR_LEGACY_COMPAT
	if (psensor_inst->status.arch)
		mutex_unlock(&psensor_inst->sensor_mutex);
	else
		mutex_unlock(&gimgsensor_mutex);
#else
	mutex_lock(&psensor_inst->sensor_mutex);
#endif
}

MINT32
imgsensor_sensor_open(struct IMGSENSOR_SENSOR *psensor)
{
	MINT32 ret = ERROR_NONE;
	struct IMGSENSOR_SENSOR_INST *psensor_inst = &psensor->inst;
	struct SENSOR_FUNCTION_STRUCT *psensor_func =  psensor->pfunc;

#ifdef CONFIG_MTK_CCU
	struct ccu_sensor_info ccuSensorInfo;
	enum IMGSENSOR_SENSOR_IDX sensor_idx = psensor->inst.sensor_idx;
	struct i2c_client *pi2c_client = NULL;
#endif

	IMGSENSOR_FUNCTION_ENTRY();

	if (psensor_func &&
	    psensor_func->SensorOpen &&
	    psensor_inst) {

		/* turn on power */
		IMGSENSOR_PROFILE_INIT(&psensor_inst->profile_time);
		if (pgimgsensor->imgsensor_oc_irq_enable != NULL)
			pgimgsensor->imgsensor_oc_irq_enable(
					psensor->inst.sensor_idx, false);

		ret = imgsensor_hw_power(&pgimgsensor->hw,
		    psensor,
		    psensor_inst->psensor_name,
		    IMGSENSOR_HW_POWER_STATUS_ON);

		if (ret != IMGSENSOR_RETURN_SUCCESS) {
			pr_err("[%s]", __func__);
			return -EIO;
		}
		/* wait for power stable */
		mDELAY(5);

		IMGSENSOR_PROFILE(&psensor_inst->profile_time,
		    "kdCISModulePowerOn");

		imgsensor_mutex_lock(psensor_inst);

		psensor_func->psensor_inst = psensor_inst;
		ret = psensor_func->SensorOpen();
		if (ret != ERROR_NONE) {
			imgsensor_hw_power(&pgimgsensor->hw,
			    psensor,
			    psensor_inst->psensor_name,
			    IMGSENSOR_HW_POWER_STATUS_OFF);

			pr_err("SensorOpen fail");
		} else {
			psensor_inst->state = IMGSENSOR_STATE_OPEN;
#ifdef CONFIG_MTK_CCU
			ccuSensorInfo.slave_addr =
			    (psensor_inst->i2c_cfg.pinst->msg->addr << 1);

			ccuSensorInfo.sensor_name_string =
			    (char *)(psensor_inst->psensor_name);

			pi2c_client = psensor_inst->i2c_cfg.pinst->pi2c_client;
			if (pi2c_client)
				ccuSensorInfo.i2c_id =
					(((struct mt_i2c *) i2c_get_adapdata(
						pi2c_client->adapter))->id);
			else
				ccuSensorInfo.i2c_id = -1;

			ccu_set_sensor_info(sensor_idx, &ccuSensorInfo);
#endif
			if (pgimgsensor->imgsensor_oc_irq_enable != NULL)
				pgimgsensor->imgsensor_oc_irq_enable(
						psensor->inst.sensor_idx, true);

		}

		imgsensor_mutex_unlock(psensor_inst);

		IMGSENSOR_PROFILE(&psensor_inst->profile_time, "SensorOpen");
	}

	IMGSENSOR_FUNCTION_EXIT();

	return ret ? -EIO : ret;
}

MUINT32
imgsensor_sensor_get_info(
	struct IMGSENSOR_SENSOR *psensor,
	MUINT32 ScenarioId,
	MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	MUINT32 ret = ERROR_NONE;
	struct IMGSENSOR_SENSOR_INST *psensor_inst = &psensor->inst;
	struct SENSOR_FUNCTION_STRUCT *psensor_func =  psensor->pfunc;

	IMGSENSOR_FUNCTION_ENTRY();

	if (psensor_func &&
	    psensor_func->SensorGetInfo &&
	    psensor_inst &&
	    pSensorInfo &&
	    pSensorConfigData) {

		imgsensor_mutex_lock(psensor_inst);

		psensor_func->psensor_inst = psensor_inst;

		ret = psensor_func->SensorGetInfo(
		    (enum MSDK_SCENARIO_ID_ENUM)(ScenarioId),
		    pSensorInfo,
		    pSensorConfigData);

		if (ret != ERROR_NONE)
			pr_err("[%s] SensorGetInfo failed\n", __func__);

		imgsensor_mutex_unlock(psensor_inst);
	}

	IMGSENSOR_FUNCTION_EXIT();

	return ret;
}

MUINT32
imgsensor_sensor_get_resolution(
	struct IMGSENSOR_SENSOR *psensor,
	MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	MUINT32 ret = ERROR_NONE;
	struct IMGSENSOR_SENSOR_INST *psensor_inst = &psensor->inst;
	struct SENSOR_FUNCTION_STRUCT *psensor_func =  psensor->pfunc;

	IMGSENSOR_FUNCTION_ENTRY();

	if (psensor_func &&
	    psensor_func->SensorGetResolution &&
	    psensor_inst) {

		imgsensor_mutex_lock(psensor_inst);

		psensor_func->psensor_inst = psensor_inst;

		ret = psensor_func->SensorGetResolution(pSensorResolution);
		if (ret != ERROR_NONE)
			pr_err("[%s]\n", __func__);

		imgsensor_mutex_unlock(psensor_inst);
	}

	IMGSENSOR_FUNCTION_EXIT();

	return ret;
}

MUINT32
imgsensor_sensor_feature_control(
	struct IMGSENSOR_SENSOR *psensor,
	MSDK_SENSOR_FEATURE_ENUM FeatureId,
	MUINT8 *pFeaturePara,
	MUINT32 *pFeatureParaLen)
{
	MUINT32 ret = ERROR_NONE;
	struct IMGSENSOR_SENSOR_INST  *psensor_inst = &psensor->inst;
	struct SENSOR_FUNCTION_STRUCT *psensor_func =  psensor->pfunc;

	IMGSENSOR_FUNCTION_ENTRY();

	if (psensor_func &&
	    psensor_func->SensorFeatureControl &&
	    psensor_inst) {

		imgsensor_mutex_lock(psensor_inst);

		psensor_func->psensor_inst = psensor_inst;

		ret = psensor_func->SensorFeatureControl(
		    FeatureId,
		    pFeaturePara,
		    pFeatureParaLen);

		if (ret != ERROR_NONE)
			pr_err("[%s]\n", __func__);

		imgsensor_mutex_unlock(psensor_inst);
	}

	IMGSENSOR_FUNCTION_EXIT();

	return ret;
}

MUINT32
imgsensor_sensor_control(
	struct IMGSENSOR_SENSOR *psensor,
	enum MSDK_SCENARIO_ID_ENUM ScenarioId)
{
	MUINT32 ret = ERROR_NONE;
	struct IMGSENSOR_SENSOR_INST *psensor_inst = &psensor->inst;
	struct SENSOR_FUNCTION_STRUCT *psensor_func =  psensor->pfunc;

	MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT image_window;
	MSDK_SENSOR_CONFIG_STRUCT sensor_config_data;

	IMGSENSOR_FUNCTION_ENTRY();

	if (psensor_func &&
	    psensor_func->SensorControl &&
	    psensor_inst) {

		IMGSENSOR_PROFILE_INIT(&psensor_inst->profile_time);

		imgsensor_mutex_lock(psensor_inst);

		psensor_func->psensor_inst = psensor_inst;
		psensor_func->ScenarioId = ScenarioId;

		ret = psensor_func->SensorControl(ScenarioId,
		    &image_window,
		    &sensor_config_data);

		if (ret != ERROR_NONE)
			pr_err("[%s]\n", __func__);

		imgsensor_mutex_unlock(psensor_inst);

		IMGSENSOR_PROFILE(
		    &psensor_inst->profile_time,
		    "SensorControl");
	}

	IMGSENSOR_FUNCTION_EXIT();

#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //xen 20170321
        switch (ScenarioId) {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
           if (psensor_inst->sensor_idx==IMGSENSOR_SENSOR_IDX_MAIN) //xen 20170325,only main camera need shelter function
              main2_cam_detect_init();
            break;
        default:
            break;
        }
#endif

	return ret;
}

MINT32
imgsensor_sensor_close(struct IMGSENSOR_SENSOR *psensor)
{
	MINT32 ret = ERROR_NONE;
	struct IMGSENSOR_SENSOR_INST  *psensor_inst = &psensor->inst;
	struct SENSOR_FUNCTION_STRUCT *psensor_func =  psensor->pfunc;

	IMGSENSOR_FUNCTION_ENTRY();

	if (psensor_func &&
	    psensor_func->SensorClose &&
	    psensor_inst) {

		imgsensor_mutex_lock(psensor_inst);

		psensor_func->psensor_inst = psensor_inst;

		ret = psensor_func->SensorClose();
		if (ret != ERROR_NONE) {
			pr_err("[%s]", __func__);
		} else {
			psensor_inst->state = IMGSENSOR_STATE_CLOSE;
			imgsensor_hw_power(&pgimgsensor->hw,
			    psensor,
			    psensor_inst->psensor_name,
			    IMGSENSOR_HW_POWER_STATUS_OFF);
		}

		imgsensor_mutex_unlock(psensor_inst);
	}

	IMGSENSOR_FUNCTION_EXIT();

#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //xen 20170321
    if (bMain2PoweronStatus == 1)  //xen 20170328
    {
    kdCISModulePowerOnMain2(0);

     bMain2PoweronStatus = 0;  //xen 20170328
    }
    
    if (bMain2_timer_status==1)   //xen 20170328
    {
     bMain2_timer_status = 0;
     del_timer_sync(&main2_detect_timer);
    }
#endif

	return ret ? -EIO : ret;
}

#if 1  //xen 20171030
/* Camera information */
#include <linux/proc_fs.h>   /* proc file use */
#define PROC_MAIN_CAM_INFO "driver/camera_info_main"
#define PROC_SUB_CAM_INFO "driver/camera_info_sub"
#define cam_info_size 50
char mtk_main_cam_name[cam_info_size] = {0};
char mtk_sub_cam_name[cam_info_size] = {0};

//main camera
static int subsys_main_cam_info_read(struct seq_file *m, void *v)
{
   //PK_ERR("subsys_tp_info_read %s\n",mtk_tp_name);
   seq_printf(m, "%s\n",mtk_main_cam_name);
   return 0;
};

static int proc_main_cam_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, subsys_main_cam_info_read, NULL);
};

static  struct file_operations cam_proc_fops_main = {
    .owner = THIS_MODULE,
    .open  = proc_main_cam_info_open,
    .read  = seq_read,
};

//sub camera
static int subsys_sub_cam_info_read(struct seq_file *m, void *v)
{
   //PK_ERR("subsys_tp_info_read %s\n",mtk_tp_name);
   seq_printf(m, "%s\n",mtk_sub_cam_name);
   return 0;
};

static int proc_sub_cam_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, subsys_sub_cam_info_read, NULL);
};

static  struct file_operations cam_proc_fops_sub = {
    .owner = THIS_MODULE,
    .open  = proc_sub_cam_info_open,
    .read  = seq_read,
};

//main2 camera
#define PROC_MAIN2_CAM_INFO "driver/camera_info_main2"
char mtk_main2_cam_name[cam_info_size] = {0};
static int subsys_main2_cam_info_read(struct seq_file *m, void *v)
{
   //PK_ERR("subsys_tp_info_read %s\n",mtk_tp_name);
   seq_printf(m, "%s\n",mtk_main2_cam_name);
   return 0;
};

static int proc_main2_cam_info_open(struct inode *inode, struct file *file)
{
    return single_open(file, subsys_main2_cam_info_read, NULL);
};

static  struct file_operations cam_proc_fops_main2 = {
    .owner = THIS_MODULE,
    .open  = proc_main2_cam_info_open,
    .read  = seq_read,
};
#endif
/************************************************************************
 * imgsensor_check_is_alive
 ************************************************************************/
static inline int imgsensor_check_is_alive(struct IMGSENSOR_SENSOR *psensor)
{
	struct IMGSENSOR_SENSOR_INST  *psensor_inst = &psensor->inst;
	UINT32 err = 0;
	MUINT32 sensorID = 0;
	MUINT32 retLen = sizeof(MUINT32);

	IMGSENSOR_PROFILE_INIT(&psensor_inst->profile_time);

	err = imgsensor_hw_power(&pgimgsensor->hw,
				psensor,
				psensor_inst->psensor_name,
				IMGSENSOR_HW_POWER_STATUS_ON);

	if (err == IMGSENSOR_RETURN_SUCCESS)
		imgsensor_sensor_feature_control(
			psensor,
			SENSOR_FEATURE_CHECK_SENSOR_ID,
			(MUINT8 *)&sensorID,
			&retLen);

	if (sensorID == 0 || sensorID == 0xFFFFFFFF) {
		pr_info("Fail to get sensor ID %x\n", sensorID);
		err = ERROR_SENSOR_CONNECT_FAIL;
	} else {
		pr_info(" Sensor found ID = 0x%x\n", sensorID);

            #if 1 // xen 2017-10-30 for camera information
            if (psensor->inst.sensor_idx == IMGSENSOR_SENSOR_IDX_MAIN)
            {
                snprintf(mtk_main_cam_name,sizeof(mtk_main_cam_name),"%s",psensor_inst->psensor_name); 
              #if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //zwl add for detect main2 on boot 20180305
                main2_cam_boot_detect();
              #endif
            }
            else if (psensor->inst.sensor_idx == IMGSENSOR_SENSOR_IDX_SUB)
            {
                snprintf(mtk_sub_cam_name,sizeof(mtk_sub_cam_name),"%s",psensor_inst->psensor_name); 
                 //#ifndef _MAIN2_CAM_SHELTER_DESIGN2_
                   //snprintf(mtk_main2_cam_name,sizeof(mtk_main2_cam_name),"%s","Main2 Camera No Support");
                 //#endif
            }
            else if (psensor->inst.sensor_idx == IMGSENSOR_SENSOR_IDX_MAIN2)
            {
                snprintf(mtk_main2_cam_name,sizeof(mtk_main2_cam_name),"%s",psensor_inst->psensor_name); 

            }
            #endif

		err = ERROR_NONE;
	}

	if (err != ERROR_NONE)
		pr_info("ERROR: No imgsensor alive\n");

	imgsensor_hw_power(&pgimgsensor->hw,
	    psensor,
	    psensor_inst->psensor_name,
	    IMGSENSOR_HW_POWER_STATUS_OFF);

	IMGSENSOR_PROFILE(&psensor_inst->profile_time, "CheckIsAlive");

	return err ? -EIO:err;
}

/************************************************************************
 * imgsensor_set_driver
 ************************************************************************/
int imgsensor_set_driver(struct IMGSENSOR_SENSOR *psensor)
{
	u32 drv_idx = 0;
	int ret = -EIO;

	struct IMGSENSOR_SENSOR_INST    *psensor_inst = &psensor->inst;
	struct IMGSENSOR_INIT_FUNC_LIST *pSensorList  = kdSensorList;
#define TOSTRING(value)           #value
#define STRINGIZE(stringizedName) TOSTRING(stringizedName)

	char *psensor_list_config = NULL, *psensor_list = NULL;
	char *sensor_configs = STRINGIZE(CONFIG_CUSTOM_KERNEL_IMGSENSOR);

	static int orderedSearchList[MAX_NUM_OF_SUPPORT_SENSOR] = {-1};
	static bool get_search_list = true;
	int i = 0;
	int j = 0;
	char *driver_name = NULL;

#if defined(_MAIN2_CAM_SHELTER_DESIGN2_)
        struct IMGSENSOR_SENSOR *psensor_main2;
        struct IMGSENSOR_SENSOR_INST    *psensor_inst_main2;

	printk("<xjl>imgsensor_set_driver sensor_idx=%d\n", psensor->inst.sensor_idx);
	
       if(IMGSENSOR_SENSOR_IDX_MAIN == psensor->inst.sensor_idx)
          {
            psensor_main2 = imgsensor_sensor_get_inst(IMGSENSOR_SENSOR_IDX_MAIN2);
            psensor_inst_main2 = &psensor_main2->inst;
    	    imgsensor_mutex_init(psensor_inst_main2);
	    //imgsensor_i2c_init(&psensor_inst_main2->i2c_cfg, imgsensor_custom_config[psensor_main2->inst.sensor_idx].i2c_dev);
		imgsensor_i2c_init(&psensor_inst_main2->i2c_cfg, 2);
	    imgsensor_i2c_filter_msg(&psensor_inst_main2->i2c_cfg, true);
	    printk("<xjl>imgsensor_set_driver set_main2\n");
          }
#endif


	imgsensor_mutex_init(psensor_inst);
	imgsensor_i2c_init(&psensor_inst->i2c_cfg,
	   imgsensor_custom_config[psensor->inst.sensor_idx].i2c_dev);

	imgsensor_i2c_filter_msg(&psensor_inst->i2c_cfg, true);

	if (get_search_list) {
		psensor_list = psensor_list_config =
		    kmalloc(strlen(sensor_configs)-1, GFP_KERNEL);

		if (psensor_list_config) {
			for (j = 0; j < MAX_NUM_OF_SUPPORT_SENSOR; j++)
				orderedSearchList[j] = -1;

			memcpy(psensor_list_config,
			    sensor_configs+1,
			    strlen(sensor_configs)-2);

			*(psensor_list_config+strlen(sensor_configs)-2) = '\0';

			pr_info("sensor_list %s\n", psensor_list_config);
			driver_name = strsep(&psensor_list_config, " \0");

			while (driver_name != NULL) {
				for (j = 0;
				    j < MAX_NUM_OF_SUPPORT_SENSOR;
				    j++) {
					if (pSensorList[j].init == NULL)
						break;
					else if (!strcmp(
						    driver_name,
						    pSensorList[j].name)) {
						orderedSearchList[i++] = j;
						break;
					}
				}
				driver_name =
				    strsep(&psensor_list_config, " \0");
			}
			get_search_list = false;
		}
		kfree(psensor_list);
	}



	/*pr_debug("get_search_list %d,\n %d %d %d %d\n %d %d %d %d\n",
	 *   get_search_list,
	 *   orderedSearchList[0],
	 *   orderedSearchList[1],
	 *   orderedSearchList[2],
	 *   orderedSearchList[3],
	 *   orderedSearchList[4],
	 *   orderedSearchList[5],
	 *   orderedSearchList[6],
	 *   orderedSearchList[7]);
	 */

	/*pr_debug(" %d %d %d %d\n %d %d %d %d\n",
	 *   orderedSearchList[8],
	 *   orderedSearchList[9],
	 *   orderedSearchList[10],
	 *   orderedSearchList[11],
	 *   orderedSearchList[12],
	 *   orderedSearchList[13],
	 *   orderedSearchList[14],
	 *   orderedSearchList[15]);
	 */

	for (i = 0; i < MAX_NUM_OF_SUPPORT_SENSOR; i++) {
		/*pr_debug("orderedSearchList[%d]=%d\n",
		 *i, orderedSearchList[i]);
		 */
		if (orderedSearchList[i] == -1)
			continue;
		drv_idx = orderedSearchList[i];
		if (pSensorList[drv_idx].init) {
			pSensorList[drv_idx].init(&psensor->pfunc);
			if (psensor->pfunc) {
				/* get sensor name */
				psensor_inst->psensor_name =
				    (char *)pSensorList[drv_idx].name;
#ifdef IMGSENSOR_LEGACY_COMPAT
				psensor_inst->status.arch =
				    psensor->pfunc->arch;
#endif
				if (!imgsensor_check_is_alive(psensor)) {
					pr_info(
					    "[%s]:[%d][%d][%s]\n",
					    __func__,
					    psensor->inst.sensor_idx,
					    drv_idx,
					    psensor_inst->psensor_name);

					ret = drv_idx;
					break;
				}
			} else {
				pr_err(
				    "ERROR:NULL g_pInvokeSensorFunc[%d][%d]\n",
				    psensor->inst.sensor_idx,
				    drv_idx);
			}
		} else {
			pr_err("ERROR:NULL sensor list[%d]\n", drv_idx);
		}

	}
	imgsensor_i2c_filter_msg(&psensor_inst->i2c_cfg, false);

	return ret;
}

/************************************************************************
 * adopt_CAMERA_HW_GetInfo
 ************************************************************************/
static inline int adopt_CAMERA_HW_GetInfo(void *pBuf)
{
	struct IMGSENSOR_GET_CONFIG_INFO_STRUCT *pSensorGetInfo;
	struct IMGSENSOR_SENSOR *psensor;

	MSDK_SENSOR_INFO_STRUCT *pInfo;
	MSDK_SENSOR_CONFIG_STRUCT *pConfig;
	MUINT32 *pScenarioId;

	pSensorGetInfo = (struct IMGSENSOR_GET_CONFIG_INFO_STRUCT *)pBuf;
	if (pSensorGetInfo == NULL ||
	     pSensorGetInfo->pInfo == NULL ||
	     pSensorGetInfo->pConfig == NULL) {
		pr_debug("[CAMERA_HW] NULL arg.\n");
		return -EFAULT;
	}

	psensor = imgsensor_sensor_get_inst(pSensorGetInfo->SensorId);
	if (psensor == NULL) {
		pr_debug("[CAMERA_HW] NULL psensor.\n");
		return -EFAULT;
	}

	pInfo = NULL;
	pConfig =  NULL;
	pScenarioId =  &(pSensorGetInfo->ScenarioId);

	pInfo = kmalloc(sizeof(MSDK_SENSOR_INFO_STRUCT), GFP_KERNEL);
	pConfig = kmalloc(sizeof(MSDK_SENSOR_CONFIG_STRUCT), GFP_KERNEL);

	if (pInfo == NULL || pConfig == NULL) {
		kfree(pInfo);
		kfree(pConfig);
		pr_err(" ioctl allocate mem failed\n");
		return -ENOMEM;
	}

	memset(pInfo, 0, sizeof(MSDK_SENSOR_INFO_STRUCT));
	memset(pConfig, 0, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

	imgsensor_sensor_get_info(psensor, *pScenarioId, pInfo, pConfig);

    /* SenorInfo */
	if (copy_to_user((void __user *)(pSensorGetInfo->pInfo),
	    (void *)pInfo,
	    sizeof(MSDK_SENSOR_INFO_STRUCT))) {

		pr_err("[CAMERA_HW][info] ioctl copy to user failed\n");
		if (pInfo != NULL)
			kfree(pInfo);
		if (pConfig != NULL)
			kfree(pConfig);
		return -EFAULT;
	}

    /* SensorConfig */
	if (copy_to_user((void __user *) (pSensorGetInfo->pConfig),
	    (void *)pConfig,
	    sizeof(MSDK_SENSOR_CONFIG_STRUCT))) {

		pr_debug("[CAMERA_HW][config] ioctl copy to user failed\n");

		if (pInfo != NULL)
			kfree(pInfo);
		if (pConfig != NULL)
			kfree(pConfig);
		return -EFAULT;
	}

	kfree(pInfo);
	kfree(pConfig);
	return 0;
}   /* adopt_CAMERA_HW_GetInfo() */

MUINT32 Get_Camera_Temperature(
	enum CAMERA_DUAL_CAMERA_SENSOR_ENUM senDevId,
	MUINT8 *valid,
	MUINT32 *temp)
{
	MUINT32 ret = IMGSENSOR_RETURN_SUCCESS;
	MUINT32 FeatureParaLen = 0;
	struct IMGSENSOR_SENSOR      *psensor =
		imgsensor_sensor_get_inst(IMGSENSOR_SENSOR_IDX_MAP(senDevId));
	struct IMGSENSOR_SENSOR_INST *psensor_inst;

	if (valid == NULL || temp == NULL || psensor == NULL)
		return IMGSENSOR_RETURN_ERROR;

	*valid =
	    SENSOR_TEMPERATURE_NOT_SUPPORT_THERMAL |
	    SENSOR_TEMPERATURE_NOT_POWER_ON;
	*temp  = 0;

	psensor_inst = &psensor->inst;

	FeatureParaLen = sizeof(MUINT32);

	/* In close state the temperature is not valid */
	if (psensor_inst->state != IMGSENSOR_STATE_CLOSE) {
		ret = imgsensor_sensor_feature_control(psensor,
		    SENSOR_FEATURE_GET_TEMPERATURE_VALUE,
		    (MUINT8 *)temp,
		    (MUINT32 *)&FeatureParaLen);

		pr_debug("senDevId(%d), temperature(%d)\n", senDevId, *temp);

		*valid &= ~SENSOR_TEMPERATURE_NOT_POWER_ON;

		if (*temp != 0) {
			*valid |=  SENSOR_TEMPERATURE_VALID;
			*valid &= ~SENSOR_TEMPERATURE_NOT_SUPPORT_THERMAL;
		}
	}

	return ret;
}
EXPORT_SYMBOL(Get_Camera_Temperature);

#ifdef CONFIG_CAM_TEMPERATURE_WORKQUEUE
static void cam_temperature_report_wq_routine(
	struct work_struct *data)
{
	MUINT8 valid[4] = {0, 0, 0, 0};
	MUINT32 temp[4] = {0, 0, 0, 0};
	MUINT32 ret = 0;

	pr_debug("Temperature Meter Report.\n");

	/* Main cam */
	ret = Get_Camera_Temperature(
	    DUAL_CAMERA_MAIN_SENSOR,
	    &valid[0],
	    &temp[0]);

	pr_info("senDevId(%d), valid(%d), temperature(%d)\n",
				DUAL_CAMERA_MAIN_SENSOR, valid[0], temp[0]);

	if (ret != ERROR_NONE)
		pr_err("Get Main cam temperature error(%d)!\n", ret);

	/* Sub cam */
	ret = Get_Camera_Temperature(
	    DUAL_CAMERA_SUB_SENSOR,
	    &valid[1],
	    &temp[1]);

	pr_info("senDevId(%d), valid(%d), temperature(%d)\n",
				DUAL_CAMERA_SUB_SENSOR, valid[1], temp[1]);

	if (ret != ERROR_NONE)
		pr_err("Get Sub cam temperature error(%d)!\n", ret);

	/* Main2 cam */
	ret = Get_Camera_Temperature(
	    DUAL_CAMERA_MAIN_2_SENSOR,
	    &valid[2],
	    &temp[2]);

	pr_info("senDevId(%d), valid(%d), temperature(%d)\n",
				DUAL_CAMERA_MAIN_2_SENSOR, valid[2], temp[2]);

	if (ret != ERROR_NONE)
		pr_err("Get Main2 cam temperature error(%d)!\n", ret);

	ret = Get_Camera_Temperature(
	    DUAL_CAMERA_SUB_2_SENSOR,
	    &valid[3],
	    &temp[3]);

	pr_info("senDevId(%d), valid(%d), temperature(%d)\n",
				DUAL_CAMERA_SUB_2_SENSOR, valid[3], temp[3]);

	if (ret != ERROR_NONE)
		pr_err("Get Sub2 cam temperature error(%d)!\n", ret);
	schedule_delayed_work(&cam_temperature_wq, HZ);

}
#endif

static inline int adopt_CAMERA_HW_GetInfo2(void *pBuf)
{
	int ret = 0;
	struct IMAGESENSOR_GETINFO_STRUCT *pSensorGetInfo;
	struct IMGSENSOR_SENSOR    *psensor;

	ACDK_SENSOR_INFO2_STRUCT *pSensorInfo = NULL;
	MSDK_SENSOR_INFO_STRUCT *pInfo = NULL;
	MSDK_SENSOR_CONFIG_STRUCT  *pConfig = NULL;
	MSDK_SENSOR_INFO_STRUCT *pInfo1 = NULL;
	MSDK_SENSOR_CONFIG_STRUCT  *pConfig1 = NULL;
	MSDK_SENSOR_INFO_STRUCT *pInfo2 = NULL;
	MSDK_SENSOR_CONFIG_STRUCT  *pConfig2 = NULL;
	MSDK_SENSOR_INFO_STRUCT *pInfo3 = NULL;
	MSDK_SENSOR_CONFIG_STRUCT  *pConfig3 = NULL;
	MSDK_SENSOR_INFO_STRUCT *pInfo4 = NULL;
	MSDK_SENSOR_CONFIG_STRUCT  *pConfig4 = NULL;
	MSDK_SENSOR_RESOLUTION_INFO_STRUCT  *psensorResolution = NULL;
	char *pmtk_ccm_name = NULL;

	pSensorGetInfo = (struct IMAGESENSOR_GETINFO_STRUCT *)pBuf;
	if (pSensorGetInfo == NULL ||
	    pSensorGetInfo->pInfo == NULL ||
	    pSensorGetInfo->pSensorResolution == NULL) {
		pr_info("[adopt_CAMERA_HW_GetInfo2] NULL arg.\n");
		return -EFAULT;
	}

	psensor = imgsensor_sensor_get_inst(pSensorGetInfo->SensorId);
	if (psensor == NULL) {
		pr_info("[adopt_CAMERA_HW_GetInfo2] NULL psensor.\n");
		return -EFAULT;
	}

	pr_debug("[adopt_CAMERA_HW_GetInfo2]Entry%d\n",
	    pSensorGetInfo->SensorId);

	pInfo =    kzalloc(sizeof(MSDK_SENSOR_INFO_STRUCT), GFP_KERNEL);
	pConfig =  kzalloc(sizeof(MSDK_SENSOR_CONFIG_STRUCT), GFP_KERNEL);
	pInfo1 =   kzalloc(sizeof(MSDK_SENSOR_INFO_STRUCT), GFP_KERNEL);
	pConfig1 = kzalloc(sizeof(MSDK_SENSOR_CONFIG_STRUCT), GFP_KERNEL);
	pInfo2 =   kzalloc(sizeof(MSDK_SENSOR_INFO_STRUCT), GFP_KERNEL);
	pConfig2 = kzalloc(sizeof(MSDK_SENSOR_CONFIG_STRUCT), GFP_KERNEL);
	pInfo3 =   kzalloc(sizeof(MSDK_SENSOR_INFO_STRUCT), GFP_KERNEL);
	pConfig3 = kzalloc(sizeof(MSDK_SENSOR_CONFIG_STRUCT), GFP_KERNEL);
	pInfo4 =   kzalloc(sizeof(MSDK_SENSOR_INFO_STRUCT), GFP_KERNEL);
	pConfig4 = kzalloc(sizeof(MSDK_SENSOR_CONFIG_STRUCT), GFP_KERNEL);
	psensorResolution =
	    kzalloc(sizeof(MSDK_SENSOR_RESOLUTION_INFO_STRUCT), GFP_KERNEL);

	pSensorInfo = kzalloc(sizeof(ACDK_SENSOR_INFO2_STRUCT), GFP_KERNEL);

	if (pConfig == NULL ||
		pConfig1 == NULL ||
		pConfig2 == NULL ||
		pConfig3 == NULL ||
		pConfig4 == NULL ||
		pSensorInfo == NULL ||
		psensorResolution == NULL) {
		pr_err(" ioctl allocate mem failed\n");
		ret = -EFAULT;
		goto IMGSENSOR_GET_INFO_RETURN;
	}

	imgsensor_sensor_get_info(
	    psensor,
	    MSDK_SCENARIO_ID_CAMERA_PREVIEW,
	    pInfo,
	    pConfig);

	imgsensor_sensor_get_info(
	    psensor,
	    MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,
	    pInfo1,
	    pConfig1);

	imgsensor_sensor_get_info(
	    psensor, MSDK_SCENARIO_ID_VIDEO_PREVIEW,
	    pInfo2,
	    pConfig2);

	imgsensor_sensor_get_info(
	    psensor,
	    MSDK_SCENARIO_ID_HIGH_SPEED_VIDEO,
	    pInfo3,
	    pConfig3);

	imgsensor_sensor_get_info(
	    psensor,
	    MSDK_SCENARIO_ID_SLIM_VIDEO,
	    pInfo4,
	    pConfig4);

	/* Basic information */
	pSensorInfo->SensorPreviewResolutionX = pInfo->SensorPreviewResolutionX;
	pSensorInfo->SensorPreviewResolutionY = pInfo->SensorPreviewResolutionY;
	pSensorInfo->SensorFullResolutionX = pInfo->SensorFullResolutionX;
	pSensorInfo->SensorFullResolutionY = pInfo->SensorFullResolutionY;
	pSensorInfo->SensorClockFreq = pInfo->SensorClockFreq;
	pSensorInfo->SensorCameraPreviewFrameRate =
	    pInfo->SensorCameraPreviewFrameRate;

	pSensorInfo->SensorVideoFrameRate = pInfo->SensorVideoFrameRate;
	pSensorInfo->SensorStillCaptureFrameRate =
	    pInfo->SensorStillCaptureFrameRate;

	pSensorInfo->SensorWebCamCaptureFrameRate =
	    pInfo->SensorWebCamCaptureFrameRate;

	pSensorInfo->SensorClockPolarity = pInfo->SensorClockPolarity;
	pSensorInfo->SensorClockFallingPolarity =
	    pInfo->SensorClockFallingPolarity;

	pSensorInfo->SensorClockRisingCount = pInfo->SensorClockRisingCount;
	pSensorInfo->SensorClockFallingCount = pInfo->SensorClockFallingCount;
	pSensorInfo->SensorClockDividCount = pInfo->SensorClockDividCount;
	pSensorInfo->SensorPixelClockCount = pInfo->SensorPixelClockCount;
	pSensorInfo->SensorDataLatchCount = pInfo->SensorDataLatchCount;
	pSensorInfo->SensorHsyncPolarity = pInfo->SensorHsyncPolarity;
	pSensorInfo->SensorVsyncPolarity = pInfo->SensorVsyncPolarity;
	pSensorInfo->SensorInterruptDelayLines =
	    pInfo->SensorInterruptDelayLines;

	pSensorInfo->SensorResetActiveHigh = pInfo->SensorResetActiveHigh;
	pSensorInfo->SensorResetDelayCount = pInfo->SensorResetDelayCount;
	pSensorInfo->SensroInterfaceType = pInfo->SensroInterfaceType;
	pSensorInfo->SensorOutputDataFormat  = pInfo->SensorOutputDataFormat;
	pSensorInfo->SensorMIPILaneNumber = pInfo->SensorMIPILaneNumber;
	pSensorInfo->CaptureDelayFrame = pInfo->CaptureDelayFrame;
	pSensorInfo->PreviewDelayFrame = pInfo->PreviewDelayFrame;
	pSensorInfo->VideoDelayFrame = pInfo->VideoDelayFrame;
	pSensorInfo->HighSpeedVideoDelayFrame =
	    pInfo->HighSpeedVideoDelayFrame;

	pSensorInfo->SlimVideoDelayFrame = pInfo->SlimVideoDelayFrame;
	pSensorInfo->Custom1DelayFrame = pInfo->Custom1DelayFrame;
	pSensorInfo->Custom2DelayFrame = pInfo->Custom2DelayFrame;
	pSensorInfo->Custom3DelayFrame = pInfo->Custom3DelayFrame;
	pSensorInfo->Custom4DelayFrame = pInfo->Custom4DelayFrame;
	pSensorInfo->Custom5DelayFrame = pInfo->Custom5DelayFrame;
	pSensorInfo->YUVAwbDelayFrame = pInfo->YUVAwbDelayFrame;
	pSensorInfo->YUVEffectDelayFrame = pInfo->YUVEffectDelayFrame;
	pSensorInfo->SensorGrabStartX_PRV = pInfo->SensorGrabStartX;
	pSensorInfo->SensorGrabStartY_PRV = pInfo->SensorGrabStartY;
	pSensorInfo->SensorGrabStartX_CAP = pInfo1->SensorGrabStartX;
	pSensorInfo->SensorGrabStartY_CAP = pInfo1->SensorGrabStartY;
	pSensorInfo->SensorGrabStartX_VD  = pInfo2->SensorGrabStartX;
	pSensorInfo->SensorGrabStartY_VD  = pInfo2->SensorGrabStartY;
	pSensorInfo->SensorGrabStartX_VD1 = pInfo3->SensorGrabStartX;
	pSensorInfo->SensorGrabStartY_VD1 = pInfo3->SensorGrabStartY;
	pSensorInfo->SensorGrabStartX_VD2 = pInfo4->SensorGrabStartX;
	pSensorInfo->SensorGrabStartY_VD2 = pInfo4->SensorGrabStartY;
	pSensorInfo->SensorDrivingCurrent = pInfo->SensorDrivingCurrent;
	pSensorInfo->SensorMasterClockSwitch = pInfo->SensorMasterClockSwitch;
	pSensorInfo->AEShutDelayFrame = pInfo->AEShutDelayFrame;
	pSensorInfo->AESensorGainDelayFrame = pInfo->AESensorGainDelayFrame;
	pSensorInfo->AEISPGainDelayFrame = pInfo->AEISPGainDelayFrame;
	pSensorInfo->FrameTimeDelayFrame = pInfo->FrameTimeDelayFrame;

	pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount =
	   pInfo->MIPIDataLowPwr2HighSpeedTermDelayCount;

	pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount =
	   pInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount;

	pSensorInfo->MIPIDataLowPwr2HSSettleDelayM0 =
	    pInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount;

	pSensorInfo->MIPIDataLowPwr2HSSettleDelayM1 =
	    pInfo1->MIPIDataLowPwr2HighSpeedSettleDelayCount;

	pSensorInfo->MIPIDataLowPwr2HSSettleDelayM2 =
	    pInfo2->MIPIDataLowPwr2HighSpeedSettleDelayCount;
	pSensorInfo->MIPIDataLowPwr2HSSettleDelayM3 =
	    pInfo3->MIPIDataLowPwr2HighSpeedSettleDelayCount;

	pSensorInfo->MIPIDataLowPwr2HSSettleDelayM4 =
	    pInfo4->MIPIDataLowPwr2HighSpeedSettleDelayCount;

	pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount =
	    pInfo->MIPICLKLowPwr2HighSpeedTermDelayCount;

	pSensorInfo->SensorWidthSampling = pInfo->SensorWidthSampling;
	pSensorInfo->SensorHightSampling = pInfo->SensorHightSampling;
	pSensorInfo->SensorPacketECCOrder = pInfo->SensorPacketECCOrder;
	pSensorInfo->MIPIsensorType = pInfo->MIPIsensorType;
	pSensorInfo->IHDR_LE_FirstLine = pInfo->IHDR_LE_FirstLine;
	pSensorInfo->IHDR_Support = pInfo->IHDR_Support;
	pSensorInfo->ZHDR_Mode = pInfo->ZHDR_Mode;
	pSensorInfo->TEMPERATURE_SUPPORT = pInfo->TEMPERATURE_SUPPORT;
	pSensorInfo->SensorModeNum = pInfo->SensorModeNum;
	pSensorInfo->SettleDelayMode = pInfo->SettleDelayMode;
	pSensorInfo->PDAF_Support = pInfo->PDAF_Support;
	pSensorInfo->HDR_Support = pInfo->HDR_Support;
	pSensorInfo->IMGSENSOR_DPCM_TYPE_PRE = pInfo->DPCM_INFO;
	pSensorInfo->IMGSENSOR_DPCM_TYPE_CAP = pInfo1->DPCM_INFO;
	pSensorInfo->IMGSENSOR_DPCM_TYPE_VD  = pInfo2->DPCM_INFO;
	pSensorInfo->IMGSENSOR_DPCM_TYPE_VD1 = pInfo3->DPCM_INFO;
	pSensorInfo->IMGSENSOR_DPCM_TYPE_VD2 = pInfo4->DPCM_INFO;
	/*Per-Frame conrol support or not */
	pSensorInfo->PerFrameCTL_Support  = pInfo->PerFrameCTL_Support;
	/*SCAM number*/
	pSensorInfo->SCAM_DataNumber = pInfo->SCAM_DataNumber;
	pSensorInfo->SCAM_DDR_En = pInfo->SCAM_DDR_En;
	pSensorInfo->SCAM_CLK_INV = pInfo->SCAM_CLK_INV;
	pSensorInfo->SCAM_DEFAULT_DELAY = pInfo->SCAM_DEFAULT_DELAY;
	pSensorInfo->SCAM_CRC_En  = pInfo->SCAM_CRC_En;
	pSensorInfo->SCAM_SOF_src = pInfo->SCAM_SOF_src;
	pSensorInfo->SCAM_Timout_Cali = pInfo->SCAM_Timout_Cali;
	/*Deskew*/
	pSensorInfo->SensorMIPIDeskew = pInfo->SensorMIPIDeskew;
	pSensorInfo->SensorVerFOV = pInfo->SensorVerFOV;
	pSensorInfo->SensorHorFOV = pInfo->SensorHorFOV;
	pSensorInfo->SensorOrientation = pInfo->SensorOrientation;

	imgsensor_sensor_get_info(
	    psensor,
	    MSDK_SCENARIO_ID_CUSTOM1,
	    pInfo,
	    pConfig);

	imgsensor_sensor_get_info(
	    psensor,
	    MSDK_SCENARIO_ID_CUSTOM2,
	    pInfo1,
	    pConfig1);

	imgsensor_sensor_get_info(
	    psensor,
	    MSDK_SCENARIO_ID_CUSTOM3,
	    pInfo2,
	    pConfig2);

	imgsensor_sensor_get_info(
	    psensor,
	    MSDK_SCENARIO_ID_CUSTOM4,
	    pInfo3,
	    pConfig3);

	imgsensor_sensor_get_info(
	    psensor,
	    MSDK_SCENARIO_ID_CUSTOM5,
	    pInfo4,
	    pConfig4);

	/* To set sensor information */
	pSensorInfo->SensorGrabStartX_CST1 = pInfo->SensorGrabStartX;
	pSensorInfo->SensorGrabStartY_CST1 = pInfo->SensorGrabStartY;
	pSensorInfo->SensorGrabStartX_CST2 = pInfo1->SensorGrabStartX;
	pSensorInfo->SensorGrabStartY_CST2 = pInfo1->SensorGrabStartY;
	pSensorInfo->SensorGrabStartX_CST3 = pInfo2->SensorGrabStartX;
	pSensorInfo->SensorGrabStartY_CST3 = pInfo2->SensorGrabStartY;
	pSensorInfo->SensorGrabStartX_CST4 = pInfo3->SensorGrabStartX;
	pSensorInfo->SensorGrabStartY_CST4 = pInfo3->SensorGrabStartY;
	pSensorInfo->SensorGrabStartX_CST5 = pInfo4->SensorGrabStartX;
	pSensorInfo->SensorGrabStartY_CST5 = pInfo4->SensorGrabStartY;

	if (copy_to_user(
	    (void __user *)(pSensorGetInfo->pInfo),
	    (void *)(pSensorInfo),
	    sizeof(ACDK_SENSOR_INFO2_STRUCT))) {

		pr_debug("[CAMERA_HW][info] ioctl copy to user failed\n");
		ret = -EFAULT;
		goto IMGSENSOR_GET_INFO_RETURN;
	}

	/* Step2 : Get Resolution */
	imgsensor_sensor_get_resolution(psensor, psensorResolution);

	pr_debug(
		"[CAMERA_HW][Pre]w=0x%x, h = 0x%x\n, [Full]w=0x%x, h = 0x%x\n, [VD]w=0x%x, h = 0x%x\n",
				psensorResolution->SensorPreviewWidth,
				psensorResolution->SensorPreviewHeight,
				psensorResolution->SensorFullWidth,
				psensorResolution->SensorFullHeight,
				psensorResolution->SensorVideoWidth,
				psensorResolution->SensorVideoHeight);

	/* Add info to proc: camera_info */
	pmtk_ccm_name = strchr(mtk_ccm_name, '\0');
	snprintf(pmtk_ccm_name,
		camera_info_size - (int)(pmtk_ccm_name - mtk_ccm_name),
				"\n\nCAM_Info[%d]:%s;",
				pSensorGetInfo->SensorId,
				psensor->inst.psensor_name);

	pmtk_ccm_name = strchr(mtk_ccm_name, '\0');
	snprintf(pmtk_ccm_name,
		camera_info_size - (int)(pmtk_ccm_name - mtk_ccm_name),
		"\nPre: TgGrab_w,h,x_,y=%5d,%5d,%3d,%3d, delay_frm=%2d",
		psensorResolution->SensorPreviewWidth,
		psensorResolution->SensorPreviewHeight,
		pSensorInfo->SensorGrabStartX_PRV,
		pSensorInfo->SensorGrabStartY_PRV,
		pSensorInfo->PreviewDelayFrame);

	pmtk_ccm_name = strchr(mtk_ccm_name, '\0');
	snprintf(pmtk_ccm_name,
		camera_info_size - (int)(pmtk_ccm_name - mtk_ccm_name),
		"\nCap: TgGrab_w,h,x_,y=%5d,%5d,%3d,%3d, delay_frm=%2d",
		psensorResolution->SensorFullWidth,
		psensorResolution->SensorFullHeight,
		pSensorInfo->SensorGrabStartX_CAP,
		pSensorInfo->SensorGrabStartY_CAP,
		pSensorInfo->CaptureDelayFrame);

	pmtk_ccm_name = strchr(mtk_ccm_name, '\0');
	snprintf(pmtk_ccm_name,
		camera_info_size - (int)(pmtk_ccm_name - mtk_ccm_name),
		"\nVid: TgGrab_w,h,x_,y=%5d,%5d,%3d,%3d, delay_frm=%2d",
		psensorResolution->SensorVideoWidth,
		psensorResolution->SensorVideoHeight,
		pSensorInfo->SensorGrabStartX_VD,
		pSensorInfo->SensorGrabStartY_VD,
		pSensorInfo->VideoDelayFrame);

	pmtk_ccm_name = strchr(mtk_ccm_name, '\0');
	snprintf(pmtk_ccm_name,
		camera_info_size - (int)(pmtk_ccm_name - mtk_ccm_name),
		"\nHSV: TgGrab_w,h,x_,y=%5d,%5d,%3d,%3d, delay_frm=%2d",
		psensorResolution->SensorHighSpeedVideoWidth,
		psensorResolution->SensorHighSpeedVideoHeight,
		pSensorInfo->SensorGrabStartX_VD1,
		pSensorInfo->SensorGrabStartY_VD1,
		pSensorInfo->HighSpeedVideoDelayFrame);

	pmtk_ccm_name = strchr(mtk_ccm_name, '\0');
	snprintf(pmtk_ccm_name,
		camera_info_size - (int)(pmtk_ccm_name - mtk_ccm_name),
		"\nSLV: TgGrab_w,h,x_,y=%5d,%5d,%3d,%3d, delay_frm=%2d",
		psensorResolution->SensorSlimVideoWidth,
		psensorResolution->SensorSlimVideoHeight,
		pSensorInfo->SensorGrabStartX_VD2,
		pSensorInfo->SensorGrabStartY_VD2,
		pSensorInfo->SlimVideoDelayFrame);

	pmtk_ccm_name = strchr(mtk_ccm_name, '\0');
	snprintf(pmtk_ccm_name,
		camera_info_size - (int)(pmtk_ccm_name - mtk_ccm_name),
		"\nSeninf_Type(0:parallel,1:mipi,2:serial)=%d, output_format(0:B,1:Gb,2:Gr,3:R)=%2d",
		pSensorInfo->SensroInterfaceType,
		pSensorInfo->SensorOutputDataFormat);

	pmtk_ccm_name = strchr(mtk_ccm_name, '\0');
	snprintf(pmtk_ccm_name,
		camera_info_size - (int)(pmtk_ccm_name - mtk_ccm_name),
		"\nDriving_Current(0:2mA,1:4mA,2:6mA,3:8mA)=%d, mclk_freq=%2d, mipi_lane=%d",
		pSensorInfo->SensorDrivingCurrent,
		pSensorInfo->SensorClockFreq,
		pSensorInfo->SensorMIPILaneNumber + 1);

	pmtk_ccm_name = strchr(mtk_ccm_name, '\0');
	snprintf(pmtk_ccm_name,
		camera_info_size - (int)(pmtk_ccm_name - mtk_ccm_name),
		"\nPDAF_Support(0:No PD,1:PD RAW,2:VC(Full),3:VC(Bin),4:Dual Raw,5:Dual VC=%2d",
		pSensorInfo->PDAF_Support);

	pmtk_ccm_name = strchr(mtk_ccm_name, '\0');
	snprintf(pmtk_ccm_name,
		camera_info_size - (int)(pmtk_ccm_name - mtk_ccm_name),
		"\nHDR_Support(0:NO HDR,1: iHDR,2:mvHDR,3:zHDR)=%2d",
		pSensorInfo->HDR_Support);

	/* Resolution */
	if (copy_to_user(
	    (void __user *) (pSensorGetInfo->pSensorResolution),
	    (void *)psensorResolution,
	    sizeof(MSDK_SENSOR_RESOLUTION_INFO_STRUCT))) {

		pr_debug("[CAMERA_HW][Resolution] ioctl copy to user failed\n");
		ret = -EFAULT;
		goto IMGSENSOR_GET_INFO_RETURN;
	}

IMGSENSOR_GET_INFO_RETURN:

	kfree(pInfo);
	kfree(pInfo1);
	kfree(pInfo2);
	kfree(pInfo3);
	kfree(pInfo4);
	kfree(pConfig);
	kfree(pConfig1);
	kfree(pConfig2);
	kfree(pConfig3);
	kfree(pConfig4);
	kfree(psensorResolution);
	kfree(pSensorInfo);

	return ret;
}   /* adopt_CAMERA_HW_GetInfo() */

/************************************************************************
 * adopt_CAMERA_HW_Control
 ************************************************************************/
static inline int adopt_CAMERA_HW_Control(void *pBuf)
{
	int ret = 0;
	struct ACDK_SENSOR_CONTROL_STRUCT *pSensorCtrl;
	struct IMGSENSOR_SENSOR *psensor;

	pSensorCtrl = (struct ACDK_SENSOR_CONTROL_STRUCT *)pBuf;
	if (pSensorCtrl == NULL) {
		pr_err("[adopt_CAMERA_HW_Control] NULL arg.\n");
		return -EFAULT;
	}

	psensor = imgsensor_sensor_get_inst(pSensorCtrl->InvokeCamera);
	if (psensor == NULL) {
		pr_err("[adopt_CAMERA_HW_Control] NULL psensor.\n");
		return -EFAULT;
	}

	ret = imgsensor_sensor_control(psensor, pSensorCtrl->ScenarioId);

	return ret;
} /* adopt_CAMERA_HW_Control */

static inline int check_length_of_para(
	enum ACDK_SENSOR_FEATURE_ENUM FeatureId, unsigned int length)
{
	int ret = 0;

	switch (FeatureId) {
	case SENSOR_FEATURE_OPEN:
	case SENSOR_FEATURE_CLOSE:
	case SENSOR_FEATURE_CHECK_IS_ALIVE:
		break;
	case SENSOR_FEATURE_SET_DRIVER:
	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
	case SENSOR_FEATURE_SET_FRAMERATE:
	case SENSOR_FEATURE_SET_HDR:
	case SENSOR_FEATURE_SET_PDAF:
	{
		if (length != 4)
			ret = -EFAULT;
	}
		break;
	case SENSOR_FEATURE_SET_ESHUTTER:
	case SENSOR_FEATURE_SET_GAIN:
	case SENSOR_FEATURE_SET_I2C_BUF_MODE_EN:
	case SENSOR_FEATURE_SET_SHUTTER_BUF_MODE:
	case SENSOR_FEATURE_SET_GAIN_BUF_MODE:
	case SENSOR_FEATURE_SET_VIDEO_MODE:
	case SENSOR_FEATURE_SET_AF_WINDOW:
	case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
	case SENSOR_FEATURE_GET_EV_AWB_REF:
	case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
	case SENSOR_FEATURE_GET_EXIF_INFO:
	case SENSOR_FEATURE_GET_DELAY_INFO:
	case SENSOR_FEATURE_SET_TEST_PATTERN:
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
	case SENSOR_FEATURE_SET_OB_LOCK:
	case SENSOR_FEATURE_SET_SENSOR_OTP_AWB_CMD:
	case SENSOR_FEATURE_SET_SENSOR_OTP_LSC_CMD:
	case SENSOR_FEATURE_GET_TEMPERATURE_VALUE:
	case SENSOR_FEATURE_GET_AE_FLASHLIGHT_INFO:
	case SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO:
	case SENSOR_FEATURE_SET_YUV_3A_CMD:
	case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
	case SENSOR_FEATURE_SET_STREAMING_RESUME:
	case SENSOR_FEATURE_GET_PERIOD:
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
	{
		if (length != 8)
			ret = -EFAULT;
	}
		break;
	case SENSOR_FEATURE_SET_DUAL_GAIN:
	case SENSOR_FEATURE_SET_YUV_CMD:
	case SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO:
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
	case SENSOR_FEATURE_GET_CROP_INFO:
	case SENSOR_FEATURE_GET_VC_INFO:
	case SENSOR_FEATURE_GET_PDAF_INFO:
	case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
	case SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY:
	case SENSOR_FEATURE_SET_SHUTTER_FRAME_TIME:
	case SENSOR_FEATURE_SET_PDFOCUS_AREA:
	case SENSOR_FEATURE_GET_PDAF_REG_SETTING:
	case SENSOR_FEATURE_SET_PDAF_REG_SETTING:
	{
		if (length != 16)
			ret = -EFAULT;
	}
		break;
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
	case SENSOR_FEATURE_SET_HDR_SHUTTER:
	case SENSOR_FEATURE_GET_PDAF_DATA:
	case SENSOR_FEATURE_GET_4CELL_DATA:
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
	case SENSOR_FEATURE_GET_PIXEL_RATE:
	{
		if (length != 24)
			ret = -EFAULT;
	}
		break;
	case SENSOR_FEATURE_SET_SENSOR_SYNC:
	case SENSOR_FEATURE_SET_ESHUTTER_GAIN:
	{
		if (length != 32)
			ret = -EFAULT;
	}
		break;
	case SENSOR_FEATURE_SET_CALIBRATION_DATA:
	{
		if (length !=
			sizeof(struct SET_SENSOR_CALIBRATION_DATA_STRUCT))
			ret = -EFAULT;
	}
		break;
	case SENSOR_FEATURE_SET_AWB_GAIN:
	{
		if (length !=
			sizeof(struct SET_SENSOR_AWB_GAIN))
			ret = -EFAULT;
	}
		break;
	case SENSOR_FEATURE_SET_REGISTER:
	case SENSOR_FEATURE_GET_REGISTER:
	{

		if (length !=
			sizeof(MSDK_SENSOR_REG_INFO_STRUCT))
			ret = -EFAULT;
	}
		break;
	/* begin of legacy feature control; Do nothing */
	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
	case SENSOR_FEATURE_SET_CCT_REGISTER:
	case SENSOR_FEATURE_SET_ENG_REGISTER:
	case SENSOR_FEATURE_SET_ITEM_INFO:
	case SENSOR_FEATURE_GET_ITEM_INFO:
	case SENSOR_FEATURE_GET_ENG_INFO:
	case SENSOR_FEATURE_MOVE_FOCUS_LENS:
	case SENSOR_FEATURE_SET_AE_WINDOW:
	case SENSOR_FEATURE_SET_MIN_MAX_FPS:
	case SENSOR_FEATURE_GET_RESOLUTION:
	case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
	case SENSOR_FEATURE_GET_CONFIG_PARA:
	case SENSOR_FEATURE_GET_GROUP_COUNT:
	case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
	case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
	case SENSOR_FEATURE_SINGLE_FOCUS_MODE:
	case SENSOR_FEATURE_CANCEL_AF:
	case SENSOR_FEATURE_CONSTANT_AF:
	/* end of legacy feature control */
	default:
		break;
	}
	if (ret != 0)
		pr_err(
			"check length failed, feature ctrl id = %d, length = %d\n",
			FeatureId,
			length);
	return ret;
}

/************************************************************************
 * adopt_CAMERA_HW_FeatureControl
 ************************************************************************/
static inline int adopt_CAMERA_HW_FeatureControl(void *pBuf)
{
	struct ACDK_SENSOR_FEATURECONTROL_STRUCT *pFeatureCtrl;
	struct IMGSENSOR_SENSOR *psensor;
	unsigned int FeatureParaLen = 0;
	void *pFeaturePara = NULL;
	struct ACDK_KD_SENSOR_SYNC_STRUCT *pSensorSyncInfo = NULL;
	signed int ret = 0;
#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //xen 20170321
	unsigned long long temp=0;
#endif
	pFeatureCtrl = (struct ACDK_SENSOR_FEATURECONTROL_STRUCT *)pBuf;
	if (pFeatureCtrl  == NULL) {
		pr_err(" NULL arg.\n");
		return -EFAULT;
	}

	psensor = imgsensor_sensor_get_inst(pFeatureCtrl->InvokeCamera);
	if (psensor == NULL) {
		pr_err("[adopt_CAMERA_HW_FeatureControl] NULL psensor.\n");
		return -EFAULT;
	}

	if (pFeatureCtrl->FeatureId == SENSOR_FEATURE_SINGLE_FOCUS_MODE ||
		pFeatureCtrl->FeatureId == SENSOR_FEATURE_CANCEL_AF ||
		pFeatureCtrl->FeatureId == SENSOR_FEATURE_CONSTANT_AF ||
		pFeatureCtrl->FeatureId == SENSOR_FEATURE_INFINITY_AF) {
		/* YUV AF_init and AF_constent and AF_single has no params */
	} else {
		if (pFeatureCtrl->pFeaturePara == NULL ||
		    pFeatureCtrl->pFeatureParaLen == NULL) {
			pr_err(" NULL arg.\n");
			return -EFAULT;
		}
		if (copy_from_user((void *)&FeatureParaLen,
				(void *) pFeatureCtrl->pFeatureParaLen,
				sizeof(unsigned int))) {

			pr_err(" ioctl copy from user failed\n");
			return -EFAULT;
		}
		if (FeatureParaLen > FEATURE_CONTROL_MAX_DATA_SIZE ||
			FeatureParaLen == 0)
			return -EINVAL;
		ret = check_length_of_para(pFeatureCtrl->FeatureId,
							FeatureParaLen);
		if (ret != 0)
			return ret;

		pFeaturePara = kmalloc(FeatureParaLen, GFP_KERNEL);
		if (pFeaturePara == NULL)
			return -ENOMEM;

		memset(pFeaturePara, 0x0, FeatureParaLen);
	}

	/* copy from user */
	switch (pFeatureCtrl->FeatureId) {
	case SENSOR_FEATURE_OPEN:
		ret = imgsensor_sensor_open(psensor);
		break;
	case SENSOR_FEATURE_CLOSE:
		ret = imgsensor_sensor_close(psensor);
		/* reset the delay frame flag */
		break;

	case SENSOR_FEATURE_SET_DRIVER:
	{
		MINT32 drv_idx;

		psensor->inst.sensor_idx = pFeatureCtrl->InvokeCamera;
		drv_idx = imgsensor_set_driver(psensor);
		memcpy(pFeaturePara, &drv_idx, FeatureParaLen);

		break;
	}
	case SENSOR_FEATURE_CHECK_IS_ALIVE:
		imgsensor_check_is_alive(psensor);
		break;
	case SENSOR_FEATURE_SET_ESHUTTER:
	case SENSOR_FEATURE_SET_GAIN:
	case SENSOR_FEATURE_SET_DUAL_GAIN:
	case SENSOR_FEATURE_SET_I2C_BUF_MODE_EN:
	case SENSOR_FEATURE_SET_SHUTTER_BUF_MODE:
	case SENSOR_FEATURE_SET_GAIN_BUF_MODE:
	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
	case SENSOR_FEATURE_SET_REGISTER:
	case SENSOR_FEATURE_GET_REGISTER:
	case SENSOR_FEATURE_SET_CCT_REGISTER:
	case SENSOR_FEATURE_SET_ENG_REGISTER:
	case SENSOR_FEATURE_SET_ITEM_INFO:
	case SENSOR_FEATURE_GET_ITEM_INFO:
	case SENSOR_FEATURE_GET_ENG_INFO:
	case SENSOR_FEATURE_SET_VIDEO_MODE:
	case SENSOR_FEATURE_SET_YUV_CMD:
	case SENSOR_FEATURE_MOVE_FOCUS_LENS:
	case SENSOR_FEATURE_SET_AF_WINDOW:
	case SENSOR_FEATURE_SET_CALIBRATION_DATA:
	case SENSOR_FEATURE_SET_AUTO_FLICKER_MODE:
	case SENSOR_FEATURE_GET_EV_AWB_REF:
	case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
	case SENSOR_FEATURE_SET_AE_WINDOW:
	case SENSOR_FEATURE_GET_EXIF_INFO:
	case SENSOR_FEATURE_GET_DELAY_INFO:
	case SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO:
	case SENSOR_FEATURE_SET_MAX_FRAME_RATE_BY_SCENARIO:
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
	case SENSOR_FEATURE_SET_TEST_PATTERN:
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
	case SENSOR_FEATURE_SET_OB_LOCK:
	case SENSOR_FEATURE_SET_SENSOR_OTP_AWB_CMD:
	case SENSOR_FEATURE_SET_SENSOR_OTP_LSC_CMD:
	case SENSOR_FEATURE_GET_TEMPERATURE_VALUE:
	case SENSOR_FEATURE_SET_FRAMERATE:
	case SENSOR_FEATURE_SET_HDR:
	case SENSOR_FEATURE_GET_CROP_INFO:
	case SENSOR_FEATURE_GET_VC_INFO:
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
	case SENSOR_FEATURE_SET_HDR_SHUTTER:
	case SENSOR_FEATURE_GET_AE_FLASHLIGHT_INFO:

	/* return TRUE:play flashlight */
	case SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO:

	/* para: ACDK_SENSOR_3A_LOCK_ENUM */
	case SENSOR_FEATURE_SET_YUV_3A_CMD:
	case SENSOR_FEATURE_SET_AWB_GAIN:
	case SENSOR_FEATURE_SET_MIN_MAX_FPS:
	case SENSOR_FEATURE_GET_PDAF_INFO:
	case SENSOR_FEATURE_GET_PDAF_DATA:
	case SENSOR_FEATURE_GET_4CELL_DATA:
	case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
	case SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY:
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
	case SENSOR_FEATURE_GET_OFFSET_TO_START_OF_EXPOSURE:
	case SENSOR_FEATURE_GET_PIXEL_RATE:
	case SENSOR_FEATURE_SET_PDAF:
	case SENSOR_FEATURE_SET_SHUTTER_FRAME_TIME:
	case SENSOR_FEATURE_SET_PDFOCUS_AREA:
	case SENSOR_FEATURE_GET_PDAF_REG_SETTING:
	case SENSOR_FEATURE_SET_PDAF_REG_SETTING:
	case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
	case SENSOR_FEATURE_SET_STREAMING_RESUME:
		if (copy_from_user(
		    (void *)pFeaturePara,
		    (void *) pFeatureCtrl->pFeaturePara,
		    FeatureParaLen)) {
			kfree(pFeaturePara);
			pr_err(
			    "[CAMERA_HW][pFeaturePara] ioctl copy from user failed\n");
			return -EFAULT;
		}
		break;
	case SENSOR_FEATURE_SET_SENSOR_SYNC:
	case SENSOR_FEATURE_SET_ESHUTTER_GAIN:
		pr_debug("[kd_sensorlist]enter kdSetExpGain\n");
		if (copy_from_user(
		    (void *)pFeaturePara,
		    (void *) pFeatureCtrl->pFeaturePara,
		    FeatureParaLen)) {
			kfree(pFeaturePara);
			pr_err(
			    "[CAMERA_HW][pFeaturePara] ioctl copy from user failed\n");
			return -EFAULT;
		}
		/* keep the information to wait Vsync synchronize */
		pSensorSyncInfo =
		    (struct ACDK_KD_SENSOR_SYNC_STRUCT *)pFeaturePara;

		FeatureParaLen = 2;

		imgsensor_sensor_feature_control(
		    psensor,
		    SENSOR_FEATURE_SET_ESHUTTER,
		    (unsigned char *)&pSensorSyncInfo->u2SensorNewExpTime,
		    (unsigned int *)&FeatureParaLen);

		imgsensor_sensor_feature_control(
		    psensor,
		    SENSOR_FEATURE_SET_GAIN,
		    (unsigned char *)&pSensorSyncInfo->u2SensorNewGain,
		    (unsigned int *) &FeatureParaLen);
		break;

	/* copy to user */
	case SENSOR_FEATURE_GET_RESOLUTION:
	case SENSOR_FEATURE_GET_PERIOD:
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
	case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
	case SENSOR_FEATURE_GET_CONFIG_PARA:
	case SENSOR_FEATURE_GET_GROUP_COUNT:
	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
	/* do nothing */
	case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
	case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
	case SENSOR_FEATURE_SINGLE_FOCUS_MODE:
	case SENSOR_FEATURE_CANCEL_AF:
	case SENSOR_FEATURE_CONSTANT_AF:
	case SENSOR_FEATURE_GET_AE_EFFECTIVE_FRAME_FOR_LE:
	case SENSOR_FEATURE_GET_AE_FRAME_MODE_FOR_LE:
	default:
		break;
	}

	/*in case that some structure are passed from user sapce by ptr */
	switch (pFeatureCtrl->FeatureId) {
	case SENSOR_FEATURE_OPEN:
	case SENSOR_FEATURE_CLOSE:
	case SENSOR_FEATURE_SET_DRIVER:
	case SENSOR_FEATURE_CHECK_IS_ALIVE:
		break;
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
	case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
	case SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY:
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
	case SENSOR_FEATURE_GET_OFFSET_TO_START_OF_EXPOSURE:
	case SENSOR_FEATURE_GET_PIXEL_RATE:
	{
		MUINT32 *pValue = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		pValue = kmalloc(sizeof(MUINT32), GFP_KERNEL);
		if (pValue == NULL) {
			kfree(pFeaturePara);
			pr_err(" ioctl allocate mem failed\n");
			return -ENOMEM;
		}

		memset(pValue, 0x0, sizeof(MUINT32));
		*(pFeaturePara_64 + 1) = (uintptr_t)pValue;

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

		*(pFeaturePara_64 + 1) = *pValue;
		kfree(pValue);
	}
	break;

	case SENSOR_FEATURE_GET_AE_STATUS:
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
	case SENSOR_FEATURE_GET_TEMPERATURE_VALUE:
	case SENSOR_FEATURE_GET_AF_STATUS:
	case SENSOR_FEATURE_GET_AWB_STATUS:
	case SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS:
	case SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS:
	case SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO:
	case SENSOR_FEATURE_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME:
	case SENSOR_FEATURE_GET_PERIOD:
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
	case SENSOR_FEATURE_GET_AE_EFFECTIVE_FRAME_FOR_LE:
	case SENSOR_FEATURE_GET_AE_FRAME_MODE_FOR_LE:
	{
		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);
	}
	break;

	case SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO:
	case SENSOR_FEATURE_AUTOTEST_CMD:
	{
		MUINT32 *pValue0 = NULL;
		MUINT32 *pValue1 = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		pValue0 = kmalloc(sizeof(MUINT32), GFP_KERNEL);
		pValue1 = kmalloc(sizeof(MUINT32), GFP_KERNEL);

		if (pValue0 == NULL || pValue1 == NULL) {
			pr_err(" ioctl allocate mem failed\n");
			kfree(pValue0);
			kfree(pValue1);
			kfree(pFeaturePara);
			return -ENOMEM;
		}
		memset(pValue1, 0x0, sizeof(MUINT32));
		memset(pValue0, 0x0, sizeof(MUINT32));
		*(pFeaturePara_64) = (uintptr_t)pValue0;
		*(pFeaturePara_64 + 1) = (uintptr_t)pValue1;

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

		*(pFeaturePara_64) = *pValue0;
		*(pFeaturePara_64 + 1) = *pValue1;
		kfree(pValue0);
		kfree(pValue1);
	}
	break;

	case SENSOR_FEATURE_GET_EV_AWB_REF:
	{
		struct SENSOR_AE_AWB_REF_STRUCT *pAeAwbRef = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		void *usr_ptr = (void *)(uintptr_t)(*(pFeaturePara_64));

		pAeAwbRef = kmalloc(
		    sizeof(struct SENSOR_AE_AWB_REF_STRUCT),
		    GFP_KERNEL);

		if (pAeAwbRef == NULL) {
			kfree(pFeaturePara);
			pr_err(" ioctl allocate mem failed\n");
			return -ENOMEM;
		}
		memset(
		    pAeAwbRef,
		    0x0,
		    sizeof(struct SENSOR_AE_AWB_REF_STRUCT));

		*(pFeaturePara_64) = (uintptr_t)pAeAwbRef;

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

		if (copy_to_user(
		    (void __user *)usr_ptr,
		    (void *)pAeAwbRef,
		    sizeof(struct SENSOR_AE_AWB_REF_STRUCT))) {
			pr_debug("[CAMERA_HW]ERROR: copy_to_user fail\n");
		}
		kfree(pAeAwbRef);
		*(pFeaturePara_64) = (uintptr_t)usr_ptr;
	}
	break;

	case SENSOR_FEATURE_GET_CROP_INFO:
	{
		struct SENSOR_WINSIZE_INFO_STRUCT *pCrop = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		void *usr_ptr = (void *)(uintptr_t) (*(pFeaturePara_64 + 1));

		pCrop = kmalloc(
		    sizeof(struct SENSOR_WINSIZE_INFO_STRUCT),
		    GFP_KERNEL);

		if (pCrop == NULL) {
			kfree(pFeaturePara);
			pr_err(" ioctl allocate mem failed\n");
			return -ENOMEM;
		}
		memset(pCrop, 0x0, sizeof(struct SENSOR_WINSIZE_INFO_STRUCT));
		*(pFeaturePara_64 + 1) = (uintptr_t)pCrop;

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

		if (copy_to_user(
		    (void __user *)usr_ptr,
		    (void *)pCrop,
		    sizeof(struct SENSOR_WINSIZE_INFO_STRUCT))) {

			pr_debug("[CAMERA_HW]ERROR: copy_to_user fail\n");
		}
		kfree(pCrop);
		*(pFeaturePara_64 + 1) = (uintptr_t)usr_ptr;
	}
	break;

	case SENSOR_FEATURE_GET_VC_INFO:
	{
		struct SENSOR_VC_INFO_STRUCT *pVcInfo = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		void *usr_ptr = (void *)(uintptr_t) (*(pFeaturePara_64 + 1));

		pVcInfo = kmalloc(
		    sizeof(struct SENSOR_VC_INFO_STRUCT),
		    GFP_KERNEL);

		if (pVcInfo == NULL) {
			pr_err(" ioctl allocate mem failed\n");
			kfree(pFeaturePara);
			return -ENOMEM;
		}
		memset(pVcInfo, 0x0, sizeof(struct SENSOR_VC_INFO_STRUCT));
		*(pFeaturePara_64 + 1) = (uintptr_t)pVcInfo;

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

		if (copy_to_user(
		    (void __user *)usr_ptr,
		    (void *)pVcInfo,
		    sizeof(struct SENSOR_VC_INFO_STRUCT))) {
			pr_debug("[CAMERA_HW]ERROR: copy_to_user fail\n");
		}
		kfree(pVcInfo);
		*(pFeaturePara_64 + 1) = (uintptr_t)usr_ptr;
	}
	break;

	case SENSOR_FEATURE_GET_PDAF_INFO:
	{
		struct SET_PD_BLOCK_INFO_T *pPdInfo = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		void *usr_ptr = (void *)(uintptr_t) (*(pFeaturePara_64 + 1));

		pPdInfo = kmalloc(
		    sizeof(struct SET_PD_BLOCK_INFO_T),
		    GFP_KERNEL);

		if (pPdInfo == NULL) {
			kfree(pFeaturePara);
			pr_err(" ioctl allocate mem failed\n");
			return -ENOMEM;
		}
		memset(pPdInfo, 0x0, sizeof(struct SET_PD_BLOCK_INFO_T));
		*(pFeaturePara_64 + 1) = (uintptr_t)pPdInfo;

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

		if (copy_to_user(
		    (void __user *)usr_ptr,
		    (void *)pPdInfo,
		    sizeof(struct SET_PD_BLOCK_INFO_T))) {
			pr_debug("[CAMERA_HW]ERROR: copy_to_user fail\n");
		}
		kfree(pPdInfo);
		*(pFeaturePara_64 + 1) = (uintptr_t)usr_ptr;
	}
	break;

	case SENSOR_FEATURE_GET_PDAF_REG_SETTING:
	case SENSOR_FEATURE_SET_PDAF_REG_SETTING:
	{
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		kal_uint32 u4RegLen = (*pFeaturePara_64);
		void *usr_ptr_Reg =
		    (void *)(uintptr_t) (*(pFeaturePara_64 + 1));
		kal_uint32 *pReg = NULL;

		if (sizeof(kal_uint8) * u4RegLen > PDAF_DATA_SIZE) {
			kfree(pFeaturePara);
			pr_debug("check: u4RegLen > PDAF_DATA_SIZE\n");
			return -EINVAL;
		}
		pReg = kmalloc_array(u4RegLen, sizeof(kal_uint8), GFP_KERNEL);
		if (pReg == NULL) {
			kfree(pFeaturePara);
			pr_err(" ioctl allocate mem failed\n");
			return -ENOMEM;
		}

		memset(pReg, 0x0, sizeof(kal_uint8)*u4RegLen);

		if (copy_from_user(
		    (void *)pReg,
		    (void *)usr_ptr_Reg,
		    sizeof(kal_uint8) * u4RegLen)) {
			pr_err("[CAMERA_HW]ERROR: copy from user fail\n");
		}

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pReg,
		    (unsigned int *)&u4RegLen);

		if (copy_to_user(
		    (void __user *)usr_ptr_Reg,
		    (void *)pReg,
		    sizeof(kal_uint8) * u4RegLen)) {
			pr_debug("[CAMERA_HW]ERROR: copy_to_user fail\n");
		}
		kfree(pReg);
	}

	break;

	case SENSOR_FEATURE_SET_AF_WINDOW:
	case SENSOR_FEATURE_SET_AE_WINDOW:
	{
		MUINT32 *pApWindows = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		void *usr_ptr = (void *)(uintptr_t) (*(pFeaturePara_64));

		pApWindows = kmalloc(sizeof(MUINT32) * 6, GFP_KERNEL);
		if (pApWindows == NULL) {
			kfree(pFeaturePara);
			pr_err(" ioctl allocate mem failed\n");
			return -ENOMEM;
		}
		memset(pApWindows, 0x0, sizeof(MUINT32) * 6);
		*(pFeaturePara_64) = (uintptr_t)pApWindows;

		if (copy_from_user(
		    (void *)pApWindows,
		    (void *)usr_ptr,
		    sizeof(MUINT32) * 6)) {
			pr_err("[CAMERA_HW]ERROR: copy from user fail\n");
		}

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

		kfree(pApWindows);
		*(pFeaturePara_64) = (uintptr_t)usr_ptr;
	}
	break;

	case SENSOR_FEATURE_GET_EXIF_INFO:
	{
		struct SENSOR_EXIF_INFO_STRUCT *pExif = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		void *usr_ptr =  (void *)(uintptr_t) (*(pFeaturePara_64));

		pExif = kmalloc(
		    sizeof(struct SENSOR_EXIF_INFO_STRUCT),
		    GFP_KERNEL);

		if (pExif == NULL) {
			kfree(pFeaturePara);
			pr_err(" ioctl allocate mem failed\n");
			return -ENOMEM;
		}
		memset(pExif, 0x0, sizeof(struct SENSOR_EXIF_INFO_STRUCT));
		*(pFeaturePara_64) = (uintptr_t)pExif;

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

		if (copy_to_user(
		    (void __user *)usr_ptr,
		    (void *)pExif,
		    sizeof(struct SENSOR_EXIF_INFO_STRUCT))) {
			pr_debug("[CAMERA_HW]ERROR: copy_to_user fail\n");
		}
		kfree(pExif);
		*(pFeaturePara_64) = (uintptr_t)usr_ptr;
	}
	break;

	case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
	{

		struct SENSOR_AE_AWB_CUR_STRUCT *pCurAEAWB = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		void *usr_ptr = (void *)(uintptr_t) (*(pFeaturePara_64));

		pCurAEAWB = kmalloc(
		    sizeof(struct SENSOR_AE_AWB_CUR_STRUCT),
		    GFP_KERNEL);

		if (pCurAEAWB == NULL) {
			kfree(pFeaturePara);
			pr_err(" ioctl allocate mem failed\n");
			return -ENOMEM;
		}
		memset(
		    pCurAEAWB,
		    0x0,
		    sizeof(struct SENSOR_AE_AWB_CUR_STRUCT));

		*(pFeaturePara_64) = (uintptr_t)pCurAEAWB;

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)
		    pFeaturePara,
		    (unsigned int *)
		    &FeatureParaLen);

		if (copy_to_user(
		    (void __user *)usr_ptr,
		    (void *)pCurAEAWB,
		    sizeof(struct SENSOR_AE_AWB_CUR_STRUCT))) {
			pr_debug("[CAMERA_HW]ERROR: copy_to_user fail\n");
		}
		kfree(pCurAEAWB);
		*(pFeaturePara_64) = (uintptr_t)usr_ptr;
	}
	break;

	case SENSOR_FEATURE_GET_DELAY_INFO:
	{
		struct SENSOR_DELAY_INFO_STRUCT *pDelayInfo = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		void *usr_ptr = (void *)(uintptr_t) (*(pFeaturePara_64));

		pDelayInfo = kmalloc(
		    sizeof(struct SENSOR_DELAY_INFO_STRUCT),
		    GFP_KERNEL);

		if (pDelayInfo == NULL) {
			kfree(pFeaturePara);
			pr_err(" ioctl allocate mem failed\n");
			return -ENOMEM;
		}
		memset(
		    pDelayInfo,
		    0x0,
		    sizeof(struct SENSOR_DELAY_INFO_STRUCT));

		*(pFeaturePara_64) = (uintptr_t)pDelayInfo;

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

		if (copy_to_user(
		    (void __user *)usr_ptr,
		    (void *)pDelayInfo,
		    sizeof(struct SENSOR_DELAY_INFO_STRUCT))) {
			pr_debug("[CAMERA_HW]ERROR: copy_to_user fail\n");
		}
		kfree(pDelayInfo);
		*(pFeaturePara_64) = (uintptr_t)usr_ptr;

	}
	break;

	case SENSOR_FEATURE_GET_AE_FLASHLIGHT_INFO:
	{
		struct SENSOR_FLASHLIGHT_AE_INFO_STRUCT *pFlashInfo = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *)pFeaturePara;

		void *usr_ptr = (void *)(uintptr_t) (*(pFeaturePara_64));

		pFlashInfo = kmalloc(
		    sizeof(struct SENSOR_FLASHLIGHT_AE_INFO_STRUCT),
		    GFP_KERNEL);

		if (pFlashInfo == NULL) {
			kfree(pFeaturePara);
			pr_err(" ioctl allocate mem failed\n");
			return -ENOMEM;
		}
		memset(
		    pFlashInfo,
		    0x0,
		    sizeof(struct SENSOR_FLASHLIGHT_AE_INFO_STRUCT));

		*(pFeaturePara_64) = (uintptr_t)pFlashInfo;

		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

		if (copy_to_user(
		    (void __user *)usr_ptr,
		    (void *)pFlashInfo,
		    sizeof(struct SENSOR_FLASHLIGHT_AE_INFO_STRUCT))) {
			pr_debug("[CAMERA_HW]ERROR: copy_to_user fail\n");
		}
		kfree(pFlashInfo);
		*(pFeaturePara_64) = (uintptr_t)usr_ptr;

	}
	break;

	case SENSOR_FEATURE_GET_PDAF_DATA:
	case SENSOR_FEATURE_GET_4CELL_DATA:
	{
		char *pPdaf_data = NULL;
		unsigned long long *pFeaturePara_64 =
		    (unsigned long long *) pFeaturePara;
		void *usr_ptr = (void *)(uintptr_t)(*(pFeaturePara_64 + 1));
		kal_uint32 buf_size = (kal_uint32) (*(pFeaturePara_64 + 2));

		if (buf_size > PDAF_DATA_SIZE) {
			kfree(pFeaturePara);
			pr_debug("check: buf_size > PDAF_DATA_SIZE\n");
			return -EINVAL;
		}
		pPdaf_data = kmalloc(
		    sizeof(char) * PDAF_DATA_SIZE,
		    GFP_KERNEL);

		if (pPdaf_data == NULL) {
			kfree(pFeaturePara);
			pr_err(" ioctl allocate mem failed\n");
			return -ENOMEM;
		}
		memset(pPdaf_data, 0xff, sizeof(char) * PDAF_DATA_SIZE);

		if (pFeaturePara_64 != NULL)
			*(pFeaturePara_64 + 1) = (uintptr_t)pPdaf_data;


		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

		if (copy_to_user(
		    (void __user *)usr_ptr,
		    (void *)pPdaf_data,
		    buf_size)) {
			pr_debug("[CAMERA_HW]ERROR: copy_to_user fail\n");
		}
		kfree(pPdaf_data);
		*(pFeaturePara_64 + 1) = (uintptr_t) usr_ptr;
	}
	break;

	default:
		ret = imgsensor_sensor_feature_control(
		    psensor,
		    pFeatureCtrl->FeatureId,
		    (unsigned char *)pFeaturePara,
		    (unsigned int *)&FeatureParaLen);

#ifdef CONFIG_MTK_CCU
		if (pFeatureCtrl->FeatureId == SENSOR_FEATURE_SET_FRAMERATE)
			ccu_set_current_fps(*((int32_t *)pFeaturePara));
#endif
		break;
	}
	/* copy to user */
	switch (pFeatureCtrl->FeatureId) {
	case SENSOR_FEATURE_SET_I2C_BUF_MODE_EN:
		imgsensor_i2c_buffer_mode(
		    (*(unsigned long long *)pFeaturePara));

		break;
	case SENSOR_FEATURE_SET_ESHUTTER:
#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //xen 20170321
                if (pFeatureCtrl->InvokeCamera==IMGSENSOR_SENSOR_IDX_MAIN) //xen 20170325,only main camera need shelter function
                {
                  temp=*(unsigned long long *)pFeaturePara;
                  printk("<xieen>set_shutter=%d\n", (int)temp);
                  main_sensor_set_shutter(temp);
                  break;
                }
#endif
	case SENSOR_FEATURE_SET_GAIN:
#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //xen 20170321
                if (pFeatureCtrl->InvokeCamera==IMGSENSOR_SENSOR_IDX_MAIN) //xen 20170325,only main camera need shelter function
                { 
                  temp=*(unsigned long long *)pFeaturePara;
                  printk("<xieen>set_gain=%d\n", (int)temp/2);
                  main_sensor_set_gain((temp)/2); 
                  if (gFirstTimeAfterPreview==1) //after preview,the first time getvalue not revolved
                  {
                     iSum = 0;
                     main2_light_current = detect_main2_light();
                     gFirstTimeAfterPreview = 0;
                     mod_timer(&main2_detect_timer, jiffies + MAIN2_DETECT_TIMEOUT);
                     bMain2_timer_status = 1; //xen 20170328
                     main2_light_pre = main2_light_current; //current light level set to pre
                     pre_shutter = main_sensor_get_shutter(); //current_shutter;
                     pre_gain = main_sensor_get_gain(); //gain; 
                  }
                  break;
                }
#endif
	case SENSOR_FEATURE_SET_DUAL_GAIN:
	case SENSOR_FEATURE_SET_SHUTTER_BUF_MODE:
	case SENSOR_FEATURE_SET_GAIN_BUF_MODE:
	case SENSOR_FEATURE_SET_GAIN_AND_ESHUTTER:
	case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
	case SENSOR_FEATURE_SET_REGISTER:
	case SENSOR_FEATURE_SET_CCT_REGISTER:
	case SENSOR_FEATURE_SET_ENG_REGISTER:
	case SENSOR_FEATURE_SET_ITEM_INFO:
	/* do nothing */
	case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
	case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
	case SENSOR_FEATURE_GET_PDAF_DATA:
	case SENSOR_FEATURE_GET_4CELL_DATA:
	case SENSOR_FEATURE_GET_PDAF_REG_SETTING:
	case SENSOR_FEATURE_SET_PDAF_REG_SETTING:
	case SENSOR_FEATURE_SET_STREAMING_SUSPEND:
	case SENSOR_FEATURE_SET_STREAMING_RESUME:
		break;
	/* copy to user */
	case SENSOR_FEATURE_SET_DRIVER:
	case SENSOR_FEATURE_GET_EV_AWB_REF:
	case SENSOR_FEATURE_GET_SHUTTER_GAIN_AWB_GAIN:
	case SENSOR_FEATURE_GET_EXIF_INFO:
	case SENSOR_FEATURE_GET_DELAY_INFO:
	case SENSOR_FEATURE_GET_AE_AWB_LOCK_INFO:
	case SENSOR_FEATURE_GET_RESOLUTION:
	case SENSOR_FEATURE_GET_PERIOD:
	case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
	case SENSOR_FEATURE_GET_REGISTER:
	case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
	case SENSOR_FEATURE_GET_CONFIG_PARA:
	case SENSOR_FEATURE_GET_GROUP_COUNT:
	case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
	case SENSOR_FEATURE_GET_ITEM_INFO:
	case SENSOR_FEATURE_GET_ENG_INFO:
	case SENSOR_FEATURE_GET_AF_STATUS:
	case SENSOR_FEATURE_GET_AE_STATUS:
	case SENSOR_FEATURE_GET_AWB_STATUS:
	case SENSOR_FEATURE_GET_AF_INF:
	case SENSOR_FEATURE_GET_AF_MACRO:
	case SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS:

	/* return TRUE:play flashlight */
	case SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO:

	/* para: ACDK_SENSOR_3A_LOCK_ENUM */
	case SENSOR_FEATURE_SET_YUV_3A_CMD:
	case SENSOR_FEATURE_GET_AE_FLASHLIGHT_INFO:
	case SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS:
	case SENSOR_FEATURE_CHECK_SENSOR_ID:
	case SENSOR_FEATURE_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
	case SENSOR_FEATURE_SET_TEST_PATTERN:
	case SENSOR_FEATURE_GET_TEST_PATTERN_CHECKSUM_VALUE:
	case SENSOR_FEATURE_GET_TEMPERATURE_VALUE:
	case SENSOR_FEATURE_SET_FRAMERATE:
	case SENSOR_FEATURE_SET_HDR:
	case SENSOR_FEATURE_SET_IHDR_SHUTTER_GAIN:
	case SENSOR_FEATURE_SET_HDR_SHUTTER:
	case SENSOR_FEATURE_GET_CROP_INFO:
	case SENSOR_FEATURE_GET_VC_INFO:
	case SENSOR_FEATURE_SET_MIN_MAX_FPS:
	case SENSOR_FEATURE_GET_PDAF_INFO:
	case SENSOR_FEATURE_GET_SENSOR_PDAF_CAPACITY:
	case SENSOR_FEATURE_GET_SENSOR_HDR_CAPACITY:
	case SENSOR_FEATURE_GET_MIPI_PIXEL_RATE:
	case SENSOR_FEATURE_GET_OFFSET_TO_START_OF_EXPOSURE:
	case SENSOR_FEATURE_GET_PIXEL_RATE:
	case SENSOR_FEATURE_SET_ISO:
	case SENSOR_FEATURE_SET_PDAF:
	case SENSOR_FEATURE_SET_SHUTTER_FRAME_TIME:
	case SENSOR_FEATURE_SET_PDFOCUS_AREA:
	case SENSOR_FEATURE_GET_AE_EFFECTIVE_FRAME_FOR_LE:
	case SENSOR_FEATURE_GET_AE_FRAME_MODE_FOR_LE:
		if (copy_to_user(
		    (void __user *) pFeatureCtrl->pFeaturePara,
		    (void *)pFeaturePara,
		    FeatureParaLen)) {

			kfree(pFeaturePara);
			pr_debug(
			    "[CAMERA_HW][pSensorRegData] ioctl copy to user failed\n");
			return -EFAULT;
		}
		break;

	default:
		break;
	}

	kfree(pFeaturePara);
	if (copy_to_user(
	    (void __user *) pFeatureCtrl->pFeatureParaLen,
	    (void *)&FeatureParaLen,
	    sizeof(unsigned int))) {

		pr_debug(
		    "[CAMERA_HW][pFeatureParaLen] ioctl copy to user failed\n");

		return -EFAULT;
	}

	return ret;
}   /* adopt_CAMERA_HW_FeatureControl() */

#ifdef CONFIG_COMPAT
static int compat_get_acdk_sensor_getinfo_struct(
	struct COMPAT_IMGSENSOR_GET_CONFIG_INFO_STRUCT __user *data32,
	struct IMGSENSOR_GET_CONFIG_INFO_STRUCT __user *data)
{
	compat_uint_t i;
	compat_uptr_t p;
	int err;

	err = get_user(i, &data32->SensorId);
	err |= put_user(i, &data->SensorId);
	err = get_user(i, &data32->ScenarioId);
	err |= put_user(i, &data->ScenarioId);
	err = get_user(p, &data32->pInfo);
	err |= put_user(compat_ptr(p), &data->pInfo);
	err = get_user(p, &data32->pConfig);
	err |= put_user(compat_ptr(p), &data->pConfig);

	return err;
}

static int compat_put_acdk_sensor_getinfo_struct(
	struct COMPAT_IMGSENSOR_GET_CONFIG_INFO_STRUCT __user *data32,
	struct IMGSENSOR_GET_CONFIG_INFO_STRUCT __user *data)
{
	compat_uint_t i;
	int err;

	err = get_user(i, &data32->SensorId);
	err |= put_user(i, &data->SensorId);
	err = get_user(i, &data->ScenarioId);
	err |= put_user(i, &data32->ScenarioId);
	err = get_user(i, &data->ScenarioId);
	err |= put_user(i, &data32->ScenarioId);
	return err;
}

static int compat_get_imagesensor_getinfo_struct(
	struct COMPAT_IMAGESENSOR_GETINFO_STRUCT __user *data32,
	struct IMAGESENSOR_GETINFO_STRUCT __user *data)
{
	compat_uptr_t p;
	compat_uint_t i;
	int err;

	err = get_user(i, &data32->SensorId);
	err |= put_user(i, &data->SensorId);
	err |= get_user(p, &data32->pInfo);
	err |= put_user(compat_ptr(p), &data->pInfo);
	err |= get_user(p, &data32->pSensorResolution);
	err |= put_user(compat_ptr(p), &data->pSensorResolution);
	return err;
}

static int compat_put_imagesensor_getinfo_struct(
	struct COMPAT_IMAGESENSOR_GETINFO_STRUCT __user *data32,
	struct IMAGESENSOR_GETINFO_STRUCT __user *data)
{
	/* compat_uptr_t p; */
	compat_uint_t i;
	int err;

	err = get_user(i, &data->SensorId);
	err |= put_user(i, &data32->SensorId);

	return err;
}

static int compat_get_acdk_sensor_featurecontrol_struct(
	struct COMPAT_ACDK_SENSOR_FEATURECONTROL_STRUCT
	__user *data32,
	struct ACDK_SENSOR_FEATURECONTROL_STRUCT __user *
	data)
{
	compat_uptr_t p;
	compat_uint_t i;
	int err;

	err = get_user(i, &data32->InvokeCamera);
	err |= put_user(i, &data->InvokeCamera);
	err |= get_user(i, &data32->FeatureId);
	err |= put_user(i, &data->FeatureId);
	err |= get_user(p, &data32->pFeaturePara);
	err |= put_user(compat_ptr(p), &data->pFeaturePara);
	err |= get_user(p, &data32->pFeatureParaLen);
	err |= put_user(compat_ptr(p), &data->pFeatureParaLen);
	return err;
}

static int compat_put_acdk_sensor_featurecontrol_struct(
	struct COMPAT_ACDK_SENSOR_FEATURECONTROL_STRUCT
	__user *data32,
	struct ACDK_SENSOR_FEATURECONTROL_STRUCT __user *
	data)
{
	MUINT8 *p;
	MUINT32 *q;
	compat_uint_t i;
	int err;

	err = get_user(i, &data->InvokeCamera);
	err |= put_user(i, &data32->InvokeCamera);
	err |= get_user(i, &data->FeatureId);
	err |= put_user(i, &data32->FeatureId);
	/* Assume pointer is not change */

	err |= get_user(p, &data->pFeaturePara);
	err |= put_user(ptr_to_compat(p), &data32->pFeaturePara);
	err |= get_user(q, &data->pFeatureParaLen);
	err |= put_user(ptr_to_compat(q), &data32->pFeatureParaLen);

	return err;
}

static int compat_get_acdk_sensor_control_struct(
	struct COMPAT_ACDK_SENSOR_CONTROL_STRUCT __user *data32,
	struct ACDK_SENSOR_CONTROL_STRUCT __user *data)
{
	compat_uptr_t p;
	compat_uint_t i;
	int err;

	err = get_user(i, &data32->InvokeCamera);
	err |= put_user(i, &data->InvokeCamera);
	err |= get_user(i, &data32->ScenarioId);
	err |= put_user(i, &data->ScenarioId);
	err |= get_user(p, &data32->pImageWindow);
	err |= put_user(compat_ptr(p), &data->pImageWindow);
	err |= get_user(p, &data32->pSensorConfigData);
	err |= put_user(compat_ptr(p), &data->pSensorConfigData);
	return err;
}

static int compat_put_acdk_sensor_control_struct(
	struct COMPAT_ACDK_SENSOR_CONTROL_STRUCT __user *data32,
	struct ACDK_SENSOR_CONTROL_STRUCT __user *data)
{
	/* compat_uptr_t p; */
	compat_uint_t i;
	int err;

	err = get_user(i, &data->InvokeCamera);
	err |= put_user(i, &data32->InvokeCamera);
	err |= get_user(i, &data->ScenarioId);
	err |= put_user(i, &data32->ScenarioId);

	return err;
}

static long
imgsensor_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret;

	if (!filp->f_op || !filp->f_op->unlocked_ioctl)
		return -ENOTTY;

	switch (cmd) {
	case COMPAT_KDIMGSENSORIOC_X_GETINFO:
	{
		struct COMPAT_IMGSENSOR_GET_CONFIG_INFO_STRUCT __user *data32;
		struct IMGSENSOR_GET_CONFIG_INFO_STRUCT __user *data;
		int err;
		/*pr_debug("CAOMPAT_KDIMGSENSORIOC_X_GETINFO E\n"); */

		data32 = compat_ptr(arg);
		data = compat_alloc_user_space(sizeof(*data));
		if (data == NULL)
			return -EFAULT;

		err = compat_get_acdk_sensor_getinfo_struct(data32, data);
		if (err)
			return err;

		ret = filp->f_op->unlocked_ioctl(
		    filp,
		    KDIMGSENSORIOC_X_GET_CONFIG_INFO,
		    (unsigned long)data);

		err = compat_put_acdk_sensor_getinfo_struct(data32, data);

		if (err != 0)
			pr_debug(
			    "[CAMERA SENSOR] compat_put_acdk_sensor_getinfo_struct failed\n");
		return ret;
	}
	case COMPAT_KDIMGSENSORIOC_X_FEATURECONCTROL:
	{
		struct COMPAT_ACDK_SENSOR_FEATURECONTROL_STRUCT __user *data32;
		struct ACDK_SENSOR_FEATURECONTROL_STRUCT __user *data;
		int err;

		/* pr_debug("CAOMPAT_KDIMGSENSORIOC_X_FEATURECONCTROL\n"); */

		data32 = compat_ptr(arg);
		data = compat_alloc_user_space(sizeof(*data));
		if (data == NULL)
			return -EFAULT;

		err = compat_get_acdk_sensor_featurecontrol_struct(
		    data32,
		    data);

		if (err)
			return err;

		ret = filp->f_op->unlocked_ioctl(
		    filp,
		    KDIMGSENSORIOC_X_FEATURECONCTROL,
		    (unsigned long)data);

		err = compat_put_acdk_sensor_featurecontrol_struct(
		    data32,
		    data);

		if (err != 0)
			pr_err(
			    "[CAMERA SENSOR] compat_put_acdk_sensor_getinfo_struct failed\n");
		return ret;
	}
	case COMPAT_KDIMGSENSORIOC_X_CONTROL:
	{
		struct COMPAT_ACDK_SENSOR_CONTROL_STRUCT __user *data32;
		struct ACDK_SENSOR_CONTROL_STRUCT __user *data;
		int err;

		pr_debug("[CAMERA SENSOR] CAOMPAT_KDIMGSENSORIOC_X_CONTROL\n");

		data32 = compat_ptr(arg);
		data = compat_alloc_user_space(sizeof(*data));
		if (data == NULL)
			return -EFAULT;

		err = compat_get_acdk_sensor_control_struct(data32, data);
		if (err)
			return err;
		ret = filp->f_op->unlocked_ioctl(
		    filp,
		    KDIMGSENSORIOC_X_CONTROL,
		    (unsigned long)data);

		err = compat_put_acdk_sensor_control_struct(data32, data);

		if (err != 0)
			pr_err(
			    "[CAMERA SENSOR] compat_put_acdk_sensor_getinfo_struct failed\n");
		return ret;
	}
	case COMPAT_KDIMGSENSORIOC_X_GETINFO2:
	{
		struct COMPAT_IMAGESENSOR_GETINFO_STRUCT __user *data32;
		struct IMAGESENSOR_GETINFO_STRUCT __user *data;
		int err;

		pr_debug("[CAMERA SENSOR] CAOMPAT_KDIMGSENSORIOC_X_GETINFO2\n");

		data32 = compat_ptr(arg);
		data = compat_alloc_user_space(sizeof(*data));
		if (data == NULL)
			return -EFAULT;

		err = compat_get_imagesensor_getinfo_struct(data32, data);
		if (err)
			return err;
		ret = filp->f_op->unlocked_ioctl(
		    filp,
		    KDIMGSENSORIOC_X_GETINFO2,
		    (unsigned long)data);

		err = compat_put_imagesensor_getinfo_struct(data32, data);

		if (err != 0)
			pr_err(
			    "[CAMERA SENSOR] compat_put_acdk_sensor_getinfo_struct failed\n");
		return ret;
	}
	case COMPAT_KDIMGSENSORIOC_X_GETRESOLUTION2:
	{
		return 0;
	}

	default:
		return filp->f_op->unlocked_ioctl(filp, cmd, arg);
	}
}
#endif

/************************************************************************
 * imgsensor_ioctl
 ************************************************************************/
static long imgsensor_ioctl(
	struct file *a_pstFile,
	unsigned int a_u4Command,
	unsigned long a_u4Param)
{
	int i4RetValue = 0;
	void *pBuff = NULL;

	if (_IOC_DIR(a_u4Command) != _IOC_NONE) {
		pBuff = kmalloc(_IOC_SIZE(a_u4Command), GFP_KERNEL);
		if (pBuff == NULL) {
			pr_debug("[CAMERA SENSOR] ioctl allocate mem failed\n");
			i4RetValue = -ENOMEM;
			goto CAMERA_HW_Ioctl_EXIT;
		}

		if (_IOC_WRITE & _IOC_DIR(a_u4Command)) {
			if (copy_from_user(
			    pBuff,
			    (void *)a_u4Param,
			    _IOC_SIZE(a_u4Command))) {

				kfree(pBuff);
				pr_debug(
				    "[CAMERA SENSOR] ioctl copy from user failed\n");
				i4RetValue =  -EFAULT;
				goto CAMERA_HW_Ioctl_EXIT;
			}
		}
	} else {
		i4RetValue =  -EFAULT;
		goto CAMERA_HW_Ioctl_EXIT;
	}

	switch (a_u4Command) {
	case KDIMGSENSORIOC_X_GET_CONFIG_INFO:
		i4RetValue = adopt_CAMERA_HW_GetInfo(pBuff);
		break;
	case KDIMGSENSORIOC_X_GETINFO2:
		i4RetValue = adopt_CAMERA_HW_GetInfo2(pBuff);
		break;
	case KDIMGSENSORIOC_X_FEATURECONCTROL:
		i4RetValue = adopt_CAMERA_HW_FeatureControl(pBuff);
		break;
	case KDIMGSENSORIOC_X_CONTROL:
		i4RetValue = adopt_CAMERA_HW_Control(pBuff);
		break;
	case KDIMGSENSORIOC_X_SET_MCLK_PLL:
		i4RetValue = imgsensor_clk_set(
		    &pgimgsensor->clk,
		    (struct ACDK_SENSOR_MCLK_STRUCT *)pBuff);
		break;
	case KDIMGSENSORIOC_X_GET_ISP_CLK:
/*E1(High):490, (Medium):364, (low):273*/
#define ISP_CLK_LOW    273
#define ISP_CLK_MEDIUM 364
#define ISP_CLK_HIGH   490
#ifdef CONFIG_MTK_SMI_EXT
		pr_debug(
		"KDIMGSENSORIOC_X_GET_ISP_CLK current_mmsys_clk=%d\n",
		current_mmsys_clk);

		if (mmdvfs_get_stable_isp_clk() == MMSYS_CLK_HIGH)
			*(unsigned int *)pBuff = ISP_CLK_HIGH;
		else if (mmdvfs_get_stable_isp_clk() == MMSYS_CLK_MEDIUM)
			*(unsigned int *)pBuff = ISP_CLK_MEDIUM;
		else
			*(unsigned int *)pBuff = ISP_CLK_LOW;
#else
		*(unsigned int *)pBuff = ISP_CLK_HIGH;
#endif
		break;
	case KDIMGSENSORIOC_X_GET_CSI_CLK:
		i4RetValue = imgsensor_clk_ioctrl_handler(pBuff);
		break;

	/*mmdvfs start*/
#ifdef IMGSENSOR_DFS_CTRL_ENABLE
	case KDIMGSENSORIOC_DFS_UPDATE:
		i4RetValue = imgsensor_dfs_ctrl(DFS_UPDATE, pBuff);
		break;
	case KDIMGSENSORIOC_GET_SUPPORTED_ISP_CLOCKS:
		i4RetValue = imgsensor_dfs_ctrl(
						DFS_SUPPORTED_ISP_CLOCKS,
						pBuff);
		break;
	case KDIMGSENSORIOC_GET_CUR_ISP_CLOCK:
		i4RetValue = imgsensor_dfs_ctrl(DFS_CUR_ISP_CLOCK, pBuff);
		break;
#endif
	/*mmdvfs end*/
	case KDIMGSENSORIOC_T_OPEN:
	case KDIMGSENSORIOC_T_CLOSE:
	case KDIMGSENSORIOC_T_CHECK_IS_ALIVE:
	case KDIMGSENSORIOC_X_SET_DRIVER:
	case KDIMGSENSORIOC_X_GETRESOLUTION2:
	case KDIMGSENSORIOC_X_GET_SOCKET_POS:
	case KDIMGSENSORIOC_X_SET_GPIO:
	case KDIMGSENSORIOC_X_SET_I2CBUS:
	case KDIMGSENSORIOC_X_RELEASE_I2C_TRIGGER_LOCK:
	case KDIMGSENSORIOC_X_SET_SHUTTER_GAIN_WAIT_DONE:
	case KDIMGSENSORIOC_X_SET_CURRENT_SENSOR:
		i4RetValue = 0;
		break;
	default:
		pr_debug("No such command %d\n", a_u4Command);
		i4RetValue = -EPERM;
		break;
	}

	if ((_IOC_READ & _IOC_DIR(a_u4Command)) &&
		    copy_to_user((void __user *) a_u4Param,
						  pBuff,
						_IOC_SIZE(a_u4Command))) {
		kfree(pBuff);
		pr_debug("[CAMERA SENSOR] ioctl copy to user failed\n");
		i4RetValue =  -EFAULT;
		goto CAMERA_HW_Ioctl_EXIT;
	}

	kfree(pBuff);
CAMERA_HW_Ioctl_EXIT:
	return i4RetValue;
}

static int imgsensor_open(struct inode *a_pstInode, struct file *a_pstFile)
{
	if (atomic_read(&pgimgsensor->imgsensor_open_cnt) == 0)
		imgsensor_clk_enable_all(&pgimgsensor->clk);

	atomic_inc(&pgimgsensor->imgsensor_open_cnt);
	pr_info(
	    "%s %d\n",
	    __func__,
	    atomic_read(&pgimgsensor->imgsensor_open_cnt));
	return 0;
}

static int imgsensor_release(struct inode *a_pstInode, struct file *a_pstFile)
{
	enum IMGSENSOR_SENSOR_IDX i = IMGSENSOR_SENSOR_IDX_MIN_NUM;
	atomic_dec(&pgimgsensor->imgsensor_open_cnt);
	if (atomic_read(&pgimgsensor->imgsensor_open_cnt) == 0) {
		imgsensor_clk_disable_all(&pgimgsensor->clk);

		if (pgimgsensor->imgsensor_oc_irq_enable != NULL) {
			for (; i < IMGSENSOR_SENSOR_IDX_MAX_NUM; i++)
				pgimgsensor->imgsensor_oc_irq_enable(i, false);
		}

		imgsensor_hw_release_all(&pgimgsensor->hw);
#ifdef IMGSENSOR_DFS_CTRL_ENABLE
		imgsensor_dfs_ctrl(DFS_RELEASE, NULL);
#endif
	}
	pr_info(
	    "%s %d\n",
	    __func__,
	    atomic_read(&pgimgsensor->imgsensor_open_cnt));
	return 0;
}

static const struct file_operations gimgsensor_file_operations = {
	.owner = THIS_MODULE,
	.open = imgsensor_open,
	.release = imgsensor_release,
	.unlocked_ioctl = imgsensor_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = imgsensor_compat_ioctl
#endif
};

static inline int imgsensor_driver_register(void)
{
	dev_t dev_no = MKDEV(IMGSENSOR_DEVICE_NNUMBER, 0);

	if (alloc_chrdev_region(&dev_no, 0, 1, IMGSENSOR_DEV_NAME)) {
		pr_debug("[CAMERA SENSOR] Allocate device no failed\n");

		return -EAGAIN;
	}

	/* Allocate driver */
	gpimgsensor_cdev = cdev_alloc();
	if (gpimgsensor_cdev ==  NULL) {
		unregister_chrdev_region(dev_no, 1);
		pr_debug("[CAMERA SENSOR] Allocate mem for kobject failed\n");
		return -ENOMEM;
	}

	/* Attatch file operation. */
	cdev_init(gpimgsensor_cdev, &gimgsensor_file_operations);

	gpimgsensor_cdev->owner = THIS_MODULE;

	/* Add to system */
	if (cdev_add(gpimgsensor_cdev, dev_no, 1)) {
		pr_debug("Attatch file operation failed\n");
		unregister_chrdev_region(dev_no, 1);
		return -EAGAIN;
	}

	gpimgsensor_class = class_create(THIS_MODULE, "sensordrv");
	if (IS_ERR(gpimgsensor_class)) {
		int ret = PTR_ERR(gpimgsensor_class);

		pr_debug("Unable to create class, err = %d\n", ret);
		return ret;
	}

	gimgsensor_device =
	    device_create(
		    gpimgsensor_class,
		    NULL,
		    dev_no,
		    NULL,
		    IMGSENSOR_DEV_NAME);

	return 0;
}

static inline void imgsensor_driver_unregister(void)
{
	/* Release char driver */
	cdev_del(gpimgsensor_cdev);

	unregister_chrdev_region(MKDEV(IMGSENSOR_DEVICE_NNUMBER, 0), 1);

	device_destroy(gpimgsensor_class, MKDEV(IMGSENSOR_DEVICE_NNUMBER, 0));
	class_destroy(gpimgsensor_class);
}

#ifdef CONFIG_MTK_SMI_EXT
int mmsys_clk_change_cb(int ori_clk_mode, int new_clk_mode)
{
	/*pr_debug("mmsys_clk_change_cb ori:%d, new:%d, current_mmsys_clk %d\n",
	 *		ori_clk_mode,
	 *		new_clk_mode,
	 *		current_mmsys_clk);
	 */
	current_mmsys_clk = new_clk_mode;
	return 1;
}
#endif

static int imgsensor_probe(struct platform_device *pdev)
{
	/* Register char driver */
	if (imgsensor_driver_register()) {
		pr_err("[CAMERA_HW] register char device failed!\n");
		return -1;
	}

	gpimgsensor_hw_platform_device = pdev;

#ifndef CONFIG_FPGA_EARLY_PORTING
	imgsensor_clk_init(&pgimgsensor->clk);
#endif
	imgsensor_hw_init(&pgimgsensor->hw);
	imgsensor_i2c_create();
	imgsensor_proc_init();

#if 1 //added by xen for camera information 20171030
	memset(mtk_main_cam_name,0,cam_info_size);
	memset(mtk_sub_cam_name,0,cam_info_size);
	proc_create(PROC_MAIN_CAM_INFO, 0, NULL, &cam_proc_fops_main);
	proc_create(PROC_SUB_CAM_INFO, 0, NULL, &cam_proc_fops_sub);
	memset(mtk_main2_cam_name,0,cam_info_size);
	proc_create(PROC_MAIN2_CAM_INFO, 0, NULL, &cam_proc_fops_main2);
#endif

	atomic_set(&pgimgsensor->imgsensor_open_cnt, 0);
#ifdef CONFIG_MTK_SMI_EXT
	mmdvfs_register_mmclk_switch_cb(
	    mmsys_clk_change_cb,
	    MMDVFS_CLIENT_ID_ISP);
#endif

#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //timer design
	init_timer(&main2_detect_timer);
	main2_detect_timer.expires = jiffies + MAIN2_DETECT_TIMEOUT;
	main2_detect_timer.function = &main2_detect_timeout;
	main2_detect_timer.data = ((unsigned long)0);

	main2_detect_workqueue = create_singlethread_workqueue("main2_detect"); //xen 20170308
	INIT_WORK(&main2_detect_work, main2_detect_work_callback);
#endif

	return 0;
}
/////////
#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //timer design

//#define SP0A38_SENSOR_ID                0x0a10            
//for SP0A20 main2 camera
#define SP0A20_WRITE_ID							        0x42
#define SP0A20_READ_ID								0x43
static kal_uint16 write_cmos_sensor_sp0a20(kal_uint8 addr, kal_uint8 para)
{
	char puSendCmd[2] = {(char)(addr & 0xFF) , (char)(para & 0xFF)};
        #if 1
	iWriteRegI2C6(puSendCmd , 2, SP0A20_WRITE_ID);
	#else
        iWriteRegI2C(puSendCmd , 2, SP0A20_WRITE_ID);
        #endif
        return 0;
}

static kal_uint16 read_cmos_sensor_sp0a20(kal_uint8 addr)
{
	kal_uint16 get_byte=0;
	char puSendCmd = { (char)(addr & 0xFF) };
        #if 1
        iReadRegI2C6(&puSendCmd , 1, (u8*)&get_byte, 1, SP0A20_WRITE_ID);
        #else
	iReadRegI2C(&puSendCmd , 1, (u8*)&get_byte, 1, SP0A20_WRITE_ID);
	#endif
	return get_byte;
}

static void sp0a20_init(void)
{
	write_cmos_sensor_sp0a20(0xfd , 0x01);
	write_cmos_sensor_sp0a20(0x36 , 0x02);
	write_cmos_sensor_sp0a20(0xfd , 0x00);
	write_cmos_sensor_sp0a20(0xe7 , 0x00);
	write_cmos_sensor_sp0a20(0xe7 , 0x00);
  
	write_cmos_sensor_sp0a20(0xfd , 0x00);
	write_cmos_sensor_sp0a20(0x0c , 0x00);
	write_cmos_sensor_sp0a20(0x92 , 0x70);
	write_cmos_sensor_sp0a20(0x1b , 0x27);
	write_cmos_sensor_sp0a20(0x12 , 0x02);
	write_cmos_sensor_sp0a20(0x13 , 0x2f);
	write_cmos_sensor_sp0a20(0x6d , 0x32);
	write_cmos_sensor_sp0a20(0x6c , 0x32);
	write_cmos_sensor_sp0a20(0x6f , 0x33);
	write_cmos_sensor_sp0a20(0x6e , 0x34);
	write_cmos_sensor_sp0a20(0x99 , 0x04);
	write_cmos_sensor_sp0a20(0x16 , 0x38);
	write_cmos_sensor_sp0a20(0x17 , 0x38);
	write_cmos_sensor_sp0a20(0x70 , 0x3a);
	write_cmos_sensor_sp0a20(0x14 , 0x02);
	write_cmos_sensor_sp0a20(0x15 , 0x20);
	write_cmos_sensor_sp0a20(0x71 , 0x23);
	write_cmos_sensor_sp0a20(0x69 , 0x25);
	write_cmos_sensor_sp0a20(0x6a , 0x1a);
	write_cmos_sensor_sp0a20(0x72 , 0x1c);
	write_cmos_sensor_sp0a20(0x75 , 0x1e);
	write_cmos_sensor_sp0a20(0x73 , 0x3c);
	write_cmos_sensor_sp0a20(0x74 , 0x21);
	write_cmos_sensor_sp0a20(0x79 , 0x00);
	write_cmos_sensor_sp0a20(0x77 , 0x10);
	write_cmos_sensor_sp0a20(0x1a , 0x4d);
	write_cmos_sensor_sp0a20(0x1c , 0x07);
	write_cmos_sensor_sp0a20(0x1e , 0x15);
	write_cmos_sensor_sp0a20(0x21 , 0x08);
	write_cmos_sensor_sp0a20(0x22 , 0x28);
	write_cmos_sensor_sp0a20(0x26 , 0x66);
	write_cmos_sensor_sp0a20(0x28 , 0x0b); 
	write_cmos_sensor_sp0a20(0x37 , 0x5a); //  4a
	//pre_gain           
	write_cmos_sensor_sp0a20(0xfd , 0x02);
	write_cmos_sensor_sp0a20(0x01 , 0x80);
	write_cmos_sensor_sp0a20(0x52 , 0x10);
	write_cmos_sensor_sp0a20(0x54 , 0x00);
	//blacklevel
	write_cmos_sensor_sp0a20(0xfd , 0x01);//blacklevel
	write_cmos_sensor_sp0a20(0x41 , 0x00);
	write_cmos_sensor_sp0a20(0x42 , 0x00);
	write_cmos_sensor_sp0a20(0x43 , 0x00);
	write_cmos_sensor_sp0a20(0x44 , 0x00);
	//ae setting 24M 7-15fps
	write_cmos_sensor_sp0a20(0xfd , 0x00);
	write_cmos_sensor_sp0a20(0x03 , 0x01);
	write_cmos_sensor_sp0a20(0x04 , 0xc2);
	write_cmos_sensor_sp0a20(0x05 , 0x00);
	write_cmos_sensor_sp0a20(0x06 , 0x00);
	write_cmos_sensor_sp0a20(0x07 , 0x00);
	write_cmos_sensor_sp0a20(0x08 , 0x00);
	write_cmos_sensor_sp0a20(0x09 , 0x02);
	write_cmos_sensor_sp0a20(0x0a , 0xf4);
	write_cmos_sensor_sp0a20(0xfd , 0x01);
	write_cmos_sensor_sp0a20(0xf0 , 0x00);
	write_cmos_sensor_sp0a20(0xf7 , 0x4b);
	write_cmos_sensor_sp0a20(0x02 , 0x0e);
	write_cmos_sensor_sp0a20(0x03 , 0x01);
	write_cmos_sensor_sp0a20(0x06 , 0x4b);
	write_cmos_sensor_sp0a20(0x07 , 0x00);
	write_cmos_sensor_sp0a20(0x08 , 0x01);
	write_cmos_sensor_sp0a20(0x09 , 0x00);
	write_cmos_sensor_sp0a20(0xfd , 0x02);
	write_cmos_sensor_sp0a20(0xbe , 0x1a);
	write_cmos_sensor_sp0a20(0xbf , 0x04);
	write_cmos_sensor_sp0a20(0xd0 , 0x1a);
	write_cmos_sensor_sp0a20(0xd1 , 0x04);

	//ae gain &status
	write_cmos_sensor_sp0a20(0xfd , 0x01);
	write_cmos_sensor_sp0a20(0x5a , 0x40);//dp rpc   
	write_cmos_sensor_sp0a20(0xfd , 0x02);
	write_cmos_sensor_sp0a20(0xbc , 0x70);//rpc_heq_low
	write_cmos_sensor_sp0a20(0xbd , 0x50);//rpc_heq_dummy
	write_cmos_sensor_sp0a20(0xb8 , 0x66);//mean_normal_dummy
	write_cmos_sensor_sp0a20(0xb9 , 0x88);//mean_dummy_normal
	write_cmos_sensor_sp0a20(0xba , 0x30);//mean_dummy_low
	write_cmos_sensor_sp0a20(0xbb , 0x45);//mean low_dummy

	//rpc
	write_cmos_sensor_sp0a20(0xfd , 0x01);//rpc                   
	write_cmos_sensor_sp0a20(0xe0 , 0x60);//0x4c;rpc_1base_max
	write_cmos_sensor_sp0a20(0xe1 , 0x48);//0x3c;rpc_2base_max
	write_cmos_sensor_sp0a20(0xe2 , 0x40);//0x34;rpc_3base_max
	write_cmos_sensor_sp0a20(0xe3 , 0x3a);//0x2e;rpc_4base_max
	write_cmos_sensor_sp0a20(0xe4 , 0x3a);//0x2e;rpc_5base_max
	write_cmos_sensor_sp0a20(0xe5 , 0x38);//0x2c;rpc_6base_max
	write_cmos_sensor_sp0a20(0xe6 , 0x38);//0x2c;rpc_7base_max
	write_cmos_sensor_sp0a20(0xe7 , 0x34);//0x2a;rpc_8base_max
	write_cmos_sensor_sp0a20(0xe8 , 0x34);//0x2a;rpc_9base_max
	write_cmos_sensor_sp0a20(0xe9 , 0x34);//0x2a;rpc_10base_max
	write_cmos_sensor_sp0a20(0xea , 0x32);//0x28;rpc_11base_max
	write_cmos_sensor_sp0a20(0xf3 , 0x32);//0x28;rpc_12base_max
	write_cmos_sensor_sp0a20(0xf4 , 0x32);//0x28;rpc_13base_max
	//ae min gain  
	write_cmos_sensor_sp0a20(0xfd , 0x01);
	write_cmos_sensor_sp0a20(0x04 , 0xa0);//rpc_max_indr
	write_cmos_sensor_sp0a20(0x05 , 0x32);//rpc_min_indr 
	write_cmos_sensor_sp0a20(0x0a , 0xa0);//rpc_max_outdr
	write_cmos_sensor_sp0a20(0x0b , 0x32);//rpc_min_outdr

	//target
	write_cmos_sensor_sp0a20(0xfd , 0x01);
	write_cmos_sensor_sp0a20(0xeb , 0x78);//target indr
	write_cmos_sensor_sp0a20(0xec , 0x78);//target outdr
	write_cmos_sensor_sp0a20(0xed , 0x05);//lock range
	write_cmos_sensor_sp0a20(0xee , 0x0c);//hold range

	write_cmos_sensor_sp0a20(0xfd , 0x01);
	write_cmos_sensor_sp0a20(0xf2 , 0x4d);
	write_cmos_sensor_sp0a20(0xfd , 0x02);
	write_cmos_sensor_sp0a20(0x5b , 0x05);//dp status
	write_cmos_sensor_sp0a20(0x5c , 0xa0);

	write_cmos_sensor_sp0a20(0xfd , 0x02);//sharp
	write_cmos_sensor_sp0a20(0xde , 0x00);

	write_cmos_sensor_sp0a20(0xfd , 0x02); 
	write_cmos_sensor_sp0a20(0xdd , 0x00);//enable

	//low_lum_offset
	write_cmos_sensor_sp0a20(0xfd , 0x01);
	write_cmos_sensor_sp0a20(0xcd , 0x10);
	write_cmos_sensor_sp0a20(0xce , 0x1f);
	write_cmos_sensor_sp0a20(0xcf , 0x30);
	write_cmos_sensor_sp0a20(0xd0 , 0x45);

	//gw
	write_cmos_sensor_sp0a20(0xfd , 0x02);//low_lum_offset
	write_cmos_sensor_sp0a20(0x31 , 0x60);
	write_cmos_sensor_sp0a20(0x32 , 0x60);
	write_cmos_sensor_sp0a20(0x33 , 0xc0);
	write_cmos_sensor_sp0a20(0x35 , 0x60);
	write_cmos_sensor_sp0a20(0x37 , 0x13);

	write_cmos_sensor_sp0a20(0xfd , 0x00);
	write_cmos_sensor_sp0a20(0x31,0x00);//06
	write_cmos_sensor_sp0a20(0xfd,0x01);
	write_cmos_sensor_sp0a20(0x32,0x00);//0x15,disable AEC
	write_cmos_sensor_sp0a20(0x33,0x00);//0xef
	write_cmos_sensor_sp0a20(0x34,0x00);//0x7
	write_cmos_sensor_sp0a20(0xd2,0x01);
	write_cmos_sensor_sp0a20(0xfb,0x25);
	write_cmos_sensor_sp0a20(0xf2,0x49);
	write_cmos_sensor_sp0a20(0x35,0x40);
	write_cmos_sensor_sp0a20(0x5d,0x11);
	write_cmos_sensor_sp0a20(0xfd , 0x01);
	write_cmos_sensor_sp0a20(0x36 , 0x00);
	write_cmos_sensor_sp0a20(0xfd , 0x00);
	write_cmos_sensor_sp0a20(0x92 , 0x01);
	write_cmos_sensor_sp0a20(0xfd , 0x00);//out en
}

//for GC8024 main2 camera
#define GC8024_WRITE_ID			0x6e
static kal_uint16 read_cmos_sensor_gc8024(kal_uint32 addr)
{
	kal_uint16 get_byte=0;

	char pu_send_cmd[1] = {(char)(addr & 0xFF) };
        #if 1
        iReadRegI2C6(pu_send_cmd, 1, (u8*)&get_byte, 1, GC8024_WRITE_ID);//imgsensor_gc8024.i2c_write_id);
        #else
	iReadRegI2C(pu_send_cmd, 1, (u8*)&get_byte, 1, GC8024_WRITE_ID);//imgsensor_gc8024.i2c_write_id);
        #endif
	return get_byte;

}

static void write_cmos_sensor_gc8024(kal_uint32 addr, kal_uint32 para)
{
		char pu_send_cmd[2] = {(char)(addr & 0xFF), (char)(para & 0xFF)};
                #if 1
                iWriteRegI2C6(pu_send_cmd, 2, GC8024_WRITE_ID);//imgsensor_gc8024.i2c_write_id);
                #else
		iWriteRegI2C(pu_send_cmd, 2, GC8024_WRITE_ID);//imgsensor_gc8024.i2c_write_id);
                #endif
}

static void gc8024_init(void)
{
	write_cmos_sensor_gc8024(0xfe,0x00);
	write_cmos_sensor_gc8024(0xfe,0x00);
	write_cmos_sensor_gc8024(0xfe,0x00);
	write_cmos_sensor_gc8024(0xf7,0x95); 
	write_cmos_sensor_gc8024(0xf8,0x08); 
	write_cmos_sensor_gc8024(0xf9,0x00);
	write_cmos_sensor_gc8024(0xfa,0x84);
	write_cmos_sensor_gc8024(0xfc,0xce); 
	
	/*analog*/
	write_cmos_sensor_gc8024(0xfe,0x00);
	write_cmos_sensor_gc8024(0x03,0x08);
	write_cmos_sensor_gc8024(0x04,0xca);
	write_cmos_sensor_gc8024(0x05,0x02);
	write_cmos_sensor_gc8024(0x06,0x1c);
	write_cmos_sensor_gc8024(0x07,0x00);
	write_cmos_sensor_gc8024(0x08,0x10);
	write_cmos_sensor_gc8024(0x09,0x00);
	write_cmos_sensor_gc8024(0x0a,0x14); 
	write_cmos_sensor_gc8024(0x0b,0x00);
	write_cmos_sensor_gc8024(0x0c,0x10); 
	write_cmos_sensor_gc8024(0x0d,0x09);
	write_cmos_sensor_gc8024(0x0e,0x9c); 
	write_cmos_sensor_gc8024(0x0f,0x0c);
	write_cmos_sensor_gc8024(0x10,0xd0); 
	write_cmos_sensor_gc8024(0x17,0xd5); 	 //mirror
	write_cmos_sensor_gc8024(0x18,0x02); 	
	write_cmos_sensor_gc8024(0x19,0x0b);
	write_cmos_sensor_gc8024(0x1a,0x19);	
	write_cmos_sensor_gc8024(0x1c,0x0c);
	write_cmos_sensor_gc8024(0x21,0x12);
	write_cmos_sensor_gc8024(0x23,0xb0);
	write_cmos_sensor_gc8024(0x28,0x5f); //[7]binning
	write_cmos_sensor_gc8024(0x29,0xd4); 
	write_cmos_sensor_gc8024(0x2f,0x4c); 
	write_cmos_sensor_gc8024(0x30,0xf8);
	write_cmos_sensor_gc8024(0xcd,0x9a);
	write_cmos_sensor_gc8024(0xce,0xfd);
	write_cmos_sensor_gc8024(0xd0,0xd2);
	write_cmos_sensor_gc8024(0xd1,0xa8);
	write_cmos_sensor_gc8024(0xd3,0x35);
	write_cmos_sensor_gc8024(0xd8,0x20);
	write_cmos_sensor_gc8024(0xda,0x03);
	write_cmos_sensor_gc8024(0xdb,0x4e);
	write_cmos_sensor_gc8024(0xdc,0xb3);
	write_cmos_sensor_gc8024(0xde,0x40);
	write_cmos_sensor_gc8024(0xe1,0x1a); 
	write_cmos_sensor_gc8024(0xe2,0x00);	
	write_cmos_sensor_gc8024(0xe3,0x71);
	write_cmos_sensor_gc8024(0xe4,0x78);	
	write_cmos_sensor_gc8024(0xe5,0x44);
	write_cmos_sensor_gc8024(0xe6,0xdf);
	write_cmos_sensor_gc8024(0xe8,0x02);
	write_cmos_sensor_gc8024(0xe9,0x01);
	write_cmos_sensor_gc8024(0xea,0x01);
	write_cmos_sensor_gc8024(0xeb,0x02);
	write_cmos_sensor_gc8024(0xec,0x02);
	write_cmos_sensor_gc8024(0xed,0x01);
	write_cmos_sensor_gc8024(0xee,0x01);
	write_cmos_sensor_gc8024(0xef,0x02);

	/*ISP*/
	write_cmos_sensor_gc8024(0x80,0x50);
	write_cmos_sensor_gc8024(0x88,0x03);
	write_cmos_sensor_gc8024(0x89,0x03);

}

//for GC030A main2 camera
#define GC030A_WRITE_ID			0x42
static kal_uint16 read_cmos_sensor_gc030a(kal_uint32 addr)
{
	kal_uint16 get_byte=0;

	char pu_send_cmd[1] = {(char)(addr & 0xFF) };
        #if 1
        iReadRegI2C6(pu_send_cmd, 1, (u8*)&get_byte, 1, GC030A_WRITE_ID);
        #else
	iReadRegI2C(pu_send_cmd, 1, (u8*)&get_byte, 1, GC030A_WRITE_ID);
        #endif
	return get_byte;

}

static void write_cmos_sensor_gc030a(kal_uint32 addr, kal_uint32 para)
{
		char pu_send_cmd[2] = {(char)(addr & 0xFF), (char)(para & 0xFF)};
                 #if 1
                 iWriteRegI2C6(pu_send_cmd, 2, GC030A_WRITE_ID);
		 #else
                 iWriteRegI2C(pu_send_cmd, 2, GC030A_WRITE_ID);
                 #endif
}

static void gc030a_init(void)
{
	/*SYS*/
        write_cmos_sensor_gc030a(0xfe, 0x80);
	write_cmos_sensor_gc030a(0xfe, 0x80);
	write_cmos_sensor_gc030a(0xfe, 0x80);
	write_cmos_sensor_gc030a(0xf7, 0x01);
	write_cmos_sensor_gc030a(0xf8, 0x05);
	write_cmos_sensor_gc030a(0xf9, 0x0f);
	write_cmos_sensor_gc030a(0xfa, 0x00);
	write_cmos_sensor_gc030a(0xfc, 0x0f);
	write_cmos_sensor_gc030a(0xfe, 0x00);
	
	/*ANALOG & CISCTL*/
	write_cmos_sensor_gc030a(0x03, 0x01);
	write_cmos_sensor_gc030a(0x04, 0xc8);
	write_cmos_sensor_gc030a(0x05, 0x03);
	write_cmos_sensor_gc030a(0x06, 0x7b);
	write_cmos_sensor_gc030a(0x07, 0x00);
	write_cmos_sensor_gc030a(0x08, 0x06);
	write_cmos_sensor_gc030a(0x0a, 0x00);
	write_cmos_sensor_gc030a(0x0c, 0x08);
        write_cmos_sensor_gc030a(0x0d, 0x01);
        write_cmos_sensor_gc030a(0x0e, 0xe8);
	write_cmos_sensor_gc030a(0x0f, 0x02);
	write_cmos_sensor_gc030a(0x10, 0x88);
	write_cmos_sensor_gc030a(0x17, 0x54);//Don't Change Here!!!
	write_cmos_sensor_gc030a(0x18, 0x12);
	write_cmos_sensor_gc030a(0x19, 0x07);
	write_cmos_sensor_gc030a(0x1a, 0x1b);
	write_cmos_sensor_gc030a(0x1d, 0x48);//40 travis20160318
	write_cmos_sensor_gc030a(0x1e, 0x50);
	write_cmos_sensor_gc030a(0x1f, 0x80);
	write_cmos_sensor_gc030a(0x23, 0x01);
	write_cmos_sensor_gc030a(0x24, 0xc8);
	write_cmos_sensor_gc030a(0x27, 0xaf);
	write_cmos_sensor_gc030a(0x28, 0x24);
	write_cmos_sensor_gc030a(0x29, 0x1a);
	write_cmos_sensor_gc030a(0x2f, 0x14);
	write_cmos_sensor_gc030a(0x30, 0x00);
	write_cmos_sensor_gc030a(0x31, 0x04);
	write_cmos_sensor_gc030a(0x32, 0x08);
	write_cmos_sensor_gc030a(0x33, 0x0c);
	write_cmos_sensor_gc030a(0x34, 0x0d);
	write_cmos_sensor_gc030a(0x35, 0x0e);
	write_cmos_sensor_gc030a(0x36, 0x0f);
	write_cmos_sensor_gc030a(0x72, 0x98);
	write_cmos_sensor_gc030a(0x73, 0x9a);
	write_cmos_sensor_gc030a(0x74, 0x47);
	write_cmos_sensor_gc030a(0x76, 0x82);
	write_cmos_sensor_gc030a(0x7a, 0xcb);
	write_cmos_sensor_gc030a(0xc2, 0x0c);
	write_cmos_sensor_gc030a(0xce, 0x03);	
	write_cmos_sensor_gc030a(0xcf, 0x48);
	write_cmos_sensor_gc030a(0xd0, 0x10);
	write_cmos_sensor_gc030a(0xdc, 0x75);
	write_cmos_sensor_gc030a(0xeb, 0x78);
	write_cmos_sensor_gc030a(0x90, 0x01);
	write_cmos_sensor_gc030a(0x92, 0x01);//Don't Change Here!!!
	write_cmos_sensor_gc030a(0x94, 0x01);//Don't Change Here!!!
	write_cmos_sensor_gc030a(0x95, 0x01);
	write_cmos_sensor_gc030a(0x96, 0xe0);
	write_cmos_sensor_gc030a(0x97, 0x02);
	write_cmos_sensor_gc030a(0x98, 0x80);
	write_cmos_sensor_gc030a(0xb0, 0x46);
	write_cmos_sensor_gc030a(0xb1, 0x01);
	write_cmos_sensor_gc030a(0xb2, 0x00);
	write_cmos_sensor_gc030a(0xb3, 0x40);
	write_cmos_sensor_gc030a(0xb4, 0x40);
	write_cmos_sensor_gc030a(0xb5, 0x40);
	write_cmos_sensor_gc030a(0xb6, 0x00);
	write_cmos_sensor_gc030a(0x40, 0x26); 
	write_cmos_sensor_gc030a(0x4e, 0x00);
	write_cmos_sensor_gc030a(0x4f, 0x3c);
	write_cmos_sensor_gc030a(0xe0, 0x9f);
	write_cmos_sensor_gc030a(0xe1, 0x90);
	write_cmos_sensor_gc030a(0xe4, 0x0f);
	write_cmos_sensor_gc030a(0xe5, 0xff);
	write_cmos_sensor_gc030a(0xfe, 0x03);
	write_cmos_sensor_gc030a(0x10, 0x00);	
	write_cmos_sensor_gc030a(0x01, 0x03);
	write_cmos_sensor_gc030a(0x02, 0x33);
	write_cmos_sensor_gc030a(0x03, 0x96);
	write_cmos_sensor_gc030a(0x04, 0x01);
	write_cmos_sensor_gc030a(0x05, 0x00);
	write_cmos_sensor_gc030a(0x06, 0x80);	
	write_cmos_sensor_gc030a(0x11, 0x2b);
	write_cmos_sensor_gc030a(0x12, 0x20);
	write_cmos_sensor_gc030a(0x13, 0x03);
	write_cmos_sensor_gc030a(0x15, 0x00);
	write_cmos_sensor_gc030a(0x21, 0x10);
	write_cmos_sensor_gc030a(0x22, 0x00);
	write_cmos_sensor_gc030a(0x23, 0x30);
	write_cmos_sensor_gc030a(0x24, 0x02);
	write_cmos_sensor_gc030a(0x25, 0x12);
	write_cmos_sensor_gc030a(0x26, 0x02);
	write_cmos_sensor_gc030a(0x29, 0x01);
	write_cmos_sensor_gc030a(0x2a, 0x0a);
	write_cmos_sensor_gc030a(0x2b, 0x03);
	write_cmos_sensor_gc030a(0xfe, 0x00);
	write_cmos_sensor_gc030a(0xf9, 0x0e);
	write_cmos_sensor_gc030a(0xfc, 0x0e);
	write_cmos_sensor_gc030a(0xfe, 0x00);
	write_cmos_sensor_gc030a(0x25, 0xa2);
	write_cmos_sensor_gc030a(0x3f, 0x1a);

	write_cmos_sensor_gc030a(0x25,0xe2);
}

#define GC0310_WRITE_ID							        0x42
#define GC0310_READ_ID								0x43
static kal_uint16 read_cmos_sensor_gc0310(kal_uint32 addr)
{
	kal_uint16 get_byte=0;

	char pu_send_cmd[1] = {(char)(addr & 0xFF) };
        #if 1
        iReadRegI2C6(pu_send_cmd, 1, (u8*)&get_byte, 1, GC0310_READ_ID);
        #else
	iReadRegI2C(pu_send_cmd, 1, (u8*)&get_byte, 1, GC0310_READ_ID);
        #endif
	return get_byte;

}

static void write_cmos_sensor_gc0310(kal_uint32 addr, kal_uint32 para)
{
		char pu_send_cmd[2] = {(char)(addr & 0xFF), (char)(para & 0xFF)};
                 #if 1
                 iWriteRegI2C6(pu_send_cmd, 2, GC0310_WRITE_ID);
		 #else
                 iWriteRegI2C(pu_send_cmd, 2, GC0310_WRITE_ID);
                 #endif
}

static void gc0310_init(void)
{
write_cmos_sensor_gc0310(0xfe,0xf0);
write_cmos_sensor_gc0310(0xfe,0xf0);
write_cmos_sensor_gc0310(0xfe,0x00);
write_cmos_sensor_gc0310(0xfc,0x0e);
write_cmos_sensor_gc0310(0xfc,0x0e);
write_cmos_sensor_gc0310(0xf2,0x80);
write_cmos_sensor_gc0310(0xf3,0x00);
write_cmos_sensor_gc0310(0xf7,0x1b);
write_cmos_sensor_gc0310(0xf8,0x04);  // from 03 to 04
write_cmos_sensor_gc0310(0xf9,0x8e);
write_cmos_sensor_gc0310(0xfa,0x11);
/////////////////////////////////////////////////      
///////////////////   MIPI   ////////////////////      
/////////////////////////////////////////////////      
write_cmos_sensor_gc0310(0xfe,0x03);
write_cmos_sensor_gc0310(0x40,0x08);
write_cmos_sensor_gc0310(0x42,0x00);
write_cmos_sensor_gc0310(0x43,0x00);
write_cmos_sensor_gc0310(0x01,0x03);
write_cmos_sensor_gc0310(0x10,0x84);
                                    
write_cmos_sensor_gc0310(0x01,0x03);             
write_cmos_sensor_gc0310(0x02,0x33); //0x00);             
write_cmos_sensor_gc0310(0x03,0x94);             
write_cmos_sensor_gc0310(0x04,0x01);            
write_cmos_sensor_gc0310(0x05,0x40);  // 40      20     
write_cmos_sensor_gc0310(0x06,0x80);             
write_cmos_sensor_gc0310(0x11,0x1e);             
write_cmos_sensor_gc0310(0x12,0x00);      
write_cmos_sensor_gc0310(0x13,0x05);             
write_cmos_sensor_gc0310(0x15,0x10);                                                                    
write_cmos_sensor_gc0310(0x21,0x10);             
write_cmos_sensor_gc0310(0x22,0x01);             
write_cmos_sensor_gc0310(0x23,0x10);                                             
write_cmos_sensor_gc0310(0x24,0x02);                                             
write_cmos_sensor_gc0310(0x25,0x10);                                             
write_cmos_sensor_gc0310(0x26,0x03);                                             
write_cmos_sensor_gc0310(0x29,0x02); //02                                            
write_cmos_sensor_gc0310(0x2a,0x0a);   //0a                                          
write_cmos_sensor_gc0310(0x2b,0x04);                                             
write_cmos_sensor_gc0310(0xfe,0x00);
        /////////////////////////////////////////////////
        /////////////////   CISCTL reg  /////////////////
        /////////////////////////////////////////////////
write_cmos_sensor_gc0310(0x00,0x2f);
write_cmos_sensor_gc0310(0x01,0x0f);
write_cmos_sensor_gc0310(0x02,0x04);
write_cmos_sensor_gc0310(0x03,0x04);
write_cmos_sensor_gc0310(0x04,0xd0);
write_cmos_sensor_gc0310(0x09,0x00);
write_cmos_sensor_gc0310(0x0a,0x00);
write_cmos_sensor_gc0310(0x0b,0x00);
write_cmos_sensor_gc0310(0x0c,0x04);//0x06
write_cmos_sensor_gc0310(0x0d,0x01);
write_cmos_sensor_gc0310(0x0e,0xe8);
write_cmos_sensor_gc0310(0x0f,0x02);
write_cmos_sensor_gc0310(0x10,0x88);
write_cmos_sensor_gc0310(0x16,0x00);
write_cmos_sensor_gc0310(0x17,0x17); //0x14 mirror
write_cmos_sensor_gc0310(0x18,0x1a);
write_cmos_sensor_gc0310(0x19,0x14);
write_cmos_sensor_gc0310(0x1b,0x48);
write_cmos_sensor_gc0310(0x1e,0x6b);
write_cmos_sensor_gc0310(0x1f,0x28);
write_cmos_sensor_gc0310(0x20,0x8b);  // from 89 to 8b
write_cmos_sensor_gc0310(0x21,0x49);
write_cmos_sensor_gc0310(0x22,0xb0);
write_cmos_sensor_gc0310(0x23,0x04);
write_cmos_sensor_gc0310(0x24,0x16);
write_cmos_sensor_gc0310(0x34,0x20);
write_cmos_sensor_gc0310(0x26,0x23); 
write_cmos_sensor_gc0310(0x28,0xff); 
write_cmos_sensor_gc0310(0x29,0x00); 
write_cmos_sensor_gc0310(0x33,0x10); 
write_cmos_sensor_gc0310(0x37,0x20); 
write_cmos_sensor_gc0310(0x38,0x10); 
write_cmos_sensor_gc0310(0x47,0x80); 
write_cmos_sensor_gc0310(0x4e,0x66); 
write_cmos_sensor_gc0310(0xa8,0x02); 
write_cmos_sensor_gc0310(0xa9,0x80);
write_cmos_sensor_gc0310(0x40,0xff); 
write_cmos_sensor_gc0310(0x41,0x21); 
write_cmos_sensor_gc0310(0x42,0xcf); 
write_cmos_sensor_gc0310(0x44,0x01); // 02 yuv 
write_cmos_sensor_gc0310(0x45,0xa0); // from a8 - a4 a4-a0
write_cmos_sensor_gc0310(0x46,0x03); 
write_cmos_sensor_gc0310(0x4a,0x11);
write_cmos_sensor_gc0310(0x4b,0x01);
write_cmos_sensor_gc0310(0x4c,0x20); 
write_cmos_sensor_gc0310(0x4d,0x05); 
write_cmos_sensor_gc0310(0x4f,0x01);
write_cmos_sensor_gc0310(0x50,0x01); 
write_cmos_sensor_gc0310(0x55,0x01); 
write_cmos_sensor_gc0310(0x56,0xe0);
write_cmos_sensor_gc0310(0x57,0x02); 
write_cmos_sensor_gc0310(0x58,0x80);
write_cmos_sensor_gc0310(0x70,0x70); 
write_cmos_sensor_gc0310(0x5a,0x84); 
write_cmos_sensor_gc0310(0x5b,0xc9); 
write_cmos_sensor_gc0310(0x5c,0xed); 
write_cmos_sensor_gc0310(0x77,0x74); 
write_cmos_sensor_gc0310(0x78,0x40); 
write_cmos_sensor_gc0310(0x79,0x5f); 
write_cmos_sensor_gc0310(0x82,0x1f); 
write_cmos_sensor_gc0310(0x83,0x0b);
write_cmos_sensor_gc0310(0x89,0xf0);
write_cmos_sensor_gc0310(0x8f,0xaa); 
write_cmos_sensor_gc0310(0x90,0x8c); 
write_cmos_sensor_gc0310(0x91,0x90);
write_cmos_sensor_gc0310(0x92,0x05); 
write_cmos_sensor_gc0310(0x93,0x05); 
write_cmos_sensor_gc0310(0x94,0x08); 
write_cmos_sensor_gc0310(0x95,0x65);//0x76 pad 
write_cmos_sensor_gc0310(0x96,0xf0); 
write_cmos_sensor_gc0310(0xfe,0x00);
write_cmos_sensor_gc0310(0x9a,0x20);
write_cmos_sensor_gc0310(0x9b,0x80);
write_cmos_sensor_gc0310(0x9c,0x40);
write_cmos_sensor_gc0310(0x9d,0x80);
write_cmos_sensor_gc0310(0xa1,0x30);
write_cmos_sensor_gc0310(0xa2,0x32);
write_cmos_sensor_gc0310(0xa4,0x30);
write_cmos_sensor_gc0310(0xa5,0x30);
write_cmos_sensor_gc0310(0xaa,0x10);
write_cmos_sensor_gc0310(0xac,0x22);
write_cmos_sensor_gc0310(0xbf,0x08); 
write_cmos_sensor_gc0310(0xc0,0x16); 
write_cmos_sensor_gc0310(0xc1,0x28); 
write_cmos_sensor_gc0310(0xc2,0x41); 
write_cmos_sensor_gc0310(0xc3,0x5a); 
write_cmos_sensor_gc0310(0xc4,0x6c); 
write_cmos_sensor_gc0310(0xc5,0x7a); 
write_cmos_sensor_gc0310(0xc6,0x96); 
write_cmos_sensor_gc0310(0xc7,0xac); 
write_cmos_sensor_gc0310(0xc8,0xbc); 
write_cmos_sensor_gc0310(0xc9,0xc9); 
write_cmos_sensor_gc0310(0xca,0xd3); 
write_cmos_sensor_gc0310(0xcb,0xdd); 
write_cmos_sensor_gc0310(0xcc,0xe5); 
write_cmos_sensor_gc0310(0xcd,0xf1); 
write_cmos_sensor_gc0310(0xce,0xfa); 
write_cmos_sensor_gc0310(0xcf,0xff);
write_cmos_sensor_gc0310(0xd0,0x40); 
write_cmos_sensor_gc0310(0xd1,0x30); //0x34 pad
write_cmos_sensor_gc0310(0xd2,0x30); //0x34 pad
write_cmos_sensor_gc0310(0xd3,0x40); //0x48 pad
write_cmos_sensor_gc0310(0xd5,0xf8); 
write_cmos_sensor_gc0310(0xd6,0xf2); 
write_cmos_sensor_gc0310(0xd7,0x1b); 
write_cmos_sensor_gc0310(0xd8,0x18); 
write_cmos_sensor_gc0310(0xdd,0x03); 
write_cmos_sensor_gc0310(0xfe,0x01);
write_cmos_sensor_gc0310(0x05,0x30); 
write_cmos_sensor_gc0310(0x06,0x75); 
write_cmos_sensor_gc0310(0x07,0x40); 
write_cmos_sensor_gc0310(0x08,0xb0); 
write_cmos_sensor_gc0310(0x0a,0xc5); 
write_cmos_sensor_gc0310(0x0b,0x01);
write_cmos_sensor_gc0310(0x0c,0x00); 
write_cmos_sensor_gc0310(0x12,0x52);
write_cmos_sensor_gc0310(0x13,0x36); //0x48 pad
write_cmos_sensor_gc0310(0x18,0x91);
write_cmos_sensor_gc0310(0x19,0x95);
 
write_cmos_sensor_gc0310(0x1f,0x20);
write_cmos_sensor_gc0310(0x20,0xc0); 
write_cmos_sensor_gc0310(0x3e,0x40); 
write_cmos_sensor_gc0310(0x3f,0x57); 
write_cmos_sensor_gc0310(0x40,0x7d); 
write_cmos_sensor_gc0310(0x03,0x60); 
write_cmos_sensor_gc0310(0x44,0x02); 
write_cmos_sensor_gc0310(0x1c, 0x91);//pad add
write_cmos_sensor_gc0310(0x21, 0x15);
write_cmos_sensor_gc0310(0x50, 0x80);
write_cmos_sensor_gc0310(0x56, 0x04);
write_cmos_sensor_gc0310(0x59, 0x08);
write_cmos_sensor_gc0310(0x5b, 0x02);
write_cmos_sensor_gc0310(0x61, 0x8d);
write_cmos_sensor_gc0310(0x62, 0xa7);
write_cmos_sensor_gc0310(0x63, 0xd0);
write_cmos_sensor_gc0310(0x65, 0x06);
write_cmos_sensor_gc0310(0x66, 0x06);
write_cmos_sensor_gc0310(0x67, 0x84);
write_cmos_sensor_gc0310(0x69, 0x08);
write_cmos_sensor_gc0310(0x6a, 0x25);
write_cmos_sensor_gc0310(0x6b, 0x01);
write_cmos_sensor_gc0310(0x6c, 0x00);
write_cmos_sensor_gc0310(0x6d, 0x02);
write_cmos_sensor_gc0310(0x6e, 0xf0);
write_cmos_sensor_gc0310(0x6f, 0x80);
write_cmos_sensor_gc0310(0x76, 0x80);
write_cmos_sensor_gc0310(0x78, 0xaf);
write_cmos_sensor_gc0310(0x79, 0x75);
write_cmos_sensor_gc0310(0x7a, 0x40);
write_cmos_sensor_gc0310(0x7b, 0x50);
write_cmos_sensor_gc0310(0x7c, 0x0c);
write_cmos_sensor_gc0310(0x90,0x00); 
write_cmos_sensor_gc0310(0x91,0x00); 
write_cmos_sensor_gc0310(0x92,0xff); 
write_cmos_sensor_gc0310(0x93,0xe3); 
write_cmos_sensor_gc0310(0x95,0x21); 
write_cmos_sensor_gc0310(0x96,0xff); 
write_cmos_sensor_gc0310(0x97,0x3f); 
write_cmos_sensor_gc0310(0x98,0x21); 
write_cmos_sensor_gc0310(0x9a,0x3f); 
write_cmos_sensor_gc0310(0x9b,0x22); 
write_cmos_sensor_gc0310(0x9c,0x63); 
write_cmos_sensor_gc0310(0x9d,0x40); 
write_cmos_sensor_gc0310(0x9f,0x00); 
write_cmos_sensor_gc0310(0xa0,0x00); 
write_cmos_sensor_gc0310(0xa1,0x00); 
write_cmos_sensor_gc0310(0xa2,0x00); 
write_cmos_sensor_gc0310(0x86,0x00); 
write_cmos_sensor_gc0310(0x87,0x00); 
write_cmos_sensor_gc0310(0x88,0x00); 
write_cmos_sensor_gc0310(0x89,0x00); 
write_cmos_sensor_gc0310(0xa4,0x00); 
write_cmos_sensor_gc0310(0xa5,0x00); 
write_cmos_sensor_gc0310(0xa6,0xc9); 
write_cmos_sensor_gc0310(0xa7,0x9d); 
write_cmos_sensor_gc0310(0xa9,0xd4); 
write_cmos_sensor_gc0310(0xaa,0x9d); 
write_cmos_sensor_gc0310(0xab,0xb0); 
write_cmos_sensor_gc0310(0xac,0x9c); 
write_cmos_sensor_gc0310(0xae,0xc1); 
write_cmos_sensor_gc0310(0xaf,0xb1); 
write_cmos_sensor_gc0310(0xb0,0xc2); 
write_cmos_sensor_gc0310(0xb1,0xa5); 
write_cmos_sensor_gc0310(0xb3,0x00); 
write_cmos_sensor_gc0310(0xb4,0x00); 
write_cmos_sensor_gc0310(0xb5,0x00); 
write_cmos_sensor_gc0310(0xb6,0x00); 
write_cmos_sensor_gc0310(0x8b,0x00); 
write_cmos_sensor_gc0310(0x8c,0x00); 
write_cmos_sensor_gc0310(0x8d,0x00); 
write_cmos_sensor_gc0310(0x8e,0x00); 
write_cmos_sensor_gc0310(0x94,0x50); 
write_cmos_sensor_gc0310(0x99,0xa6); 
write_cmos_sensor_gc0310(0x9e,0xaa); 
write_cmos_sensor_gc0310(0xa3,0x00); 
write_cmos_sensor_gc0310(0x8a,0x00); 
write_cmos_sensor_gc0310(0xa8,0x50); 
write_cmos_sensor_gc0310(0xad,0x55); 
write_cmos_sensor_gc0310(0xb2,0x55); 
write_cmos_sensor_gc0310(0xb7,0x00); 
write_cmos_sensor_gc0310(0x8f,0x00); 
write_cmos_sensor_gc0310(0xb8,0xd9); 
write_cmos_sensor_gc0310(0xb9,0x86); 
write_cmos_sensor_gc0310(0xfe,0x01);
write_cmos_sensor_gc0310(0xd0,0x2e);
write_cmos_sensor_gc0310(0xd1,0xf8);
write_cmos_sensor_gc0310(0xd2,0x08);
write_cmos_sensor_gc0310(0xd3,0xe8);
write_cmos_sensor_gc0310(0xd4,0x40);
write_cmos_sensor_gc0310(0xd5,0x08);
write_cmos_sensor_gc0310(0xd6,0x30);
write_cmos_sensor_gc0310(0xd7,0x00);
write_cmos_sensor_gc0310(0xd8,0x0a);
write_cmos_sensor_gc0310(0xd9,0x16);
write_cmos_sensor_gc0310(0xda,0x39);
write_cmos_sensor_gc0310(0xdb,0xf8);
write_cmos_sensor_gc0310(0xfe,0x01); 
write_cmos_sensor_gc0310(0xc1,0x3c); 
write_cmos_sensor_gc0310(0xc2,0x50); 
write_cmos_sensor_gc0310(0xc3,0x00); 
write_cmos_sensor_gc0310(0xc4,0x40); 
write_cmos_sensor_gc0310(0xc5,0x30); 
write_cmos_sensor_gc0310(0xc6,0x30); 
write_cmos_sensor_gc0310(0xc7,0x10); 
write_cmos_sensor_gc0310(0xc8,0x00); 
write_cmos_sensor_gc0310(0xc9,0x00); 
write_cmos_sensor_gc0310(0xdc,0x20); 
write_cmos_sensor_gc0310(0xdd,0x10); 
write_cmos_sensor_gc0310(0xdf,0x00); 
write_cmos_sensor_gc0310(0xde,0x00); 
write_cmos_sensor_gc0310(0x01,0x10); 
write_cmos_sensor_gc0310(0x0b,0x31); 
write_cmos_sensor_gc0310(0x0e,0x50); 
write_cmos_sensor_gc0310(0x0f,0x0f); 
write_cmos_sensor_gc0310(0x10,0x6e); 
write_cmos_sensor_gc0310(0x12,0xa0); 
write_cmos_sensor_gc0310(0x15,0x60); 
write_cmos_sensor_gc0310(0x16,0x60); 
write_cmos_sensor_gc0310(0x17,0xe0); 
write_cmos_sensor_gc0310(0xcc,0x0c);  
write_cmos_sensor_gc0310(0xcd,0x10); 
write_cmos_sensor_gc0310(0xce,0xa0); 
write_cmos_sensor_gc0310(0xcf,0xe6); 
write_cmos_sensor_gc0310(0x45,0xf7);
write_cmos_sensor_gc0310(0x46,0xff); 
write_cmos_sensor_gc0310(0x47,0x15);
write_cmos_sensor_gc0310(0x48,0x03); 
write_cmos_sensor_gc0310(0x4f,0x60); 
write_cmos_sensor_gc0310(0xfe,0x00);
write_cmos_sensor_gc0310(0x05,0x02);
write_cmos_sensor_gc0310(0x06,0xd1); //HB
write_cmos_sensor_gc0310(0x07,0x00);
write_cmos_sensor_gc0310(0x08,0x22); //VB
write_cmos_sensor_gc0310(0xfe,0x01);
write_cmos_sensor_gc0310(0x25,0x00); //step 
write_cmos_sensor_gc0310(0x26,0x6a); 
write_cmos_sensor_gc0310(0x27,0x04);
write_cmos_sensor_gc0310(0x28,0x24);
write_cmos_sensor_gc0310(0x29,0x05);
write_cmos_sensor_gc0310(0x2a,0xcc);
write_cmos_sensor_gc0310(0x2b,0x08);
write_cmos_sensor_gc0310(0x2c,0xb2);
write_cmos_sensor_gc0310(0x2d,0x09);
write_cmos_sensor_gc0310(0x2e,0xf0);
write_cmos_sensor_gc0310(0x3c,0x20);
write_cmos_sensor_gc0310(0xfe,0x00);
write_cmos_sensor_gc0310(0xfe,0x03);
write_cmos_sensor_gc0310(0x10,0x94);  
write_cmos_sensor_gc0310(0xfe,0x00); 
}

#define SP0A38_WRITE_ID							        0x42
#define SP0A38_READ_ID								0x43

static kal_uint16 read_cmos_sensor_sp0a38(kal_uint32 addr)
{
	kal_uint16 get_byte=0;

	char pu_send_cmd[1] = {(char)(addr & 0xFF) };
        #if 1
        iReadRegI2C6(pu_send_cmd, 1, (u8*)&get_byte, 1, SP0A38_READ_ID);
        #else
	iReadRegI2C(pu_send_cmd, 1, (u8*)&get_byte, 1, SP0A38_READ_ID);
        #endif
	return get_byte;

}

static void write_cmos_sensor_sp0a38(kal_uint32 addr, kal_uint32 para)
{
        char pu_send_cmd[2] = {(char)(addr & 0xFF), (char)(para & 0xFF)};
        #if 1
        iWriteRegI2C6(pu_send_cmd, 2, SP0A38_WRITE_ID);
        #else
        iWriteRegI2C(pu_send_cmd, 2, SP0A38_WRITE_ID);
        #endif
}

static void sp0a38_init(void)
{
 write_cmos_sensor_sp0a38(0xfd,0x00);
  write_cmos_sensor_sp0a38(0x0c,0x00); 
  write_cmos_sensor_sp0a38(0x0f,0xff);
  write_cmos_sensor_sp0a38(0x10,0xfe);  
  write_cmos_sensor_sp0a38(0x11,0x00);
  write_cmos_sensor_sp0a38(0x13,0x18);
  write_cmos_sensor_sp0a38(0x6c,0x19);
  write_cmos_sensor_sp0a38(0x6d,0x19);
  write_cmos_sensor_sp0a38(0x6f,0x19);
  write_cmos_sensor_sp0a38(0x6e,0x1a);    
  write_cmos_sensor_sp0a38(0x69,0x32);
  write_cmos_sensor_sp0a38(0x71,0x30);
  write_cmos_sensor_sp0a38(0x14,0x00);
  write_cmos_sensor_sp0a38(0x15,0x2f); 
  write_cmos_sensor_sp0a38(0x16,0x3f);
  write_cmos_sensor_sp0a38(0x17,0x40);
  write_cmos_sensor_sp0a38(0x70,0x41); 
  write_cmos_sensor_sp0a38(0x6a,0x18);
  write_cmos_sensor_sp0a38(0x72,0x20);
  write_cmos_sensor_sp0a38(0x1b,0x0c);
  write_cmos_sensor_sp0a38(0x1d,0x0a);
  write_cmos_sensor_sp0a38(0x1a,0x0d);
  write_cmos_sensor_sp0a38(0x1e,0x0b);   
  write_cmos_sensor_sp0a38(0x21,0x4a);
  write_cmos_sensor_sp0a38(0x22,0x35); 
  write_cmos_sensor_sp0a38(0x27,0xfb);
  write_cmos_sensor_sp0a38(0x28,0x7d); 
  write_cmos_sensor_sp0a38(0xfd,0x01); 
  write_cmos_sensor_sp0a38(0x32,0x00); 
  write_cmos_sensor_sp0a38(0x33,0xef); 
  write_cmos_sensor_sp0a38(0x34,0xef); 
  write_cmos_sensor_sp0a38(0x35,0x00);
  write_cmos_sensor_sp0a38(0xfd,0x00);         
  write_cmos_sensor_sp0a38(0x30,0x00);
  write_cmos_sensor_sp0a38(0x31,0x00);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0xf2,0x29);
  write_cmos_sensor_sp0a38(0xfd,0x02);
  write_cmos_sensor_sp0a38(0x00,0x80);
  write_cmos_sensor_sp0a38(0x01,0x80);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0x22,0x00);
  write_cmos_sensor_sp0a38(0x23,0x00);        
  write_cmos_sensor_sp0a38(0x24,0x00);
  write_cmos_sensor_sp0a38(0x25,0x00);
  write_cmos_sensor_sp0a38(0xfd,0x00);
  write_cmos_sensor_sp0a38(0x03,0x01);
  write_cmos_sensor_sp0a38(0x04,0x90);
  write_cmos_sensor_sp0a38(0x05,0x00);
  write_cmos_sensor_sp0a38(0x06,0x00);
  write_cmos_sensor_sp0a38(0x07,0x00);
  write_cmos_sensor_sp0a38(0x08,0x00);
  write_cmos_sensor_sp0a38(0x09,0x00);
  write_cmos_sensor_sp0a38(0x0a,0x00);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0xf0,0x00);
  write_cmos_sensor_sp0a38(0xf7,0x64);
  write_cmos_sensor_sp0a38(0x02,0x0a);
  write_cmos_sensor_sp0a38(0x03,0x01);
  write_cmos_sensor_sp0a38(0x06,0x73);
  write_cmos_sensor_sp0a38(0x07,0x00);
  write_cmos_sensor_sp0a38(0x08,0x01);
  write_cmos_sensor_sp0a38(0x09,0x00);
  write_cmos_sensor_sp0a38(0xfd,0x02);
  write_cmos_sensor_sp0a38(0xbe,0x7e);
  write_cmos_sensor_sp0a38(0xbf,0x04);
  write_cmos_sensor_sp0a38(0xd0,0x7e);
  write_cmos_sensor_sp0a38(0xd1,0x04);

  write_cmos_sensor_sp0a38(0xfd,0x02);
  write_cmos_sensor_sp0a38(0xb8,0x60);
  write_cmos_sensor_sp0a38(0xb9,0x70);
  write_cmos_sensor_sp0a38(0xba,0x28);
  write_cmos_sensor_sp0a38(0xbb,0x38);
  write_cmos_sensor_sp0a38(0xbc,0x60);
  write_cmos_sensor_sp0a38(0xbd,0x40);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0xc0,0x6c);
  write_cmos_sensor_sp0a38(0xc1,0x54);
  write_cmos_sensor_sp0a38(0xc2,0x48);
  write_cmos_sensor_sp0a38(0xc3,0x40);
  write_cmos_sensor_sp0a38(0xc4,0x40);
  write_cmos_sensor_sp0a38(0xc5,0x3e);
  write_cmos_sensor_sp0a38(0xc6,0x3e);
  write_cmos_sensor_sp0a38(0xc7,0x3a);
  write_cmos_sensor_sp0a38(0xc8,0x3a);
  write_cmos_sensor_sp0a38(0xc9,0x3a);
  write_cmos_sensor_sp0a38(0xca,0x38);
  write_cmos_sensor_sp0a38(0xf3,0x38);
  write_cmos_sensor_sp0a38(0xf4,0x38);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0x04,0x80);
  write_cmos_sensor_sp0a38(0x05,0x38);
  write_cmos_sensor_sp0a38(0x0a,0x80);
  write_cmos_sensor_sp0a38(0x0b,0x38);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0xcb,0x84);
  write_cmos_sensor_sp0a38(0xcc,0x84);
  write_cmos_sensor_sp0a38(0xcd,0x05);
  write_cmos_sensor_sp0a38(0xce,0x0a);

  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0x1a,0x80);
  write_cmos_sensor_sp0a38(0x1b,0x4f);
  write_cmos_sensor_sp0a38(0x1c,0x00);
  write_cmos_sensor_sp0a38(0x1d,0x20);
  write_cmos_sensor_sp0a38(0x1e,0x00);
  write_cmos_sensor_sp0a38(0x1f,0x03);
  write_cmos_sensor_sp0a38(0x20,0x00);
  write_cmos_sensor_sp0a38(0x21,0x00);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0x84,0x2c);
  write_cmos_sensor_sp0a38(0x85,0x28);
  write_cmos_sensor_sp0a38(0x86,0x2e);
  write_cmos_sensor_sp0a38(0x87,0x1e);
  write_cmos_sensor_sp0a38(0x88,0x24);
  write_cmos_sensor_sp0a38(0x89,0x28);
  write_cmos_sensor_sp0a38(0x8a,0x2e);
  write_cmos_sensor_sp0a38(0x8b,0x1e);
  write_cmos_sensor_sp0a38(0x8c,0x23);
  write_cmos_sensor_sp0a38(0x8d,0x24);
  write_cmos_sensor_sp0a38(0x8e,0x2e);
  write_cmos_sensor_sp0a38(0x8f,0x1e);
  write_cmos_sensor_sp0a38(0x90,0x0d);
  write_cmos_sensor_sp0a38(0x91,0x0a);
  write_cmos_sensor_sp0a38(0x92,0x08);
  write_cmos_sensor_sp0a38(0x93,0x0c);
  write_cmos_sensor_sp0a38(0x94,0x0b);
  write_cmos_sensor_sp0a38(0x95,0x02);
  write_cmos_sensor_sp0a38(0x96,0x08);
  write_cmos_sensor_sp0a38(0x97,0x08);
  write_cmos_sensor_sp0a38(0x98,0x02);
  write_cmos_sensor_sp0a38(0x99,0x04);
  write_cmos_sensor_sp0a38(0x9a,0x08);
  write_cmos_sensor_sp0a38(0x9b,0x04);
  write_cmos_sensor_sp0a38(0xfd,0x00);

  write_cmos_sensor_sp0a38(0xfd,0x02);
  write_cmos_sensor_sp0a38(0x09,0x09);
  write_cmos_sensor_sp0a38(0x0d,0x1a);
  write_cmos_sensor_sp0a38(0x1d,0x03);
  write_cmos_sensor_sp0a38(0x1f,0x07);

  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0x32,0x00);
  write_cmos_sensor_sp0a38(0xfd,0x02);
  write_cmos_sensor_sp0a38(0x26,0xb7);
  write_cmos_sensor_sp0a38(0x27,0x9c);
  write_cmos_sensor_sp0a38(0x10,0x00);
  write_cmos_sensor_sp0a38(0x11,0x00);
  write_cmos_sensor_sp0a38(0x1b,0x80);
  write_cmos_sensor_sp0a38(0x1a,0x80);
  write_cmos_sensor_sp0a38(0x18,0x27);
  write_cmos_sensor_sp0a38(0x19,0x26);
  write_cmos_sensor_sp0a38(0x2a,0x01);
  write_cmos_sensor_sp0a38(0x2b,0x10);
  write_cmos_sensor_sp0a38(0x28,0xf8);
  write_cmos_sensor_sp0a38(0x29,0x08);

  write_cmos_sensor_sp0a38(0x66,0x52);
  write_cmos_sensor_sp0a38(0x67,0x74);
  write_cmos_sensor_sp0a38(0x68,0xc9);
  write_cmos_sensor_sp0a38(0x69,0xec);
  write_cmos_sensor_sp0a38(0x6a,0xa5);
  write_cmos_sensor_sp0a38(0x7c,0x40);
  write_cmos_sensor_sp0a38(0x7d,0x60);
  write_cmos_sensor_sp0a38(0x7e,0xf7);
  write_cmos_sensor_sp0a38(0x7f,0x16);
  write_cmos_sensor_sp0a38(0x80,0xa6);
  write_cmos_sensor_sp0a38(0x70,0x3a);
  write_cmos_sensor_sp0a38(0x71,0x57);
  write_cmos_sensor_sp0a38(0x72,0x19);
  write_cmos_sensor_sp0a38(0x73,0x37);
  write_cmos_sensor_sp0a38(0x74,0xaa);
  write_cmos_sensor_sp0a38(0x6b,0x17);
  write_cmos_sensor_sp0a38(0x6c,0x3a);
  write_cmos_sensor_sp0a38(0x6d,0x21);
  write_cmos_sensor_sp0a38(0x6e,0x43);
  write_cmos_sensor_sp0a38(0x6f,0xaa);
  write_cmos_sensor_sp0a38(0x61,0xfc);
  write_cmos_sensor_sp0a38(0x62,0x17);
  write_cmos_sensor_sp0a38(0x63,0x3f);
  write_cmos_sensor_sp0a38(0x64,0x57);
  write_cmos_sensor_sp0a38(0x65,0x6a);
  write_cmos_sensor_sp0a38(0x75,0x80);
  write_cmos_sensor_sp0a38(0x76,0x09);
  write_cmos_sensor_sp0a38(0x77,0x02);
  write_cmos_sensor_sp0a38(0x0e,0x12);
  write_cmos_sensor_sp0a38(0x3b,0x09);
  write_cmos_sensor_sp0a38(0xfd,0x02);
  write_cmos_sensor_sp0a38(0xde,0x0f);
  write_cmos_sensor_sp0a38(0xd7,0x08);
  write_cmos_sensor_sp0a38(0xd8,0x08);
  write_cmos_sensor_sp0a38(0xd9,0x10);
  write_cmos_sensor_sp0a38(0xda,0x14);
  write_cmos_sensor_sp0a38(0xe8,0x60);
  write_cmos_sensor_sp0a38(0xe9,0x5c);
  write_cmos_sensor_sp0a38(0xea,0x40);
  write_cmos_sensor_sp0a38(0xeb,0x20);
  write_cmos_sensor_sp0a38(0xec,0x64);
  write_cmos_sensor_sp0a38(0xed,0x50);
  write_cmos_sensor_sp0a38(0xee,0x40);
  write_cmos_sensor_sp0a38(0xef,0x20);
  write_cmos_sensor_sp0a38(0xd3,0x20);
  write_cmos_sensor_sp0a38(0xd4,0x48);
  write_cmos_sensor_sp0a38(0xd5,0x20);
  write_cmos_sensor_sp0a38(0xd6,0x08);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0xb1,0x20);
  write_cmos_sensor_sp0a38(0xfd,0x02);
  write_cmos_sensor_sp0a38(0xdc,0x05);
  write_cmos_sensor_sp0a38(0x05,0x20);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0x62,0xf0);
  write_cmos_sensor_sp0a38(0x63,0x80);
  write_cmos_sensor_sp0a38(0x64,0x80);
  write_cmos_sensor_sp0a38(0x65,0x20);
  write_cmos_sensor_sp0a38(0xfd,0x02);
  write_cmos_sensor_sp0a38(0xdd,0x0f);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0xa8,0x06);
  write_cmos_sensor_sp0a38(0xa9,0x0d);
  write_cmos_sensor_sp0a38(0xaa,0x10);
  write_cmos_sensor_sp0a38(0xab,0x10);
  write_cmos_sensor_sp0a38(0xd3,0x06);
  write_cmos_sensor_sp0a38(0xd4,0x0d);
  write_cmos_sensor_sp0a38(0xd5,0x10);
  write_cmos_sensor_sp0a38(0xd6,0x10);
  write_cmos_sensor_sp0a38(0xcf,0xe0);
  write_cmos_sensor_sp0a38(0xd0,0xe0);
  write_cmos_sensor_sp0a38(0xd1,0x80);
  write_cmos_sensor_sp0a38(0xd2,0x40);
  write_cmos_sensor_sp0a38(0xdf,0xe0);
  write_cmos_sensor_sp0a38(0xe0,0xe0);
  write_cmos_sensor_sp0a38(0xe1,0x80);
  write_cmos_sensor_sp0a38(0xe2,0x40);
  write_cmos_sensor_sp0a38(0xe3,0xff);
  write_cmos_sensor_sp0a38(0xe4,0xe0);
  write_cmos_sensor_sp0a38(0xe5,0x80);
  write_cmos_sensor_sp0a38(0xe6,0x00);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0x6e,0x00);
  write_cmos_sensor_sp0a38(0x6f,0x05);
  write_cmos_sensor_sp0a38(0x70,0x0f);
  write_cmos_sensor_sp0a38(0x71,0x19);
  write_cmos_sensor_sp0a38(0x72,0x26);
  write_cmos_sensor_sp0a38(0x73,0x41);
  write_cmos_sensor_sp0a38(0x74,0x5a);
  write_cmos_sensor_sp0a38(0x75,0x6c);
  write_cmos_sensor_sp0a38(0x76,0x7c);
  write_cmos_sensor_sp0a38(0x77,0x94);
  write_cmos_sensor_sp0a38(0x78,0xa4);
  write_cmos_sensor_sp0a38(0x79,0xb3);
  write_cmos_sensor_sp0a38(0x7a,0xbd);
  write_cmos_sensor_sp0a38(0x7b,0xc7);
  write_cmos_sensor_sp0a38(0x7c,0xd0);
  write_cmos_sensor_sp0a38(0x7d,0xd8);
  write_cmos_sensor_sp0a38(0x7e,0xdf);
  write_cmos_sensor_sp0a38(0x7f,0xe6);
  write_cmos_sensor_sp0a38(0x80,0xeb);
  write_cmos_sensor_sp0a38(0x81,0xf0);
  write_cmos_sensor_sp0a38(0x82,0xf5);
  write_cmos_sensor_sp0a38(0x83,0xfa);
  write_cmos_sensor_sp0a38(0xfd,0x02);
  write_cmos_sensor_sp0a38(0x15,0xc6);
  write_cmos_sensor_sp0a38(0x16,0x92);

  write_cmos_sensor_sp0a38(0xa0,0x7c);
  write_cmos_sensor_sp0a38(0xa1,0x00);
  write_cmos_sensor_sp0a38(0xa2,0x00);
  write_cmos_sensor_sp0a38(0xa3,0xf3);
  write_cmos_sensor_sp0a38(0xa4,0x92);
  write_cmos_sensor_sp0a38(0xa5,0x00);
  write_cmos_sensor_sp0a38(0xa6,0x00);
  write_cmos_sensor_sp0a38(0xa7,0xe6);
  write_cmos_sensor_sp0a38(0xa8,0x9a);

  write_cmos_sensor_sp0a38(0xac,0x66);
  write_cmos_sensor_sp0a38(0xad,0x20);
  write_cmos_sensor_sp0a38(0xae,0xfa);
  write_cmos_sensor_sp0a38(0xaf,0xf9);
  write_cmos_sensor_sp0a38(0xb0,0x80);
  write_cmos_sensor_sp0a38(0xb1,0x06);
  write_cmos_sensor_sp0a38(0xb2,0xf3);
  write_cmos_sensor_sp0a38(0xb3,0xb3);
  write_cmos_sensor_sp0a38(0xb4,0xd9);

  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0xb3,0x76);
  write_cmos_sensor_sp0a38(0xb4,0x70);
  write_cmos_sensor_sp0a38(0xb5,0x58);
  write_cmos_sensor_sp0a38(0xb6,0x44);

  write_cmos_sensor_sp0a38(0xb7,0x76);
  write_cmos_sensor_sp0a38(0xb8,0x70);
  write_cmos_sensor_sp0a38(0xb9,0x58);
  write_cmos_sensor_sp0a38(0xba,0x44);

  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0xbd,0x20);
  write_cmos_sensor_sp0a38(0xbe,0x10);
  write_cmos_sensor_sp0a38(0xbf,0xff);
  write_cmos_sensor_sp0a38(0x00,0x00);

  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0x9c,0xaa);
  write_cmos_sensor_sp0a38(0x9d,0xaa);
  write_cmos_sensor_sp0a38(0x9e,0x77);
  write_cmos_sensor_sp0a38(0x9f,0x77);

  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0xa4,0x20);
  write_cmos_sensor_sp0a38(0xa5,0x1f);
  write_cmos_sensor_sp0a38(0xa6,0x50);
  write_cmos_sensor_sp0a38(0xa7,0x65);

  write_cmos_sensor_sp0a38(0xfd,0x02);
  write_cmos_sensor_sp0a38(0x30,0x10);
  write_cmos_sensor_sp0a38(0x31,0x60);
  write_cmos_sensor_sp0a38(0x32,0x60);
  write_cmos_sensor_sp0a38(0x33,0xff);
  write_cmos_sensor_sp0a38(0x35,0x60);
  write_cmos_sensor_sp0a38(0x36,0x28);
  write_cmos_sensor_sp0a38(0x37,0x13);

  write_cmos_sensor_sp0a38(0xfd,0x01);  
  write_cmos_sensor_sp0a38(0x0e,0x80);
  write_cmos_sensor_sp0a38(0x0f,0x20);
  write_cmos_sensor_sp0a38(0x10,0x98);
  write_cmos_sensor_sp0a38(0x11,0x98);
  write_cmos_sensor_sp0a38(0x12,0x88);
  write_cmos_sensor_sp0a38(0x13,0x80);
  write_cmos_sensor_sp0a38(0x14,0xd0);
  write_cmos_sensor_sp0a38(0x15,0xda);
  write_cmos_sensor_sp0a38(0x16,0xbc);
  write_cmos_sensor_sp0a38(0x17,0xb8);

  write_cmos_sensor_sp0a38(0xfd,0x02);
  write_cmos_sensor_sp0a38(0x48,0xf8);
  write_cmos_sensor_sp0a38(0x49,0xf4);
  write_cmos_sensor_sp0a38(0x4a,0x0f);
  write_cmos_sensor_sp0a38(0xfd,0x01);
  write_cmos_sensor_sp0a38(0x32,0x15);
  write_cmos_sensor_sp0a38(0x33,0xef);
  write_cmos_sensor_sp0a38(0x34,0xef);
  write_cmos_sensor_sp0a38(0xfb,0x33);
  write_cmos_sensor_sp0a38(0xf2,0x6c);
  write_cmos_sensor_sp0a38(0x35,0x10);
  write_cmos_sensor_sp0a38(0x5d,0x01);
  write_cmos_sensor_sp0a38(0xfd,0x00);
  write_cmos_sensor_sp0a38(0x2e,0x00);
  write_cmos_sensor_sp0a38(0x2c,0x00);
  write_cmos_sensor_sp0a38(0xfd,0x00);
  write_cmos_sensor_sp0a38(0xe7,0x00);

}

//SP0A08
#define SP0A08_WRITE_ID							        0x7A
#define SP0A08_READ_ID								0x7B

static kal_uint16 read_cmos_sensor_sp0a08(kal_uint32 addr)
{
	kal_uint16 get_byte=0;

	char pu_send_cmd[1] = {(char)(addr & 0xFF) };
        #if 1
        iReadRegI2C6(pu_send_cmd, 1, (u8*)&get_byte, 1, SP0A08_READ_ID);
        #else
	iReadRegI2C(pu_send_cmd, 1, (u8*)&get_byte, 1, SP0A08_READ_ID);
        #endif
	return get_byte;

}

static void write_cmos_sensor_sp0a08(kal_uint32 addr, kal_uint32 para)
{
        char pu_send_cmd[2] = {(char)(addr & 0xFF), (char)(para & 0xFF)};
        #if 1
        iWriteRegI2C6(pu_send_cmd, 2, SP0A08_WRITE_ID);
        #else
        iWriteRegI2C(pu_send_cmd, 2, SP0A08_WRITE_ID);
        #endif
}

static void sp0a08_init(void)
{
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0x63,0x1e);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0x7c,0x6c);
write_cmos_sensor_sp0a08(0x71,0x02);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0x69,0x07);
write_cmos_sensor_sp0a08(0x6d,0x03);
write_cmos_sensor_sp0a08(0x71,0x02);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0x1C,0x00);
write_cmos_sensor_sp0a08(0x32,0x00);
write_cmos_sensor_sp0a08(0x0e,0x00);
write_cmos_sensor_sp0a08(0x0f,0x40);
write_cmos_sensor_sp0a08(0x10,0x40);
write_cmos_sensor_sp0a08(0x11,0x10);
write_cmos_sensor_sp0a08(0x12,0xa0);
write_cmos_sensor_sp0a08(0x13,0xf0);
write_cmos_sensor_sp0a08(0x14,0x30);
write_cmos_sensor_sp0a08(0x15,0x00);
write_cmos_sensor_sp0a08(0x16,0x08);
write_cmos_sensor_sp0a08(0x1A,0x37);
write_cmos_sensor_sp0a08(0x1B,0x17);
write_cmos_sensor_sp0a08(0x1C,0x2f);
write_cmos_sensor_sp0a08(0x1d,0x00);
write_cmos_sensor_sp0a08(0x1E,0x57);
write_cmos_sensor_sp0a08(0x21,0x34);
write_cmos_sensor_sp0a08(0x22,0x12);
write_cmos_sensor_sp0a08(0x24,0x80);
write_cmos_sensor_sp0a08(0x25,0x02);
write_cmos_sensor_sp0a08(0x26,0x03);
write_cmos_sensor_sp0a08(0x27,0xeb);
write_cmos_sensor_sp0a08(0x28,0x5f);
write_cmos_sensor_sp0a08(0x2f,0x01);
write_cmos_sensor_sp0a08(0x5f,0x02);
write_cmos_sensor_sp0a08(0xfb,0x33);
write_cmos_sensor_sp0a08(0xf4,0x09);
write_cmos_sensor_sp0a08(0xe7,0x03);
write_cmos_sensor_sp0a08(0xe7,0x00);
write_cmos_sensor_sp0a08(0x65,0x18);
write_cmos_sensor_sp0a08(0x66,0x18);
write_cmos_sensor_sp0a08(0x67,0x18);
write_cmos_sensor_sp0a08(0x68,0x18);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0x03,0x01);
write_cmos_sensor_sp0a08(0x04,0x2c);
write_cmos_sensor_sp0a08(0x05,0x00);
write_cmos_sensor_sp0a08(0x06,0x00);
write_cmos_sensor_sp0a08(0x09,0x01);
write_cmos_sensor_sp0a08(0x0a,0x54);
write_cmos_sensor_sp0a08(0xf0,0x64);
write_cmos_sensor_sp0a08(0xf1,0x00);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0x90,0x64);
write_cmos_sensor_sp0a08(0x92,0x01);
write_cmos_sensor_sp0a08(0x98,0x64);
write_cmos_sensor_sp0a08(0x99,0x00);
write_cmos_sensor_sp0a08(0x9a,0x01);
write_cmos_sensor_sp0a08(0x9b,0x00);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0xce,0x58);
write_cmos_sensor_sp0a08(0xcf,0x02);
write_cmos_sensor_sp0a08(0xd0,0x58);
write_cmos_sensor_sp0a08(0xd1,0x02);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0xc4,0x6c);
write_cmos_sensor_sp0a08(0xc5,0x7c);
write_cmos_sensor_sp0a08(0xca,0x30);
write_cmos_sensor_sp0a08(0xcb,0x45);
write_cmos_sensor_sp0a08(0xcc,0x60);
write_cmos_sensor_sp0a08(0xcd,0x60);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0x45,0x00);
write_cmos_sensor_sp0a08(0x46,0x99);
write_cmos_sensor_sp0a08(0x79,0xff);
write_cmos_sensor_sp0a08(0x7a,0xff);
write_cmos_sensor_sp0a08(0x7b,0x10);
write_cmos_sensor_sp0a08(0x7c,0x10);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0x35,0x14);
write_cmos_sensor_sp0a08(0x36,0x14);
write_cmos_sensor_sp0a08(0x37,0x1c);
write_cmos_sensor_sp0a08(0x38,0x1c);
write_cmos_sensor_sp0a08(0x39,0x10);
write_cmos_sensor_sp0a08(0x3a,0x10);
write_cmos_sensor_sp0a08(0x3b,0x1a);
write_cmos_sensor_sp0a08(0x3c,0x1a);
write_cmos_sensor_sp0a08(0x3d,0x10);
write_cmos_sensor_sp0a08(0x3e,0x10);
write_cmos_sensor_sp0a08(0x3f,0x15);
write_cmos_sensor_sp0a08(0x40,0x20);
write_cmos_sensor_sp0a08(0x41,0x0a);
write_cmos_sensor_sp0a08(0x42,0x08);
write_cmos_sensor_sp0a08(0x43,0x08);
write_cmos_sensor_sp0a08(0x44,0x0a);
write_cmos_sensor_sp0a08(0x45,0x00);
write_cmos_sensor_sp0a08(0x46,0x00);
write_cmos_sensor_sp0a08(0x47,0x00);
write_cmos_sensor_sp0a08(0x48,0xfd);
write_cmos_sensor_sp0a08(0x49,0x00);
write_cmos_sensor_sp0a08(0x4a,0x00);
write_cmos_sensor_sp0a08(0x4b,0x04);
write_cmos_sensor_sp0a08(0x4c,0xfd);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0xa1,0x20);
write_cmos_sensor_sp0a08(0xa2,0x20);
write_cmos_sensor_sp0a08(0xa3,0x20);
write_cmos_sensor_sp0a08(0xa4,0xff);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0xde,0x0f);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0x57,0x08);
write_cmos_sensor_sp0a08(0x58,0x0e);
write_cmos_sensor_sp0a08(0x56,0x10);
write_cmos_sensor_sp0a08(0x59,0x10);
write_cmos_sensor_sp0a08(0x89,0x08);
write_cmos_sensor_sp0a08(0x8a,0x0e);
write_cmos_sensor_sp0a08(0x9c,0x10);
write_cmos_sensor_sp0a08(0x9d,0x10);
write_cmos_sensor_sp0a08(0x81,0xd8);
write_cmos_sensor_sp0a08(0x82,0x98);
write_cmos_sensor_sp0a08(0x83,0x80);
write_cmos_sensor_sp0a08(0x84,0x80);
write_cmos_sensor_sp0a08(0x85,0xd8);
write_cmos_sensor_sp0a08(0x86,0x98);
write_cmos_sensor_sp0a08(0x87,0x80);
write_cmos_sensor_sp0a08(0x88,0x80);
write_cmos_sensor_sp0a08(0x5a,0xff);
write_cmos_sensor_sp0a08(0x5b,0xb8);
write_cmos_sensor_sp0a08(0x5c,0xa0);
write_cmos_sensor_sp0a08(0x5d,0xa0);
write_cmos_sensor_sp0a08(0xa7,0xff);
write_cmos_sensor_sp0a08(0xa8,0xff);
write_cmos_sensor_sp0a08(0xa9,0xff);
write_cmos_sensor_sp0a08(0xaa,0xff);
write_cmos_sensor_sp0a08(0x9e,0x10);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0xe2,0x30);
write_cmos_sensor_sp0a08(0xe4,0xa0);
write_cmos_sensor_sp0a08(0xe5,0x08);
write_cmos_sensor_sp0a08(0xd3,0x10);
write_cmos_sensor_sp0a08(0xd7,0x08);
write_cmos_sensor_sp0a08(0xe6,0x08);
write_cmos_sensor_sp0a08(0xd4,0x10);
write_cmos_sensor_sp0a08(0xd8,0x08);
write_cmos_sensor_sp0a08(0xe7,0x10);
write_cmos_sensor_sp0a08(0xd5,0x10);
write_cmos_sensor_sp0a08(0xd9,0x10);
write_cmos_sensor_sp0a08(0xd2,0x10);
write_cmos_sensor_sp0a08(0xd6,0x10);
write_cmos_sensor_sp0a08(0xda,0x10);
write_cmos_sensor_sp0a08(0xe8,0x28);
write_cmos_sensor_sp0a08(0xec,0x38);
write_cmos_sensor_sp0a08(0xe9,0x20);
write_cmos_sensor_sp0a08(0xed,0x38);
write_cmos_sensor_sp0a08(0xea,0x10);
write_cmos_sensor_sp0a08(0xef,0x20);
write_cmos_sensor_sp0a08(0xeb,0x10);
write_cmos_sensor_sp0a08(0xf0,0x10);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0xa0,0x8a);
write_cmos_sensor_sp0a08(0xa1,0xfb);
write_cmos_sensor_sp0a08(0xa2,0xfc);
write_cmos_sensor_sp0a08(0xa3,0xfe);
write_cmos_sensor_sp0a08(0xa4,0x93);
write_cmos_sensor_sp0a08(0xa5,0xf0);
write_cmos_sensor_sp0a08(0xa6,0x0b);
write_cmos_sensor_sp0a08(0xa7,0xd8);
write_cmos_sensor_sp0a08(0xa8,0x9d);
write_cmos_sensor_sp0a08(0xa9,0x3c);
write_cmos_sensor_sp0a08(0xaa,0x33);
write_cmos_sensor_sp0a08(0xab,0x0c);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0x8b,0x00);
write_cmos_sensor_sp0a08(0x8c,0x0c);
write_cmos_sensor_sp0a08(0x8d,0x19);
write_cmos_sensor_sp0a08(0x8e,0x2c);
write_cmos_sensor_sp0a08(0x8f,0x49);
write_cmos_sensor_sp0a08(0x90,0x61);
write_cmos_sensor_sp0a08(0x91,0x77);
write_cmos_sensor_sp0a08(0x92,0x8a);
write_cmos_sensor_sp0a08(0x93,0x9b);
write_cmos_sensor_sp0a08(0x94,0xa9);
write_cmos_sensor_sp0a08(0x95,0xb5);
write_cmos_sensor_sp0a08(0x96,0xc0);
write_cmos_sensor_sp0a08(0x97,0xca);
write_cmos_sensor_sp0a08(0x98,0xd4);
write_cmos_sensor_sp0a08(0x99,0xdd);
write_cmos_sensor_sp0a08(0x9a,0xe6);
write_cmos_sensor_sp0a08(0x9b,0xef);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0x8d,0xf7);
write_cmos_sensor_sp0a08(0x8e,0xff);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0x28,0xc4);
write_cmos_sensor_sp0a08(0x29,0x9e);
write_cmos_sensor_sp0a08(0x11,0x13);
write_cmos_sensor_sp0a08(0x12,0x13);
write_cmos_sensor_sp0a08(0x2e,0x13);
write_cmos_sensor_sp0a08(0x2f,0x13);
write_cmos_sensor_sp0a08(0x16,0x1c);
write_cmos_sensor_sp0a08(0x17,0x1a);
write_cmos_sensor_sp0a08(0x18,0x1a);
write_cmos_sensor_sp0a08(0x19,0x54);
write_cmos_sensor_sp0a08(0x1a,0xa5);
write_cmos_sensor_sp0a08(0x1b,0x9a);
write_cmos_sensor_sp0a08(0x2a,0xef);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0xe0,0x3a);
write_cmos_sensor_sp0a08(0xe1,0x2c);
write_cmos_sensor_sp0a08(0xe2,0x26);
write_cmos_sensor_sp0a08(0xe3,0x22);
write_cmos_sensor_sp0a08(0xe4,0x22);
write_cmos_sensor_sp0a08(0xe5,0x20);
write_cmos_sensor_sp0a08(0xe6,0x20);
write_cmos_sensor_sp0a08(0xe8,0x20);
write_cmos_sensor_sp0a08(0xe9,0x20);
write_cmos_sensor_sp0a08(0xea,0x20);
write_cmos_sensor_sp0a08(0xeb,0x1e);
write_cmos_sensor_sp0a08(0xf5,0x1e);
write_cmos_sensor_sp0a08(0xf6,0x1e);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0x94,0x60);
write_cmos_sensor_sp0a08(0x95,0x1e);
write_cmos_sensor_sp0a08(0x9c,0x60);
write_cmos_sensor_sp0a08(0x9d,0x1e);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0xed,0x94);
write_cmos_sensor_sp0a08(0xf7,0x90);
write_cmos_sensor_sp0a08(0xf8,0x88);
write_cmos_sensor_sp0a08(0xec,0x84);
write_cmos_sensor_sp0a08(0xef,0x88);
write_cmos_sensor_sp0a08(0xf9,0x84);
write_cmos_sensor_sp0a08(0xfa,0x7c);
write_cmos_sensor_sp0a08(0xee,0x78);
write_cmos_sensor_sp0a08(0xfd,0x01);
write_cmos_sensor_sp0a08(0x30,0x40);
write_cmos_sensor_sp0a08(0x31,0x70);
write_cmos_sensor_sp0a08(0x32,0x20);
write_cmos_sensor_sp0a08(0x33,0xef);
write_cmos_sensor_sp0a08(0x34,0x02);
write_cmos_sensor_sp0a08(0x4d,0x40);
write_cmos_sensor_sp0a08(0x4e,0x15);
write_cmos_sensor_sp0a08(0x4f,0x13);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0xbe,0x5a);
write_cmos_sensor_sp0a08(0xc0,0xff);
write_cmos_sensor_sp0a08(0xc1,0xff);
write_cmos_sensor_sp0a08(0xd3,0x78);
write_cmos_sensor_sp0a08(0xd4,0x78);
write_cmos_sensor_sp0a08(0xd6,0x70);
write_cmos_sensor_sp0a08(0xd7,0x60);
write_cmos_sensor_sp0a08(0xd8,0x78);
write_cmos_sensor_sp0a08(0xd9,0x78);
write_cmos_sensor_sp0a08(0xda,0x70);
write_cmos_sensor_sp0a08(0xdb,0x60);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0xdc,0x00);
write_cmos_sensor_sp0a08(0xdd,0x88);
write_cmos_sensor_sp0a08(0xde,0x90);
write_cmos_sensor_sp0a08(0xdf,0x80);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0xc2,0x08);
write_cmos_sensor_sp0a08(0xc3,0x08);
write_cmos_sensor_sp0a08(0xc4,0x08);
write_cmos_sensor_sp0a08(0xc5,0x10);
write_cmos_sensor_sp0a08(0xc6,0x80);
write_cmos_sensor_sp0a08(0xc7,0x80);
write_cmos_sensor_sp0a08(0xc8,0x80);
write_cmos_sensor_sp0a08(0xc9,0x80);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0xb2,0x18);
write_cmos_sensor_sp0a08(0xb3,0x1f);
write_cmos_sensor_sp0a08(0xb4,0x30);
write_cmos_sensor_sp0a08(0xb5,0x45);
write_cmos_sensor_sp0a08(0xfd,0x00);
write_cmos_sensor_sp0a08(0x32,0x0d);
write_cmos_sensor_sp0a08(0x31,0x70);
write_cmos_sensor_sp0a08(0x34,0x7e);
write_cmos_sensor_sp0a08(0x33,0xef);
write_cmos_sensor_sp0a08(0x35,0x00);
}

//zwl add for gc032a  W:0x42/R0x43
static kal_uint16 read_cmos_sensor_gc032a(kal_uint32 addr)
{
     return read_cmos_sensor_gc0310(addr);
}

static void write_cmos_sensor_gc032a(kal_uint32 addr, kal_uint32 para)
{
    write_cmos_sensor_gc0310(addr, para);
}

static void gc032a_init(void)
{
  //write_cmos_sensor_gc032a()
        write_cmos_sensor_gc032a(0xf3,0x83);
	write_cmos_sensor_gc032a(0xf5,0x0c);
	write_cmos_sensor_gc032a(0xf7,0x01); // gavin 20160820
	write_cmos_sensor_gc032a(0xf8,0x01);
	write_cmos_sensor_gc032a(0xf9,0x4e);
	write_cmos_sensor_gc032a(0xfa,0x00);// gavin 20160820
	write_cmos_sensor_gc032a(0xfc,0x02);
	write_cmos_sensor_gc032a(0xfe,0x02);
	write_cmos_sensor_gc032a(0x81,0x03); 

	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0x77,0x64);
	write_cmos_sensor_gc032a(0x78,0x40);
	write_cmos_sensor_gc032a(0x79,0x60);
	
	/*Analog&Cisctl*/
	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0x03,0x01);
	write_cmos_sensor_gc032a(0x04,0xce);
	write_cmos_sensor_gc032a(0x05,0x01);
	write_cmos_sensor_gc032a(0x06,0xad);
	write_cmos_sensor_gc032a(0x07,0x00);
	write_cmos_sensor_gc032a(0x08,0x10);
	write_cmos_sensor_gc032a(0x0a,0x00);
	write_cmos_sensor_gc032a(0x0c,0x00);
	write_cmos_sensor_gc032a(0x0d,0x01);
	write_cmos_sensor_gc032a(0x0e,0xe8);
	write_cmos_sensor_gc032a(0x0f,0x02);
	write_cmos_sensor_gc032a(0x10,0x88);
	write_cmos_sensor_gc032a(0x17,0x55);//54
	write_cmos_sensor_gc032a(0x19,0x08);
	write_cmos_sensor_gc032a(0x1a,0x0a);
	write_cmos_sensor_gc032a(0x1f,0x40);
	write_cmos_sensor_gc032a(0x20,0x30);
	write_cmos_sensor_gc032a(0x2e,0x80);
	write_cmos_sensor_gc032a(0x2f,0x2b);
	write_cmos_sensor_gc032a(0x30,0x1a);
	
	write_cmos_sensor_gc032a(0xfe,0x02);
	
	write_cmos_sensor_gc032a(0x03,0x02);
	write_cmos_sensor_gc032a(0x05,0xd7);
	write_cmos_sensor_gc032a(0x06,0x60);
	write_cmos_sensor_gc032a(0x08,0x80);
	write_cmos_sensor_gc032a(0x12,0x89);

	/*SPI*/
	write_cmos_sensor_gc032a(0xfe,0x03);
	write_cmos_sensor_gc032a(0x51,0x01);
	write_cmos_sensor_gc032a(0x52,0x78);//78 DDR Enable gavin 20160820
	write_cmos_sensor_gc032a(0x53,0xa4);
	write_cmos_sensor_gc032a(0x54,0x20);
	write_cmos_sensor_gc032a(0x55,0x00);
	write_cmos_sensor_gc032a(0x59,0x10);
	write_cmos_sensor_gc032a(0x5a,0x00);
	write_cmos_sensor_gc032a(0x5b,0x80);
	write_cmos_sensor_gc032a(0x5c,0x02);
	write_cmos_sensor_gc032a(0x5d,0xe0);
	write_cmos_sensor_gc032a(0x5e,0x01);

	write_cmos_sensor_gc032a(0x64,0x06); //SCK Always ON gavin 20160820
	/*blk*/
	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0x18,0x02);

	write_cmos_sensor_gc032a(0xfe,0x02);

	write_cmos_sensor_gc032a(0x40,0x22);
	write_cmos_sensor_gc032a(0x45,0x00);
	write_cmos_sensor_gc032a(0x46,0x00);
	write_cmos_sensor_gc032a(0x49,0x20);
	write_cmos_sensor_gc032a(0x4b,0x3c);
	write_cmos_sensor_gc032a(0x50,0x20);
	write_cmos_sensor_gc032a(0x42,0x10);

	/*isp*/
	write_cmos_sensor_gc032a(0xfe,0x01);
	write_cmos_sensor_gc032a(0x0a,0xc5);
	write_cmos_sensor_gc032a(0x45,0x00);
	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0x40,0xff);
	write_cmos_sensor_gc032a(0x41,0x25);
	write_cmos_sensor_gc032a(0x42,0xcf);
	write_cmos_sensor_gc032a(0x43,0x10);
	write_cmos_sensor_gc032a(0x44,0x83);//82
	write_cmos_sensor_gc032a(0x46,0x22);
	write_cmos_sensor_gc032a(0x49,0x03);
	write_cmos_sensor_gc032a(0x52,0x02);
	write_cmos_sensor_gc032a(0x54,0x00);	
	write_cmos_sensor_gc032a(0xfe,0x02);
	write_cmos_sensor_gc032a(0x22,0xf6);
	
	/*Shading*/
	write_cmos_sensor_gc032a(0xfe,0x01);
	write_cmos_sensor_gc032a(0xc1,0x38);
	write_cmos_sensor_gc032a(0xc2,0x4c);
	write_cmos_sensor_gc032a(0xc3,0x00);
	write_cmos_sensor_gc032a(0xc4,0x32);
	write_cmos_sensor_gc032a(0xc5,0x24);
	write_cmos_sensor_gc032a(0xc6,0x16);
	write_cmos_sensor_gc032a(0xc7,0x08);
	write_cmos_sensor_gc032a(0xc8,0x08);
	write_cmos_sensor_gc032a(0xc9,0x00);
	write_cmos_sensor_gc032a(0xca,0x20);
	write_cmos_sensor_gc032a(0xdc,0x8a);
	write_cmos_sensor_gc032a(0xdd,0xa0);
	write_cmos_sensor_gc032a(0xde,0xa6);
	write_cmos_sensor_gc032a(0xdf,0x75);
	
	/*AWB*//*20170110*/
	write_cmos_sensor_gc032a(0xfe, 0x01);
	write_cmos_sensor_gc032a(0x7c, 0x09);
	write_cmos_sensor_gc032a(0x65, 0x06);
	write_cmos_sensor_gc032a(0x7c, 0x08);
	write_cmos_sensor_gc032a(0x56, 0xf4);
	write_cmos_sensor_gc032a(0x66, 0x0f);
	write_cmos_sensor_gc032a(0x67, 0x84);
	write_cmos_sensor_gc032a(0x6b, 0x80);
	write_cmos_sensor_gc032a(0x6d, 0x12);
	write_cmos_sensor_gc032a(0x6e, 0xb0);
	write_cmos_sensor_gc032a(0xfe, 0x01);
	write_cmos_sensor_gc032a(0x90, 0x00);
	write_cmos_sensor_gc032a(0x91, 0x00);
	write_cmos_sensor_gc032a(0x92, 0xf4);
	write_cmos_sensor_gc032a(0x93, 0xd5);
	write_cmos_sensor_gc032a(0x95, 0x0f);
	write_cmos_sensor_gc032a(0x96, 0xf4);
	write_cmos_sensor_gc032a(0x97, 0x2d);
	write_cmos_sensor_gc032a(0x98, 0x0f);
	write_cmos_sensor_gc032a(0x9a, 0x2d);
	write_cmos_sensor_gc032a(0x9b, 0x0f);
	write_cmos_sensor_gc032a(0x9c, 0x59);
	write_cmos_sensor_gc032a(0x9d, 0x2d);
	write_cmos_sensor_gc032a(0x9f, 0x67);
	write_cmos_sensor_gc032a(0xa0, 0x59);
	write_cmos_sensor_gc032a(0xa1, 0x00);
	write_cmos_sensor_gc032a(0xa2, 0x00);
	write_cmos_sensor_gc032a(0x86, 0x00);
	write_cmos_sensor_gc032a(0x87, 0x00);
	write_cmos_sensor_gc032a(0x88, 0x00);
	write_cmos_sensor_gc032a(0x89, 0x00);
	write_cmos_sensor_gc032a(0xa4, 0x00);
	write_cmos_sensor_gc032a(0xa5, 0x00);
	write_cmos_sensor_gc032a(0xa6, 0xd4);
	write_cmos_sensor_gc032a(0xa7, 0x9f);
	write_cmos_sensor_gc032a(0xa9, 0xd4);
	write_cmos_sensor_gc032a(0xaa, 0x9f);
	write_cmos_sensor_gc032a(0xab, 0xac);
	write_cmos_sensor_gc032a(0xac, 0x9f);
	write_cmos_sensor_gc032a(0xae, 0xd4);
	write_cmos_sensor_gc032a(0xaf, 0xac);
	write_cmos_sensor_gc032a(0xb0, 0xd4);
	write_cmos_sensor_gc032a(0xb1, 0xa3);
	write_cmos_sensor_gc032a(0xb3, 0xd4);
	write_cmos_sensor_gc032a(0xb4, 0xac);
	write_cmos_sensor_gc032a(0xb5, 0x00);
	write_cmos_sensor_gc032a(0xb6, 0x00);
	write_cmos_sensor_gc032a(0x8b, 0x00);
	write_cmos_sensor_gc032a(0x8c, 0x00);
	write_cmos_sensor_gc032a(0x8d, 0x00);
	write_cmos_sensor_gc032a(0x8e, 0x00);
	write_cmos_sensor_gc032a(0x94, 0x50);
	write_cmos_sensor_gc032a(0x99, 0xa6);
	write_cmos_sensor_gc032a(0x9e, 0xaa);
	write_cmos_sensor_gc032a(0xa3, 0x0a);
	write_cmos_sensor_gc032a(0x8a, 0x00);
	write_cmos_sensor_gc032a(0xa8, 0x50);
	write_cmos_sensor_gc032a(0xad, 0x55);
	write_cmos_sensor_gc032a(0xb2, 0x55);
	write_cmos_sensor_gc032a(0xb7, 0x05);
	write_cmos_sensor_gc032a(0x8f, 0x00);
	write_cmos_sensor_gc032a(0xb8, 0xb3);
	write_cmos_sensor_gc032a(0xb9, 0xb6);
	
	/*CC*/
	write_cmos_sensor_gc032a(0xfe,0x01);	
	write_cmos_sensor_gc032a(0xd0,0x40);
	write_cmos_sensor_gc032a(0xd1,0xf8);
	write_cmos_sensor_gc032a(0xd2,0x00);
	write_cmos_sensor_gc032a(0xd3,0xfa);
	write_cmos_sensor_gc032a(0xd4,0x45);
	write_cmos_sensor_gc032a(0xd5,0x02);
	write_cmos_sensor_gc032a(0xd6,0x30);
	write_cmos_sensor_gc032a(0xd7,0xfa);
	write_cmos_sensor_gc032a(0xd8,0x08);
	write_cmos_sensor_gc032a(0xd9,0x08);
	write_cmos_sensor_gc032a(0xda,0x58);
	write_cmos_sensor_gc032a(0xdb,0x02);
	write_cmos_sensor_gc032a(0xfe,0x00);	

	/*Gamma*/
	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0xba,0x00);
	write_cmos_sensor_gc032a(0xbb,0x04);
	write_cmos_sensor_gc032a(0xbc,0x0a);
	write_cmos_sensor_gc032a(0xbd,0x0e);
	write_cmos_sensor_gc032a(0xbe,0x22);
	write_cmos_sensor_gc032a(0xbf,0x30);
	write_cmos_sensor_gc032a(0xc0,0x3d);
	write_cmos_sensor_gc032a(0xc1,0x4a);
	write_cmos_sensor_gc032a(0xc2,0x5d);
	write_cmos_sensor_gc032a(0xc3,0x6b);
	write_cmos_sensor_gc032a(0xc4,0x7a);
	write_cmos_sensor_gc032a(0xc5,0x85);
	write_cmos_sensor_gc032a(0xc6,0x90);
	write_cmos_sensor_gc032a(0xc7,0xa5);
	write_cmos_sensor_gc032a(0xc8,0xb5);
	write_cmos_sensor_gc032a(0xc9,0xc2);
	write_cmos_sensor_gc032a(0xca,0xcc);
	write_cmos_sensor_gc032a(0xcb,0xd5);
	write_cmos_sensor_gc032a(0xcc,0xde);
	write_cmos_sensor_gc032a(0xcd,0xea);
	write_cmos_sensor_gc032a(0xce,0xf5);
	write_cmos_sensor_gc032a(0xcf,0xff);
	
	/*Auto Gamma*/                      
	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0x5a,0x08);
	write_cmos_sensor_gc032a(0x5b,0x0f);
	write_cmos_sensor_gc032a(0x5c,0x15);
	write_cmos_sensor_gc032a(0x5d,0x1c);
	write_cmos_sensor_gc032a(0x5e,0x28);
	write_cmos_sensor_gc032a(0x5f,0x36);
	write_cmos_sensor_gc032a(0x60,0x45);
	write_cmos_sensor_gc032a(0x61,0x51);
	write_cmos_sensor_gc032a(0x62,0x6a);
	write_cmos_sensor_gc032a(0x63,0x7d);
	write_cmos_sensor_gc032a(0x64,0x8d);
	write_cmos_sensor_gc032a(0x65,0x98);
	write_cmos_sensor_gc032a(0x66,0xa2);
	write_cmos_sensor_gc032a(0x67,0xb5);
	write_cmos_sensor_gc032a(0x68,0xc3);
	write_cmos_sensor_gc032a(0x69,0xcd);
	write_cmos_sensor_gc032a(0x6a,0xd4);
	write_cmos_sensor_gc032a(0x6b,0xdc);
	write_cmos_sensor_gc032a(0x6c,0xe3);
	write_cmos_sensor_gc032a(0x6d,0xf0);
	write_cmos_sensor_gc032a(0x6e,0xf9);
	write_cmos_sensor_gc032a(0x6f,0xff);
	
	/*Gain*/
	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0x70,0x50);

	/*AEC*/
	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0x4f,0x01);
	write_cmos_sensor_gc032a(0xfe,0x01);
	write_cmos_sensor_gc032a(0x0d,0x00);//08 add 20170110 	
	write_cmos_sensor_gc032a(0x12,0xa0);
	write_cmos_sensor_gc032a(0x13,0x3a);
	write_cmos_sensor_gc032a(0x44,0x04);
	write_cmos_sensor_gc032a(0x1f,0x30);
	write_cmos_sensor_gc032a(0x20,0x40);
	write_cmos_sensor_gc032a(0x26,0x9a);
	write_cmos_sensor_gc032a(0x3e,0x20);
	write_cmos_sensor_gc032a(0x3f,0x2d);
	write_cmos_sensor_gc032a(0x40,0x40);
	write_cmos_sensor_gc032a(0x41,0x5b);
	write_cmos_sensor_gc032a(0x42,0x82);
	write_cmos_sensor_gc032a(0x43,0xb7);
	write_cmos_sensor_gc032a(0x04,0x0a);
	write_cmos_sensor_gc032a(0x02,0x79);
	write_cmos_sensor_gc032a(0x03,0xc0);

	/*measure window*/
	write_cmos_sensor_gc032a(0xfe,0x01);
	write_cmos_sensor_gc032a(0xcc,0x08);
	write_cmos_sensor_gc032a(0xcd,0x08);
	write_cmos_sensor_gc032a(0xce,0xa4);
	write_cmos_sensor_gc032a(0xcf,0xec);

	/*DNDD*/
	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0x81,0xb8);
	write_cmos_sensor_gc032a(0x82,0x12);
	write_cmos_sensor_gc032a(0x83,0x0a);
	write_cmos_sensor_gc032a(0x84,0x01);
	write_cmos_sensor_gc032a(0x86,0x50);
	write_cmos_sensor_gc032a(0x87,0x18);
	write_cmos_sensor_gc032a(0x88,0x10);
	write_cmos_sensor_gc032a(0x89,0x70);
	write_cmos_sensor_gc032a(0x8a,0x20);
	write_cmos_sensor_gc032a(0x8b,0x10);
	write_cmos_sensor_gc032a(0x8c,0x08);
	write_cmos_sensor_gc032a(0x8d,0x0a);

	/*Intpee*/
	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0x8f,0xaa);
	write_cmos_sensor_gc032a(0x90,0x9c);
	write_cmos_sensor_gc032a(0x91,0x52);
	write_cmos_sensor_gc032a(0x92,0x03);
	write_cmos_sensor_gc032a(0x93,0x03);
	write_cmos_sensor_gc032a(0x94,0x08);
	write_cmos_sensor_gc032a(0x95,0x44);
	write_cmos_sensor_gc032a(0x97,0x00);
	write_cmos_sensor_gc032a(0x98,0x00);
	
	/*ASDE*/
	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0xa1,0x30);
	write_cmos_sensor_gc032a(0xa2,0x41);
	write_cmos_sensor_gc032a(0xa4,0x30);
	write_cmos_sensor_gc032a(0xa5,0x20);
	write_cmos_sensor_gc032a(0xaa,0x30);
	write_cmos_sensor_gc032a(0xac,0x32);

	/*YCP*/
	write_cmos_sensor_gc032a(0xfe,0x00);
	write_cmos_sensor_gc032a(0xd1,0x3c);
	write_cmos_sensor_gc032a(0xd2,0x3c);
	write_cmos_sensor_gc032a(0xd3,0x38);
	write_cmos_sensor_gc032a(0xd6,0xf4);
	write_cmos_sensor_gc032a(0xd7,0x1d);
	write_cmos_sensor_gc032a(0xdd,0x73);
	write_cmos_sensor_gc032a(0xde,0x84);
        printk("gc032a init ok !!");
}

#define SP0821_WRITE_ID			0x86
static kal_uint16 read_cmos_sensor_sp0821(kal_uint32 addr)
{
	kal_uint16 get_byte=0;

	char pu_send_cmd[1] = {(char)(addr & 0xFF) };
	//kdSetI2CSpeed(100);
	iReadRegI2C6(pu_send_cmd, 1, (u8*)&get_byte, 1, SP0821_WRITE_ID);
	return get_byte;

}

static void write_cmos_sensor_sp0821(kal_uint32 addr, kal_uint32 para)
{
		char pu_send_cmd[2] = {(char)(addr & 0xFF), (char)(para & 0xFF)};
		//kdSetI2CSpeed(100);
		iWriteRegI2C6(pu_send_cmd, 2, SP0821_WRITE_ID);
}

static void sp0821_init(void)
{
	write_cmos_sensor_sp0821(0x31,0x02);

write_cmos_sensor_sp0821(0x19,0x04);
write_cmos_sensor_sp0821(0x2c,0x0f);
write_cmos_sensor_sp0821(0x2e,0x3c);
write_cmos_sensor_sp0821(0x30,0x01);

//0x10, 0x22,
write_cmos_sensor_sp0821(0x28,0x2e);//0x22,mjb1023
write_cmos_sensor_sp0821(0x29,0x1f);
write_cmos_sensor_sp0821(0x0a,0x48);
write_cmos_sensor_sp0821(0x0f,0x30);
write_cmos_sensor_sp0821(0x14,0xb0);
write_cmos_sensor_sp0821(0x38,0x50);
write_cmos_sensor_sp0821(0x39,0x52);
write_cmos_sensor_sp0821(0x3a,0x60);
write_cmos_sensor_sp0821(0x3b,0x10);
write_cmos_sensor_sp0821(0x3c,0xe0);
write_cmos_sensor_sp0821(0x85,0x01);
write_cmos_sensor_sp0821(0xe0,0x02);
write_cmos_sensor_sp0821(0xe5,0x60);
write_cmos_sensor_sp0821(0xf5,0x02);
write_cmos_sensor_sp0821(0xf1,0x03);//sig num
write_cmos_sensor_sp0821(0xf3,0x40);//scnt_dds  //mjb taiyangheizi
write_cmos_sensor_sp0821(0x41,0x00);

write_cmos_sensor_sp0821(0x03,0x00);//0x01
write_cmos_sensor_sp0821(0x04,0x9c);//0xc2
write_cmos_sensor_sp0821(0x05,0x00);//0x00
write_cmos_sensor_sp0821(0x06,0x00);//0x00
write_cmos_sensor_sp0821(0x07,0x00);
write_cmos_sensor_sp0821(0x08,0x00);
write_cmos_sensor_sp0821(0x09,0x00);//0x02;00
write_cmos_sensor_sp0821(0x0a,0x34);//0xf4;9c
write_cmos_sensor_sp0821(0x24,0x80);
write_cmos_sensor_sp0821(0x0D,0x01);//1
write_cmos_sensor_sp0821(0xc8,0x10);//y_bot_th
write_cmos_sensor_sp0821(0x32,0x00);//awb 
write_cmos_sensor_sp0821(0xc5,0xdf);//r 
write_cmos_sensor_sp0821(0xc6,0xb1);//b
write_cmos_sensor_sp0821(0xe7,0x03);//
write_cmos_sensor_sp0821(0xe7,0x00);//
write_cmos_sensor_sp0821(0x29,0x1e);//[0] 0ffset shift????  ;black level en


//caprure preview daylight 24M 50hz 14.07-8FPS
write_cmos_sensor_sp0821(0x03,0x00);	 
write_cmos_sensor_sp0821(0x04,0x8d);// 
write_cmos_sensor_sp0821(0xe7,0x03);//
write_cmos_sensor_sp0821(0xe7,0x00);//
write_cmos_sensor_sp0821(0x32,0x07);//			
write_cmos_sensor_sp0821(0x9b,0x2f);//4b ;base			 
write_cmos_sensor_sp0821(0x9c,0x00);//		 
write_cmos_sensor_sp0821(0xa2,0xd6);//1a;b0 ;exp			 
write_cmos_sensor_sp0821(0xa3,0x01);//04;04 ; 		 
write_cmos_sensor_sp0821(0xa4,0x2f);//4b ; 		 
write_cmos_sensor_sp0821(0xa5,0x00);//		 
write_cmos_sensor_sp0821(0xa8,0x2f);//4b ; 		 
write_cmos_sensor_sp0821(0xa9,0x00);//		 
write_cmos_sensor_sp0821(0xaa,0x01);// 		 
write_cmos_sensor_sp0821(0xab,0x00);//
                 
write_cmos_sensor_sp0821(0x4c,0x80);//gr pregain
write_cmos_sensor_sp0821(0x4d,0x80);//gb
                 
write_cmos_sensor_sp0821(0xa6,0xf0);//max indoor gain
write_cmos_sensor_sp0821(0xa7,0x10);//
write_cmos_sensor_sp0821(0xac,0xf0);//max rpc outdoor
write_cmos_sensor_sp0821(0xad,0x10);//
write_cmos_sensor_sp0821(0x8a,0x1f);//
write_cmos_sensor_sp0821(0x8b,0x18);//
write_cmos_sensor_sp0821(0x8c,0x15);//
write_cmos_sensor_sp0821(0x8d,0x13);//
write_cmos_sensor_sp0821(0x8e,0x13);//
write_cmos_sensor_sp0821(0x8f,0x12);//
write_cmos_sensor_sp0821(0x90,0x12);//
write_cmos_sensor_sp0821(0x91,0x11);//
write_cmos_sensor_sp0821(0x92,0x11);//
write_cmos_sensor_sp0821(0x93,0x11);//
write_cmos_sensor_sp0821(0x94,0x10);//
write_cmos_sensor_sp0821(0x95,0x10);//
write_cmos_sensor_sp0821(0x96,0x10);//
                 
//sat(by zhaifei)
write_cmos_sensor_sp0821(0x17,0x7a);//
write_cmos_sensor_sp0821(0x18,0x80);//
write_cmos_sensor_sp0821(0x4e,0x70);//;78 ;vsat
write_cmos_sensor_sp0821(0x4f,0x70);//;78 ;U
write_cmos_sensor_sp0821(0x58,0x80);// ;ku heq 90;
write_cmos_sensor_sp0821(0x59,0xa0);//90 ;kl  87;
write_cmos_sensor_sp0821(0x5a,0x80);//;hep mean
                 
//lum(by )
write_cmos_sensor_sp0821(0x86,0x08);//  auto lum lowlight
write_cmos_sensor_sp0821(0x87,0x0f);//
write_cmos_sensor_sp0821(0x88,0x30);//
write_cmos_sensor_sp0821(0x89,0x45);//
write_cmos_sensor_sp0821(0x98,0x88);//94target
write_cmos_sensor_sp0821(0x9e,0x84);//90
write_cmos_sensor_sp0821(0x9f,0x7c);//88
write_cmos_sensor_sp0821(0x97,0x78);//84
write_cmos_sensor_sp0821(0x9a,0x84);//90
write_cmos_sensor_sp0821(0xa0,0x80);//8c
write_cmos_sensor_sp0821(0xa1,0x78);//84
write_cmos_sensor_sp0821(0x99,0x88);//80 
write_cmos_sensor_sp0821(0x9d,0x0A);//
write_cmos_sensor_sp0821(0x34,0x0f);//
                 
//AWB;           
write_cmos_sensor_sp0821(0xB1,0x04);//skin Y_BOT  
write_cmos_sensor_sp0821(0xb3,0x00);//              
write_cmos_sensor_sp0821(0x47,0x40);//skin num th2       
write_cmos_sensor_sp0821(0xb8,0x04);//skin area                      
write_cmos_sensor_sp0821(0xb9,0x28);//skin num th              
write_cmos_sensor_sp0821(0x3f,0x18);//wt num th          
write_cmos_sensor_sp0821(0xbf,0x33);//ff;rg_limit_log/bg_limit_log   
write_cmos_sensor_sp0821(0xc1,0xff);//e0;rgain_top    
write_cmos_sensor_sp0821(0xc2,0x40);//rgain_bot    
write_cmos_sensor_sp0821(0xc3,0xff);//e0;bgain_top    
write_cmos_sensor_sp0821(0xc4,0x40);//bgain_bot    
write_cmos_sensor_sp0821(0xc5,0xdf);//77;buf_rgain    
write_cmos_sensor_sp0821(0xc6,0xb1);//cd;buf_bgain    
write_cmos_sensor_sp0821(0xc7,0xef);//y_top_th     
write_cmos_sensor_sp0821(0xc8,0x10);//y_bot_th     
write_cmos_sensor_sp0821(0x50,0x20);//18;09;0e;rg_dif_th2           
write_cmos_sensor_sp0821(0x51,0x20);//12;09;0e;bg_dif_th2           
write_cmos_sensor_sp0821(0x52,0x2f);//1f;0a;1f;rgb_limit2           
write_cmos_sensor_sp0821(0x53,0x90);//ee;9e;a8;rg_base2 e6,//            
write_cmos_sensor_sp0821(0x54,0xff);//b5;b4;bf;bg_base2 e7,//            
write_cmos_sensor_sp0821(0x5c,0x1e);//09;0e;rg_dif_th1   
write_cmos_sensor_sp0821(0x5d,0x21);//09;0e;bg_dif_th1   
write_cmos_sensor_sp0821(0x5e,0x1a);//0a;1f;rgb_limit1   
write_cmos_sensor_sp0821(0x5f,0xd8);//c3;c5;b0;rg_base1     
write_cmos_sensor_sp0821(0x60,0xaa);//8d;8c;8e;bg_base1     
write_cmos_sensor_sp0821(0xcb,0x12);//09;0e;rg_dif_th3   
write_cmos_sensor_sp0821(0xcc,0x12);//09;0e;bg_dif_th3                          
write_cmos_sensor_sp0821(0xcd,0x1f);//0a;1f;rgb_limit3   
write_cmos_sensor_sp0821(0xce,0x90);//79;79;79;75;rg_base3     
write_cmos_sensor_sp0821(0xcf,0xfa);//d5;d0;d5;b4;cc;bg_base3             
                 
//ccm      75                                        80 75 
write_cmos_sensor_sp0821(0x79,0x57);//5C;62;63;58;40;40;40;40;40;80;// 08/22   58;58;5C;
write_cmos_sensor_sp0821(0x7a,0xe0);//D7;DE;E9;E7;0 ;0 ;0 ;0 ;0 ;0 ;           E7;E7;D7;
write_cmos_sensor_sp0821(0x7b,0x9);//D ;0 ;F4;1 ;0 ;0 ;0 ;0 ;0 ;0 ;           0 ;1 ;D ;
write_cmos_sensor_sp0821(0x7c,0x2 );//5 ;5 ;F ;C ;F2;F2;Ec;EF;F6;EC;           C ;C ;5 ;
write_cmos_sensor_sp0821(0x7d,0x40);//36;36;3C;38;50;58;5F;5F;56;AD;           38;38;36;
write_cmos_sensor_sp0821(0x7e,0xfe );//5 ;5 ;F5;FC;FE;F6;F5;F2;F4;E7;           FB;FC;5 ;
write_cmos_sensor_sp0821(0x7f,0xb );//9 ;A ;D ;C ;F5;F7;F6;F6;FD;FA;           C ;C ;9 ;
write_cmos_sensor_sp0821(0x80,0xdf);//D7;D6;D9;E1;F1;F1;E8;E8;E4;C8;           E0;E1;D7;
write_cmos_sensor_sp0821(0x81,0x56);//
                 
                 
//gamma;         
write_cmos_sensor_sp0821(0x63,0x0 );//0822
write_cmos_sensor_sp0821(0x64,0xA );
write_cmos_sensor_sp0821(0x65,0x13);
write_cmos_sensor_sp0821(0x66,0x1C);
write_cmos_sensor_sp0821(0x67,0x25);
write_cmos_sensor_sp0821(0x68,0x37);
write_cmos_sensor_sp0821(0x69,0x46);
write_cmos_sensor_sp0821(0x6a,0x52);
write_cmos_sensor_sp0821(0x6b,0x5E);
write_cmos_sensor_sp0821(0x6c,0x75);
write_cmos_sensor_sp0821(0x6d,0x88);
write_cmos_sensor_sp0821(0x6e,0x9A);
write_cmos_sensor_sp0821(0x6f,0xA9);
write_cmos_sensor_sp0821(0x70,0xB5);
write_cmos_sensor_sp0821(0x71,0xC0);
write_cmos_sensor_sp0821(0x72,0xCA);
write_cmos_sensor_sp0821(0x73,0xD4);
write_cmos_sensor_sp0821(0x74,0xDD);
write_cmos_sensor_sp0821(0x75,0xE6);
write_cmos_sensor_sp0821(0x76,0xEc);
write_cmos_sensor_sp0821(0x77,0xF2);
write_cmos_sensor_sp0821(0x78,0xF6);
                 
//dns,sharpness( zhaifei)
write_cmos_sensor_sp0821(0x1b,0x0f);//02;01;02;02;03 ;05;dns_flat_thr1	dns       //822
write_cmos_sensor_sp0821(0x1c,0x1a);//06;02;03;04;05 ;08;dns_flat_thr2           
write_cmos_sensor_sp0821(0x1d,0x20);//0f;0a;04;05;0c;05 ;0a;dns_flat_thr3           
write_cmos_sensor_sp0821(0x1e,0x2f);//1a;10;07;08;0a;0f;05 ;ff;dns_flat_thr4        
write_cmos_sensor_sp0821(0x1f,0x0f);//06;04;04 ;05;sharp_flat_thr1 sharp      
write_cmos_sensor_sp0821(0x20,0x1a);//0A;06;07 ;0a;sharp_flat_thr2            
write_cmos_sensor_sp0821(0x21,0x20);//0D;08;12; 0a ;sharp_flat_thr3           
write_cmos_sensor_sp0821(0x22,0x2f);//12;10;1a; 12 ;0a;sharp_flat_thr4        
write_cmos_sensor_sp0821(0x56,0x37);//03;ff; raw_sharp_pos raw_sharp_neg
write_cmos_sensor_sp0821(0x1a,0x16);//
write_cmos_sensor_sp0821(0x34,0x1f);//
write_cmos_sensor_sp0821(0x82,0x10);//
write_cmos_sensor_sp0821(0x83,0x00);//
write_cmos_sensor_sp0821(0x84,0xff);//
write_cmos_sensor_sp0821(0xd7,0x70);//
write_cmos_sensor_sp0821(0xd8,0x1f);//
write_cmos_sensor_sp0821(0xd9,0x30);//

}


void main_sensor_set_shutter(kal_uint16 shutter)
{
        current_shutter = shutter;
}

kal_uint16 main_sensor_get_shutter(void)
{
        kal_uint16 temp = current_shutter;
        return temp;
}

void main_sensor_set_gain(kal_uint16 gain)
{
        current_gain = gain;
}

kal_uint16 main_sensor_get_gain(void)
{
        kal_uint16 temp = current_gain;
        return temp;
}

kal_uint16 detect_main2_light(void)
{
        kal_uint16 main2_light;
        //static int i = 0, j = 0, k = 0;
        //printk("<xieen>detect_main2_light\n");
  if (main2_sensor_id==GC8024MIPI_SENSOR_ID)
        {
          write_cmos_sensor_gc8024(0xfe,0x00);
          main2_light = read_cmos_sensor_gc8024(0xac);
          printk("<xieen>GC8024 0xab=0x%x\n",read_cmos_sensor_gc8024(0xab));
        }
        else if (main2_sensor_id==SP0A20_SENSOR_ID)
        {
          write_cmos_sensor_sp0a20(0xfd,0x02);
          main2_light = read_cmos_sensor_sp0a20(0xf2);
          printk("<mian2>SP0A20 0xf2=0x%x\n",read_cmos_sensor_sp0a20(0xf2));
        }
		else if (main2_sensor_id==GC030A_SENSOR_ID)
        {
  
             write_cmos_sensor_gc030a(0xfe,0x00);
             main2_light = read_cmos_sensor_gc030a(0xef);
	     printk("<xieen>GC030A 0xee=0x%x\n",read_cmos_sensor_gc030a(0xee));
             printk("<xieen>GC030A 0xef=0x%x\n",read_cmos_sensor_gc030a(0xef));
          }
	  else if (main2_sensor_id==GC0310_SENSOR_ID) //zwl add 20180308
        {
  
             write_cmos_sensor_gc0310(0xfe,0x01);
             main2_light = read_cmos_sensor_gc0310(0x14);
		  printk("<xieen>GC0310 0x14=0x%x\n",read_cmos_sensor_gc0310(0x14));
        }
        else if (main2_sensor_id==SP0A38_SENSOR_ID) //sp0a38_mipi_raw
        {
             write_cmos_sensor_sp0a38(0xfd,0x02);
             main2_light = read_cmos_sensor_sp0a38(0xf2);
             printk("<main2>sp0a38 0xf2=0x%x\n",read_cmos_sensor_sp0a38(0xf2));
        }
        else if (main2_sensor_id==SP0A08_SENSOR_ID) //sp0a08_mipi_raw
        {
             write_cmos_sensor_sp0a08(0xfd,0x00);
             main2_light = read_cmos_sensor_sp0a08(0xf2);
             printk("<main2>sp0a08 0xf2=0x%x\n",read_cmos_sensor_sp0a08(0xf2));
           }
		else if (main2_sensor_id==SP0821_SENSOR_ID)
        {
             write_cmos_sensor_sp0821(0xfd,0x00);
             main2_light = read_cmos_sensor_sp0821(0xb0);
             printk("<main2>sp0821 0xb0=0x%x\n",read_cmos_sensor_sp0821(0xb0));
        }
        else
          main2_light = 0xFF;

        //printk("<xieen>detect_main2_light=0x%x\n",main2_light);
        return main2_light;
}

void main2_cam_boot_detect(void)
{
      kdCISModulePowerOnMain2(1);

     bMain2PoweronStatus = 1;  //xen 20170328
    //main2_sensor_id = SP0A20_SENSOR_ID; 
#if defined(YK_CAM_SHELTER_SP0A20_MAIN_GC8024)
    if (main2_sensor_id==0xFFFF)
      {
        write_cmos_sensor_sp0a20(0xfd,0x00);
	main2_sensor_id = read_cmos_sensor_sp0a20(0x02);
        printk("<xieen>main2 sp0a20cam2 ID=0x%x\n", main2_sensor_id);

        if (main2_sensor_id != SP0A20_SENSOR_ID)
           {
              main2_sensor_id = (read_cmos_sensor_gc030a(0xf0) << 8)|read_cmos_sensor_gc030a(0xf1);
	      printk("<xieen>main2 gc030acam2 ID=0x%x\n", main2_sensor_id);

                if(main2_sensor_id != GC030A_SENSOR_ID) //zwl add start 20180303
                    {
                       main2_sensor_id = (read_cmos_sensor_gc0310(0xf0) << 8)|read_cmos_sensor_gc0310(0xf1);
	               printk("<xieen>main2 gc0310cam2 ID=0x%x\n", main2_sensor_id);
 
                         if(main2_sensor_id != GC0310_SENSOR_ID) 
                             { 
                                write_cmos_sensor_sp0a38(0xfd,0x00);
                                main2_sensor_id = (read_cmos_sensor_sp0a38(0x01) << 8)|read_cmos_sensor_sp0a38(0x02);
			                      printk("<xieen>main2 sp0a38cam2 ID=0x%x\n", main2_sensor_id);
                             }
                               if(main2_sensor_id != SP0A38_SENSOR_ID) //zwl add gc032a
                               { 
                                 main2_sensor_id = (read_cmos_sensor_gc032a(0xf0) << 8)|read_cmos_sensor_gc032a(0xf1);
			                      printk("<xieen>main2 gc032acam2 ID=0x%x\n", main2_sensor_id);
                               }
                                
                     }   //zwl add end
           }
      }
#else
    if (main2_sensor_id==0xFFFF)
    {
      main2_sensor_id = (read_cmos_sensor_gc8024(0xf0) << 8)|read_cmos_sensor_gc8024(0xf1);
      printk("<xieen>main2 cam ID=0x%x\n", main2_sensor_id);
      if (main2_sensor_id != GC8024MIPI_SENSOR_ID)
      {
	write_cmos_sensor_sp0a20(0xfd,0x00);
	main2_sensor_id = read_cmos_sensor_sp0a20(0x02);
        printk("<xieen>main2 cam2 ID=0x%x\n", main2_sensor_id);
		
		if(main2_sensor_id != SP0A20_SENSOR_ID)
		{
			main2_sensor_id = (read_cmos_sensor_gc030a(0xf0) << 8)|read_cmos_sensor_gc030a(0xf1);
			printk("<xieen>main2 cam2 ID=0x%x\n", main2_sensor_id);
           
                       if(main2_sensor_id != GC030A_SENSOR_ID) //zwl add start 20180303
                           {
                            main2_sensor_id = (read_cmos_sensor_gc0310(0xf0) << 8)|read_cmos_sensor_gc0310(0xf1);
			    printk("<xieen>main2 cam2 ID=0x%x\n", main2_sensor_id);
                                
                                if(main2_sensor_id != GC030A_SENSOR_ID) //gc0310
                                  { 
                                     main2_sensor_id = (read_cmos_sensor_gc0310(0xf0) << 8)|read_cmos_sensor_gc0310(0xf1);
	                             printk("<xieen>main2 gc030acam2 ID=0x%x\n", main2_sensor_id);
        
                                     if(main2_sensor_id != GC0310_SENSOR_ID)  //sp0a38
                                        { 
                                           write_cmos_sensor_sp0a38(0xfd,0x00);
                                           main2_sensor_id = (read_cmos_sensor_sp0a38(0x01) << 8)|read_cmos_sensor_sp0a38(0x02);
			                   printk("<xieen>main2 sp0a38cam2 ID=0x%x\n", main2_sensor_id);
                                         
                                           if(main2_sensor_id != SP0A38_SENSOR_ID) //zwl add gc032a
                                             { 
                                                main2_sensor_id = (read_cmos_sensor_gc032a(0xf0) << 8)|read_cmos_sensor_gc032a(0xf1);
			                        printk("<xieen>main2 gc032acam2 ID=0x%x\n", main2_sensor_id);

		                                if(main2_sensor_id != GC032A_SENSOR_ID)
		                                { 
                                           	    write_cmos_sensor_sp0a08(0xfd,0x00);
                                                    main2_sensor_id = read_cmos_sensor_sp0a08(0x02);
					            printk("<xieen>main2 sp0a08 cam2 ID=0x%x\n", main2_sensor_id);
							   							if (main2_sensor_id != SP0A08_SENSOR_ID)
														{
														write_cmos_sensor_sp0821(0xfd, 0x00);
														main2_sensor_id = (read_cmos_sensor_sp0821(0x01) << 8)|read_cmos_sensor_sp0821(0x02);
														printk("<xieen>main2 cam sp0821id=0x%x\n", main2_sensor_id);
														}
											if(main2_sensor_id == 0)
											{
												main2_sensor_id=0xFFFF;
											}
		                                 }
                                             }
                                        }   
                                 }   //sp0a38
                          }   
  
		}
                
       }
    }
#endif  
    //if (main2_sensor_id==GC8024MIPI_SENSOR_ID&&m_SensorSel == 2)  //add for main camera gc8024 yx 20170804
    if (main2_sensor_id==GC8024MIPI_SENSOR_ID)
    {
       snprintf(mtk_main2_cam_name,sizeof(mtk_main2_cam_name),"%s","gc8024_mipi_raw"); 
    }
    else if (main2_sensor_id==SP0A20_SENSOR_ID)
    {
       snprintf(mtk_main2_cam_name,sizeof(mtk_main2_cam_name),"%s","sp0a20_mipi_yuv"); 
    }
    else if (main2_sensor_id==GC030A_SENSOR_ID)
    {
       snprintf(mtk_main2_cam_name,sizeof(mtk_main2_cam_name),"%s","gc030a_mipi_raw"); 
    }
    else if (main2_sensor_id==GC0310_SENSOR_ID) //zwl add 20180303
    {
       snprintf(mtk_main2_cam_name,sizeof(mtk_main2_cam_name),"%s","gc0310_mipi_yuv"); 
    }
    else if (main2_sensor_id==SP0A38_SENSOR_ID) //zwl add 20180303
    {
       snprintf(mtk_main2_cam_name,sizeof(mtk_main2_cam_name),"%s","sp0a38_mipi_raw"); 
    }
    else if (main2_sensor_id==GC032A_SENSOR_ID) //zwl add 20180303
    {
       snprintf(mtk_main2_cam_name,sizeof(mtk_main2_cam_name),"%s","gc032a_mipi_raw"); 
    }
    else if (main2_sensor_id==SP0A08_SENSOR_ID)
    {
       snprintf(mtk_main2_cam_name,sizeof(mtk_main2_cam_name),"%s","sp0a08_mipi_raw"); 
    }
 else if (main2_sensor_id==SP0821_SENSOR_ID) 
    {
       snprintf(mtk_main2_cam_name,sizeof(mtk_main2_cam_name),"%s","sp0821"); 
    }
 else
    { 
      snprintf(mtk_main2_cam_name,sizeof(mtk_main2_cam_name),"%s","Main2 no found"); 
    }

 if (bMain2PoweronStatus == 1)
    { 
      bMain2PoweronStatus = 0;
      kdCISModulePowerOnMain2(0);
    }
}
void main2_cam_detect_init(void)
{
    kal_uint16 i=0;
    	//mtkcam_gpio_set(1);
	//imgsensor_hw_power(&pgimgsensor->hw, psensor, psensor_inst->psensor_name, IMGSENSOR_HW_POWER_STATUS_ON);
      kdCISModulePowerOnMain2(1);

     bMain2PoweronStatus = 1;  //xen 20170328
    //main2_sensor_id = SP0A20_SENSOR_ID; 
#if defined(YK_CAM_SHELTER_SP0A20_MAIN_GC8024)
    if (main2_sensor_id==0xFFFF)
      {
        write_cmos_sensor_sp0a20(0xfd,0x00);
	main2_sensor_id = read_cmos_sensor_sp0a20(0x02);
        printk("<xieen>main2 cam2 SP0A20ID=0x%x\n", main2_sensor_id);

        if (main2_sensor_id != SP0A20_SENSOR_ID)
           {
              main2_sensor_id = (read_cmos_sensor_gc030a(0xf0) << 8)|read_cmos_sensor_gc030a(0xf1);
	      printk("<xieen>main2 cam2 GC030AID=0x%x\n", main2_sensor_id);

                if(main2_sensor_id != GC030A_SENSOR_ID) //zwl add start 20180303
                    {
                           main2_sensor_id = (read_cmos_sensor_gc0310(0xf0) << 8)|read_cmos_sensor_gc0310(0xf1);
	                   printk("<xieen>main2 gc030acam2 ID=0x%x\n", main2_sensor_id);
                           if(main2_sensor_id != GC0310_SENSOR_ID) 
                             {      
                                 write_cmos_sensor_sp0a38(0xfd,0x00);
                                 main2_sensor_id = (read_cmos_sensor_sp0a38(0x01) << 8)|read_cmos_sensor_sp0a38(0x02);
			         printk("<xieen>main2 sp0a38cam2 ID=0x%x\n", main2_sensor_id);
                                 if(main2_sensor_id != SP0A38_SENSOR_ID) //zwl add gc032a
                                    { 
                                       main2_sensor_id = (read_cmos_sensor_gc032a(0xf0) << 8)|read_cmos_sensor_gc032a(0xf1);
			               printk("<xieen>main2 gc032acam2 ID=0x%x\n", main2_sensor_id);
                                     }
                              }   
                                         
                            }  				   
                     }   //zwl add end
           }
#else
    if (main2_sensor_id==0xFFFF)
    {
      main2_sensor_id = (read_cmos_sensor_gc8024(0xf0) << 8)|read_cmos_sensor_gc8024(0xf1);
      printk("<xieen>main2 cam gc8024ID=0x%x\n", main2_sensor_id);
       if (main2_sensor_id != GC8024MIPI_SENSOR_ID)
        {
	  write_cmos_sensor_sp0a20(0xfd,0x00);
	  main2_sensor_id = read_cmos_sensor_sp0a20(0x02);
          printk("<xieen>main2 cam2 sp0a20ID=0x%x\n", main2_sensor_id);
		
		if(main2_sensor_id != SP0A20_SENSOR_ID)
		  {
			main2_sensor_id = (read_cmos_sensor_gc030a(0xf0) << 8)|read_cmos_sensor_gc030a(0xf1);
			printk("<xieen>main2 cam2 gc030aID=0x%x\n", main2_sensor_id);
           
                       if(main2_sensor_id != GC030A_SENSOR_ID) //zwl add start 20180303
                           {
                                main2_sensor_id = (read_cmos_sensor_gc0310(0xf0) << 8)|read_cmos_sensor_gc0310(0xf1);
			        printk("<xieen>main2 cam2 gc0310ID=0x%x\n", main2_sensor_id);
      
                               if(main2_sensor_id != GC0310_SENSOR_ID) 
                                      {
                                          write_cmos_sensor_sp0a38(0xfd,0x00);
                                          main2_sensor_id = (read_cmos_sensor_sp0a38(0x01) << 8)|read_cmos_sensor_sp0a38(0x02);
			                  printk("<xieen>main2 sp0a38cam2 ID=0x%x\n", main2_sensor_id);

                                             if(main2_sensor_id != SP0A38_SENSOR_ID) //zwl add gc032a
                                               { 
                                                  main2_sensor_id = (read_cmos_sensor_gc032a(0xf0) << 8)|read_cmos_sensor_gc032a(0xf1);
			                          printk("<xieen>main2 gc032acam2 ID=0x%x\n", main2_sensor_id);

		                                  if(main2_sensor_id != GC032A_SENSOR_ID)
		                                  { 
                                           	     write_cmos_sensor_sp0a08(0xfd,0x00);
                                                     main2_sensor_id = read_cmos_sensor_sp0a08(0x02);
					             printk("<xieen>main2 sp0a08 cam2 ID=0x%x\n", main2_sensor_id);
											   				if (main2_sensor_id != SP0A08_SENSOR_ID)
														{
														write_cmos_sensor_sp0821(0xfd, 0x00);
														main2_sensor_id = (read_cmos_sensor_sp0821(0x01) << 8)|read_cmos_sensor_sp0821(0x02);
														printk("<xieen>main2 cam sp0821id=0x%x\n", main2_sensor_id);
														}
		                                  }
                                               }  
                                       }  
                            }     
                      }  	
            }
        }
#endif  
    //if (main2_sensor_id==GC8024MIPI_SENSOR_ID&&m_SensorSel == 2)  //add for main camera gc8024 yx 20170804
    if (main2_sensor_id==GC8024MIPI_SENSOR_ID)
    {
      gc8024_init();
      mDarkness_threshold = 0x40; //shelter light level theshold
      mShelterNear_threshold = 0x0A;
      mShelterFar_threshold = 0x10;
    }
    else if (main2_sensor_id==SP0A20_SENSOR_ID)
    {
      sp0a20_init();
      mDarkness_threshold = 0x09; //shelter light level theshold
      mShelterNear_threshold = 0x03;
      mShelterFar_threshold = 0x04;
    }
    else if (main2_sensor_id==GC030A_SENSOR_ID)
    {
      gc030a_init();
      mDarkness_threshold = 0x09; //shelter light level theshold
      mShelterNear_threshold = 0x03;
      mShelterFar_threshold = 0x04;
    }
    else if (main2_sensor_id==GC0310_SENSOR_ID) //zwl add 20180303
    {
      gc0310_init();
      mDarkness_threshold = 0x09; //shelter light level theshold
      mShelterNear_threshold = 0x03;
      mShelterFar_threshold = 0x04;
    }
    else if (main2_sensor_id==SP0A38_SENSOR_ID) //zwl add 20180303
    {
      sp0a38_init();
      mDarkness_threshold = 0x09; //shelter light level theshold
      mShelterNear_threshold = 0x03;
      mShelterFar_threshold = 0x04;
    }
    else if (main2_sensor_id==GC032A_SENSOR_ID) //zwl add 20180409
    {
      gc032a_init();
      mDarkness_threshold = 0x09; 
      mShelterNear_threshold = 0x03;
      mShelterFar_threshold = 0x04;
    }
    else if (main2_sensor_id==SP0A08_SENSOR_ID)
    {
      sp0a08_init();
      mDarkness_threshold = 0x09; //shelter light level theshold
      mShelterNear_threshold = 0x03;
      mShelterFar_threshold = 0x04;
    }	

		else if (main2_sensor_id==SP0821_SENSOR_ID) 
    {
      sp0821_init();
      mDarkness_threshold = 0x40; 
      mShelterNear_threshold = 0x30;
      mShelterFar_threshold = 0x38;
    }
      mDarkness_threshold = 0;//0x40; //shelter light level theshold
      mShelterNear_threshold = 0x0A;
      mShelterFar_threshold = 0x10;

    //re-initialize variables
    gFirstTimeAfterPreview = 1;

    for (i=0;i<5;i++)
      gMain2LightLevel[i] = 0;

    main2_light_pre=0x0;
    main2_light_current=0;
    pre_shutter = 0xFFFF;
    current_shutter = 0;
    pre_gain = 0xFFFF;
    isNotification = 0;
    isNotification_pre = 0;
    needNotifyMMI_near = 0; //xen 20170301
    needNotifyMMI_far = 0;

    notifyTimes=0; // xen 20170303

    main2_light_pre_pre = 0;

}

void main2_detect_work_callback(struct work_struct *work)
{
        kal_uint16 j = 0;
        kal_uint16 temp=0;
        kal_uint16 temp_main2_pre_pre=main2_light_pre_pre; //xen 20170308
        kal_uint16 temp_main2_pre=main2_light_pre; //xen 20170306
        kal_uint16 temp_current_shutter = main_sensor_get_shutter();
        kal_uint16 temp_current_gain = main_sensor_get_gain();
        static kal_uint16 temp_Main2LightLevel[5]={0}; //xen 20170330
        kal_uint16 flag0=0; //xen 20170331
        kal_uint16 flag1=0;
        //kal_uint16 flag2=0;

        kal_uint16 temp_1=0; //added by xen 20170308
        static kal_uint16 near_times=0; //if 3 times satisfied condition by xen 20170308
        //kal_uint16 far_times=0;

        //printk("<xieen>main2_detect_work_callback\n");

        printk("<xieen>sensor driver:gain value=%d,pre_gain=%d\n",temp_current_gain,pre_gain);  
        main2_light_current = detect_main2_light();
        printk("<xieen>sensor driver:main2 light=%d,pre_main2_light=%d,pre_pre_main2_light=%d\n",
                 main2_light_current,temp_main2_pre,temp_main2_pre_pre);
        printk("<xieen>sensor driver:current shutter=%d,pre_shutter=%d\n",temp_current_shutter,pre_shutter); //current_shutter
        printk("<xieen>sensor driver:notifyTimes=%d\n",notifyTimes);

        if (notifyTimes==0) //(needNotifyMMI_near==0)&&(needNotifyMMI_far==0)) //while (notifyTimes==1),main2 is in shelter area
        {
           if (iSum==5)
           {
              temp_1 = temp_Main2LightLevel[0];//gMain2LightLevel[0];  //temparily save the first value
              for (j=0;j<(iSum-1);j++)
              {
                gMain2LightLevel[j] = gMain2LightLevel[j+1];
                temp_Main2LightLevel[j] = temp_Main2LightLevel[j+1];//gMain2LightLevel[j+1]; //xen 20170330
              }
              gMain2LightLevel[iSum-1] = main2_light_current;
              temp_Main2LightLevel[iSum-1] = main2_light_current;  //xen 20170330
           }
           else
           {
              gMain2LightLevel[iSum] = main2_light_current;
              temp_Main2LightLevel[iSum] = main2_light_current;
           }

           if (iSum<5)
              iSum++;
        }
        else //added by xen 20170401,if entering shelter area, temp_Main2LightLevel values still need to be recorded
        {
           if (iSum==5)
           {
              for (j=0;j<(iSum-1);j++)
              {
                //gMain2LightLevel[j] = gMain2LightLevel[j+1];
                temp_Main2LightLevel[j] = temp_Main2LightLevel[j+1];//gMain2LightLevel[j+1]; //xen 20170330
              }
              //gMain2LightLevel[iSum-1] = main2_light_current;
              temp_Main2LightLevel[iSum-1] = main2_light_current;  //xen 20170330
           }
           else
           {
              temp_Main2LightLevel[iSum] = main2_light_current;
           }

           if (iSum<5)
              iSum++;
        }

        if (iSum==5) //after 5 times,begin to judge
        {
           for (j=0;j<iSum;j++)
           {
             //printk("<xieen>gMain2LightLevel[%d]=%d\n", j, gMain2LightLevel[j]);  //xen 20170330
             //temp += gMain2LightLevel[j];
             printk("<xieen>temp_gMain2LightLevel[%d]=%d\n", j, temp_Main2LightLevel[j]);
             temp += gMain2LightLevel[j];//temp_Main2LightLevel[j];
           }
 
           mDarkness_threshold = temp/10;  //5 times and then half
           mShelterNear_threshold = mDarkness_threshold/2;
           mShelterFar_threshold = mDarkness_threshold/2;

           if (mDarkness_threshold<5) mDarkness_threshold=5;  // 1
           if (mShelterNear_threshold<3) mShelterNear_threshold=3;  // 1
           if (mShelterFar_threshold<3) mShelterFar_threshold=3;  // 1
           printk("<xieen2>mDarkness_threshold=%d,NearThr=%d,FarThr=%d\n", mDarkness_threshold,mShelterNear_threshold,mShelterFar_threshold);

           //added by xen 20170331 begin // filter 00010 or 00110 situation
           if ((temp_Main2LightLevel[4]<mDarkness_threshold)&&(temp_Main2LightLevel[2]<mDarkness_threshold)&&(temp_Main2LightLevel[1]<mDarkness_threshold)&&(temp_Main2LightLevel[0]<mDarkness_threshold)&&(temp_Main2LightLevel[3]>mDarkness_threshold))
               flag0 = 1;

           if ((temp_Main2LightLevel[4]<mDarkness_threshold)&&(temp_Main2LightLevel[1]<mDarkness_threshold)&&(temp_Main2LightLevel[0]<mDarkness_threshold)&&(temp_Main2LightLevel[3]>mDarkness_threshold)&&(temp_Main2LightLevel[2]>mDarkness_threshold))
               flag1 = 1;
           //added by xen 20170331 End

           if (main2_light_current<mDarkness_threshold) //0x40)  //this value judges whether camera lens darkness sheltered or not
           {
                 //printk("<xieen>Into Shelter Area!!!\n");
                 isNotification = 1;
           }
           else if (main2_light_current>mDarkness_threshold) //If these two values are equal,ignore this
           {
                 //printk("<xieen>Out of Shelter Area!!!\n");
                 isNotification = 0;
           }
           //printk("<xieen>main2_light=%d,main2_light_pre=%d\n",main2_light_current,temp_main2_pre);
           if ((isNotification == 1)&&(isNotification_pre==0)&&((main2_light_current+mShelterNear_threshold)<=temp_main2_pre)) //near to
           {
              printk("<xieen>From out of Shelter into Shelter area!!\n");
              needNotifyMMI_far = 0; //xen 20170308
			  #if !defined(YK686_CUSTOMER_YINMAI_S64390_HDPLUS)
              if (((temp_current_gain>=pre_gain)&&(current_shutter>pre_shutter))||((temp_current_gain>pre_gain)&&(current_shutter>=pre_shutter))) //if main camera light level also decrease,we dont set sheltered
              {
                 needNotifyMMI_near = 0;
                 near_times = 0; //xen 20170331
              }
              else
			  #endif
              {
                 if ((flag0==0)&&(flag1==0)) //xen 20170331
                 {
                 needNotifyMMI_near = 1;
                 near_times++; //xen 20170331
                 }
                 else
                 {
                 needNotifyMMI_near = 0;
                 near_times = 0; 
                 }
              }
           }
           //else if ((isNotification==1)&&(isNotification_pre==1)&&((main2_light_current+mShelterNear_threshold)<=temp_main2_pre_pre))//xen 0331
           else if ((isNotification==1)&&(isNotification_pre==1)&&((main2_light_current+mShelterNear_threshold)<temp_main2_pre_pre)&&
            (main2_light_current<=temp_main2_pre)&&(temp_main2_pre_pre>=mDarkness_threshold)) //less than last time,2nd time,0401 from > to >=
           {
              printk("<xieen>2nd time into Shelter area!!\n");
              needNotifyMMI_far = 0; //xen 20170308
			  #if !defined(YK686_CUSTOMER_YINMAI_S64390_HDPLUS)
              if (((temp_current_gain>=pre_gain)&&(current_shutter>pre_shutter))||((temp_current_gain>pre_gain)&&(current_shutter>=pre_shutter))) //if main camera light level also decrease,we dont set sheltered
              {
                 needNotifyMMI_near = 0;
                 near_times = 0; //xen 20170331
              }
              else
			  #endif
              {
                 if ((flag0==0)&&(flag1==0)) //xen 20170331
                 {
                 needNotifyMMI_near = 1;
                 near_times++; //xen 20170331
                 }
                 else
                 {
                 needNotifyMMI_near = 0;
                 near_times = 0; 
                 }
              }
           }
           //3rd time near by xen 20170331
           else if ((isNotification==1)&&(isNotification_pre==1)&&((main2_light_current+mShelterNear_threshold)<temp_Main2LightLevel[1])&&
            (main2_light_current<=temp_main2_pre)&&(temp_main2_pre<=temp_main2_pre_pre)&&(temp_main2_pre_pre<=temp_Main2LightLevel[1])&&(temp_Main2LightLevel[1]>=mDarkness_threshold)) //0401 from > to >=
           {
              printk("<xieen>3rd time into Shelter area!!\n");
              needNotifyMMI_far = 0; //xen 20170308
			  #if !defined(YK686_CUSTOMER_YINMAI_S64390_HDPLUS)
              if (((temp_current_gain>=pre_gain)&&(current_shutter>pre_shutter))||((temp_current_gain>pre_gain)&&(current_shutter>=pre_shutter))) //if main camera light level also decrease,we dont set sheltered
              {
                 needNotifyMMI_near = 0;
                 near_times = 0; //xen 20170331
              }
              else
			  #endif
              {
                 if ((flag0==0)&&(flag1==0)) //xen 20170331
                 {
                 needNotifyMMI_near = 1;
                 near_times++; //xen 20170331
                 }
                 else
                 {
                 needNotifyMMI_near = 0;
                 near_times = 0; 
                 }
              }
           }
           else if ((isNotification == 0)&&(isNotification_pre==1)&&((temp_main2_pre+mShelterFar_threshold)<main2_light_current)) //far away
           {
               printk("<xieen>From inner of Shelter out from Shelter area!!\n");
               needNotifyMMI_near = 0; //xen 20170308
               near_times = 0; //xen 20170331
               if (notifyTimes==1)  //last period near function happened
               {
                 needNotifyMMI_far = 1;
               }
               else
                 needNotifyMMI_far = 0;
           }
           //added by xen for special situations 20170401-begin
           else if ((isNotification == 0)&&(isNotification_pre==0)&&((mDarkness_threshold+mShelterFar_threshold)<main2_light_current)) //far away
           {
               printk("<xieen>Continual out of Shelter area!!\n");
               needNotifyMMI_near = 0; //xen 20170308
               near_times = 0; //xen 20170331
               if (notifyTimes==1)  //last period near function happened
               {
                 needNotifyMMI_far = 1;
               }
               else
                 needNotifyMMI_far = 0;
           }
           //added by xen for special situations 20170401-end
           else
           {
                 near_times = 0; //xen 20170331
                 needNotifyMMI_near = 0;
                 needNotifyMMI_far = 0;
           }
        }

        printk("<xieen>sensor driver:near_times=%d\n",near_times);
        if ((needNotifyMMI_near==1)&&(notifyTimes==0)) //isNotification==1)
        {
              ///printk("<xieen>Send Shelter Message to MMI!!!!\n");
              //near_times++;  //xen 20170308,xen 20170331
              if (near_times>=3) // 2,modified by xen 20170331
              {
                printk("<xieen>Send Shelter Message to MMI!!!!\n");
                notifyTimes++;
                kpd_detect_shelter_near_handler();
              }
             //dismiss this time main2 light value from SUM light values by xen 20170308
              for (j=0;j<4;j++)
              {
                gMain2LightLevel[j+1] = gMain2LightLevel[j];
              }
              gMain2LightLevel[0] = temp_1;
        }
        else if ((needNotifyMMI_far==1)&&(notifyTimes==1))
        {
             printk("<xieen>Send Far-Away Message to MMI!!!!\n");
             notifyTimes--;
             kpd_detect_shelter_far_handler();
             near_times=0; //xen 20170308
        }
        else
        {
             near_times=0; //xen 20170308
        }


        isNotification_pre = isNotification;

        main2_light_pre_pre = main2_light_pre; //xen 20170308

        main2_light_pre = main2_light_current; //current light level set to pre
        pre_shutter = main_sensor_get_shutter();//current_shutter;
        pre_gain = main_sensor_get_gain(); //current_gain;
        
        if (bMain2_timer_status==1) //xen 20170328
          mod_timer(&main2_detect_timer, jiffies + MAIN2_DETECT_TIMEOUT);
}

void main2_detect_timeout(unsigned long a)
{
        //printk("<xieen>main2_detect_timeout\n");
        if (bMain2_timer_status==1) //xen 20170329
          queue_work(main2_detect_workqueue, &main2_detect_work);

}
#endif

static int imgsensor_remove(struct platform_device *pdev)
{
	imgsensor_i2c_delete();
	imgsensor_driver_unregister();

	return 0;
}

static int imgsensor_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	return 0;
}

static int imgsensor_resume(struct platform_device *pdev)
{
	return 0;
}

/*
 * platform driver
 */

#ifdef CONFIG_OF
static const struct of_device_id gimgsensor_of_device_id[] = {
	{ .compatible = "mediatek,camera_hw", },
	{}
};
#endif

static struct platform_driver gimgsensor_platform_driver = {
	.probe      = imgsensor_probe,
	.remove     = imgsensor_remove,
	.suspend    = imgsensor_suspend,
	.resume     = imgsensor_resume,
	.driver     = {
		.name   = "image_sensor",
		.owner  = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = gimgsensor_of_device_id,
#endif
	}
};

/*
 * imgsensor_init()
 */
static int __init imgsensor_init(void)
{
	pr_info("[camerahw_probe] start\n");

	if (platform_driver_register(&gimgsensor_platform_driver)) {
		pr_err("failed to register CAMERA_HW driver\n");
		return -ENODEV;
	}

#ifdef CONFIG_CAM_TEMPERATURE_WORKQUEUE
	memset((void *)&cam_temperature_wq, 0, sizeof(cam_temperature_wq));
	INIT_DELAYED_WORK(
	    &cam_temperature_wq,
	    cam_temperature_report_wq_routine);

	schedule_delayed_work(&cam_temperature_wq, HZ);
#endif
#ifdef IMGSENSOR_DFS_CTRL_ENABLE
	imgsensor_dfs_ctrl(DFS_CTRL_ENABLE, NULL);
#endif

	return 0;
}

/*
 * imgsensor_exit()
 */
static void __exit imgsensor_exit(void)
{
#ifdef IMGSENSOR_DFS_CTRL_ENABLE
	imgsensor_dfs_ctrl(DFS_CTRL_DISABLE, NULL);
#endif
	platform_driver_unregister(&gimgsensor_platform_driver);
}

module_init(imgsensor_init);
module_exit(imgsensor_exit);

MODULE_DESCRIPTION("image sensor driver");
MODULE_AUTHOR("Mediatek");
MODULE_LICENSE("GPL v2");

