#include <windows.h>
#include <tchar.h>
#include <stdint.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <initguid.h>
#include <stdio.h>
#include <string.h>
#include "gpu.h"

DEFINE_GUID(GUID_DEVCLASS_DISPLAY,
    0x4d36e968, 0xe325, 0x11ce,
    0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18);
#define NVAPI_OK 0
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "advapi32.lib")
typedef int NvAPI_Status;
typedef unsigned int NvU32;
typedef struct {
    NvU32 version;
    NvU32 dedicatedVideoMemory;
    NvU32 availableDedicatedVideoMemory;
    NvU32 systemVideoMemory;
    NvU32 sharedSystemMemory;
} NV_DISPLAY_DRIVER_MEMORY_INFO;


#define NV_DISPLAY_DRIVER_MEMORY_INFO_VER (sizeof(NV_DISPLAY_DRIVER_MEMORY_INFO) | (1 << 16))

typedef NvAPI_Status (__cdecl *NvAPI_QueryInterface_t)(unsigned int offset);
typedef NvAPI_Status (__cdecl *NvAPI_Initialize_t)(void);
typedef NvAPI_Status (__cdecl *NvAPI_EnumPhysicalGPUs_t)(void **handles, NvU32 *count);
typedef NvAPI_Status (__cdecl *NvAPI_GPU_GetMemoryInfo_t)(void *handle, NV_DISPLAY_DRIVER_MEMORY_INFO *info);

static NvAPI_QueryInterface_t NvAPI_QueryInterface = NULL;
static NvAPI_Initialize_t NvAPI_Initialize = NULL;
static NvAPI_EnumPhysicalGPUs_t NvAPI_EnumPhysicalGPUs = NULL;
static NvAPI_GPU_GetMemoryInfo_t NvAPI_GPU_GetMemoryInfo = NULL;

int init_nvapi()
{
    HMODULE h = LoadLibraryA("nvapi64.dll");
    if (!h) return 0;

    NvAPI_QueryInterface = (NvAPI_QueryInterface_t)GetProcAddress(h, "nvapi_QueryInterface");
    if (!NvAPI_QueryInterface) return 0;

    NvAPI_Initialize = (NvAPI_Initialize_t)(uintptr_t)NvAPI_QueryInterface(0x0150E828);
    NvAPI_EnumPhysicalGPUs = (NvAPI_EnumPhysicalGPUs_t)(uintptr_t)NvAPI_QueryInterface(0xE5AC921F);
    NvAPI_GPU_GetMemoryInfo = (NvAPI_GPU_GetMemoryInfo_t)(uintptr_t)NvAPI_QueryInterface(0x0703F2E2);

    if (!NvAPI_Initialize || !NvAPI_EnumPhysicalGPUs || !NvAPI_GPU_GetMemoryInfo)
        return 0;

    if (NvAPI_Initialize() != NVAPI_OK)
        return 0;

    return 1;
}

int get_vram_usage_nvapi(int gpuIndex,
                         unsigned long long *used,
                         unsigned long long *total)
{
    *used = 0;
    *total = 0;

    if (!init_nvapi())
        return 0;

    void *gpuHandles[16] = {0};
    NvU32 count = 0;

    if (NvAPI_EnumPhysicalGPUs(gpuHandles, &count) != NVAPI_OK)
        return 0;

    if (gpuIndex >= (int)count)
        return 0;

    NV_DISPLAY_DRIVER_MEMORY_INFO info;
    memset(&info, 0, sizeof(info));
    info.version = NV_DISPLAY_DRIVER_MEMORY_INFO_VER;

    if (NvAPI_GPU_GetMemoryInfo(gpuHandles[gpuIndex], &info) != NVAPI_OK)
        return 0;

    // Convert MB → bytes
    unsigned long long totalBytes = (unsigned long long)info.dedicatedVideoMemory * 1024ULL * 1024ULL;
    unsigned long long freeBytes  = (unsigned long long)info.availableDedicatedVideoMemory * 1024ULL * 1024ULL;
    unsigned long long usedBytes  = totalBytes - freeBytes;

    *used  = usedBytes;
    *total = totalBytes;

    return 1;
}

void get_gpus(char gpuNames[2][256], int *gpuCount) {
    *gpuCount = 0;

    HDEVINFO hDevInfo = SetupDiGetClassDevsA(
        &GUID_DEVCLASS_DISPLAY,
        NULL,
        NULL,
        DIGCF_PRESENT
    );

    if (hDevInfo == INVALID_HANDLE_VALUE)
        return;

    SP_DEVINFO_DATA devInfo;
    devInfo.cbSize = sizeof(SP_DEVINFO_DATA);

    for (DWORD i = 0; i < 10 && *gpuCount < 2; i++) {
        if (!SetupDiEnumDeviceInfo(hDevInfo, i, &devInfo))
            break;

        char name[256];
        DWORD size = sizeof(name);
        DWORD type = 0;

        if (!SetupDiGetDeviceRegistryPropertyA(
                hDevInfo,
                &devInfo,
                SPDRP_DEVICEDESC,
                &type,
                (BYTE*)name,
                size,
                &size))
            continue;

        if (strstr(name, "Microsoft Basic Render"))
            continue;

        strncpy(gpuNames[*gpuCount], name, 256);
        gpuNames[*gpuCount][255] = '\0';
        (*gpuCount)++;
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
}

unsigned long long get_vram_registry(int gpuIndex) {
    HKEY hKey;
    char path[512];

    snprintf(path, sizeof(path),
        "SYSTEM\\CurrentControlSet\\Control\\Video");

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
        return 0;

    char subkeyName[256];
    DWORD subkeyLen = sizeof(subkeyName);
    DWORD index = 0;

    while (RegEnumKeyExA(hKey, index, subkeyName, &subkeyLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {

        char fullPath[512];
        snprintf(fullPath, sizeof(fullPath),
            "SYSTEM\\CurrentControlSet\\Control\\Video\\%s\\0000",
            subkeyName);

        HKEY hSub;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, fullPath, 0, KEY_READ, &hSub) == ERROR_SUCCESS) {

            unsigned long long mem = 0;
            DWORD memSize = sizeof(mem);
            // Try modern key
            if (RegQueryValueExA(hSub, "HardwareInformation.MemorySize", NULL, NULL,
                                 (LPBYTE)&mem, &memSize) == ERROR_SUCCESS && mem > 0) {
                if (gpuIndex == 0) {
                    RegCloseKey(hSub);
                    RegCloseKey(hKey);
                    return mem;
                }
                gpuIndex--;
            }
            // Try legacy key
            mem = 0;
            memSize = sizeof(mem);
            if (RegQueryValueExA(hSub, "HardwareInformation.MemorySizeLegacy", NULL, NULL,
                                 (LPBYTE)&mem, &memSize) == ERROR_SUCCESS && mem > 0) {
                if (gpuIndex == 0) {
                    RegCloseKey(hSub);
                    RegCloseKey(hKey);
                    return mem;
                }
                gpuIndex--;
            }
            // Try old key
            mem = 0;
            memSize = sizeof(mem);
            if (RegQueryValueExA(hSub, "HardwareInformation.qwMemorySize", NULL, NULL,
                                 (LPBYTE)&mem, &memSize) == ERROR_SUCCESS && mem > 0) {
                if (gpuIndex == 0) {
                    RegCloseKey(hSub);
                    RegCloseKey(hKey);
                    return mem;
                }
                gpuIndex--;
            }

            RegCloseKey(hSub);
        }

        subkeyLen = sizeof(subkeyName);
        index++;
    }

    RegCloseKey(hKey);
    return 0;
}

