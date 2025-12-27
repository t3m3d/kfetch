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

    if (!GetVersionExA((OSVERSIONINFOA*)&vi)) {
        strncpy(buf, "Windows (version unknown)", size);
        buf[size - 1] = '\0';
        return;
    }

    DWORD build = vi.dwBuildNumber;
    const char *versionName = "Windows";
    const char *editionName = "Unknown Edition";

    HKEY hKey;
    char editionBuf[128] = {0};
    DWORD type = REG_SZ;
    DWORD dataSize = sizeof(editionBuf);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueExA(hKey, "EditionID", NULL, &type,
            (LPBYTE)editionBuf, &dataSize) == ERROR_SUCCESS)
        {
            editionBuf[sizeof(editionBuf) - 1] = '\0';
        }
        RegCloseKey(hKey);
    }

    if (strcmp(editionBuf, "Professional") == 0) editionName = "Pro";
    else if (strcmp(editionBuf, "Core") == 0) editionName = "Home";
    else if (strcmp(editionBuf, "Enterprise") == 0) editionName = "Enterprise";
    else if (strcmp(editionBuf, "Education") == 0) editionName = "Education";
    else if (strcmp(editionBuf, "ProfessionalWorkstation") == 0) editionName = "Pro Workstation";
    else if (strcmp(editionBuf, "IoTEnterprise") == 0) editionName = "IoT Enterprise";
    else if (strcmp(editionBuf, "ProfessionalN") == 0) editionName = "Pro N";
    else if (strcmp(editionBuf, "CoreN") == 0) editionName = "Home N";
    else if (strcmp(editionBuf, "CoreSingleLanguage") == 0) editionName = "Home Single Language";
    else if (editionBuf[0] != '\0') editionName = editionBuf;

    if      (build >= 26100) versionName = "Windows 11";
    else if (build >= 22631) versionName = "Windows 11";
    else if (build >= 22621) versionName = "Windows 11";
    else if (build >= 22000) versionName = "Windows 11";
    else if (build >= 19045) versionName = "Windows 10";
    else if (build >= 19044) versionName = "Windows 10";
    else if (build >= 19043) versionName = "Windows 10";
    else if (build >= 19042) versionName = "Windows 10";
    else if (build >= 19041) versionName = "Windows 10";
    else if (build >= 18363) versionName = "Windows 10";
    else if (build >= 18362) versionName = "Windows 10";
    else if (build >= 17763) versionName = "Windows 10";
    else if (build >= 17134) versionName = "Windows 10";
    else if (build >= 16299) versionName = "Windows 10";
    else if (build >= 15063) versionName = "Windows 10";
    else if (build >= 14393) versionName = "Windows 10";
    else if (build >= 10586) versionName = "Windows 10";
    else if (build >= 10240) versionName = "Windows 10";
    else versionName = "Windows (legacy version)";

    _snprintf(buf, size, "%s %s (build %lu)",
              versionName, editionName, build);
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

void get_uptime(char *buf, size_t size) {
    ULONGLONG ms = GetTickCount64();

    ULONGLONG totalSeconds = ms / 1000;
    ULONGLONG minutes = totalSeconds / 60;
    ULONGLONG hours = minutes / 60;
    ULONGLONG days = hours / 24;

    hours %= 24;
    minutes %= 60;

    if (days > 0) {
        _snprintf(buf, size, "%llu days, %llu hrs, %llu mins",
              days, hours, minutes);
    } else if (hours > 0) {
        _snprintf(buf, size, "%llu hrs, %llu mins",
              hours, minutes);
    } else {
        _snprintf(buf, size, "%llu mins",
              minutes);
    }
}

