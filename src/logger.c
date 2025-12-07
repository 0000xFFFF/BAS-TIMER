#include "logger.h"
#include "globals.h"
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static pthread_mutex_t s_mutex_file_errors = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_mutex_file_requests = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_mutex_file_changes = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t s_mutex_file_wttrin = PTHREAD_MUTEX_INITIALIZER;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"

static void logger_write(const char* filename, const char* fmt, va_list args)
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

#pragma GCC diagnostic pop

void logger_errors_write(const char* fmt, ...)
{

    pthread_mutex_lock(&s_mutex_file_errors);
    va_list args;
    va_start(args, fmt);
    logger_write(VAR_DIR_FILE_ERRORS_LOG, fmt, args);
    va_end(args);
    pthread_mutex_unlock(&s_mutex_file_errors);
}

void logger_requests_write(const char* fmt, ...)
{
    pthread_mutex_lock(&s_mutex_file_requests);
    va_list args;
    va_start(args, fmt);
    logger_write(VAR_DIR_FILE_REQUESTS_LOG, fmt, args);
    va_end(args);
    pthread_mutex_unlock(&s_mutex_file_requests);
}

void logger_changes_write(const char* fmt, ...)
{
    pthread_mutex_lock(&s_mutex_file_changes);
    va_list args;
    va_start(args, fmt);
    logger_write(VAR_DIR_FILE_CHANGES_LOG, fmt, args);
    va_end(args);
    pthread_mutex_unlock(&s_mutex_file_changes);
}

void logger_wttrin_write(const char* fmt, ...)
{
    pthread_mutex_lock(&s_mutex_file_wttrin);
    va_list args;
    va_start(args, fmt);
    logger_write(VAR_DIR_FILE_WTTRIN_LOG, fmt, args);
    va_end(args);
    pthread_mutex_unlock(&s_mutex_file_wttrin);
}

size_t logger_changes_sumtime(char* buffer, size_t buffer_size, const char* pattern)
{
    pthread_mutex_lock(&s_mutex_file_changes);
    FILE* f = fopen(VAR_DIR_FILE_CHANGES_LOG, "r");
    if (f == NULL) {
        perror("Failed to open log file");
        pthread_mutex_unlock(&s_mutex_file_changes);
        return 0;
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

    size_t c = total_seconds_to_string(buffer, buffer_size, total_seconds, true);

    pthread_mutex_unlock(&s_mutex_file_changes);
    return c;
}
