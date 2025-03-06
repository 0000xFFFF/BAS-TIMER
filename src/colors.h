#ifndef COLORS_H
#define COLORS_H

#define COLOR_ON 46
#define COLOR_OFF 255

#define TEMP_COLORS_SIZE 18
extern int TEMP_COLORS[TEMP_COLORS_SIZE];

#define CTEXT_FG(color, text) "\033[38;5;" #color "m" text "\033[0m"

char* ctext_fg(int color, const char* text);
char* cnum_fg(int color, const int number);
char* ctext_bg(int color, const char* text);
int contrast_color(int color);
char* ctext_fg_con(int color, const char* text);
char* ctext_bg_con(int color, const char* text);
int temperature_to_color(double temp, double* temp_min, double* temp_max);
char* temp_to_ctext_fg(double temp, double* temp_min, double* temp_max);
char* temp_to_ctext_bg(double temp, double* temp_min, double* temp_max);
char* temp_to_ctext_fg_con(double temp, double* temp_min, double* temp_max);
char* temp_to_ctext_bg_con(double temp, double* temp_min, double* temp_max);

#endif // COLORS_H
