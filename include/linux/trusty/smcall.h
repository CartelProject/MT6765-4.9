/*
 * Copyright (C) 2018 MediaTek Inc.
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

#ifndef __LINUX_TRUSTY_SMCALL_MAPPING_H
#define __LINUX_TRUSTY_SMCALL_MAPPING_H

#ifdef CONFIG_MICROTRUST_TEE_SUPPORT
#define SMC_ENTITY_LAUNCH		61	/* Trusted OS launch call */
#endif
#ifdef CONFIG_MICROTRUST_TEE_SUPPORT
#define SMC_GET_TEE_STATUS	SMC_STDCALL_NR(SMC_ENTITY_SECURE_MONITOR, 4)
#define SMC_NT_SCHED_T		SMC_STDCALL_NR(SMC_ENTITY_SECURE_MONITOR, 5)
#define SMC_SWITCH_CORE		SMC_STDCALL_NR(SMC_ENTITY_SECURE_MONITOR, 6)
#endif

#ifdef CONFIG_MICROTRUST_TEE_SUPPORT
#define SMC_FC_SEND_LOG_ADDR	SMC_FASTCALL_NR(SMC_ENTITY_SECURE_MONITOR, 12)
#define SMC_FC_START_TIME	SMC_FASTCALL_NR(SMC_ENTITY_SECURE_MONITOR, 13)
#endif
#ifdef CONFIG_MICROTRUST_TEE_SUPPORT
/* Launch TA */
#define SMC_SC_LOAD_TA		SMC_STDCALL_NR(SMC_ENTITY_LAUNCH, 1)
#define SMC_SC_SEND_MODEL_INFO	SMC_STDCALL_NR(SMC_ENTITY_LAUNCH, 2)
#endif
#if defined(CONFIG_MTK_NEBULA_VM_SUPPORT) && defined(CONFIG_GZ_SMC_CALL_REMAP)
#include <linux/trusty/hvcall.h>
#include <linux/trusty/smcall_remap.h>
#else
#include <linux/trusty/smcall_trusty.h>
#endif

#endif /* __LINUX_TRUSTY_SMCALL_MAPPING_H */
