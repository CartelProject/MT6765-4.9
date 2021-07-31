/*
 * Copyright (C) 2015 MediaTek Inc.
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

#ifndef __LCM_PMIC_H__
#define __LCM_PMIC_H__

#include "lcm_drv.h"
#include "lcm_common.h"
#ifdef BUILD_LK //xen 20180205
int PMU_REG_MASK (kal_uint8 addr, kal_uint8 val, kal_uint8 mask);
#endif
#endif

