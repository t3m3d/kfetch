#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "os.h"

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