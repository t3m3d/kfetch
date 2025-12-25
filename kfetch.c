#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

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
    /* Note: GetVersionEx is deprecated but OK for a toy tool.
       For production, use Version Helper APIs or RtlGetVersion. */
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

/* ---------- Printing helpers ---------- */

void print_label_value(const char *label, const char *value) {
    set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("%-12s", label);
    reset_color();
    printf(": %s\n", value);
}

void print_label_value_u64(const char *label, DWORDLONG value) {
    char tmp[64];
    _snprintf(tmp, sizeof(tmp), "%llu MB", (unsigned long long)value);
    print_label_value(label, tmp);
}

int main(void) {
    SetConsoleOutputCP(CP_UTF8);
    char username[256];
    char computer[256];
    char osver[256];
    char arch[64];
    char cpu[256];

    DWORDLONG totalMB, freeMB;
    short cols, rows;
    UINT cp;

    get_username(username, sizeof(username));
    get_computername(computer, sizeof(computer));
    get_os_version(osver, sizeof(osver));
    get_arch(arch, sizeof(arch));
    get_cpu_brand(cpu, sizeof(cpu));
    get_memory_info(&totalMB, &freeMB);
    get_console_size(&cols, &rows);
    cp = GetConsoleOutputCP();

    system("cls");

    set_color(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    printf("KFetch-Lite\n");
    reset_color();
    printf("\n");

    char line1[256], line2[256], line3[256], line4[256], line5[256];
    _snprintf(line1, sizeof(line1), "%s@%s", username, computer);
    _snprintf(line2, sizeof(line2), "%s (%s)", osver, arch);

    char memLine[256];
    _snprintf(memLine, sizeof(memLine), "RAM: %lluMB / %lluMB free",
              (unsigned long long)totalMB,
              (unsigned long long)freeMB);

    char termLine[256];
    _snprintf(termLine, sizeof(termLine), "Terminal: %hdx%hd  CP %u",
              cols, rows, cp);

    strncpy(line3, cpu, sizeof(line3)); line3[sizeof(line3)-1] = '\0';
    strncpy(line4, memLine, sizeof(line4)); line4[sizeof(line4)-1] = '\0';
    strncpy(line5, termLine, sizeof(line5)); line5[sizeof(line5)-1] = '\0';

    const char *infoLines[5] = { line1, line2, line3, line4, line5 };

    int i;
    for (i = 0; i < 5; ++i) {
        /* Logo (left) */
        set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
 
        switch (i) {
    case 0: printf(" _   __ "); break;
            case 1: printf("| | / / "); break;
            case 2: printf("| |/ /  "); break;
            case 3: printf("| |\\ \\  "); break;
            case 4: printf("|_| \\_\\ "); break;
        }
        reset_color();

        printf("     ");

        set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        if (i == 0) {
            printf("%-10s", "User");
        } else if (i == 1) {
            printf("%-10s", "OS");
        } else if (i == 2) {
            printf("%-10s", "CPU");
        } else if (i == 3) {
            printf("%-10s", "Memory");
        } else if (i == 4) {
            printf("%-10s", "Terminal");
        }
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