#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <initguid.h>
#include "version.h"

DEFINE_GUID(GUID_DEVCLASS_DISPLAY,
    0x4d36e968, 0xe325, 0x11ce,
    0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18);

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "advapi32.lib")

void set_color(WORD attr) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, attr);
}

void reset_color() {
    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void get_username(char *buf, DWORD size) {
    DWORD len = size;
    if (!GetUserNameA(buf, &len)) {
        strncpy(buf, "Unknown", size);
        buf[size - 1] = '\0';
    }
}

void get_computername(char *buf, DWORD size) {
    DWORD len = size;
    if (!GetComputerNameA(buf, &len)) {
        strncpy(buf, "Unknown", size);
        buf[size - 1] = '\0';
    }
}

void get_os_version(char *buf, size_t size) {
    OSVERSIONINFOEXA vi;
    ZeroMemory(&vi, sizeof(vi));
    vi.dwOSVersionInfoSize = sizeof(vi);
    if (GetVersionExA((OSVERSIONINFOA*)&vi)) {
        _snprintf(buf, size, "Windows %lu.%lu (build %lu)",
            vi.dwMajorVersion,
            vi.dwMinorVersion,
            vi.dwBuildNumber);
    } else {
        strncpy(buf, "Windows (version unknown)", size);
        buf[size - 1] = '\0';
    }
}

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

            // Count logical processors (threads)
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

void get_console_size(short *cols, short *rows) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(h, &csbi)) {
        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        *cols = 0;
        *rows = 0;
    }
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

