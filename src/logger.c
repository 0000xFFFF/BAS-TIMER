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
            char *sep = strstr(line, "--");
            if (sep) {
                sep += 2; // move past "--"
                while (*sep == ' ') sep++; // skip spaces

                int h, m, s;
                if (sscanf(sep, "%d:%d:%d", &h, &m, &s) == 3) {
                    total_seconds += h * 3600 + m * 60 + s;
                }
            }
        }
    }

    // Convert total_seconds back to hh:mm:ss
    int total_h = total_seconds / 3600;
    int total_m = (total_seconds % 3600) / 60;
    int total_s = total_seconds % 60;

    printf("Total time for 'StatusPumpe4 = 0': %02d:%02d:%02d\n", total_h, total_m, total_s);
}
