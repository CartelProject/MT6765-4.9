#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/atomic.h>


#include "kd_camera_typedef.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "gc13023mipi_Pdaf.h"

/****************************Modify Following Strings for Debug****************************/
#define PFX "GC13023MIPI"
#define LOG_1 LOG_INF("GC13023MIPI, 4LANE\n")
/****************************   Modify end    *******************************************/

#undef GC13023_DEBUG
#if defined(GC13023_DEBUG)
#define LOG_INF(format, args...)	pr_debug(PFX "[%s] " format, __func__, ##args)
#else
#define LOG_INF(format, args...)
#endif

#define USHORT             unsigned short
#define BYTE               unsigned char
#define Sleep(ms)          mdelay(ms)

/**************  CONFIG BY SENSOR >>> ************/
#define EEPROM_WRITE_ID   0xA0
#define MAX_OFFSET		  0xFFFF
#define DATA_SIZE         1404
#define START_ADDR        0x1801
BYTE GC13023_eeprom_data[DATA_SIZE] = { 0 };
/**************  CONFIG BY SENSOR <<< ************/

static kal_uint16 read_cmos_sensor_byte(kal_uint16 addr)
{
	kal_uint16 get_byte = 0;
	char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };

	iReadRegI2C(pu_send_cmd, 2, (u8 *)&get_byte, 1, EEPROM_WRITE_ID);
	return get_byte;
}

static bool _read_eeprom(kal_uint16 addr, kal_uint32 size)
{
	/* continue read reg by byte */
	int i = 0;

	for (; i < size; i++) {
		GC13023_eeprom_data[i] = read_cmos_sensor_byte(addr + i);
		LOG_INF("addr = 0x%x,\tvalue = 0x%x", i, GC13023_eeprom_data[i]);
	}
	LOG_INF("");
	return true;
}

bool GC13023_read_eeprom(kal_uint16 addr, BYTE *data, kal_uint32 size)
{
	addr = START_ADDR;
	size = DATA_SIZE;

	LOG_INF("Read EEPROM, addr = 0x%x, size = 0d%d\n", addr, size);

	if (!_read_eeprom(addr, size)) {
		LOG_INF("error:read_eeprom fail!\n");
		return false;
	}

	memcpy(data, GC13023_eeprom_data, size);
	return true;
}
