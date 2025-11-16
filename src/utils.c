#include "utils.h"
#include <ctype.h>
#include <float.h>
#include <libgen.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

long long timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL); // Get the current time

    // Ensure 64-bit arithmetic by explicitly casting tv.tv_sec to long long
    long long timestamp_ms = ((long long)tv.tv_sec) * 1000LL + (tv.tv_usec / 1000);

    return timestamp_ms;
}

size_t strftime_HM(char* buffer, size_t size, struct tm* timeinfo)
{
    return strftime(buffer, size, "%H:%M", timeinfo);
}
size_t strftime_HMS(char* buffer, size_t size, struct tm* timeinfo)
{
    return strftime(buffer, size, "%H:%M:%S", timeinfo);
}
size_t strftime_YmdHMS(char* buffer, size_t size, struct tm* timeinfo)
{
    return strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

size_t dt_HM(char* buffer, size_t size)
{
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return strftime_HM(buffer, size, timeinfo);
}

size_t dt_HMS(char* buffer, size_t size)
{
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return strftime_HMS(buffer, size, timeinfo);
}

size_t dt_full(char* buffer, size_t size)
{
    time_t rawtime;
    struct tm* timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    return strftime_YmdHMS(buffer, size, timeinfo);
}

int localtime_hour(void)
{
    time_t t;
    struct tm* tm_info;
    time(&t);                // Get current time
    tm_info = localtime(&t); // Convert to local time
    return tm_info->tm_hour; // Extract the hour in 24-hour format
}

size_t elapsed_str(char* buffer, size_t size, time_t end, time_t start)
{
    time_t elapsed = end - start;
    struct tm* tm_info = gmtime(&elapsed);
    return strftime(buffer, size, "%H:%M:%S", tm_info);
}

static size_t get_local_ip_exec(char* buffer, size_t size, char* command)
{
    FILE* fp = popen(command, "r");
    if (fp == NULL) { return (size_t)snprintf(buffer, size, "Can't get ip."); }
    int found = 0;
    while (fgets(buffer, (int)size, fp) != NULL) { found = 1; }
    pclose(fp);
    if (!found) { return (size_t)snprintf(buffer, size, "No IP found."); }
    return 0;
}

size_t get_local_ip(char* buffer, size_t size)
{
    char* command = "ip -o -4 addr show | awk '{print $4}' | cut -d/ -f1 | grep -v '127.0.0.1' | head -n 1 | tr -d '\n'";
    return get_local_ip_exec(buffer, size, command);
}

size_t get_local_ips(char* buffer, size_t size)
{
    char* command = "ip -o -4 addr show | awk '{print $4}' | cut -d/ -f1 | grep -v '127.0.0.1' | tr '\n' ' '";
    return get_local_ip_exec(buffer, size, command);
}

double min_dv(int count, ...)
{
    va_list args;
    va_start(args, count);
    double m = va_arg(args, double);
    for (int i = 1; i < count; i++) {
        double val = va_arg(args, double);
        if (val < m) m = val;
    }
    va_end(args);
    return m;
}

double max_dv(int count, ...)
{
    va_list args;
    va_start(args, count);
    double m = va_arg(args, double);
    for (int i = 1; i < count; i++) {
        double val = va_arg(args, double);
        if (val > m) m = val;
    }
    va_end(args);
    return m;
}

const char* istrstr(const char* haystack, const char* needle)
{
    if (!*needle) return (const char*)haystack;

    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;

        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++;
            n++;
        }

        if (!*n)
            return (const char*)haystack;
    }
    return NULL;
}

int hms_to_seconds(const char* str)
{
    int h, m, s;
    sscanf(str, "%d:%d:%d", &h, &m, &s);
    return h * 3600 + m * 60 + s;
}

int now_seconds(void)
{
    time_t t = time(NULL);
    struct tm* tm = localtime(&t);
    return tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
}

#include <stdbool.h>
#include <stdio.h>

size_t total_seconds_to_string(char* buffer, size_t buffer_size, long total_seconds, bool append_total_seconds)
{
    if (!buffer || buffer_size == 0) {
        return 0;
    }

    buffer[0] = '\0';

    long total_minutes = total_seconds / 60;
    long total_hours = total_minutes / 60;
    long total_days = total_hours / 24;
    long total_years = total_days / 365;

    long seconds = total_seconds % 60;
    long minutes = total_minutes % 60;
    long hours = total_hours % 24;
    long days = total_days % 30;
    long months = (total_days / 30) % 12;

    size_t written = 0; // characters actually placed in buffer
    size_t wanted = 0;  // characters that *would* have been written

#define SAFE_APPEND(fmt, value)                                                \
    do {                                                                       \
        int n = snprintf(buffer + written, buffer_size - written, fmt, value); \
        if (n < 0) return wanted; /* encoding error */                         \
        wanted += (size_t)n;                                                   \
        if (written < buffer_size) {                                           \
            if ((size_t)n >= buffer_size - written) {                          \
                written = buffer_size - 1; /* truncated */                     \
            }                                                                  \
            else {                                                             \
                written += (size_t)n;                                          \
            }                                                                  \
        }                                                                      \
    } while (0)

    if (total_years) SAFE_APPEND("%ldy ", total_years);
    if (months) SAFE_APPEND("%ldM ", months);
    if (days) SAFE_APPEND("%ldd ", days);
    if (hours) SAFE_APPEND("%ldh ", hours);
    if (minutes) SAFE_APPEND("%ldm ", minutes);
    if (seconds) SAFE_APPEND("%lds ", seconds);

    if (append_total_seconds) {
        SAFE_APPEND("(%lds)", total_seconds);
    }
    else {
        // remove trailing space if present
        if (written > 0 && buffer[written - 1] == ' ') {
            buffer[written - 1] = '\0';
            written--;
            wanted--;
        }
    }

    buffer[written] = '\0';
    return wanted; // total characters that would have been produced
}

void trim_spaces(char* buffer)
{
    char* src = buffer;
    char* dst = buffer;

    while (*src) {
        if (*src != ' ' &&  // space
            *src != '\t' && // tab
            *src != '\n' && // newline
            *src != '\r' && // carriage return
            *src != '\v' && // vertical tab
            *src != '\f') { // form feed
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}
