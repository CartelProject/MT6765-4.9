#
# Copyright (C) 2015 MediaTek Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#

#
# Makefile for misc devices that really don't fit anywhere else.
#

MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
ARCH_MTK_PROJECT := $(subst ",,$(CONFIG_ARCH_MTK_PROJECT))

MTK_CUSTOM_PATH  := $(srctree)/drivers/misc/mediatek


subdir-ccflags-y += -I$(MTK_CUSTOM_PATH)/lens/main/inc
subdir-ccflags-y += -I$(MTK_CUSTOM_PATH)/lens/main/common/lc898122af
subdir-ccflags-y += -I$(MTK_CUSTOM_PATH)/lens/main/common/lc898212xdaf/inc
ifeq ($(CONFIG_MTK_LENS_BU63165AF_SUPPORT), y)
  subdir-ccflags-y += -I$(MTK_CUSTOM_PATH)/lens/main/common/bu63165af/inc
else
  subdir-ccflags-y += -I$(MTK_CUSTOM_PATH)/lens/main/common/bu63169af/inc
endif
ccflags-y += -I$(srctree)/drivers/misc/mediatek/timer/timesync/

ifeq ($(CONFIG_MTK_LENS_USE_MODULE), y)

MAIN_CFILES += main_lens.c

MAIN_CFILES  += common/fm50af/FM50AF.c
MAIN_CFILES  += common/fp5510e2af/FP5510E2AF.c
MAIN_CFILES  += common/bu6424af/BU6424AF.c
MAIN_CFILES  += common/bu6429af/BU6429AF.c
MAIN_CFILES  += common/bu6429af/bu64748af.c
MAIN_CFILES  += common/bu6429af/bu64748_function.c
MAIN_CFILES += common/dw9714af/DW9714AF.c
MAIN_CFILES += common/dw9763af/DW9763AF.c
MAIN_CFILES  += common/dw9814af/DW9814AF.c
MAIN_CFILES  += common/dw9718af/DW9718AF.c
MAIN_CFILES  += common/dw9718saf/DW9718SAF.c
MAIN_CFILES  += common/dw9719taf/DW9719TAF.c
MAIN_CFILES += common/lc898212af/LC898212AF.c
MAIN_CFILES  += common/lc898212xdaf/LC898212XDAF.c
MAIN_CFILES  += common/lc898212xdaf/AfSTMV.c
MAIN_CFILES  += common/lc898212xdaf/AfInter.c
MAIN_CFILES  += common/lc898212xdaf/AfInit.c
MAIN_CFILES  += common/lc898214af/LC898214AF.c
MAIN_CFILES  += common/lc898217af/LC898217AF.c
MAIN_CFILES  += common/lc898217afa/LC898217AFA.c
MAIN_CFILES  += common/lc898217afb/LC898217AFB.c
MAIN_CFILES  += common/lc898217afc/LC898217AFC.c
MAIN_CFILES  += common/lc898122af/LC898122AF.c
MAIN_CFILES  += common/lc898122af/OisCmd.c
MAIN_CFILES  += common/lc898122af/OisIni.c
MAIN_CFILES  += common/ad5820af/AD5820AF.c
MAIN_CFILES  += common/wv511aaf/WV511AAF.c
MAIN_CFILES  += common/ak7371af/AK7371AF.c
ifeq ($(CONFIG_MTK_LENS_BU63165AF_SUPPORT), y)
MAIN_CFILES  += common/bu63165af/BU63165AF.c
MAIN_CFILES  += common/bu63165af/OIS_func.c
MAIN_CFILES  += common/bu63165af/OIS_main.c
MAIN_CFILES  += common/bu63165af/OIS_user.c
else
MAIN_CFILES  += common/bu63169af/BU63169AF.c
MAIN_CFILES  += common/bu63169af/OIS_func.c
MAIN_CFILES  += common/bu63169af/OIS_main.c
MAIN_CFILES  += common/bu63169af/OIS_user.c
endif

obj-m := mainaf.o
mainaf-objs := $(MAIN_CFILES:.c=.o)

else

obj-y  += main_lens.o

obj-$(CONFIG_MTK_LENS_FM50AF_SUPPORT) += common/fm50af/
obj-$(CONFIG_MTK_LENS_BU6424AF_SUPPORT) += common/bu6424af/
obj-$(CONFIG_MTK_LENS_BU6429AF_SUPPORT) += common/bu6429af/
obj-$(CONFIG_MTK_LENS_DW9714AF_SUPPORT) += common/dw9714af/
obj-$(CONFIG_MTK_LENS_DW9814AF_SUPPORT) += common/dw9814af/
obj-$(CONFIG_MTK_LENS_DW9718AF_SUPPORT) += common/dw9718af/
obj-$(CONFIG_MTK_LENS_DW9718SAF_SUPPORT) += common/dw9718saf/
obj-$(CONFIG_MTK_LENS_LC898212AF_SUPPORT) += common/lc898212af/
obj-$(CONFIG_MTK_LENS_LC89821AF_SUPPORT) += common/lc89821af/
obj-$(CONFIG_MTK_LENS_LC898212XDAF_SUPPORT) += common/lc898212xdaf/
obj-$(CONFIG_MTK_LENS_LC898212XDAF_TVC700_SUPPORT) += common/lc898212xdaf_tvc700/
obj-$(CONFIG_MTK_LENS_LC898214AF_SUPPORT) += common/lc898214af/
obj-$(CONFIG_MTK_LENS_LC898217AF_SUPPORT) += common/lc898217af/
obj-$(CONFIG_MTK_LENS_LC898217AFA_SUPPORT) += common/lc898217afa/
obj-$(CONFIG_MTK_LENS_LC898217AFB_SUPPORT) += common/lc898217afb/
obj-$(CONFIG_MTK_LENS_LC898217AFC_SUPPORT) += common/lc898217afc/
obj-$(CONFIG_MTK_LENS_LC898122AF_SUPPORT) += common/lc898122af/
obj-$(CONFIG_MTK_LENS_WV511AAF_SUPPORT) += common/wv511aaf/
obj-$(CONFIG_MTK_LENS_AK7371AF_SUPPORT) += common/ak7371af/

subdir-ccflags-$(CONFIG_MTK_LENS_FM50AF_SUPPORT) += -DFM50AF
subdir-ccflags-$(CONFIG_MTK_LENS_BU6424AF_SUPPORT) += -DBU6424AF
subdir-ccflags-$(CONFIG_MTK_LENS_BU6429AF_SUPPORT) += -DBU6429AF
subdir-ccflags-$(CONFIG_MTK_LENS_DW9714AF_SUPPORT) += -DDW9714AF
subdir-ccflags-$(CONFIG_MTK_LENS_DW9814AF_SUPPORT) += -DDW9814AF
subdir-ccflags-$(CONFIG_MTK_LENS_DW9718AF_SUPPORT) += -DDW9718AF
subdir-ccflags-$(CONFIG_MTK_LENS_DW9718SAF_SUPPORT) += -DDW9718SAF
subdir-ccflags-$(CONFIG_MTK_LENS_LC898212AF_SUPPORT) += -DLC898212AF
subdir-ccflags-$(CONFIG_MTK_LENS_LC89821AF_SUPPORT) += -DLC89821AF
subdir-ccflags-$(CONFIG_MTK_LENS_LC898212XDAF_SUPPORT) += -DLC898212XDAF
subdir-ccflags-$(CONFIG_MTK_LENS_LC898212XDAF_TVC700_SUPPORT) += -DLC898212XDAF_TVC700
subdir-ccflags-$(CONFIG_MTK_LENS_LC898214AF_SUPPORT) += -DLC898214AF
subdir-ccflags-$(CONFIG_MTK_LENS_LC898217AF_SUPPORT) += -DLC898217AF
subdir-ccflags-$(CONFIG_MTK_LENS_LC898217AFA_SUPPORT) += -DLC898217AFA
subdir-ccflags-$(CONFIG_MTK_LENS_LC898217AFB_SUPPORT) += -DLC898217AFB
subdir-ccflags-$(CONFIG_MTK_LENS_LC898217AFC_SUPPORT) += -DLC898217AFC
subdir-ccflags-$(CONFIG_MTK_LENS_LC898122AF_SUPPORT) += -DLC898122AF
subdir-ccflags-$(CONFIG_MTK_LENS_WV511AAF_SUPPORT) += -DWV511AAF
subdir-ccflags-$(CONFIG_MTK_LENS_AK7371AF_SUPPORT) += -DAK7371AF

endif