void get_disk_info(char *out, size_t outSize) {
    char drives[256];
    DWORD len = GetLogicalDriveStringsA(sizeof(drives), drives);

    out[0] = '\0';

    if (len == 0 || len > sizeof(drives))
        return;

    char *p = drives;
    while (*p) {
        ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;

        if (GetDiskFreeSpaceExA(p, &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
            char line[256];

            unsigned long long totalGB = totalBytes.QuadPart / (1024ULL * 1024ULL * 1024ULL);
            unsigned long long freeGB  = totalFreeBytes.QuadPart / (1024ULL * 1024ULL * 1024ULL);
            unsigned long long usedGB  = totalGB - freeGB;

            snprintf(line, sizeof(line),
                     "%s %lluGB used / %lluGB total\n",
                     p, usedGB, totalGB);

            strncat(out, line, outSize - strlen(out) - 1);
        }

        p += strlen(p) + 1;
    }
}

int main(int argc, char** argv) {
    SetConsoleOutputCP(CP_UTF8);
        // Handle command-line flags
    if (argc > 1) {
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            printf("kfetch version %s\n", KFETCH_VERSION);
            return 0;
        }
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("KryptonFetch - System Information Tool\n\n");
            printf("Usage: kfetch [options]\n\n");
            printf("Options:\n");
            printf("  --version, -v   Show version information\n");
            printf("  --help, -h      Show this help message\n");
            return 0;
        }
    }


    char username[256];
    char computer[256];
    char osver[256];
    char arch[64];
    char cpu[256];
    char diskInfo[1024];
    char diskLine[1024];
    DWORDLONG totalMB, freeMB;
    short cols, rows;
    UINT cp;
    char gpus[2][256];
    int gpuCount = 0;
    int cpuCores = 0;
    int cpuThreads = 0;

    // Collect system info
    get_username(username, sizeof(username));
    get_computername(computer, sizeof(computer));
    get_os_version(osver, sizeof(osver));
    get_arch(arch, sizeof(arch));
    get_cpu_brand(cpu, sizeof(cpu));
    get_cpu_core_info(&cpuCores, &cpuThreads);
    get_memory_info(&totalMB, &freeMB);
    get_console_size(&cols, &rows);
    cp = GetConsoleOutputCP();
    get_gpus(gpus, &gpuCount);
    get_disk_info(diskInfo, sizeof(diskInfo));
    snprintf(diskLine, sizeof(diskLine), "%s", diskInfo);

    // Split disk lines
    char disk1[256] = "";
    char disk2[256] = "";
    char disk3[256] = "";

    {
        char temp[1024];
        strncpy(temp, diskLine, sizeof(temp));
        temp[sizeof(temp) - 1] = '\0';

        char *p = strtok(temp, "\n");
        if (p) strncpy(disk1, p, sizeof(disk1));
        p = strtok(NULL, "\n");
        if (p) strncpy(disk2, p, sizeof(disk2));
        p = strtok(NULL, "\n");
        if (p) strncpy(disk3, p, sizeof(disk3));
    }

    system("cls");

    set_color(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    printf("KryptonFetch\n");
    reset_color();
    printf("\n");

    char line1[256], line2[256], line3[256], line4[256], line5[256];

    _snprintf(line1, sizeof(line1), "%s@%s", username, computer);
    _snprintf(line2, sizeof(line2), "%s (%s)", osver, arch);

    _snprintf(line3, sizeof(line3),
              "%s (%d cores / %d threads)",
              cpu, cpuCores, cpuThreads);

    char memLine[256];
    _snprintf(memLine, sizeof(memLine), "RAM: %lluMB / %lluMB free",
              (unsigned long long)totalMB,
              (unsigned long long)freeMB);

    char termLine[256];
    _snprintf(termLine, sizeof(termLine), "Terminal: %hdx%hd  CP %u",
              cols, rows, cp);

    strncpy(line4, memLine, sizeof(line4));
    line4[sizeof(line4) - 1] = '\0';
    strncpy(line5, termLine, sizeof(line5));
    line5[sizeof(line5) - 1] = '\0';

    // GPU lines
    char gpu1[256] = "";
    char gpu2[256] = "";

    if (gpuCount == 0) {
        snprintf(gpu1, sizeof(gpu1), "None detected");
        gpu2[0] = '\0';
    } else if (gpuCount == 1) {
        snprintf(gpu1, sizeof(gpu1), "%s", gpus[0]);
        gpu2[0] = '\0';
    } else {
        snprintf(gpu1, sizeof(gpu1), "%s", gpus[0]);
        snprintf(gpu2, sizeof(gpu2), "%s", gpus[1]);
    }

    // 10 rows total: user, os, cpu, gpu1, gpu2, mem, term, disk1-3
    const char *infoLines[10] = {
        line1,  // 0 User
        line2,  // 1 OS
        line3,  // 2 CPU
        gpu1,   // 3 GPU line 1
        gpu2,   // 4 GPU line 2
        line4,  // 5 Memory
        line5,  // 6 Terminal
        disk1,  // 7 Disk line 1
        disk2,  // 8 Disk line 2
        disk3   // 9 Disk line 3
    };

    int i;
    for (i = 0; i < 10; ++i) {

        set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        switch (i) {
            case 0: printf("┌──────────┬──────────┐"); break;
            case 1: printf("│ ████████ │ ████████ │"); break;
            case 2: printf("│ ████████ │ ████████ │"); break;
            case 3: printf("│ ████████ │ ████████ │"); break;
            case 4: printf("├──────────┼──────────┤"); break;
            case 5: printf("│ ████████ │ ████████ │"); break;
            case 6: printf("│ ████████ │ ████████ │"); break;
            case 7: printf("│ ████████ │ ████████ │"); break;
            case 8: printf("└──────────┴──────────┘"); break;
            case 9: printf("                       "); break; // Blank row under logo
        }
        reset_color();

        printf("     ");

        set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        if (i == 0) printf("%-10s", "User");
        else if (i == 1) printf("%-10s", "OS");
        else if (i == 2) printf("%-10s", "CPU");
        else if (i == 3) printf("%-10s", "GPU");
        else if (i == 4) printf("%-10s", "");
        else if (i == 5) printf("%-10s", "Memory");
        else if (i == 6) printf("%-10s", "Terminal");
        else if (i == 7) printf("%-10s", "Disk");
        else printf("%-10s", "");
        reset_color();

        printf(": %s\n", infoLines[i]);
    }

    printf("\n");

    set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("====[ KryptonFetch ]====\n");
    printf("===[ By KryptonBytes ]===\n");
    reset_color();
    printf("\n");

    return 0;
}

