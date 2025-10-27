#include "logger.h"
#include "globals.h"
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define LOG_CHANGES  "changes.log"
#define LOG_REQUESTS "requests.log"
#define LOG_ERRORS   "errors.log"

// General logging function
void logger_write(const char* filename, const char* fmt, va_list args)
{
    if (fmt == NULL || filename == NULL) { return; }

    FILE* f = fopen(filename, "a");
    if (f == NULL) {
        perror("Failed to open log file");
        return;
    }

    char time[MIDBUFF] = {0};
    dt_full(time, MIDBUFF);
    fprintf(f, "%s - ", time);

    vfprintf(f, fmt, args);
    fclose(f);
}

void logger_errors_write(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logger_write(LOG_ERRORS, fmt, args);
    va_end(args);
}

void logger_requests_write(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logger_write(LOG_REQUESTS, fmt, args);
    va_end(args);
}

void logger_changes_write(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    logger_write(LOG_CHANGES, fmt, args);
    va_end(args);
}

int logger_sumtime(char* buffer, int buffer_size, const char* filename, const char* pattern)
{
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        perror("Failed to open log file");
        return -1;
    }

    char line[1024 * 4] = {0};
    long total_seconds = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, pattern)) {
            // Find the "--" part
            char* sep = strstr(line, "--");
            if (sep) {
                sep += 2;                  // move past "--"
                while (*sep == ' ') sep++; // skip spaces

                int h, m, s;
                if (sscanf(sep, "%d:%d:%d", &h, &m, &s) == 3) {
                    total_seconds += h * 3600 + m * 60 + s;
                }
            }
        }
    }

    // Convert total_seconds into years, months, days, hours, minutes, seconds
    long total_minutes = total_seconds / 60;
    long total_hours = total_minutes / 60;
    long total_days = total_hours / 24;
    long total_years = total_days / 365; // approximate

    int seconds = total_seconds % 60;
    int minutes = total_minutes % 60;
    int hours = total_hours % 24;
    int days = total_days % 30; // remainder days after months
    int months = (total_days / 30) % 12;

    int c = 0;

    if (total_years != 0) {
        c += snprintf(buffer+c, buffer_size, "%dy", (int)total_years);
        c += snprintf(buffer+c, buffer_size, "%s", " ");
    }

    if (months != 0) {
        c += snprintf(buffer+c, buffer_size, "%dM", months);
        c += snprintf(buffer+c, buffer_size, "%s", " ");
    }

    if (days != 0) {
        c += snprintf(buffer+c, buffer_size, "%dd", days);
        c += snprintf(buffer+c, buffer_size, "%s", " ");
    }

    if (hours != 0) {
        c += snprintf(buffer+c, buffer_size, "%dh", hours);
        c += snprintf(buffer+c, buffer_size, "%s", " ");
    }

    if (minutes != 0) {
        c += snprintf(buffer+c, buffer_size, "%dm", minutes);
        c += snprintf(buffer+c, buffer_size, "%s", " ");
    }

    if (seconds != 0) {
        c += snprintf(buffer+c, buffer_size, "%ds", seconds);
    }

    c += snprintf(buffer+c, buffer_size, " (%lu sec)", total_seconds);

    return c;
}
