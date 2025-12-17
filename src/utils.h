#ifndef UTILS_HPP
#define UTILS_HPP

#include <inttypes.h>
#include <stdbool.h>
#include <time.h>

extern long long timestamp(void);
extern size_t strftime_HM(char* buffer, size_t size, struct tm* timeinfo);
extern size_t strftime_HMS(char* buffer, size_t size, struct tm* timeinfo);
extern size_t strftime_YmdHMS(char* buffer, size_t size, struct tm* timeinfo);
extern size_t dt_HM(char* buffer, size_t size);
extern size_t dt_HMS(char* buffer, size_t size);
extern size_t dt_full(char* buffer, size_t size);
extern int localtime_hour(void);
extern size_t elapsed_str(char* buffer, size_t size, time_t end, time_t start);
extern size_t get_local_ip(char* buffer, size_t size);
extern size_t get_local_ips(char* buffer, size_t size);
extern bool is_connection_healthy();
extern bool can_get_local_ips();
extern double min_dv(int count, ...);
extern double max_dv(int count, ...);
extern const char* istrstr(const char* haystack, const char* needle);
extern int hms_to_today_seconds(int hour, int min, int sec);
extern int hms_to_today_seconds_str(const char* str);
extern int now_to_today_seconds(void);
extern bool today_seconds_in_window(int sec_today, int from, int to);
extern size_t total_seconds_to_string(char* buffer, size_t buffer_size, uint64_t total_seconds, bool append_total_seconds);
extern void human_readable_time(char* buffer, size_t buffer_size, uint64_t seconds);
extern void trim_spaces(char* buffer);
extern void trim_left(char* buffer, size_t n);
extern void trim_right(char* buffer, size_t n);
extern char* bool_to_str(bool value);

#endif // UTILS_HPP
