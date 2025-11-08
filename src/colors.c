#include "colors.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static int s_temp_colors[] = {51, 45, 39, 38, 33, 32, 27, 26, 21, 190, 226, 220, 214, 208, 202, 124, 160, 196};
#define s_temp_colors_size ((int)(sizeof(s_temp_colors) / sizeof(s_temp_colors[0])))

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
    if (temp <= temp_min) { return s_temp_colors[0]; }
    if (temp >= temp_max) { return s_temp_colors[s_temp_colors_size - 1]; }
    int diff = temp_max - temp_min;
    int index = (temp - temp_min) * (s_temp_colors_size - 1) / (diff > 0 ? diff : 1);
    return s_temp_colors[index];
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

static int s_radiator_temp_colors[] = {51, 50, 45, 44, 43, 39, 38, 37, 36, 154, 190, 226, 148, 184, 214, 136, 172, 208, 130, 166, 202, 124, 160, 196};
#define s_radiator_temp_colors_size ((int)(sizeof(s_radiator_temp_colors) / sizeof(s_radiator_temp_colors[0])))

// Warm-up and cool-down durations (in seconds)
static const int RADIATOR_WARMUP_SEC = 8 * 60;                    // 8 minutes
static const int RADIATOR_COOLDOWN_SEC = 1 * 60 * 60 + (30 * 60); // 1 hour and 30 min

// State tracking
static time_t s_heating_started = 0;
static time_t s_heating_stopped = 0;
static int s_is_heating = 0;

int radiator_color_update(int is_heating_now)
{
    time_t now = time(NULL);

    // Detect transitions: off -> on
    if (is_heating_now && !s_is_heating) {
        s_heating_started = now;
    }

    // Detect transitions: on -> off
    if (!is_heating_now && s_is_heating) {
        s_heating_stopped = now;
    }

    s_is_heating = is_heating_now;

    // --- HEATING ---
    if (s_is_heating) {
        double elapsed = difftime(now, s_heating_started);

        if (elapsed >= RADIATOR_WARMUP_SEC) {
            return s_radiator_temp_colors[s_radiator_temp_colors_size - 1]; // fully hot
        }

        double ratio = elapsed / RADIATOR_WARMUP_SEC;
        int index = (int)(ratio * (s_radiator_temp_colors_size - 1));
        return s_radiator_temp_colors[index];
    }

    // --- COOLING ---
    if (s_heating_stopped == 0) {
        // Never heated yet, cold
        return s_radiator_temp_colors[0];
    }

    double elapsed = difftime(now, s_heating_stopped);

    if (elapsed >= RADIATOR_COOLDOWN_SEC) {
        return s_radiator_temp_colors[0]; // fully cold
    }

    double ratio = elapsed / RADIATOR_COOLDOWN_SEC;
    int index = (s_radiator_temp_colors_size - 1) - (int)(ratio * (s_radiator_temp_colors_size - 1));
    return s_radiator_temp_colors[index];
}
