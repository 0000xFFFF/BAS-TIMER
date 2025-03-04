// colors.cpp
#include "colors.h"
#include <stdio.h>
#include <stdlib.h>

int TEMP_COLORS[TEMP_COLORS_SIZE] = {51, 45, 39, 38, 33, 32, 27, 26, 21, 190, 226, 220, 214, 208, 202, 124, 160, 196};
int APPEAR_COLORS[APPEAR_COLORS_SIZE] = {240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255};

double g_temp_max = TEMP_MAX;
double g_temp_min = TEMP_MIN;

#define BUFFER_SIZE 1024

// Function to convert int to string and format with foreground color
char* ctext_fg(int color, const char* text) {
    char* result = (char*)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(result, BUFFER_SIZE, "\033[38;5;%dm%s\033[0m", color, text);
    return result;
}

char* cnum_fg(int color, const int number) {
    char* result = (char*)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(result, BUFFER_SIZE, "\033[38;5;%dm%d\033[0m", color, number);
    return result;
}

// Function to convert int to string and format with background color
char* ctext_bg(int color, const char* text) {
    char* result = (char*)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(result, BUFFER_SIZE, "\033[48;5;%dm%s\033[0m", color, text);
    return result;
}

// Function to determine the contrast color
int contrast_color(int color) {
    if (color < 16) return (color == 0) ? 15 : 0;
    if (color > 231) return (color < 244) ? 15 : 0;
    int g = ((color - 16) % 36) / 6;
    return (g > 2) ? 0 : 15;
}

// Function for foreground with contrast color
char* ctext_fg_con(int color, const char* text) {
    int contrast = contrast_color(color);
    char* result = (char*)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(result, BUFFER_SIZE, "\033[48;5;%dm\033[38;5;%dm%s\033[0m\033[0m", contrast, color, text);
    return result;
}

// Function for background with contrast color
char* ctext_bg_con(int color, const char* text) {
    int contrast = contrast_color(color);
    char* result = (char*)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(result, BUFFER_SIZE, "\033[48;5;%dm\033[38;5;%dm%s\033[0m\033[0m", color, contrast, text);
    return result;
}

// Convert int value to a color within the APPEAR_COLORS range
int int_to_color(int val, int min, int max) {
    if (val >= max) return APPEAR_COLORS[APPEAR_COLORS_SIZE - 1];
    if (val <= min) return APPEAR_COLORS[0];
    int index = (val - min) * (APPEAR_COLORS_SIZE - 1) / (max - min);
    return APPEAR_COLORS[index];
}

// Return a foreground colored string based on an integer value
char* int_to_ctext_fg(int val, int min, int max) {
    int color = int_to_color(val, min, max);
    char* result = (char*)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(result, BUFFER_SIZE, "\033[38;5;%dm%d\033[0m", color, val);
    return result;
}

// Convert temperature to a color
int temperature_to_color(double temp) {
    if (temp >= g_temp_max) {
        g_temp_max = temp;
        return TEMP_COLORS[TEMP_COLORS_SIZE - 1];
    }
    if (temp <= g_temp_min) {
        g_temp_min = temp;
        return TEMP_COLORS[0];
    }
    int index = (temp - g_temp_min) * (TEMP_COLORS_SIZE - 1) / (g_temp_max - g_temp_min);
    return TEMP_COLORS[index];
}

// Return a foreground colored string based on temperature
char* temp_to_ctext_fg(double temp) {
    int color = temperature_to_color(temp);
    char* result = (char*)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(result, BUFFER_SIZE, "\033[38;5;%dm%7.2f 󰔄\033[0m", color, temp);
    return result;
}

// Return a background colored string based on temperature
char* temp_to_ctext_bg(double temp) {
    int color = temperature_to_color(temp);
    char* result = (char*)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(result, BUFFER_SIZE, "\033[48;5;%dm%7.2f 󰔄\033[0m", color, temp);
    return result;
}

// Return a foreground colored string with contrast based on temperature
char* temp_to_ctext_fg_con(double temp) {
    int color = temperature_to_color(temp);
    char* result = (char*)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(result, BUFFER_SIZE, "\033[48;5;%dm\033[38;5;%dm%7.2f 󰔄\033[0m", contrast_color(color), color, temp);
    return result;
}

// Return a background colored string with contrast based on temperature
char* temp_to_ctext_bg_con(double temp) {
    int color = temperature_to_color(temp);
    char* result = (char*)malloc(BUFFER_SIZE * sizeof(char));
    snprintf(result, BUFFER_SIZE, "\033[48;5;%dm\033[38;5;%dm%7.2f 󰔄\033[0m", color, contrast_color(color), temp);
    return result;
}

// Return a colored string based on boolean value (foreground)
char* bool_to_ctext_fg(int b) {
    return ctext_fg(b ? COLOR_ON : COLOR_OFF, b ? "true" : "false");
}

// Return a colored string based on boolean value (background)
char* bool_to_ctext_bg(int b) {
    return ctext_bg(b ? COLOR_ON : COLOR_OFF, b ? "true" : "false");
}

// Return a colored string based on boolean value with contrast (foreground)
char* bool_to_ctext_fg_con(int b) {
    return ctext_fg_con(b ? COLOR_ON : COLOR_OFF, b ? "true" : "false");
}

// Return a colored string based on boolean value with contrast (background)
char* bool_to_ctext_bg_con(int b) {
    return ctext_bg_con(b ? COLOR_ON : COLOR_OFF, b ? "true" : "false");
}

// Return a colored string for boolean with both foreground and background
char* bool_to_ctext_bi(int b) {
    return b ? ctext_bg_con(COLOR_ON, " true ") : ctext_fg(COLOR_OFF, " false ");
}
