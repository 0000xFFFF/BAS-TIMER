#include "logger.h"
#include "globals.h"
#include "utils.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
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

void logger_write_errors(const char* fmt, ...)
{

    pthread_mutex_lock(&s_mutex_file_errors);
    va_list args;
    va_start(args, fmt);
    logger_write(VAR_DIR_FILE_ERRORS_LOG, fmt, args);
    va_end(args);
    pthread_mutex_unlock(&s_mutex_file_errors);
}

void logger_write_requests(const char* fmt, ...)
{
    pthread_mutex_lock(&s_mutex_file_requests);
    va_list args;
    va_start(args, fmt);
    logger_write(VAR_DIR_FILE_REQUESTS_LOG, fmt, args);
    va_end(args);
    pthread_mutex_unlock(&s_mutex_file_requests);
}

void logger_write_changes(const char* fmt, ...)
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
    uint64_t total_seconds = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, pattern)) {
            // Find the "--" part
            char* sep = strstr(line, "--");
            if (sep) {
                sep += 2;                  // move past "--"
                while (*sep == ' ') sep++; // skip spaces

                int h, m, s;
                if (sscanf(sep, "%d:%d:%d", &h, &m, &s) == 3) {
                    total_seconds += (uint64_t)hms_to_today_seconds(h, m, s);
                }
            }
        }
    }

    fclose(f);
    size_t c = total_seconds_to_string(buffer, buffer_size, total_seconds, true);
    pthread_mutex_unlock(&s_mutex_file_changes);
    return c;
}

// NOTE: must call free
char* logger_get_mod_rada_intervals_today_json(void)
{
    time_t now = time(NULL);

    struct tm tm_now;
    localtime_r(&now, &tm_now);

    int today_y = tm_now.tm_year + 1900;
    int today_m = tm_now.tm_mon + 1;
    int today_d = tm_now.tm_mday;

    struct tm tm_yesterday = tm_now;
    tm_yesterday.tm_mday -= 1;
    mktime(&tm_yesterday);

    int yest_y = tm_yesterday.tm_year + 1900;
    int yest_m = tm_yesterday.tm_mon + 1;
    int yest_d = tm_yesterday.tm_mday;

    pthread_mutex_lock(&s_mutex_file_changes);

    FILE* f = fopen(VAR_DIR_FILE_CHANGES_LOG, "r");
    if (!f) {
        pthread_mutex_unlock(&s_mutex_file_changes);
        return NULL;
    }

    size_t cap = 512;
    size_t len = 0;
    char* out = malloc(cap);
    if (!out) {
        fclose(f);
        pthread_mutex_unlock(&s_mutex_file_changes);
        return NULL;
    }

    out[0] = '[';
    out[1] = '\0';
    len = 1;

    char line[4096];

    int heating_on = 0;
    int today_initialized = 0;
    int yesterday_last_state = -1;
    uint32_t start_sec = 0;
    int first = 1;

    while (fgets(line, sizeof(line), f)) {
        if (!strstr(line, "mod_rada"))
            continue;

        int year, mon, day, h, m, s, val;
        if (sscanf(line,
                   "%d-%d-%d %d:%d:%d - mod_rada = %d",
                   &year, &mon, &day, &h, &m, &s, &val) != 7)
            continue;

        /* Track yesterday's final state */
        if (year == yest_y && mon == yest_m && day == yest_d) {
            yesterday_last_state = val;
            continue;
        }

        /* Process today only */
        if (year != today_y || mon != today_m || day != today_d)
            continue;

        uint32_t sec = (uint32_t)hms_to_today_seconds(h, m, s);

        /* First entry of today: inherit midnight state */
        if (!today_initialized) {
            today_initialized = 1;
            if (yesterday_last_state == 1) {
                heating_on = 1;
                start_sec = 0; // midnight
            }
        }

        if (val == 1 && !heating_on) {
            heating_on = 1;
            start_sec = sec;
        }
        else if (val == 0 && heating_on) {
            heating_on = 0;

            char entry[128];
            size_t n = (size_t)snprintf(entry, sizeof(entry),
                                "%s{ \"start\": %u, \"end\": %u }",
                                first ? "" : ",",
                                start_sec, sec);

            if (len + n + 2 >= cap) {
                cap *= 2;
                char* tmp = realloc(out, cap);
                if (!tmp) {
                    free(out);
                    fclose(f);
                    pthread_mutex_unlock(&s_mutex_file_changes);
                    return NULL;
                }
                out = tmp;
            }

            memcpy(out + len, entry, n);
            len += n;
            out[len] = '\0';
            first = 0;
        }
    }

    /* Still ON at end of day */
    if (heating_on) {
        char entry[128];
        size_t n = (size_t)snprintf(entry, sizeof(entry),
                            "%s{ \"start\": %u, \"end\": %u }",
                            first ? "" : ",",
                            start_sec, 86400);

        if (len + n + 2 >= cap) {
            cap *= 2;
            char* tmp = realloc(out, cap);
            if (!tmp) {
                free(out);
                fclose(f);
                pthread_mutex_unlock(&s_mutex_file_changes);
                return NULL;
            }
            out = tmp;
        }

        memcpy(out + len, entry, n);
        len += n;
        out[len] = '\0';
    }

    out[len++] = ']';
    out[len] = '\0';

    fclose(f);
    pthread_mutex_unlock(&s_mutex_file_changes);
    return out; // CALLER FREES
}
