#ifndef UTILS_HPP
#define UTILS_HPP

#include <time.h>

void change_to_bin_dir();
long long timestamp();
char* get_current_time();
int get_current_hour();
char* elapsed_str(time_t f, time_t s);
char* get_local_ips();
void escape_quotes(const char *input, char *output);

#endif // UTILS_HPP

