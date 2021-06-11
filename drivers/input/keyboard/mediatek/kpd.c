/*
 * Copyright (C) 2010 MediaTek, Inc.
 *
 * Author: Terry Chang <terry.chang@mediatek.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "kpd.h"
#ifdef CONFIG_PM_WAKELOCKS
#include <linux/pm_wakeup.h>
#else
#include <linux/wakelock.h>
#endif
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/clk.h>
#include <linux/debugfs.h>

#define KPD_NAME	"mtk-kpd"

#ifdef CONFIG_LONG_PRESS_MODE_EN
struct timer_list Long_press_key_timer;
atomic_t vol_down_long_press_flag = ATOMIC_INIT(0);
#endif

int kpd_klog_en;
void __iomem *kp_base;
static unsigned int kp_irqnr;
struct input_dev *kpd_input_dev;
static struct dentry *kpd_droot;
static struct dentry *kpd_dklog;
unsigned long call_status;
static bool kpd_suspend;
static unsigned int kp_irqnr;
static u32 kpd_keymap[KPD_NUM_KEYS];
static u16 kpd_keymap_state[KPD_NUM_MEMS];

struct input_dev *kpd_input_dev;
#ifdef CONFIG_PM_WAKELOCKS
struct wakeup_source kpd_suspend_lock;
#else
struct wake_lock kpd_suspend_lock;
#endif
struct keypad_dts_data kpd_dts_data;

#if defined(HALL_FUNCTION_SUPPORT)     //added by xen for test HALL function 20130819
//#include "cust_eint.h"
//#define KPD_HALL_SWITCH_EINT           CUST_EINT_MHALL_NUM
//#define KPD_HALL_SWITCH_DEBOUNCE       50    //CUST_EINT_MHALL_DEBOUNCE_CN
//#define KPD_HALL_SWITCH_POLARITY       0     //CUST_EINT_MHALL_POLARITY
//#define KPD_HALL_SWITCH_SENSITIVE      CUST_EINT_MHALL_TYPE   //CUST_EINT_MHALL_SENSITIVE
//#define KPD_HALL_SWITCH_POLARITY       CUST_EINT_MHALL_POLARITY
//#define KPD_HALL_SWITCH_SENSITIVE      CUST_EINT_MHALL_SENSITIVE
u32 mhall_irq_number = 0;
struct delayed_work mhall_eint_work;
//static void kpd_halldet_handler(unsigned long data);
//static DECLARE_TASKLET(kpd_halldet_tasklet, kpd_halldet_handler, 0);
static u8 kpd_halldet_state = 1; //!KPD_HALL_SWITCH_POLARITY; //HALL_FAR_AWAY;  //!KPD_PWRKEY_POLARITY;
#endif


/* for keymap handling */
static void kpd_keymap_handler(unsigned long data);
static DECLARE_TASKLET(kpd_keymap_tasklet, kpd_keymap_handler, 0);

static void kpd_memory_setting(void);
static int kpd_pdrv_probe(struct platform_device *pdev);
static int kpd_pdrv_suspend(struct platform_device *pdev, pm_message_t state);
static int kpd_pdrv_resume(struct platform_device *pdev);
static struct platform_driver kpd_pdrv;

static void kpd_memory_setting(void)
{
	kpd_init_keymap(kpd_keymap);
	kpd_init_keymap_state(kpd_keymap_state);
}

static ssize_t kpd_store_call_state(struct device_driver *ddri,
		const char *buf, size_t count)
{
	int ret;

	ret = kstrtoul(buf, 10, &call_status);
	if (ret) {
		kpd_print("kpd call state: Invalid values\n");
		return -EINVAL;
	}

	switch (call_status) {
	case 1:
		kpd_print("kpd call state: Idle state!\n");
		break;
	case 2:
		kpd_print("kpd call state: ringing state!\n");
		break;
	case 3:
		kpd_print("kpd call state: active or hold state!\n");
		break;

	default:
		kpd_print("kpd call state: Invalid values\n");
		break;
	}
	return count;
}

static ssize_t kpd_show_call_state(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%ld\n", call_status);
	return res;
}

static DRIVER_ATTR(kpd_call_state, 0644, kpd_show_call_state,
		kpd_store_call_state);
#if defined(YK676_CUSTOMER_CHUNMEI_HDPLUS)
#define YK_NO_ALSPS_CONFIG
#endif

#if defined(CONFIG_TERACUBE_2E)
u8 gesture_open_state = 1;
static ssize_t kpd_store_tp_gesture_state(struct device_driver *ddri, const char *buf, size_t count)
{
	if(strncmp("yes",buf,3)==0){
		gesture_open_state = 1;
		kpd_print("[TP_GESTURE] tp_sysfs_tpgesturet_store on.\n");
	}
	else if(strncmp("no",buf,2)==0){
		gesture_open_state = 0;
		kpd_print("[TP_GESTURE] tp_sysfs_tpgesturet_store off.\n");
	}

	return count;
}