void get_disk_info(char *out, size_t outSize) {
    out[0] = '\0';

    DWORD drives = GetLogicalDrives();

    for (char letter = 'A'; letter <= 'Z'; letter++) {
        if (!(drives & (1 << (letter - 'A'))))
            continue;

        char root[4];
        snprintf(root, sizeof(root), "%c:\\", letter);

        ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
        if (!GetDiskFreeSpaceExA(root, &freeBytes, &totalBytes, &totalFreeBytes)) {
            char line[256];
            snprintf(line, sizeof(line), "%s  (access denied)\n", root);
            strncat(out, line, outSize - strlen(out) - 1);
            continue;
        }

        unsigned long long totalGB = totalBytes.QuadPart / (1024ULL*1024ULL*1024ULL);
        unsigned long long freeGB  = totalFreeBytes.QuadPart / (1024ULL*1024ULL*1024ULL);
        unsigned long long usedGB  = totalGB - freeGB;

        double percent = (totalGB > 0)
            ? ((double)usedGB / (double)totalGB * 100.0)
            : 0.0;

        char bar[64] = "";
        int width = 20;
        int used = (int)((percent / 100.0) * width);
        int free = width - used;

        for (int i = 0; i < used; i++) strcat(bar, "\xE2\x96\x92");
        for (int i = 0; i < free; i++) strcat(bar, "█");

        char line[256];
        snprintf(line, sizeof(line),
                 "%s  %lluGB / %lluGB  %.0f%%  [%s]\n",
                 root, usedGB, totalGB, percent, bar);

        strncat(out, line, outSize - strlen(out) - 1);
    }
}

// VRAM detection

