#ifndef COLORS_H
#define COLORS_H

#define TEMP_MIN 45
#define TEMP_MAX 60
#define COLOR_ON 46
#define COLOR_OFF 255

#define TEMP_COLORS_SIZE 18
extern int TEMP_COLORS[TEMP_COLORS_SIZE];

#define APPEAR_COLORS_SIZE 16
extern int APPEAR_COLORS[APPEAR_COLORS_SIZE];

#define CTEXT_FG(color, text) "\033[38;5;" #color "m" text "\033[0m"

char* ctext_fg(int color, const char* text);      // Function to convert int to string and format with foreground color
char* cnum_fg(int color, const int number);
char* ctext_bg(int color, const char* text);      // Function to convert int to string and format with background color
int contrast_color(int color);                    // Function to determine the contrast color
char* ctext_fg_con(int color, const char* text);  // Function for foreground with contrast color
char* ctext_bg_con(int color, const char* text);  // Function for background with contrast color
int int_to_color(int val, int min, int max);      // Convert int value to a color within the APPEAR_COLORS range
char* int_to_ctext_fg(int val, int min, int max); // Return a foreground colored string based on an integer value
int temperature_to_color(double temp);            // Convert temperature to a color
char* temp_to_ctext_fg(double temp);              // Return a foreground colored string based on temperature
char* temp_to_ctext_bg(double temp);              // Return a background colored string based on temperature
char* temp_to_ctext_fg_con(double temp);          // Return a foreground colored string with contrast based on temperature
char* temp_to_ctext_bg_con(double temp);          // Return a background colored string with contrast based on temperature
char* bool_to_ctext_fg(int b);                    // Return a colored string based on boolean value (foreground)
char* bool_to_ctext_bg(int b);                    // Return a colored string based on boolean value (background)
char* bool_to_ctext_fg_con(int b);                // Return a colored string based on boolean value with contrast (foreground)
char* bool_to_ctext_bg_con(int b);                // Return a colored string based on boolean value with contrast (background)
char* bool_to_ctext_bi(int b);                    // Return a colored string for boolean with both foreground and background

#endif // COLORS_H
