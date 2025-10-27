#include "utils.h"
#include "globals.h"
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

void change_to_bin_dir()
{
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);

    if (len != -1) {
        path[len] = '\0';          // Null-terminate the string
        char* dir = dirname(path); // Extract directory part

        if (chdir(dir) != 0) { perror("chdir failed"); }
    }
    else {
        perror("readlink failed");
    }
}

long long timestamp()
{
    struct timeval tv;
    gettimeofday(&tv, NULL); // Get the current time

    // Ensure 64-bit arithmetic by explicitly casting tv.tv_sec to long long
    long long timestamp_ms = ((long long)tv.tv_sec) * 1000LL + (tv.tv_usec / 1000);

    return timestamp_ms;
}

#define TIME_BUFFER_SIZE 32

size_t strftime_HM(char* buffer, size_t size, struct tm* timeinfo) { return strftime(buffer, size, "%H:%M", timeinfo); }
size_t strftime_HMS(char* buffer, size_t size, struct tm* timeinfo) { return strftime(buffer, size, "%H:%M:%S", timeinfo); }
size_t strftime_YmdHMS(char* buffer, size_t size, struct tm* timeinfo) { return strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo); }

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

int localtime_hour()
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

size_t get_local_ips(char* buffer, size_t size)
{
    char* command =
#if PRINT_ONLY_ONE_IP
        "ip -o -4 addr show | awk '{print $4}' | cut -d/ -f1 | grep -v '127.0.0.1' | head -n 1 | tr -d '\n'";
#else
        "ip -o -4 addr show | awk '{print $4}' | cut -d/ -f1 | grep -v '127.0.0.1' | tr '\n' ' '";
#endif

    FILE* fp = popen(command, "r");
    if (fp == NULL) { return snprintf(buffer, size, "Can't get ip."); }
    int found = 0;
    while (fgets(buffer, size, fp) != NULL) { found = 1; }
    pclose(fp);
    if (!found) { return snprintf(buffer, size, "No IP found."); }
    return 0;
}

void escape_quotes(const char* input, char* output)
{
    int i = 0, j = 0;

    // Iterate through each character in the input string
    while (input[i] != '\0') {
        switch (input[i]) {
            case '"':
                output[j++] = '\\';
                output[j++] = '"'; // Escape double quote
                break;
            case '\\':
                output[j++] = '\\';
                output[j++] = '\\'; // Escape backslash
                break;
            case '\b':
                output[j++] = '\\';
                output[j++] = 'b'; // Escape backspace
                break;
            case '\f':
                output[j++] = '\\';
                output[j++] = 'f'; // Escape form feed
                break;
            case '\n':
                output[j++] = '\\';
                output[j++] = 'n'; // Escape newline
                break;
            case '\r':
                output[j++] = '\\';
                output[j++] = 'r'; // Escape carriage return
                break;
            case '\t':
                output[j++] = '\\';
                output[j++] = 't'; // Escape tab
                break;
            default:
                output[j++] = input[i]; // Copy other characters as is
                break;
        }
        i++;
    }

    // Null-terminate the output string
    output[j] = '\0';
}
