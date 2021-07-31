#ifndef TEEI_BOOTPROF_H
#define TEEI_BOOTPROF_H

extern void log_boot(char *str);
#define TEEI_BOOT_FOOTPRINT(str) log_boot(str)

#endif
