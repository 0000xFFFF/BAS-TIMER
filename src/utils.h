#ifndef UTILS_HPP
#define UTILS_HPP

#include <time.h>

extern void change_to_bin_dir();
extern long long timestamp();
extern size_t strftime_HM(char* buffer, size_t size, struct tm* timeinfo);
extern size_t strftime_HMS(char* buffer, size_t size, struct tm* timeinfo);
extern size_t strftime_YmdHMS(char* buffer, size_t size, struct tm* timeinfo);
extern size_t dt_HM(char* buffer, size_t size);
extern size_t dt_HMS(char* buffer, size_t size);
extern size_t dt_full(char* buffer, size_t size);
extern int localtime_hour();
extern size_t elapsed_str(char* buffer, size_t size, time_t end, time_t start);
extern size_t get_local_ips(char* buffer, size_t size);
extern void escape_quotes(const char* input, char* output);

#endif // UTILS_HPP
