#include "logger.h"
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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

    char* t = get_current_time();
    fprintf(f, "%s - ", t);
    free(t);

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

void logger_sumtime(const char* filename, const char* pattern)
{

    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        perror("Failed to open log file");
        return;
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
    long total_months = total_days / 30; // approximate
    long total_years = total_days / 365; // approximate

    int seconds = total_seconds % 60;
    int minutes = total_minutes % 60;
    int hours = total_hours % 24;
    int days = total_days % 30; // remainder days after months
    int months = (total_days / 30) % 12;

    printf("Total time for '%s':\n", pattern);
    printf("  %02d years, %02d months, %02d days, %02d:%02d:%02d\n",
           (int)total_years, months, days, hours, minutes, seconds);
}
