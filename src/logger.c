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
