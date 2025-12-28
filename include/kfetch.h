#ifndef KFETCH_H
#define KFETCH_H

#include <windows.h>

void set_color(WORD attr);
void reset_color();

void get_username(char *buf, DWORD size);
void get_computername(char *buf, DWORD size);
void get_os_version(char *buf, size_t size);

void get_cpu_core_info(int *cores, int *threads);
void get_arch(char *buf, size_t size);
void get_memory_info(DWORDLONG *totalMB, DWORDLONG *freeMB);
void get_cpu_brand(char *buf, DWORD size);

void get_console_size(short *cols, short *rows);
void get_gpus(char gpuNames[2][256], int *gpuCount);
void get_uptime(char *buf, size_t size);

void get_disk_info(char *out, size_t outSize);
unsigned long long get_vram_registry(int gpuIndex);

void make_vram_bar(double used_gb, double total_gb, char* out, size_t outSize);

void run_kfetch(int argc, char **argv);

#endif