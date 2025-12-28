#ifndef GPU_H
#define GPU_H

#include <stddef.h>

void get_gpus(char gpuNames[2][256], int *gpuCount);

int get_vram_usage_nvapi(int gpuIndex,
                         unsigned long long *used,
                         unsigned long long *total);


unsigned long long get_vram_registry(int gpuIndex);

#endif
