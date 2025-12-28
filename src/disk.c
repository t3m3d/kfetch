#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "disk.h"

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

        unsigned long long totalGB = totalBytes.QuadPart / (1024ULL * 1024ULL * 1024ULL);
        unsigned long long freeGB  = totalFreeBytes.QuadPart / (1024ULL * 1024ULL * 1024ULL);
        unsigned long long usedGB  = totalGB - freeGB;

        double percent = (totalGB > 0)
            ? ((double)usedGB / (double)totalGB * 100.0)
            : 0.0;

        char bar[64] = "";
        int width = 20;
        int used = (int)((percent / 100.0) * width);
        int free = width - used;

        for (int i = 0; i < used; i++)
            strcat(bar, "\xE2\x96\x92");

        for (int i = 0; i < free; i++)
            strcat(bar, "█");

        char line[256];
        snprintf(line, sizeof(line),
                 "%s  %lluGB / %lluGB  %.0f%%  [%s]\n",
                 root, usedGB, totalGB, percent, bar);

        strncat(out, line, outSize - strlen(out) - 1);
    }
}