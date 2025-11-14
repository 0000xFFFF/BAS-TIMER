#include "colors.h"
#include "globals.h"
#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int s_temp_colors[] = {51, 45, 39, 38, 33, 32, 27, 26, 190, 226, 220, 214, 208, 202, 124, 160, 196};
#define s_temp_colors_size ((int)(sizeof(s_temp_colors) / sizeof(s_temp_colors[0])))

static int s_radiator_temp_colors[] = {51, 50, 45, 44, 43, 39, 38, 37, 36, 154, 190, 226, 148, 184, 214, 136, 172, 208, 130, 166, 202, 124, 160, 196};
#define s_radiator_temp_colors_size ((int)(sizeof(s_radiator_temp_colors) / sizeof(s_radiator_temp_colors[0])))

size_t ctext_fg(char* buffer, size_t size, int color, const char* text)
{
    return (size_t)snprintf(buffer, size, "\033[38;5;%dm%s\033[0m", color, text);
}

size_t ctext_u(char* buffer, size_t size, const char* text)
{
    return (size_t)snprintf(buffer, size, "\033[4m%s\033[0m", text);
}

size_t ctext_uc(char* buffer, size_t size, int underline_color, const char* text)
{
    return (size_t)snprintf(buffer, size,
                            "\033[4:1:%dm%s\033[0m",
                            underline_color,
                            text);
}

size_t cnum_fg(char* buffer, size_t size, int color, const int number)
{
    return (size_t)snprintf(buffer, size, "\033[38;5;%dm%d\033[0m", color, number);
}

size_t ctext_bg(char* buffer, size_t size, int color, const char* text)
{
    return (size_t)snprintf(buffer, size, "\033[48;5;%dm%s\033[0m", color, text);
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
    return (size_t)snprintf(buffer, size, "\033[48;5;%dm\033[38;5;%dm%s\033[0m\033[0m", contrast, color, text);
}

size_t ctext_bg_con(char* buffer, size_t size, int color, const char* text)
{
    int contrast = contrast_color(color);
    return (size_t)snprintf(buffer, size, "\033[48;5;%dm\033[38;5;%dm%s\033[0m\033[0m", color, contrast, text);
}

int temperature_to_color(double temp, double temp_min, double temp_max)
{
    if (temp <= temp_min) { return s_temp_colors[0]; }
    if (temp >= temp_max) { return s_temp_colors[s_temp_colors_size - 1]; }
    int diff = (int)(temp_max - temp_min);
    int index = (int)((temp - temp_min) * (s_temp_colors_size - 1) / (diff > 0 ? diff : 1));
    return s_temp_colors[index];
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
static size_t safe_format(char* buf, size_t sz, const char* fmt, double v)
{
    /* reject formats containing %n */
    if (strstr(fmt, "%n") != NULL)
        return 0;

    return (size_t)snprintf(buf, sz, fmt, v);
}
#pragma GCC diagnostic pop

size_t temp_to_ctext_fg(char* buffer, size_t size, double t, double t_min, double t_max, const char* const num_format)
{
    int color = temperature_to_color(t, t_min, t_max);

    size_t b = 0;
    b += (size_t)snprintf(buffer + b, size - b, "\033[38;5;%dm", color);
    b += (size_t)safe_format(buffer + b, size - b, num_format, t);
    b += (size_t)snprintf(buffer + b, size - b, "󰔄\033[0m");
    return b;
}

size_t temp_to_ctext_bg(char* buffer, size_t size, double t, double t_min, double t_max, const char* const num_format)
{
    int color = temperature_to_color(t, t_min, t_max);

    size_t b = 0;
    b += (size_t)snprintf(buffer + b, size - b, "\033[48;5;%dm", color);
    b += (size_t)safe_format(buffer + b, size - b, num_format, t);
    b += (size_t)snprintf(buffer + b, size - b, "󰔄\033[0m");
    return b;
}

int radiator_color_update(struct BasInfo* info)
{
    time_t now = time(NULL);

    // Initialize on first call
    if (info->radiator_color_last_update == 0) { info->radiator_color_last_update = now; }

    double delta_time = difftime(now, info->radiator_color_last_update);
    info->radiator_color_last_update = now;

    // HEATING
    if (info->mod_rada) {
        double warmup_rate = delta_time / RADIATOR_WARMUP_SEC;
        info->radiator_color_current_temp_ratio += warmup_rate;
        if (info->radiator_color_current_temp_ratio > 1.0) {
            info->radiator_color_current_temp_ratio = 1.0;
        }
    }
    // COOLING
    else {
        double cooldown_rate = delta_time / RADIATOR_COOLDOWN_SEC;
        info->radiator_color_current_temp_ratio -= cooldown_rate;
        if (info->radiator_color_current_temp_ratio < 0.0) {
            info->radiator_color_current_temp_ratio = 0.0;
        }
    }

    // Map temperature ratio to color index
    info->radiator_color_index = (int)(info->radiator_color_current_temp_ratio * (s_radiator_temp_colors_size - 1));
    info->radiator_color = s_radiator_temp_colors[info->radiator_color_index];
    return info->radiator_color;
}
