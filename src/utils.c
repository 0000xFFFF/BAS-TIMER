#include "utils.h"
#include "globals.h"
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

void mkdir_safe(const char* dir)
{
    struct stat st = {0};
    if (stat(dir, &st) == -1) { mkdir(dir, 0700); }
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

size_t get_local_ip_exec(char* buffer, size_t size, char* command)
{
    FILE* fp = popen(command, "r");
    if (fp == NULL) { return snprintf(buffer, size, "Can't get ip."); }
    int found = 0;
    while (fgets(buffer, size, fp) != NULL) { found = 1; }
    pclose(fp);
    if (!found) { return snprintf(buffer, size, "No IP found."); }
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

typedef struct {
    char color[8];
    char bg_color[8];
    bool bold;
    bool has_style;
} StyleState;

// Convert 256-color index to RGB hex string
void ansi_256_to_rgb(int color_index, char* result)
{
    if (color_index < 16) {
        // Basic 16 colors
        const char* basic_colors[] = {
            "#000000", "#800000", "#008000", "#808000",
            "#000080", "#800080", "#008080", "#c0c0c0",
            "#808080", "#ff0000", "#00ff00", "#ffff00",
            "#0000ff", "#ff00ff", "#00ffff", "#ffffff"};
        strcpy(result, basic_colors[color_index]);
    }
    else if (16 <= color_index && color_index <= 231) {
        // 6x6x6 color cube
        color_index -= 16;
        int r = (color_index / 36) * 51;
        int g = ((color_index % 36) / 6) * 51;
        int b = (color_index % 6) * 51;
        sprintf(result, "#%02x%02x%02x", r, g, b);
    }
    else if (232 <= color_index && color_index <= 255) {
        // Grayscale
        int shade = (color_index - 232) * 10 + 8;
        sprintf(result, "#%02x%02x%02x", shade, shade, shade);
    }
    else {
        // Default
        strcpy(result, "#ffffff");
    }
}

// Parse ANSI escape sequence and update style state
void parse_ansi_sequence(const char* seq, StyleState* state)
{
    state->has_style = false;

    char* sequence = strdup(seq);
    char* token = strtok(sequence, ";");

    while (token != NULL) {
        int code = atoi(token);

        if (code == 0) {
            // Reset
            state->color[0] = '\0';
            state->bg_color[0] = '\0';
            state->bold = false;
            state->has_style = false;
        }
        else if (code == 1) {
            // Bold
            state->bold = true;
            state->has_style = true;
        }
        else if (code == 38) {
            // Foreground color
            token = strtok(NULL, ";");
            if (token != NULL && atoi(token) == 5) {
                token = strtok(NULL, ";");
                if (token != NULL) {
                    ansi_256_to_rgb(atoi(token), state->color);
                    state->has_style = true;
                }
            }
        }
        else if (code == 48) {
            // Background color
            token = strtok(NULL, ";");
            if (token != NULL && atoi(token) == 5) {
                token = strtok(NULL, ";");
                if (token != NULL) {
                    ansi_256_to_rgb(atoi(token), state->bg_color);
                    state->has_style = true;
                }
            }
        }

        token = strtok(NULL, ";");
    }

    free(sequence);
}

void generate_style_string(const StyleState* state, char* style_str, size_t max_len)
{
    style_str[0] = '\0';

    if (state->color[0] != '\0') {
        char color_style[32];
        sprintf(color_style, "color: %s; ", state->color);
        strncat(style_str, color_style, max_len - strlen(style_str) - 1);
    }

    if (state->bg_color[0] != '\0') {
        char bg_style[32];
        sprintf(bg_style, "background-color: %s; ", state->bg_color);
        strncat(style_str, bg_style, max_len - strlen(style_str) - 1);
    }

    if (state->bold) {
        strncat(style_str, "font-weight: bold; ", max_len - strlen(style_str) - 1);
    }
}

size_t ansi_to_html(const char* text, char* result)
{
    if (text == NULL) return 0;
    if (result == NULL) return 0;

    size_t text_len = strlen(text);

    result[0] = '\0';
    size_t result_pos = 0;

    // Track whether we're in a span
    bool in_span = false;
    StyleState current_style = {0};

    // Initial span
    strcpy(result, "<span>");
    result_pos = 6;
    in_span = true;

    size_t i = 0;
    while (i < text_len) {
        // Handle ANSI escape sequence
        if (text[i] == '\033' && text[i + 1] == '[') {
            i += 2; // Skip the '\033['

            // Find the end of the escape sequence (marked by 'm')
            const char* seq_start = text + i;
            char* seq_end = strchr(seq_start, 'm');

            if (seq_end != NULL) {
                size_t seq_len = seq_end - seq_start;
                char* seq = (char*)malloc(seq_len + 1);

                if (seq != NULL) {
                    strncpy(seq, seq_start, seq_len);
                    seq[seq_len] = '\0';

                    // Check for reset sequence
                    if (strcmp(seq, "0") == 0) {
                        // Close current span and reset styles
                        if (in_span) {
                            strcpy(result + result_pos, "</span>");
                            result_pos += 7;
                            in_span = false;
                        }

                        // Start a new span
                        strcpy(result + result_pos, "<span>");
                        result_pos += 6;
                        in_span = true;

                        // Reset style state
                        memset(&current_style, 0, sizeof(StyleState));
                    }
                    else {
                        // Parse the sequence and update style
                        if (in_span) {
                            strcpy(result + result_pos, "</span>");
                            result_pos += 7;
                            in_span = false;
                        }

                        parse_ansi_sequence(seq, &current_style);

                        // Only add a span if we have style to apply
                        if (current_style.has_style) {
                            char style_str[128] = {0};
                            generate_style_string(&current_style, style_str, sizeof(style_str));

                            result_pos += sprintf(result + result_pos, "<span style=\"%s\">", style_str);
                        }
                        else {
                            strcpy(result + result_pos, "<span>");
                            result_pos += 6;
                        }

                        in_span = true;
                    }

                    free(seq);
                }

                i += seq_len + 1; // Skip past the sequence and 'm'
            }
            else {
                // Malformed escape sequence, treat as plain text
                result[result_pos++] = text[i - 2];
                result[result_pos++] = text[i - 1];
                i++;
            }
        }
        else if (text[i] == ' ') {
            // Convert spaces to &nbsp;
            strcpy(result + result_pos, "&nbsp;");
            result_pos += 6;
            i++;
        }
        else if (text[i] == '\n') {
            // Handle newlines
            if (in_span) {
                strcpy(result + result_pos, "</span><br><span>");
                result_pos += 17;
            }
            else {
                strcpy(result + result_pos, "<br><span>");
                result_pos += 10;
                in_span = true;
            }
            i++;
        }
        else {
            // Regular character
            result[result_pos++] = text[i++];
        }
    }

    // Close the final span if needed
    if (in_span) {
        strcpy(result + result_pos, "</span>");
        result_pos += 7;
    }

    // Null-terminate the result
    result[result_pos] = '\0';

    return 1;
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

/*
** - handles quoted values like "hello world"
** - handles unquoted values like hello
** - trims whitespace
** - strips newline
** - works with spaces inside quotes
** - does not modify the original line buffer in unsafe ways
*/
int load_env(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (!file) return 0;

    char line[BIGBUFF];

    while (fgets(line, sizeof(line), file)) {
        // Trim leading spaces
        char* p = line;
        while (*p == ' ' || *p == '\t') p++;

        // Skip blank lines or comments
        if (*p == '#' || *p == '\n' || *p == '\0') continue;

        // Find '='
        char* eq = strchr(p, '=');
        if (!eq) continue;

        *eq = '\0';
        char* key = p;
        char* value = eq + 1;

        // Trim trailing whitespace from key
        for (char* end = key + strlen(key) - 1;
             end > key && (*end == ' ' || *end == '\t');
             end--) {
            *end = '\0';
        }

        // Trim newline and trailing spaces on value
        size_t len = strlen(value);
        while (len > 0 && (value[len - 1] == '\n' || value[len - 1] == ' ' || value[len - 1] == '\t'))
            value[--len] = '\0';

        // Strip surrounding quotes if present
        if (value[0] == '"' && len >= 2 && value[len - 1] == '"') {
            value[len - 1] = '\0'; // remove ending quote
            value++;               // skip starting quote
        }

        setenv(key, value, 1);
    }

    fclose(file);
    return 1;
}
