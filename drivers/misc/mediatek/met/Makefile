MTK_PLATFORM := $(subst ",,$(CONFIG_MTK_PLATFORM))
MET_ROOT_DIR := $(srctree)/drivers/misc/mediatek/met
MET_COMMON_DIR := $(wildcard $(MET_ROOT_DIR)/common)
MET_PLF_DIR := $(wildcard $(MET_ROOT_DIR)/$(MTK_PLATFORM))
MET_BUILD_DEFAULT := n

ifeq ($(CONFIG_MODULES),y)

ifeq ($(CONFIG_FTRACE),y)
    ifeq ($(CONFIG_TRACING),y)
        FTRACE_READY := y
    endif
endif

ifneq ($(MET_PLF_DIR),)
    ifeq ($(FTRACE_READY),y)
        include $(MET_COMMON_DIR)/Kbuild
    else
        MET_BUILD_DEFAULT = y
    endif
else
    MET_BUILD_DEFAULT = y
endif
else #CONFIG_MODULES = n
    MET_BUILD_DEFAULT := y
endif

ifeq ($(MET_BUILD_DEFAULT),y)
    MET_DEF_DIR := $(MET_ROOT_DIR)/default
    include $(MET_DEF_DIR)/Kbuild
endif