static ssize_t kpd_show_tp_gesture_state(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%d\n", gesture_open_state);
	return res;
}

static DRIVER_ATTR(kpd_tp_gesture_state, S_IWUSR | S_IRUGO, kpd_show_tp_gesture_state, kpd_store_tp_gesture_state);

#endif
extern char mtk_lcm_name[300];
static ssize_t lcm_info_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%s\n", mtk_lcm_name);
	return res;
}

static DRIVER_ATTR(lcm_info, 0444, lcm_info_show,NULL);


extern char mtk_main_cam_name[50];
static ssize_t cam_main_info_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%s\n", mtk_main_cam_name);
	return res;
}

static DRIVER_ATTR(cam_main_info, 0444, cam_main_info_show,NULL);

extern char mtk_main2_cam_name[50];
static ssize_t cam_main2_info_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%s\n", mtk_main2_cam_name);
	return res;
}

static DRIVER_ATTR(cam_main2_info, 0444, cam_main2_info_show,NULL);

extern char mtk_sub_cam_name[50];
static ssize_t cam_sub_info_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%s\n", mtk_sub_cam_name);
	return res;
}

static DRIVER_ATTR(cam_sub_info, 0444, cam_sub_info_show,NULL);

#if !(defined(YK_NO_ACCEL_CONFIG)||defined(CONFIG_CUSTOM_KERNEL_SENSORHUB))
extern char mtk_accel_name[128];
static ssize_t gsensor_info_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%s\n", mtk_accel_name);
	return res;
}

static DRIVER_ATTR(gsensor_info, 0444, gsensor_info_show,NULL);
#endif

#if !(defined(YK_NO_ALSPS_CONFIG)||defined(CONFIG_CUSTOM_KERNEL_SENSORHUB))
extern char mtk_alsps_name[128];
static ssize_t alsps_info_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%s\n", mtk_alsps_name);
	return res;
}

static DRIVER_ATTR(alsps_info, 0444, alsps_info_show,NULL);
#endif

#define fingerprint_info_size 128
char mtk_fingerprint_name[fingerprint_info_size] = {0};
static ssize_t fingerprint_info_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;
#if defined(FINGERPRINT_SUPPORT) //xen 20170427
	res = snprintf(buf, PAGE_SIZE, "%s\n", mtk_fingerprint_name);
#else
	res = snprintf(buf, PAGE_SIZE, "%s\n", "fingerprint not support");
#endif

	return res;
}

static DRIVER_ATTR(fingerprint_info, 0444, fingerprint_info_show,NULL);


extern char mtk_tp_name[128];
static ssize_t tp_info_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%s\n", mtk_tp_name);
	return res;
}

static DRIVER_ATTR(tp_info, 0444, tp_info_show,NULL);

char mtk_tp_version[128] = {0};
static ssize_t tp_version_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%s\n", mtk_tp_version);
	return res;
}

static DRIVER_ATTR(tp_version, 0444, tp_version_show,NULL);


extern char mtk_flash_cid[128];
static ssize_t flash_info_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%s\n", mtk_flash_cid);
	return res;
}

static DRIVER_ATTR(flash_info, 0444, flash_info_show,NULL);


//tplink
extern unsigned int breath_mode;//tplink
static ssize_t kpd_store_breath_mode(struct device_driver *ddri,
		const char *buf, size_t count)
{
	int ret;

	ret = kstrtouint(buf, 0, &breath_mode);
	if (ret) {
		kpd_print("kpd breath mode: Invalid values\n");
		return -EINVAL;
	}


	return count;
}

static ssize_t kpd_show_breath_mode(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%d\n", breath_mode);
	return res;
}

static DRIVER_ATTR(led_breath_mode, 0644, kpd_show_breath_mode,
		kpd_store_breath_mode);
//end

extern int pmic_get_charging_current(void);
extern int mtk_chr_is_charger_exist(unsigned char *exist);
static ssize_t Charging_Current_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	int ret_value = 0;
	unsigned char exist = 0;
        ret_value=pmic_get_charging_current();
	mtk_chr_is_charger_exist(&exist);
        if(ret_value<0)
	        ret_value=0;
	if(exist==0)
		ret_value=0;
	res = sprintf(buf, "%d\n", ret_value);
	return res;
}

static DRIVER_ATTR(Charging_Current, 0444, Charging_Current_show,NULL);
#if defined(CONFIG_NFC_CHIP_SUPPORT)
extern int is_nfc_exist;
static ssize_t nfc_info_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	int ret_value = 0;
	ret_value=is_nfc_exist;

	res = sprintf(buf, "%d\n", ret_value);
	return res;
}

