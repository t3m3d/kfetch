#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "kfetch.h"
#include "version.h"

int main(int argc, char **argv) {
    SetConsoleOutputCP(CP_UTF8);

    // Handle CLI flags
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

    run_kfetch();

    return 0;
}