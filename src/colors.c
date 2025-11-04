#include "colors.h"
#include <stdio.h>
#include <stdlib.h>

static int TEMP_COLORS[] = {51, 45, 39, 38, 33, 32, 27, 26, 21, 190, 226, 220, 214, 208, 202, 124, 160, 196};
#define TEMP_COLORS_SIZE ((int)(sizeof(TEMP_COLORS) / sizeof(TEMP_COLORS[0])))

size_t ctext_fg(char* buffer, size_t size, int color, const char* text)
{
    return snprintf(buffer, size, "\033[38;5;%dm%s\033[0m", color, text);
}

size_t ctext_u(char* buffer, size_t size, const char* text)
{
    return snprintf(buffer, size, "\033[4m%s\033[0m", text);
}

size_t ctext_uc(char* buffer, size_t size, int underline_color, const char* text)
{
    return snprintf(buffer, size,
                    "\033[4:1:%dm%s\033[0m",
                    underline_color,
                    text);
}

size_t cnum_fg(char* buffer, size_t size, int color, const int number)
{
    return snprintf(buffer, size, "\033[38;5;%dm%d\033[0m", color, number);
}

size_t ctext_bg(char* buffer, size_t size, int color, const char* text)
{
    return snprintf(buffer, size, "\033[48;5;%dm%s\033[0m", color, text);
}

int contrast_color(int color)
{
    if (color < 16) return (color == 0) ? 15 : 0;
    if (color > 231) return (color < 244) ? 15 : 0;
    int g = ((color - 16) % 36) / 6;
    return (g > 2) ? 0 : 15;
}

size_t ctext_fg_con(char* buffer, size_t size, int color, const char* text)
{
    int contrast = contrast_color(color);
    return snprintf(buffer, size, "\033[48;5;%dm\033[38;5;%dm%s\033[0m\033[0m", contrast, color, text);
}

size_t ctext_bg_con(char* buffer, size_t size, int color, const char* text)
{
    int contrast = contrast_color(color);
    return snprintf(buffer, size, "\033[48;5;%dm\033[38;5;%dm%s\033[0m\033[0m", color, contrast, text);
}

int temperature_to_color(double temp, double temp_min, double temp_max)
{
    if (temp <= temp_min) { return TEMP_COLORS[0]; }
    if (temp >= temp_max) { return TEMP_COLORS[TEMP_COLORS_SIZE - 1]; }
    int index = (temp - temp_min) * (TEMP_COLORS_SIZE - 1) / (temp_max - temp_min);
    return TEMP_COLORS[index];
}

size_t temp_to_ctext_fg(char* buffer, size_t size, double temp, double temp_min, double temp_max)
{
    int color = temperature_to_color(temp, temp_min, temp_max);
    return snprintf(buffer, size, "\033[38;5;%dm%.0f󰔄\033[0m", color, temp);
}

size_t temp_to_ctext_bg(char* buffer, size_t size, double temp, double temp_min, double temp_max)
{
    int color = temperature_to_color(temp, temp_min, temp_max);
    return snprintf(buffer, size, "\033[48;5;%dm%7.2f 󰔄\033[0m", color, temp);
}

size_t temp_to_ctext_fg_con(char* buffer, size_t size, double temp, double temp_min, double temp_max)
{
    int color = temperature_to_color(temp, temp_min, temp_max);
    return snprintf(buffer, size, "\033[48;5;%dm\033[38;5;%dm%7.2f 󰔄\033[0m", contrast_color(color), color, temp);
}

size_t temp_to_ctext_bg_con(char* buffer, size_t size, double temp, double temp_min, double temp_max)
{
    int color = temperature_to_color(temp, temp_min, temp_max);
    return snprintf(buffer, size, "\033[48;5;%dm\033[38;5;%dm%7.2f 󰔄\033[0m", color, contrast_color(color), temp);
}