static DRIVER_ATTR(nfc_info, 0444, nfc_info_show,NULL);
#endif

extern unsigned int yk_stop_percent;
static ssize_t kpd_store_stop_charging_percent(struct device_driver *ddri,
		const char *buf, size_t count)
{
	int ret;

	ret = kstrtouint(buf, 0, &yk_stop_percent);
	if (ret) {
		kpd_print("kpd yk_stop_percent: Invalid values\n");
		return -EINVAL;
	}


	return count;
}

static ssize_t kpd_show_stop_charging_percent(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%d\n", yk_stop_percent);
	return res;
}

static DRIVER_ATTR(stop_charging_percent, 0644, kpd_show_stop_charging_percent,
		kpd_store_stop_charging_percent);

#if defined(CONFIG_TERACUBE_2E)
extern int yk_wireless_charge_flag;
static ssize_t wireless_info_show(struct device_driver *ddri, char *buf)
{
	ssize_t res;
	int ret_value = 0;
	ret_value=yk_wireless_charge_flag;

	res = sprintf(buf, "%d\n", ret_value);
	return res;
}

static DRIVER_ATTR(wireless_info, 0444, wireless_info_show,NULL);
#endif
#if defined(CONFIG_USB_MTK_OTG)
extern unsigned int yk_otg_enable;
extern int eta6937_enable_otg_ex( bool en);//chenjingchen add 
static ssize_t kpd_store_yk_otg_enable(struct device_driver *ddri,
		const char *buf, size_t count)
{
	int ret;

	ret = kstrtouint(buf, 0, &yk_otg_enable);
	if (ret) {
		kpd_print("kpd yk_otg_enable: Invalid values\n");
		return -EINVAL;
	}
    //chenjingchen add start
    if(yk_otg_enable==1)
    {
        eta6937_enable_otg_ex(1);
    }
    else
    {
        eta6937_enable_otg_ex(0);
    }//chenjingchen end



	return count;
}

static ssize_t kpd_show_yk_otg_enable(struct device_driver *ddri, char *buf)
{
	ssize_t res;

	res = snprintf(buf, PAGE_SIZE, "%d\n", yk_otg_enable);
	return res;
}

static DRIVER_ATTR(yk_otg_enable, 0644, kpd_show_yk_otg_enable,
		kpd_store_yk_otg_enable);
#endif

static struct driver_attribute *kpd_attr_list[] = {
	&driver_attr_kpd_call_state,
#if defined(CONFIG_TERACUBE_2E)
	&driver_attr_kpd_tp_gesture_state,
#endif
	&driver_attr_lcm_info,
	&driver_attr_tp_info,
	&driver_attr_tp_version,
	&driver_attr_cam_main_info,
	&driver_attr_cam_main2_info,
	&driver_attr_cam_sub_info,
	#if !(defined(YK_NO_ACCEL_CONFIG)||defined(CONFIG_CUSTOM_KERNEL_SENSORHUB))
	&driver_attr_gsensor_info,
	#endif
	#if !(defined(YK_NO_ALSPS_CONFIG)||defined(CONFIG_CUSTOM_KERNEL_SENSORHUB))
	&driver_attr_alsps_info,
	#endif
	&driver_attr_fingerprint_info,
	&driver_attr_flash_info,
        &driver_attr_led_breath_mode,//tplink
	&driver_attr_Charging_Current,
	#if defined(CONFIG_NFC_CHIP_SUPPORT)
	&driver_attr_nfc_info,
	#endif
	&driver_attr_stop_charging_percent,
	#if defined(CONFIG_TERACUBE_2E)
	&driver_attr_wireless_info,
	#endif
    #if defined(CONFIG_USB_MTK_OTG)
    &driver_attr_yk_otg_enable,
    #endif
};

static int kpd_create_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = ARRAY_SIZE(kpd_attr_list);

	if (driver == NULL)
		return -EINVAL;

	for (idx = 0; idx < num; idx++) {
		err = driver_create_file(driver, kpd_attr_list[idx]);
		if (err) {
			kpd_info("driver_create_file (%s) = %d\n",
				kpd_attr_list[idx]->attr.name, err);
			break;
		}
	}
	return err;
}

static int kpd_delete_attr(struct device_driver *driver)
{
	int idx, err = 0;
	int num = ARRAY_SIZE(kpd_attr_list);

	if (!driver)
		return -EINVAL;

	for (idx = 0; idx < num; idx++)
		driver_remove_file(driver, kpd_attr_list[idx]);

	return err;
}

