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

#include "gpio.h"
#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //xjl 20180619
#include <linux/delay.h>
#endif

struct GPIO_PINCTRL gpio_pinctrl_list_cam[GPIO_CTRL_STATE_MAX_NUM_CAM] = {
	/* Main */
	{"pnd1"},
	{"pnd0"},
	{"rst1"},
	{"rst0"},
	{"vcama_on"},
	{"vcama_off"},
	{"vcamd_on"},
	{"vcamd_off"},
	{"vcamio_on"},
	{"vcamio_off"},
};

#ifdef MIPI_SWITCH
struct GPIO_PINCTRL gpio_pinctrl_list_switch[GPIO_CTRL_STATE_MAX_NUM_SWITCH] = {
	{"cam_mipi_switch_en_1"},
	{"cam_mipi_switch_en_0"},
	{"cam_mipi_switch_sel_1"},
	{"cam_mipi_switch_sel_0"}
};
#endif

#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //xjl 20180619
struct pinctrl *camctrl = NULL;
struct pinctrl_state *cam2_pnd_h = NULL;
struct pinctrl_state *cam2_pnd_l = NULL;
struct pinctrl_state *cam2_rst_h = NULL;
struct pinctrl_state *cam2_rst_l = NULL;

#endif

static struct GPIO gpio_instance;

/*
 * reset all state of gpio to default value
 */
static enum IMGSENSOR_RETURN gpio_release(void *pinstance)
{
	int i, j;
	struct platform_device *pplatform_dev = gpimgsensor_hw_platform_device;
	struct GPIO            *pgpio         = (struct GPIO *)pinstance;
	enum   IMGSENSOR_RETURN ret           = IMGSENSOR_RETURN_SUCCESS;
	char *lookup_names = NULL;

	pgpio->ppinctrl = devm_pinctrl_get(&pplatform_dev->dev);
	if (IS_ERR(pgpio->ppinctrl))
		return IMGSENSOR_RETURN_ERROR;
	for (j = IMGSENSOR_SENSOR_IDX_MIN_NUM;
	j < IMGSENSOR_SENSOR_IDX_MAX_NUM;
	j++) {
		for (i = GPIO_CTRL_STATE_PDN_L;
		i < GPIO_CTRL_STATE_MAX_NUM_CAM;
		i += 2) {
			lookup_names =
				gpio_pinctrl_list_cam[i].ppinctrl_lookup_names;
			mutex_lock(&pinctrl_mutex);
			if (lookup_names != NULL &&
				pgpio->ppinctrl_state_cam[j][i] != NULL &&
				  !IS_ERR(pgpio->ppinctrl_state_cam[j][i]) &&
				pinctrl_select_state(pgpio->ppinctrl,
					pgpio->ppinctrl_state_cam[j][i])) {
				pr_info(
				    "%s : pinctrl err, PinIdx %d name %s\n",
				    __func__,
				    i,
				    lookup_names);
			}
			mutex_unlock(&pinctrl_mutex);
		}
	}

	return ret;
}
static enum IMGSENSOR_RETURN gpio_init(void *pinstance)
{
	int    i, j;
	struct platform_device *pplatform_dev = gpimgsensor_hw_platform_device;
	struct GPIO            *pgpio         = (struct GPIO *)pinstance;
	enum   IMGSENSOR_RETURN ret           = IMGSENSOR_RETURN_SUCCESS;
	char str_pinctrl_name[LENGTH_FOR_SNPRINTF];
	char *lookup_names = NULL;


	pgpio->ppinctrl = devm_pinctrl_get(&pplatform_dev->dev);
	if (IS_ERR(pgpio->ppinctrl)) {
		pr_err("%s : Cannot find camera pinctrl!", __func__);
		return IMGSENSOR_RETURN_ERROR;
	}
	for (j = IMGSENSOR_SENSOR_IDX_MIN_NUM;
	j < IMGSENSOR_SENSOR_IDX_MAX_NUM;
	j++) {
		for (i = 0; i < GPIO_CTRL_STATE_MAX_NUM_CAM; i++) {
			lookup_names =
				gpio_pinctrl_list_cam[i].ppinctrl_lookup_names;
			if (lookup_names) {
				snprintf(str_pinctrl_name,
					sizeof(str_pinctrl_name),
					"cam%d_%s",
					j,
					lookup_names);
				pgpio->ppinctrl_state_cam[j][i] =
					pinctrl_lookup_state(
					    pgpio->ppinctrl,
					    str_pinctrl_name);

				if (pgpio->ppinctrl_state_cam[j][i] == NULL ||
				    IS_ERR(pgpio->ppinctrl_state_cam[j][i])) {
					pr_info(
					    "%s : pinctrl err, %s\n",
					    __func__,
					    str_pinctrl_name);

					ret = IMGSENSOR_RETURN_ERROR;
				}
			}
		}
	}
#ifdef MIPI_SWITCH
	for (i = 0; i < GPIO_CTRL_STATE_MAX_NUM_SWITCH; i++) {
		if (gpio_pinctrl_list_switch[i].ppinctrl_lookup_names) {
			pgpio->ppinctrl_state_switch[i] =
				pinctrl_lookup_state(
					pgpio->ppinctrl,
			gpio_pinctrl_list_switch[i].ppinctrl_lookup_names);
		}

		if (pgpio->ppinctrl_state_switch[i] == NULL ||
			IS_ERR(pgpio->ppinctrl_state_switch[i])) {
			pr_info(
				"%s : pinctrl err, %s\n",
				__func__,
			gpio_pinctrl_list_switch[i].ppinctrl_lookup_names);
			ret = IMGSENSOR_RETURN_ERROR;
		}
	}
#endif
#if defined(_MAIN2_CAM_SHELTER_DESIGN2_) //xjl 20180619
       	camctrl = devm_pinctrl_get(&pplatform_dev->dev);
	if (IS_ERR(camctrl)) {
		dev_err(&pplatform_dev->dev, "Cannot find camera pinctrl!");
		ret = PTR_ERR(camctrl);
	}
    
