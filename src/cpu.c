#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"


void get_cpu_core_info(int *cores, int *threads) {
    *cores = 0;
    *threads = 0;

    DWORD len = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, NULL, &len);

    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *buffer =
        (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(len);

    if (!buffer)
        return;

    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, buffer, &len)) {
        free(buffer);
        return;
    }

    char *ptr = (char*)buffer;
    char *end = ptr + len;

    while (ptr < end) {
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *info =
            (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)ptr;

        if (info->Relationship == RelationProcessorCore) {
            (*cores)++;

            KAFFINITY mask = info->Processor.GroupMask[0].Mask;
            int count = 0;
            while (mask) {
                count += mask & 1;
                mask >>= 1;
            }
            *threads += count;
        }

        ptr += info->Size;
    }

    free(buffer);
}

void get_arch(char *buf, size_t size) {
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);

    const char *arch = "Unknown";
    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: arch = "x64"; break;
        case PROCESSOR_ARCHITECTURE_INTEL: arch = "x86"; break;
        case PROCESSOR_ARCHITECTURE_ARM:   arch = "ARM"; break;
        case PROCESSOR_ARCHITECTURE_ARM64: arch = "ARM64"; break;
        default: break;
    }

    _snprintf(buf, size, "%s", arch);
}
void get_cpu_brand(char *buf, DWORD size) {
    HKEY hKey;
    LONG status = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0,
        KEY_READ,
        &hKey);

    if (status != ERROR_SUCCESS) {
        strncpy(buf, "Unknown CPU", size);
        buf[size - 1] = '\0';
        return;
    }

    DWORD type = 0;
    DWORD dataSize = size;
    status = RegQueryValueExA(hKey, "ProcessorNameString", NULL, &type, (LPBYTE)buf, &dataSize);

    RegCloseKey(hKey);

    if (status != ERROR_SUCCESS || type != REG_SZ) {
        strncpy(buf, "Unknown CPU", size);
        buf[size - 1] = '\0';
    } else {
        buf[size - 1] = '\0';
    }
}