unsigned long long get_vram_registry(int gpuIndex) {
    HKEY hKey;
    char path[512];
    unsigned long long vram = 0;

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

            if (RegQueryValueExA(hSub, "HardwareInformation.qwMemorySize", NULL, NULL,
                                 (LPBYTE)&mem, &memSize) == ERROR_SUCCESS) {

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

// VRAM bar

void make_vram_bar(double used_gb, double total_gb, char* out, size_t outSize) {
    const int barWidth = 20;

    if (total_gb <= 0.0) {
        snprintf(out, outSize, "[%*s] 0.0 / 0.0 GB", barWidth, "");
        return;
    }

    double free_gb = total_gb - used_gb;
    if (free_gb < 0) free_gb = 0;

    int usedBlocks = (int)((used_gb / total_gb) * barWidth);
    if (usedBlocks < 0) usedBlocks = 0;
    if (usedBlocks > barWidth) usedBlocks = barWidth;

    int freeBlocks = barWidth - usedBlocks;

    char bar[128] = {0};

    // Used on the left (solid), free on the right (light shade)
    for (int i = 0; i < usedBlocks; i++)
        strcat(bar, "█");

    for (int i = 0; i < freeBlocks; i++)
        strcat(bar, "\xE2\x96\x92");

    snprintf(out, outSize, "[%s] %.1f / %.1f GB", bar, used_gb, total_gb);
}

/* -------------------- main -------------------- */

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
    char uptime[128];

    get_uptime(uptime, sizeof(uptime));
    get_username(username, sizeof(username));
    get_computername(computer, sizeof(computer));
    get_os_version(osver, sizeof(osver));
    get_arch(arch, sizeof(arch));
    get_cpu_brand(cpu, sizeof(cpu));
    get_cpu_core_info(&cpuCores, &cpuThreads);
    get_memory_info(&totalMB, &freeMB);

    unsigned long long usedMB = totalMB - freeMB;
    double ramPercent = 0.0;

    if (totalMB > 0) {
        ramPercent = (double)usedMB / (double)totalMB * 100.0;
    }

    int barWidth = 20;
    int usedFilled = (int)((ramPercent / 100.0) * barWidth);
    int freeFilled = barWidth - usedFilled;
    char bar[128];
    memset(bar, 0, sizeof(bar));

    for (int i = 0; i < freeFilled; i++) {
        strcat(bar, "\xE2\x96\x92");
    }
    for (int i = 0; i < usedFilled; i++) {
        strcat(bar, "█");
    }

    char ramBar[64];
    memset(ramBar, 0, sizeof(ramBar));
    _snprintf(ramBar, sizeof(ramBar),
          "[%s]", bar);

    get_console_size(&cols, &rows);
    cp = GetConsoleOutputCP();
    get_gpus(gpus, &gpuCount);
    get_disk_info(diskInfo, sizeof(diskInfo));
    snprintf(diskLine, sizeof(diskLine), "%s", diskInfo);

    char disk1[1024] = "";
    char disk2[1024] = "";
    char disk3[1024] = "";

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
    _snprintf(memLine, sizeof(memLine),
          "RAM: %lluMB used / %lluMB total %.0f%%  %s",
          usedMB, totalMB, ramPercent, ramBar);

    char termLine[256];
    _snprintf(termLine, sizeof(termLine), "Terminal: %hdx%hd  CP %u",
              cols, rows, cp);

    strncpy(line4, memLine, sizeof(line4));
    line4[sizeof(line4) - 1] = '\0';
    strncpy(line5, termLine, sizeof(line5));
    line5[sizeof(line5) - 1] = '\0';

    // GPU lines + VRAM bars
    char gpu1[256] = "";
    char gpu2[256] = "";
    char gpu1_vram[256] = "";
    char gpu2_vram[256] = "";

    if (gpuCount == 0) {
        snprintf(gpu1, sizeof(gpu1), "None detected");
        gpu1_vram[0] = '\0';
        gpu2[0] = '\0';
        gpu2_vram[0] = '\0';
    } else if (gpuCount == 1) {
        unsigned long long vram0 = get_vram_registry(0);
        double total0 = (double)vram0 / (1024.0 * 1024.0 * 1024.0);
        double used0 = total0;

        snprintf(gpu1, sizeof(gpu1), "%s (%.1f GB VRAM)", gpus[0], total0);
        make_vram_bar(used0, total0, gpu1_vram, sizeof(gpu1_vram));

        gpu2[0] = '\0';
        gpu2_vram[0] = '\0';
    } else {
        unsigned long long vram0 = get_vram_registry(0);
        unsigned long long vram1 = get_vram_registry(1);

        double total0 = (double)vram0 / (1024.0 * 1024.0 * 1024.0);
        double total1 = (double)vram1 / (1024.0 * 1024.0 * 1024.0);

        double used0 = total0;
        double used1 = total1;

        snprintf(gpu1, sizeof(gpu1), "%s (%.1f GB VRAM)", gpus[0], total0);
        snprintf(gpu2, sizeof(gpu2), "%s (%.1f GB VRAM)", gpus[1], total1);

        make_vram_bar(used0, total0, gpu1_vram, sizeof(gpu1_vram));
        make_vram_bar(used1, total1, gpu2_vram, sizeof(gpu2_vram));
    }

    const char *infoLines[13] = {
        line1,
        line2,
        line3,
        gpu1,
        gpu1_vram,
        gpu2,
        gpu2_vram,
        line5,
        disk1,
        disk2,
        disk3,
        line4,
        uptime
    };

    int i;
    for (i = 0; i < 13; ++i) {

        set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);

    if (i == 0) {
        printf("┌──────────┬──────────┐");
    }
    else if (i == 6) {
        printf("├──────────┼──────────┤");
    }
    else if (i == 12) {
        printf("└──────────┴──────────┘");
    }
    else {
        printf("│ ████████ │ ████████ │");
    }


        reset_color();

        printf("     ");

        set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        if (i == 0)      printf("%-10s", "User");
        else if (i == 1) printf("%-10s", "OS");
        else if (i == 2) printf("%-10s", "CPU");
        else if (i == 3) printf("%-10s", "Main GPU");
        else if (i == 4) printf("%-10s", "");          // VRAM bar under Main GPU
        else if (i == 5) printf("%-10s", "Alt GPU");
        else if (i == 6) printf("%-10s", "");          // VRAM bar under Alt GPU
        else if (i == 7) printf("%-10s", "Terminal");
        else if (i == 8) printf("%-10s", "Disk 1");
        else if (i == 9) printf("%-10s", "Disk 2");
        else if (i == 10) printf("%-10s", "Disk 3");
        else if (i == 11) printf("%-10s", "Memory");
        else if (i == 12) printf("%-10s", "Uptime");
        else              printf("%-10s", "");
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