        cam2_pnd_h = pinctrl_lookup_state(camctrl, "cam2_pnd1");
	if (IS_ERR(cam2_pnd_h)) {
		ret = PTR_ERR(cam2_pnd_h);
		pr_debug("%s : pinctrl err, cam2_pnd_h\n", __func__);
	}

	cam2_pnd_l = pinctrl_lookup_state(camctrl, "cam2_pnd0");
	if (IS_ERR(cam2_pnd_l)) {
		ret = PTR_ERR(cam2_pnd_l);
		pr_debug("%s : pinctrl err, cam2_pnd_l\n", __func__);
	}

	cam2_rst_h = pinctrl_lookup_state(camctrl, "cam2_rst1");
	if (IS_ERR(cam2_rst_h)) {
		ret = PTR_ERR(cam2_rst_h);
		pr_debug("%s : pinctrl err, cam2_rst_h\n", __func__);
	}

	cam2_rst_l = pinctrl_lookup_state(camctrl, "cam2_rst0");
	if (IS_ERR(cam2_rst_l)) {
		ret = PTR_ERR(cam2_rst_l);
		pr_debug("%s : pinctrl err, cam2_rst_l\n", __func__);
	}

#endif
	return ret;
}

static enum IMGSENSOR_RETURN gpio_set(
	void *pinstance,
	enum IMGSENSOR_SENSOR_IDX   sensor_idx,
	enum IMGSENSOR_HW_PIN       pin,
	enum IMGSENSOR_HW_PIN_STATE pin_state)
{
	struct pinctrl_state        *ppinctrl_state;
	struct GPIO                 *pgpio = (struct GPIO *)pinstance;
	enum   GPIO_STATE            gpio_state;


	if (pin < IMGSENSOR_HW_PIN_PDN ||
#ifdef MIPI_SWITCH
	   pin > IMGSENSOR_HW_PIN_MIPI_SWITCH_SEL ||
#else
	   pin > IMGSENSOR_HW_PIN_AFVDD ||
#endif
	   pin_state < IMGSENSOR_HW_PIN_STATE_LEVEL_0 ||
	   pin_state > IMGSENSOR_HW_PIN_STATE_LEVEL_HIGH)
		return IMGSENSOR_RETURN_ERROR;

	gpio_state = (pin_state > IMGSENSOR_HW_PIN_STATE_LEVEL_0)
	    ? GPIO_STATE_H : GPIO_STATE_L;

#ifdef MIPI_SWITCH
	if (pin == IMGSENSOR_HW_PIN_MIPI_SWITCH_EN)
		ppinctrl_state = pgpio->ppinctrl_state_switch[
		    GPIO_CTRL_STATE_MIPI_SWITCH_EN_H + gpio_state];

	else if (pin == IMGSENSOR_HW_PIN_MIPI_SWITCH_SEL)
		ppinctrl_state = pgpio->ppinctrl_state_switch[
		    GPIO_CTRL_STATE_MIPI_SWITCH_SEL_H + gpio_state];

	else
#endif
	{
		ppinctrl_state =
		    pgpio->ppinctrl_state_cam[sensor_idx][
			((pin - IMGSENSOR_HW_PIN_PDN) << 1) + gpio_state];

	}
	/*pr_debug("%s : pinctrl , state indx %d\n",
	 *	    __func__,
	 *	    ctrl_state_offset +
	 *	    ((pin - IMGSENSOR_HW_PIN_PDN) << 1) + gpio_state);
	 */

	mutex_lock(&pinctrl_mutex);
	if (ppinctrl_state == NULL ||
		IS_ERR(ppinctrl_state) ||
		pinctrl_select_state(pgpio->ppinctrl, ppinctrl_state))
		pr_err(
		    "%s : pinctrl err, PinIdx %d, Val %d\n",
		    __func__,
		    pin, pin_state);