#if defined(HALL_FUNCTION_SUPPORT)          //added by xen for test HALL function 20130819
//extern void lcm_backlight_close(void);     //test by xen
//extern void lcm_backlight_open(void); 
//extern int old_level;
//static void kpd_halldet_handler(unsigned long data)
static irqreturn_t kpd_halldet_eint_handler(int irq, void *desc);
static void kpd_halldet_handler(struct work_struct *work)
{
	bool pressed;
	//u8 old_state = kpd_halldet_state;

        printk("\nxxx==kpd_halldet_handler hall_state=%d\n", kpd_halldet_state);
	kpd_halldet_state = 1 - kpd_halldet_state;
	pressed = (kpd_halldet_state == 0); //!!KPD_HALL_SWITCH_POLARITY);
	//if (kpd_show_hw_keycode) {
	//	printk(KPD_SAY "(%s) HW keycode = using EINT\n",
	//	       pressed ? "pressed" : "released");
	//}

	if (pressed)  //kpd_halldet_state==HALL_CLOSE_TO)
	{
	   input_report_key(kpd_input_dev, KEY_HALL_CLOSE, 1);
	   input_sync(kpd_input_dev);
	   input_report_key(kpd_input_dev, KEY_HALL_CLOSE, 0);
	   input_sync(kpd_input_dev);

	   printk("\nxxxHALL_CLOSE_TO===\n");
	   irq_set_irq_type(mhall_irq_number, IRQ_TYPE_LEVEL_HIGH);

	}
	else
	{
	   input_report_key(kpd_input_dev, KEY_HALL_FARAWAY, 1);
	   input_sync(kpd_input_dev);
	   input_report_key(kpd_input_dev, KEY_HALL_FARAWAY, 0);
	   input_sync(kpd_input_dev);

	   printk("\nxxxHALL_FAR_AWAY===\n");
	   irq_set_irq_type(mhall_irq_number, IRQ_TYPE_LEVEL_LOW);

	}

	/* for detecting the return to old_state */
	//mt_eint_set_polarity(KPD_HALL_SWITCH_EINT, old_state);
	//mt_eint_unmask(KPD_HALL_SWITCH_EINT);
	//mt_eint_set_polarity(KPD_HALL_SWITCH_EINT, old_state);
	//mt_eint_unmask(KPD_HALL_SWITCH_EINT);
	//request_irq(mhall_irq_number, kpd_halldet_eint_handler, IRQF_TRIGGER_HIGH, "mhall-eint", NULL)
	enable_irq(mhall_irq_number);
}

//static void kpd_halldet_eint_handler(void)
static irqreturn_t kpd_halldet_eint_handler(int irq, void *desc)
{
        //printk("\nxxx==kpd_halldet_eint_handler===\n");
	disable_irq_nosync(mhall_irq_number);
	//tasklet_schedule(&kpd_halldet_tasklet);
	schedule_delayed_work(&mhall_eint_work, 0);
	return IRQ_HANDLED;
}
#endif

#if defined (ALSPS_GESTURE)  // wanghe 2014-01-08
void kpd_alsps_gesture_handler(int key_code)
{
	   input_report_key(kpd_input_dev, key_code, 1);
	   input_sync(kpd_input_dev);
	   input_report_key(kpd_input_dev, key_code, 0);
	   input_sync(kpd_input_dev);
}
#endif

#if defined (CONFIG_TERACUBE_2E)
void kpd_touchpanel_gesture_handler(int key_code)
{
	//for vibrate soon
	//upmu_set_rg_vibr_en(1); //xjl 20140526
	//mdelay(50);
	//upmu_set_rg_vibr_en(0);

	input_report_key(kpd_input_dev, key_code, 1);
	input_sync(kpd_input_dev);
	input_report_key(kpd_input_dev, key_code, 0);
	input_sync(kpd_input_dev);
}
#endif
#if defined(_SUB2_CAM_SHELTER_DESIGN2_)||defined(_MAIN2_CAM_SHELTER_DESIGN2_) //xen 20170211
void kpd_detect_shelter_near_handler(void)
{
	input_report_key(kpd_input_dev, KEY_CAM_SHELTER_NEAR, 1);
	input_sync(kpd_input_dev);
	input_report_key(kpd_input_dev, KEY_CAM_SHELTER_NEAR, 0);
	input_sync(kpd_input_dev);
        
}

void kpd_detect_shelter_far_handler(void)
{
	input_report_key(kpd_input_dev, KEY_CAM_SHELTER_FAR, 1);
	input_sync(kpd_input_dev);
	input_report_key(kpd_input_dev, KEY_CAM_SHELTER_FAR, 0);
	input_sync(kpd_input_dev);
}
#endif

