#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "utils.h"


void set_color(WORD attr) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, attr);
}

void reset_color() {
    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
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
        strcat(bar, "\xE2\x96\x92");

    for (int i = 0; i < freeBlocks; i++)
        strcat(bar, "█");

    snprintf(out, outSize, "[%s] %.1f / %.1f GB", bar, used_gb, total_gb);
}

int visible_width(const char *s) {
    int width = 0, in_escape = 0;
    while (*s) {
        if (*s == '\x1b') in_escape = 1;
        else if (in_escape && *s == 'm') in_escape = 0;
        else if (!in_escape) width++;
        s++;
    }
    return width;
}

void print_padded_ansi(const char *s, int total_width) {
    int w = visible_width(s);
    int pad = total_width - w;
    if (pad < 0) pad = 0;
    printf("%s", s);
    for (int i = 0; i < pad; i++) putchar(' ');
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