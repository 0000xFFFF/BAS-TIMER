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

void change_to_bin_dir() {
    char path[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);

    if (len != -1) {
        path[len] = '\0';          // Null-terminate the string
        char* dir = dirname(path); // Extract directory part

        if (chdir(dir) != 0) { perror("chdir failed"); }

    } else {
        perror("readlink failed");
    }
}

long long timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL); // Get the current time

    // Convert the time to milliseconds
    long long timestamp_ms = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

    return timestamp_ms;
}

#define TIME_BUFFER_SIZE 32
char* get_current_time() {
    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char buffer[TIME_BUFFER_SIZE];
    strftime(buffer, TIME_BUFFER_SIZE, "%Y-%m-%d %H:%M:%S", timeinfo);
    return strdup(buffer);
}

int get_current_hour() {
    time_t t;
    struct tm* tm_info;
    time(&t);                // Get current time
    tm_info = localtime(&t); // Convert to local time
    return tm_info->tm_hour; // Extract the hour in 24-hour format
}

char* elapsed_str(time_t f, time_t s) {
    time_t elapsed = f - s;
    struct tm* tm_info = gmtime(&elapsed);

    // Allocate memory for the formatted string
    char* buffer = (char*)calloc(9, sizeof(char)); // "HH:MM:SS" + null terminator

    if (buffer != NULL) {
        // Format the elapsed time as HH:MM:SS
        strftime(buffer, 9, "%H:%M:%S", tm_info);
    }

    return buffer;
}

#define BUFFER_SIZE 1024
char* get_local_ips() {
    char* buffer = malloc(BUFFER_SIZE);

    char* command =
#if PRINT_ONLY_ONE_IP
        "ip -o -4 addr show | awk '{print $4}' | cut -d/ -f1 | grep -v '127.0.0.1' | head -n 1 | tr -d '\n'";
#else
        "ip -o -4 addr show | awk '{print $4}' | cut -d/ -f1 | grep -v '127.0.0.1' | tr '\n' ' '";
#endif

    FILE* fp = popen(command, "r");

    if (fp == NULL) {
        snprintf(buffer, BUFFER_SIZE, "Can't get ip.");
        return buffer;
    }

    int found = 0;
    while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
        found = 1;
    }

    pclose(fp);

    if (!found) {
        snprintf(buffer, BUFFER_SIZE, "No IP found.");
    }

    return buffer;
}

void escape_quotes(const char* input, char* output) {
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