/****************************************/
#ifdef CONFIG_LONG_PRESS_MODE_EN
void vol_down_long_press(unsigned long pressed)
{
	atomic_set(&vol_down_long_press_flag, 1);
}
#endif

#if defined(FINGERPRINT_SUPPORT) //xen 20170427
void fingerprint_set_name(char* name)
{
    snprintf(mtk_fingerprint_name,sizeof(mtk_fingerprint_name),"%s",name);
}
#endif
/*****************************************/

#ifdef CONFIG_KPD_PWRKEY_USE_PMIC
void kpd_pwrkey_pmic_handler(unsigned long pressed)
{
	kpd_print("Power Key generate, pressed=%ld\n", pressed);
	if (!kpd_input_dev) {
		kpd_print("KPD input device not ready\n");
		return;
	}
	kpd_pmic_pwrkey_hal(pressed);
}
#endif

void kpd_pmic_rstkey_handler(unsigned long pressed)
{
	kpd_print("PMIC reset Key generate, pressed=%ld\n", pressed);
	if (!kpd_input_dev) {
		kpd_print("KPD input device not ready\n");
		return;
	}
	kpd_pmic_rstkey_hal(pressed);
}

static void kpd_keymap_handler(unsigned long data)
{
	u16 i, j;
	int32_t pressed;
	u16 new_state[KPD_NUM_MEMS], change, mask;
	u16 hw_keycode, linux_keycode;
	void *dest;

	kpd_get_keymap_state(new_state);
#ifdef CONFIG_PM_WAKELOCKS
	__pm_wakeup_event(&kpd_suspend_lock, 500);
#else
	wake_lock_timeout(&kpd_suspend_lock, HZ / 2);
#endif
	for (i = 0; i < KPD_NUM_MEMS; i++) {
		change = new_state[i] ^ kpd_keymap_state[i];
		if (change == 0U)
			continue;

		for (j = 0; j < 16U; j++) {
			mask = (u16) 1 << j;
			if ((change & mask) == 0U)
				continue;

			hw_keycode = (i << 4) + j;

			if (hw_keycode >= KPD_NUM_KEYS)
				continue;

			/* bit is 1: not pressed, 0: pressed */
			pressed = ((new_state[i] & mask) == 0U) ? 1 : 0;
			kpd_print("(%s) HW keycode = %d\n",
				(pressed == 1) ? "pressed" : "released",
					hw_keycode);

			linux_keycode = kpd_keymap[hw_keycode];
			if (linux_keycode == 0U)
				continue;
			input_report_key(kpd_input_dev, linux_keycode, pressed);
			input_sync(kpd_input_dev);
			kpd_print("report Linux keycode = %d\n", linux_keycode);

#ifdef CONFIG_LONG_PRESS_MODE_EN
			if (pressed) {
				init_timer(&Long_press_key_timer);
				Long_press_key_timer.expires = jiffies + 5*HZ;
				Long_press_key_timer.data =
					(unsigned long)pressed;
				Long_press_key_timer.function =
					vol_down_long_press;
				add_timer(&Long_press_key_timer);
			} else {
				del_timer_sync(&Long_press_key_timer);
			}
			if (!pressed &&
				atomic_read(&vol_down_long_press_flag)) {
				atomic_set(&vol_down_long_press_flag, 0);
			}
#endif
		}
	}

	dest = memcpy(kpd_keymap_state, new_state, sizeof(new_state));
	enable_irq(kp_irqnr);
}

static irqreturn_t kpd_irq_handler(int irq, void *dev_id)
{
	/* use _nosync to avoid deadlock */
	disable_irq_nosync(kp_irqnr);
	tasklet_schedule(&kpd_keymap_tasklet);
	return IRQ_HANDLED;
}

static int kpd_open(struct input_dev *dev)
{
	/* void __user *uarg = (void __user *)arg; */
	return 0;
}

