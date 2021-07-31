#include $(srctree)/drivers/misc/mediatek/Makefile.custom
MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
ifeq ($(MTK_PLATFORM),mt6761)
obj-y += gc5025fmipi_Sensor_mt61.o
else
obj-y += gc5025fmipi_Sensor.o
endif


