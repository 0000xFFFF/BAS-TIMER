// colors.cpp
#include "colors.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>

int TEMP_COLORS[TEMP_COLORS_SIZE] = {51, 45, 39, 38, 33, 32, 27, 26, 21, 190, 226, 220, 214, 208, 202, 124, 160, 196};

double g_temp_solar_min = TEMP_SOLAR_MIN;
double g_temp_solar_max = TEMP_SOLAR_MAX;
double g_temp_human_min = TEMP_HUMAN_MIN;
double g_temp_human_max = TEMP_HUMAN_MAX;
double g_temp_buf_min = TEMP_BUF_MIN;
double g_temp_buf_max = TEMP_BUF_MAX;
double g_temp_circ_min = TEMP_CIRC_MIN;
double g_temp_circ_max = TEMP_CIRC_MAX;

#define BUFFER_SIZE 1024

// Function to convert int to string and format with foreground color
size_t ctext_fg(char* buffer, size_t size, int color, const char* text)
{
    return snprintf(buffer, size, "\033[38;5;%dm%s\033[0m", color, text);
}

size_t cnum_fg(char* buffer, size_t size, int color, const int number)
{
    return snprintf(buffer, size, "\033[38;5;%dm%d\033[0m", color, number);
}

// Function to convert int to string and format with background color
size_t ctext_bg(char* buffer, size_t size, int color, const char* text)
{
    return snprintf(buffer, size, "\033[48;5;%dm%s\033[0m", color, text);
}

// Function to determine the contrast color
int contrast_color(int color)
{
    if (color < 16) return (color == 0) ? 15 : 0;
    if (color > 231) return (color < 244) ? 15 : 0;
    int g = ((color - 16) % 36) / 6;
    return (g > 2) ? 0 : 15;
}

// Function for foreground with contrast color
size_t ctext_fg_con(char* buffer, size_t size, int color, const char* text)
{
    int contrast = contrast_color(color);
    return snprintf(buffer, size, "\033[48;5;%dm\033[38;5;%dm%s\033[0m\033[0m", contrast, color, text);
}

// Function for background with contrast color
size_t ctext_bg_con(char* buffer, size_t size, int color, const char* text)
{
    int contrast = contrast_color(color);
    return snprintf(buffer, size, "\033[48;5;%dm\033[38;5;%dm%s\033[0m\033[0m", color, contrast, text);
}

// Convert temperature to a color
int temperature_to_color(double temp, double* temp_min, double* temp_max)
{
    if (temp <= *temp_min) {
        *temp_min = temp;
        return TEMP_COLORS[0];
    }
    if (temp >= *temp_max) {
        *temp_max = temp;
        return TEMP_COLORS[TEMP_COLORS_SIZE - 1];
    }
    int index = (temp - *temp_min) * (TEMP_COLORS_SIZE - 1) / (*temp_max - *temp_min);
    return TEMP_COLORS[index];
}

// Return a foreground colored string based on temperature
size_t temp_to_ctext_fg(char* buffer, size_t size, double temp, double* temp_min, double* temp_max)
{
    int color = temperature_to_color(temp, temp_min, temp_max);
    return snprintf(buffer, size, "\033[38;5;%dm%7.2f 󰔄\033[0m", color, temp);
}

// Return a background colored string based on temperature
size_t temp_to_ctext_bg(char* buffer, size_t size, double temp, double* temp_min, double* temp_max)
{
    int color = temperature_to_color(temp, temp_min, temp_max);
    return snprintf(buffer, size, "\033[48;5;%dm%7.2f 󰔄\033[0m", color, temp);
}

// Return a foreground colored string with contrast based on temperature
size_t temp_to_ctext_fg_con(char* buffer, size_t size, double temp, double* temp_min, double* temp_max)
{
    int color = temperature_to_color(temp, temp_min, temp_max);
    return snprintf(buffer, size, "\033[48;5;%dm\033[38;5;%dm%7.2f 󰔄\033[0m", contrast_color(color), color, temp);
}

// Return a background colored string with contrast based on temperature
size_t temp_to_ctext_bg_con(char* buffer, size_t size, double temp, double* temp_min, double* temp_max)
{
    int color = temperature_to_color(temp, temp_min, temp_max);
    return snprintf(buffer, size, "\033[48;5;%dm\033[38;5;%dm%7.2f 󰔄\033[0m", color, contrast_color(color), temp);
}