void kpd_get_dts_info(struct device_node *node)
{
	int32_t ret;

	of_property_read_u32(node, "mediatek,kpd-key-debounce",
		&kpd_dts_data.kpd_key_debounce);
	of_property_read_u32(node, "mediatek,kpd-sw-pwrkey",
		&kpd_dts_data.kpd_sw_pwrkey);
	of_property_read_u32(node, "mediatek,kpd-hw-pwrkey",
		&kpd_dts_data.kpd_hw_pwrkey);
	of_property_read_u32(node, "mediatek,kpd-sw-rstkey",
		&kpd_dts_data.kpd_sw_rstkey);
	of_property_read_u32(node, "mediatek,kpd-hw-rstkey",
		&kpd_dts_data.kpd_hw_rstkey);
	of_property_read_u32(node, "mediatek,kpd-use-extend-type",
		&kpd_dts_data.kpd_use_extend_type);
	of_property_read_u32(node, "mediatek,kpd-hw-dl-key1",
		&kpd_dts_data.kpd_hw_dl_key1);
	of_property_read_u32(node, "mediatek,kpd-hw-dl-key2",
		&kpd_dts_data.kpd_hw_dl_key2);
	of_property_read_u32(node, "mediatek,kpd-hw-dl-key3",
		&kpd_dts_data.kpd_hw_dl_key3);
	of_property_read_u32(node, "mediatek,kpd-hw-recovery-key",
		&kpd_dts_data.kpd_hw_recovery_key);
	of_property_read_u32(node, "mediatek,kpd-hw-factory-key",
		&kpd_dts_data.kpd_hw_factory_key);
	of_property_read_u32(node, "mediatek,kpd-hw-map-num",
		&kpd_dts_data.kpd_hw_map_num);
	ret = of_property_read_u32_array(node, "mediatek,kpd-hw-init-map",
		kpd_dts_data.kpd_hw_init_map,
			kpd_dts_data.kpd_hw_map_num);

	if (ret) {
		kpd_print("kpd-hw-init-map was not defined in dts.\n");
		memset(kpd_dts_data.kpd_hw_init_map, 0,
			sizeof(kpd_dts_data.kpd_hw_init_map));
	}

	kpd_print("deb= %d, sw-pwr= %d, hw-pwr= %d, hw-rst= %d, sw-rst= %d\n",
		  kpd_dts_data.kpd_key_debounce, kpd_dts_data.kpd_sw_pwrkey,
			kpd_dts_data.kpd_hw_pwrkey, kpd_dts_data.kpd_hw_rstkey,
				kpd_dts_data.kpd_sw_rstkey);
}

static int32_t kpd_gpio_init(struct device *dev)
{
	struct pinctrl *keypad_pinctrl;
	struct pinctrl_state *kpd_default;
	int32_t ret;

	if (dev == NULL) {
		kpd_print("kpd device is NULL!\n");
		ret = -1;
	} else {
		keypad_pinctrl = devm_pinctrl_get(dev);
		if (IS_ERR(keypad_pinctrl)) {
			ret = -1;
			kpd_print("Cannot find keypad_pinctrl!\n");
		} else {
			kpd_default = pinctrl_lookup_state(keypad_pinctrl,
				"default");
			if (IS_ERR(kpd_default)) {
				ret = -1;
				kpd_print("Cannot find ecall_state!\n");
			} else
				ret = pinctrl_select_state(keypad_pinctrl,
					kpd_default);
		}
	}
	return ret;
}

static int mt_kpd_debugfs(void)
{
#ifdef CONFIG_MTK_ENG_BUILD
	kpd_klog_en = 1;
#else
	kpd_klog_en = 0;
#endif
	kpd_droot = debugfs_create_dir("keypad", NULL);
	if (IS_ERR_OR_NULL(kpd_droot))
		return PTR_ERR(kpd_droot);

	kpd_dklog = debugfs_create_u32("debug", 0600, kpd_droot, &kpd_klog_en);

	return 0;
}

