#ifndef COLORS_H
#define COLORS_H

#include <stddef.h>
#define COLOR_ON_AUTO 48
#define COLOR_ON_MANUAL 46

#define COLOR_OFF_AUTO 255
#define COLOR_OFF_MANUAL 203

#define COLOR_ON COLOR_ON_AUTO
#define COLOR_OFF COLOR_OFF_AUTO

#define TEMP_COLORS_SIZE 18
extern int TEMP_COLORS[TEMP_COLORS_SIZE];

#define CTEXT_FG(color, text) "\033[38;5;" #color "m" text "\033[0m"

extern size_t ctext_fg(char* buffer, size_t size, int color, const char* text);
extern size_t cnum_fg(char* buffer, size_t size, int color, const int number);
extern size_t ctext_bg(char* buffer, size_t size, int color, const char* text);
extern int contrast_color(int color);
extern size_t ctext_fg_con(char* buffer, size_t size, int color, const char* text);
extern size_t ctext_bg_con(char* buffer, size_t size, int color, const char* text);
extern int temperature_to_color(double temp, double temp_min, double temp_max);
extern size_t temp_to_ctext_fg(char* buffer, size_t size, double temp, double temp_min, double temp_max);
extern size_t temp_to_ctext_bg(char* buffer, size_t size, double temp, double temp_min, double temp_max);
extern size_t temp_to_ctext_fg_con(char* buffer, size_t size, double temp, double temp_min, double temp_max);
extern size_t temp_to_ctext_bg_con(char* buffer, size_t size, double temp, double temp_min, double temp_max);

#endif // COLORS_H
