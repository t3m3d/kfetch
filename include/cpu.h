#ifndef CPU_H
#define CPU_H

#include <windows.h>
#include <stddef.h>

void get_cpu_core_info(int *cores, int *threads);

void get_arch(char *buf, size_t size);

void get_cpu_brand(char *buf, DWORD size);

#endif