static int kpd_pdrv_probe(struct platform_device *pdev)
{
	struct clk *kpd_clk = NULL;
	u32 i;
	int32_t err = 0;
#if defined(HALL_FUNCTION_SUPPORT) //added by xen 20160413
	struct device_node *hall_node = NULL;
	u32 ints[2] = { 0, 0 };
#endif

	if (!pdev->dev.of_node) {
		kpd_notice("no kpd dev node\n");
		return -ENODEV;
	}

	kpd_clk = devm_clk_get(&pdev->dev, "kpd-clk");
	if (!IS_ERR(kpd_clk)) {
		err = clk_prepare_enable(kpd_clk);
		if (err)
			kpd_notice("get kpd-clk fail: %d\n", err);
	} else {
		kpd_notice("kpd-clk is default set by ccf.\n");
	}

	kp_base = of_iomap(pdev->dev.of_node, 0);
	if (!kp_base) {
		kpd_notice("KP iomap failed\n");
		return -ENODEV;
	};

	kp_irqnr = irq_of_parse_and_map(pdev->dev.of_node, 0);
	if (!kp_irqnr) {
		kpd_notice("KP get irqnr failed\n");
		return -ENODEV;
	}
	kpd_info("kp base: 0x%p, addr:0x%p,  kp irq: %d\n",
			kp_base, &kp_base, kp_irqnr);
	err = kpd_gpio_init(&pdev->dev);
	if (err != 0)
		kpd_print("gpio init failed\n");

	kpd_get_dts_info(pdev->dev.of_node);

	kpd_memory_setting();

	kpd_input_dev = devm_input_allocate_device(&pdev->dev);
	if (!kpd_input_dev) {
		kpd_notice("input allocate device fail.\n");
		return -ENOMEM;
	}

	kpd_input_dev->name = KPD_NAME;
	kpd_input_dev->id.bustype = BUS_HOST;
	kpd_input_dev->id.vendor = 0x2454;
	kpd_input_dev->id.product = 0x6500;
	kpd_input_dev->id.version = 0x0010;
	kpd_input_dev->open = kpd_open;
	kpd_input_dev->dev.parent = &pdev->dev;

	__set_bit(EV_KEY, kpd_input_dev->evbit);
#if defined(CONFIG_KPD_PWRKEY_USE_PMIC)
	__set_bit(kpd_dts_data.kpd_sw_pwrkey, kpd_input_dev->keybit);
	kpd_keymap[8] = 0;
#endif
	if (!kpd_dts_data.kpd_use_extend_type) {
		for (i = 17; i < KPD_NUM_KEYS; i += 9)
			kpd_keymap[i] = 0;
	}
	for (i = 0; i < KPD_NUM_KEYS; i++) {
		if (kpd_keymap[i] != 0)
			__set_bit(kpd_keymap[i], kpd_input_dev->keybit);
	}

#if defined(HALL_FUNCTION_SUPPORT)          //added by xen for test HALL function 20130819
        __set_bit(KEY_HALL_CLOSE, kpd_input_dev->keybit);
        __set_bit(KEY_HALL_FARAWAY, kpd_input_dev->keybit);
#endif

#if defined (ALSPS_GESTURE)  // wanghe 2014-01-08
	__set_bit(KEY_GESTURE_UP, kpd_input_dev->keybit);
	__set_bit(KEY_GESTURE_DOWN, kpd_input_dev->keybit);
#endif

#if defined (CONFIG_TERACUBE_2E)
	__set_bit(KEY_TPGESTURE_UP, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_DOWN, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_LEFT, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_RIGHT, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_DOUBLE, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_C, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_E, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_M, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_O, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_S, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_V, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_W, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_Z, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_ARROWUP, kpd_input_dev->keybit);
	__set_bit(KEY_TPGESTURE_ARROWRIGHT, kpd_input_dev->keybit);
        __set_bit(KEY_F4, kpd_input_dev->keybit);//tplink
#endif


#if defined(_SUB2_CAM_SHELTER_DESIGN2_)||defined(_MAIN2_CAM_SHELTER_DESIGN2_) //xen 20170211
	//__set_bit(KEY_CAMERA, kpd_input_dev->keybit);
	__set_bit(KEY_CAM_SHELTER_NEAR, kpd_input_dev->keybit);
	__set_bit(KEY_CAM_SHELTER_FAR, kpd_input_dev->keybit);
#endif

//#if defined(FINGERPRINT_SUPPORT) //xen 20170427
       memset(mtk_fingerprint_name,0,fingerprint_info_size);
       //proc_create(PROC_FINGERPRINT_INFO, 0, NULL, &fingerprint_proc_fops1);
#if defined(FINGERPRINT_SUPPORT)
       snprintf(mtk_fingerprint_name,sizeof(mtk_fingerprint_name),"%s","No Detected");
#endif
//#endif

	if (kpd_dts_data.kpd_sw_rstkey)
		__set_bit(kpd_dts_data.kpd_sw_rstkey, kpd_input_dev->keybit);
#ifdef KPD_KEY_MAP
	__set_bit(KPD_KEY_MAP, kpd_input_dev->keybit);
#endif
#ifdef CONFIG_MTK_MRDUMP_KEY
	__set_bit(KEY_RESTART, kpd_input_dev->keybit);
#endif

	err = input_register_device(kpd_input_dev);
	if (err) {
		kpd_notice("register input device failed (%d)\n", err);
		return err;
	}
#ifdef CONFIG_PM_WAKELOCKS
	wakeup_source_init(&kpd_suspend_lock, "kpd wakelock");
#else
	wake_lock_init(&kpd_suspend_lock, WAKE_LOCK_SUSPEND, "kpd wakelock");
#endif
	/* register IRQ and EINT */
	kpd_set_debounce(kpd_dts_data.kpd_key_debounce);
	err = request_irq(kp_irqnr, kpd_irq_handler, IRQF_TRIGGER_NONE,
			KPD_NAME, NULL);
	if (err) {
		kpd_notice("register IRQ failed (%d)\n", err);
		input_unregister_device(kpd_input_dev);
		return err;
	}
#ifdef CONFIG_MTK_MRDUMP_KEY
	mt_eint_register();
#endif

#if defined(HALL_FUNCTION_SUPPORT)     //added by xen for test HALL function 20130819
        //mt_set_gpio_mode(GPIO_MHALL_EINT_PIN, GPIO_MHALL_EINT_PIN_M_EINT);
        //mt_set_gpio_dir(GPIO_MHALL_EINT_PIN, GPIO_DIR_IN);
        //mt_set_gpio_pull_enable(GPIO_MHALL_EINT_PIN, GPIO_PULL_DISABLE); //To disable GPIO PULL.
        //mt_eint_set_sens(KPD_HALL_SWITCH_EINT, KPD_HALL_SWITCH_SENSITIVE);
        //mt_eint_set_hw_debounce(KPD_HALL_SWITCH_EINT, KPD_HALL_SWITCH_DEBOUNCE);
        //mt65xx_eint_registration(KPD_HALL_SWITCH_EINT, true,KPD_HALL_SWITCH_POLARITY, kpd_halldet_eint_handler, false);
        //mt_eint_registration(KPD_HALL_SWITCH_EINT, CUST_EINT_MHALL_TYPE, kpd_halldet_eint_handler, 0); 
        //mt_eint_unmask(KPD_HALL_SWITCH_EINT);  
        INIT_DELAYED_WORK(&mhall_eint_work, kpd_halldet_handler);

        hall_node = of_find_compatible_node(NULL, NULL, "mediatek, mhall-eint");
	if (hall_node) {
		of_property_read_u32_array(hall_node, "debounce", ints,
					   ARRAY_SIZE(ints));
		//gpio_request(ints[0], "hallfunc");
		gpio_set_debounce(ints[0], ints[1]);
		kpd_print("ints[0] = %d, ints[1] = %d!!\n", ints[0], ints[1]);
                //pinctrl_select_state(pinctrl, pins_cfg);
		mhall_irq_number = irq_of_parse_and_map(hall_node, 0);
		kpd_print("mhall_irq_number = %d\n", mhall_irq_number);
		if (!mhall_irq_number) {
			kpd_print("mhall irq_of_parse_and_map fail!!\n");
			return -EINVAL;
		}
		if (request_irq(mhall_irq_number, kpd_halldet_eint_handler, IRQF_TRIGGER_LOW, "mhall-eint", NULL)) {
			kpd_print("IRQ LINE NOT AVAILABLE!!\n");
			return -EINVAL;
		}
		enable_irq(mhall_irq_number);
	}
	else
	{
		kpd_print("mhall null irq node!!\n");
		return -EINVAL;
	}
#endif

#ifdef CONFIG_MTK_PMIC_NEW_ARCH
	long_press_reboot_function_setting();
#endif
	err = kpd_create_attr(&kpd_pdrv.driver);
	if (err) {
		kpd_notice("create attr file fail\n");
		kpd_delete_attr(&kpd_pdrv.driver);
		return err;
	}
	/* Add kpd debug node */
	mt_kpd_debugfs();

	kpd_info("kpd_probe OK.\n");

	return err;
}

