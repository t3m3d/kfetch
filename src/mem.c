#include <windows.h>
#include "mem.h"

void get_memory_info(DWORDLONG *totalMB, DWORDLONG *freeMB) {
    MEMORYSTATUSEX ms;
    ZeroMemory(&ms, sizeof(ms));
    ms.dwLength = sizeof(ms);

    if (GlobalMemoryStatusEx(&ms)) {
        *totalMB = ms.ullTotalPhys / (1024 * 1024);
        *freeMB  = ms.ullAvailPhys / (1024 * 1024);
    } else {
        *totalMB = 0;
        *freeMB = 0;
    }
}