	mutex_unlock(&pinctrl_mutex);

	return IMGSENSOR_RETURN_SUCCESS;
}

#if defined(_MAIN2_CAM_SHELTER_DESIGN2_)  //xjl 20180619
int mtkcam_gpio_set(enum GPIO_CTRL_STATE shelter_pin_state)
{
        int ret = 0;

        mutex_lock(&pinctrl_mutex);
	switch (shelter_pin_state) {
	        case GPIO_CTRL_STATE_CAM2_RST_L:
                            if (cam2_rst_l != NULL && !IS_ERR(cam2_rst_l))
                                {
				pinctrl_select_state(camctrl, cam2_rst_l); 
                                pr_err("<zwl> main2 rst low!!\n");
                                break;
                                  }  
	        case GPIO_CTRL_STATE_CAM2_RST_H:
                             if (cam2_rst_h != NULL && !IS_ERR(cam2_rst_h)){
                           	pinctrl_select_state(camctrl, cam2_rst_h);
                                pr_err("<zwl> mian2 rst HIGH!!\n");
		                break;
                                }
	        case GPIO_CTRL_STATE_CAM2_PDN_L:
                              if (cam2_pnd_l != NULL && !IS_ERR(cam2_pnd_l)){
                                pinctrl_select_state(camctrl, cam2_pnd_l);
                                pr_err("<zwl> mian2 pdn low!!\n");
		                break;
                                 }
                case GPIO_CTRL_STATE_CAM2_PDN_H:
                               if (cam2_pnd_h != NULL && !IS_ERR(cam2_pnd_h)){
                                pinctrl_select_state(camctrl, cam2_pnd_h);   
                                pr_err("<zwl> mian2 pdn high!!\n");
		                break;
                                }

               default:
		                pr_err("PwrType(%d) is invalid !!\n", shelter_pin_state);
                                ret = 1;
		               break; 
                              }
                mutex_unlock(&pinctrl_mutex);
               return 0;
}

int kdCISModulePowerOnMain2(bool On)
{
    if (On)
       {
#if 1

          if(mtkcam_gpio_set(GPIO_CTRL_STATE_CAM2_RST_L)){pr_err("[CAMERA SENSOR] set gpio failed!! \n");}
          mdelay(5);//10
          if(mtkcam_gpio_set(GPIO_CTRL_STATE_CAM2_RST_H)){pr_err("[CAMERA SENSOR] set gpio failed!! \n");} 
          mdelay(2);//10
        #if defined(REVERSE_PDN_ONE_ZERO) //reseve the pdn seq
          if(mtkcam_gpio_set(GPIO_CTRL_STATE_CAM2_PDN_H)){pr_err("[CAMERA LENS] set gpio failed!! \n");}
        #else
          if(mtkcam_gpio_set(GPIO_CTRL_STATE_CAM2_PDN_L)){pr_err("[CAMERA LENS] set gpio failed!! \n");} 
	  #if 1 //defined(YK739_CUSTOMER_ZHUOXINTE_Z5201_HD720) //add for sp0a38
          mdelay(1);//10
          if(mtkcam_gpio_set(GPIO_CTRL_STATE_CAM2_PDN_H)){pr_err("[CAMERA LENS] set gpio failed!! \n");}
          mdelay(1);//10
          if(mtkcam_gpio_set(GPIO_CTRL_STATE_CAM2_PDN_L)){pr_err("[CAMERA LENS] set gpio failed!! \n");} 
          #endif
        #endif
       }
       else
       {
#if 1

          //mdelay(10);        
          if(mtkcam_gpio_set(GPIO_CTRL_STATE_CAM2_RST_L)){pr_err("[CAMERA SENSOR] set gpio failed!! \n");} 
        #if defined(REVERSE_PDN_ONE_ZERO)
          if(mtkcam_gpio_set(GPIO_CTRL_STATE_CAM2_PDN_L)){pr_err("[CAMERA LENS] set gpio failed!! \n");} 
        #else
          mdelay(3);
          if(mtkcam_gpio_set(GPIO_CTRL_STATE_CAM2_PDN_H)){pr_err("[CAMERA LENS] set gpio failed!! \n");} 
        #endif
#endif

        }
#endif
      return 0;
}

#endif



static struct IMGSENSOR_HW_DEVICE device = {
	.pinstance = (void *)&gpio_instance,
	.init      = gpio_init,
	.set       = gpio_set,
	.release   = gpio_release,
	.id        = IMGSENSOR_HW_ID_GPIO
};

enum IMGSENSOR_RETURN imgsensor_hw_gpio_open(
	struct IMGSENSOR_HW_DEVICE **pdevice)
{
	*pdevice = &device;
	return IMGSENSOR_RETURN_SUCCESS;
}