static int kpd_pdrv_suspend(struct platform_device *pdev, pm_message_t state)
{
	kpd_suspend = true;
#ifdef MTK_KP_WAKESOURCE
	if (call_status == 2) {
		kpd_print("kpd_early_suspend wake up source enable!! (%d)\n",
				kpd_suspend);
	} else {
		kpd_wakeup_src_setting(0);
		kpd_print("kpd_early_suspend wake up source disable!! (%d)\n",
				kpd_suspend);
	}
#endif
	kpd_print("suspend!! (%d)\n", kpd_suspend);
	return 0;
}

static int kpd_pdrv_resume(struct platform_device *pdev)
{
	kpd_suspend = false;
#ifdef MTK_KP_WAKESOURCE
	if (call_status == 2) {
		kpd_print("kpd_early_suspend wake up source enable!! (%d)\n",
				kpd_suspend);
	} else {
		kpd_print("kpd_early_suspend wake up source resume!! (%d)\n",
				kpd_suspend);
		kpd_wakeup_src_setting(1);
	}
#endif
	kpd_print("resume!! (%d)\n", kpd_suspend);
	return 0;
}

static const struct of_device_id kpd_of_match[] = {
	{.compatible = "mediatek,kp"},
	{},
};

static struct platform_driver kpd_pdrv = {
	.probe = kpd_pdrv_probe,
	.suspend = kpd_pdrv_suspend,
	.resume = kpd_pdrv_resume,
	.driver = {
		   .name = KPD_NAME,
		   .owner = THIS_MODULE,
		   .of_match_table = kpd_of_match,
		   },
};

module_platform_driver(kpd_pdrv);

MODULE_AUTHOR("Mediatek Corporation");
MODULE_DESCRIPTION("MTK Keypad (KPD) Driver");
MODULE_LICENSE("GPL");
