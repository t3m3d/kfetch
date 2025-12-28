#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cpu.h"
#include "gpu.h"
#include "mem.h"
#include "os.h"
#include "disk.h"
#include "utils.h"
#include "version.h"
#include "kfetch.h"

void run_kfetch(int argc, char **argv) {
    SetConsoleOutputCP(CP_UTF8);

    if (argc > 1) {
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            printf("kfetch version %s\n", KFETCH_VERSION);
            return;
        }
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("KryptonFetch - System Information Tool\n\n");
            printf("Usage: kfetch [options]\n\n");
            printf("Options:\n");
            printf("  --version, -v   Show version information\n");
            printf("  --help, -h      Show this help message\n");
            return;
        }
    }

    #define LOGO_WIDTH 36
    const char *windows_logo[] = {
        "\x1b[34m        ,.=:!!t3Z3z.,\x1b[0m",
        "\x1b[34m       :tt:::tt333EE3\x1b[0m",
        "\x1b[34m       Et:::ztt33EEEL\x1b[36m @Ee.,      ..,\x1b[0m",
        "\x1b[34m      ;tt:::tt333EE7\x1b[36m ;EEEEEEttttt33#\x1b[0m",
        "\x1b[34m     :Et:::zt333EEQ\x1b[36m  SEEEEEttttt33QL\x1b[0m",
        "\x1b[34m     it::::tt333EEF\x1b[36m @EEEEttttt33F\x1b[0m",
        "\x1b[34m    ;3=*^```\"*4EEV\x1b[36m :EEEEttttt33@.\x1b[0m",
        "\x1b[36m    ,.=::::!t=.,\x1b[34m  @EEEEtttz33QF\x1b[0m",
        "\x1b[36m   ;::::::::zt33)\x1b[34m   \"4EEEtttji\x1b[0m",
        "\x1b[36m  :t::::::::tt33.\x1b[34m :Z3z..  ``\x1b[0m",
        "\x1b[36m  i::::::::zt33F\x1b[34m AEEEtttt::::ztF\x1b[0m",
        "\x1b[36m ;:::::::::t33V\x1b[34m ;EEEttttt::::t3\x1b[0m",
        "\x1b[36m E::::::::zt33L\x1b[34m @EEEtttt::::z3F\x1b[0m",
        "\x1b[36m{3=*^```\"*4E3)\x1b[34m ;EEEtttt:::::tZ`\x1b[0m",
        "\x1b[36m             `\x1b[34m :EEEEtttt::::z7\x1b[0m",
        "\x1b[34m                 \"VEzjt:;;z>*`\x1b[0m"
    };
    const int windows_logo_lines = 16;

    char username[256], computer[256], osver[256], arch[64], cpu[256];
    char diskInfo[1024], diskLine[1024];
    DWORDLONG totalMB = 0, freeMB = 0;
    short cols = 0, rows = 0;
    UINT cp = 0;
    char gpus[2][256];
    int gpuCount = 0, cpuCores = 0, cpuThreads = 0;
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
    double ramPercent = (totalMB > 0)
        ? ((double)usedMB / (double)totalMB * 100.0)
        : 0.0;

    int barWidth = 20;
    int usedFilled = (int)((ramPercent / 100.0) * barWidth);
    int freeFilled = barWidth - usedFilled;

    char bar[128] = {0};
    for (int i = 0; i < freeFilled; i++)
        strcat(bar, "\xE2\x96\x92");
    for (int i = 0; i < usedFilled; i++)
        strcat(bar, "█");

    char ramBar[64];
    _snprintf(ramBar, sizeof(ramBar), "[%s]", bar);

    get_console_size(&cols, &rows);
    cp = GetConsoleOutputCP();
    get_gpus(gpus, &gpuCount);
    get_disk_info(diskInfo, sizeof(diskInfo));
    snprintf(diskLine, sizeof(diskLine), "%s", diskInfo);

    char disk1[1024] = "", disk2[1024] = "", disk3[1024] = "";
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
    _snprintf(line3, sizeof(line3), "%s (%d cores / %d threads)",
              cpu, cpuCores, cpuThreads);

    char memLine[256];
    _snprintf(memLine, sizeof(memLine),
              "RAM: %lluMB used / %lluMB total %.0f%%  %s",
              usedMB, totalMB, ramPercent, ramBar);

    char termLine[256];
    _snprintf(termLine, sizeof(termLine),
              "Terminal: %hdx%hd  CP %u", cols, rows, cp);

    strncpy(line4, memLine, sizeof(line4));
    line4[sizeof(line4) - 1] = '\0';
    strncpy(line5, termLine, sizeof(line5));
    line5[sizeof(line5) - 1] = '\0';

    char gpu1[256] = "", gpu2[256] = "";
    char gpu1_vram[256] = "", gpu2_vram[256] = "";

    if (gpuCount == 0) {
        snprintf(gpu1, sizeof(gpu1), "None detected");
    } else if (gpuCount == 1) {
        unsigned long long used0 = 0, total0 = 0;
        int printed = 0;

        if (strstr(gpus[0], "NVIDIA") || strstr(gpus[0], "GeForce")) {
            if (get_vram_usage_nvapi(0, &used0, &total0)) {
                double usedGB  = used0  / (1024.0 * 1024.0 * 1024.0);
                double totalGB = total0 / (1024.0 * 1024.0 * 1024.0);

                snprintf(gpu1, sizeof(gpu1),
                         "%s (%.1f GB VRAM)", gpus[0], totalGB);
                make_vram_bar(usedGB, totalGB,
                              gpu1_vram, sizeof(gpu1_vram));
                printed = 1;
            }
        }

        if (!printed) {
            unsigned long long vram0 = get_vram_registry(0);
            double totalGB = vram0 / (1024.0 * 1024.0 * 1024.0);

            snprintf(gpu1, sizeof(gpu1),
                     "%s (%.1f GB VRAM)", gpus[0], totalGB);
            snprintf(gpu1_vram, sizeof(gpu1_vram),
                     "[VRAM usage unavailable]");
        }
    } else { // 2 GPUs
        unsigned long long used0 = 0, total0 = 0;
        unsigned long long used1 = 0, total1 = 0;
        int printed0 = 0, printed1 = 0;

        if (strstr(gpus[0], "NVIDIA") || strstr(gpus[0], "GeForce")) {
            if (get_vram_usage_nvapi(0, &used0, &total0)) {
                double usedGB0  = used0  / (1024.0 * 1024.0 * 1024.0);
                double totalGB0 = total0 / (1024.0 * 1024.0 * 1024.0);

                snprintf(gpu1, sizeof(gpu1),
                         "%s (%.1f GB VRAM)", gpus[0], totalGB0);
                make_vram_bar(usedGB0, totalGB0,
                              gpu1_vram, sizeof(gpu1_vram));
                printed0 = 1;
            }
        }

        if (strstr(gpus[1], "NVIDIA") || strstr(gpus[1], "GeForce")) {
            if (get_vram_usage_nvapi(1, &used1, &total1)) {
                double usedGB1  = used1  / (1024.0 * 1024.0 * 1024.0);
                double totalGB1 = total1 / (1024.0 * 1024.0 * 1024.0);

                snprintf(gpu2, sizeof(gpu2),
                         "%s (%.1f GB VRAM)", gpus[1], totalGB1);
                make_vram_bar(usedGB1, totalGB1,
                              gpu2_vram, sizeof(gpu2_vram));
                printed1 = 1;
            }
        }

        if (!printed0) {
            unsigned long long vram0 = get_vram_registry(0);
            double totalGB0 = vram0 / (1024.0 * 1024.0 * 1024.0);

            snprintf(gpu1, sizeof(gpu1),
                     "%s (%.1f GB VRAM)", gpus[0], totalGB0);
            snprintf(gpu1_vram, sizeof(gpu1_vram),
                     "[VRAM usage unavailable]");
        }

        if (!printed1) {
            unsigned long long vram1 = get_vram_registry(1);
            double totalGB1 = vram1 / (1024.0 * 1024.0 * 1024.0);

            snprintf(gpu2, sizeof(gpu2),
                     "%s (%.1f GB VRAM)", gpus[1], totalGB1);
            snprintf(gpu2_vram, sizeof(gpu2_vram),
                     "[VRAM usage unavailable]");
        }
    }

    const char *infoLines[13] = {
        line1, line2, line3,
        gpu1, gpu1_vram,
        gpu2, gpu2_vram,
        line5,
        disk1, disk2, disk3,
        line4,
        uptime
    };

    const char *labels[13] = {
        "User", "OS", "CPU",
        "GPU1", "",
        "GPU2", "",
        "Term",
        "Disk 1", "Disk 2", "Disk 3",
        "Memory",
        "Uptime"
    };

    int maxLines = 13;
    int total = (maxLines > windows_logo_lines)
        ? maxLines : windows_logo_lines;

    for (int i = 0; i < total; i++) {
        if (i < windows_logo_lines)
            print_padded_ansi(windows_logo[i], LOGO_WIDTH);
        else
            print_padded_ansi("", LOGO_WIDTH);

        printf("    ");

        if (i < maxLines) {
            if (labels[i][0] != '\0') {
                set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                printf("%s: ", labels[i]);
                reset_color();
            } else {
                printf("      ");
            }
            printf("%s\n", infoLines[i]);
        } else {
            printf("\n");
        }
    }

    printf("\n");
    set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("====[ KryptonFetch ]====\n");
    printf("===[ By KryptonBytes ]===\n");
    reset_color();
    printf("\n");
}