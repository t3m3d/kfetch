#ifndef UTILS_H
#define UTILS_H

#include <windows.h>
#include <stddef.h>

void set_color(WORD attr);
void reset_color(void);

void make_vram_bar(double used_gb, double total_gb, char *out, size_t outSize);

int visible_width(const char *s);

void print_padded_ansi(const char *s, int total_width);

void get_username(char *buf, DWORD size);
void get_computername(char *buf, DWORD size);

void get_console_size(short *cols, short *rows);

#endif

