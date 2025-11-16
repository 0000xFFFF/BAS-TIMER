#define _XOPEN_SOURCE 700 // needed for wcwidth
#include "term.h"
#include <float.h>
#include <libgen.h>
#include <limits.h>
#include <regex.h>
#include <stdarg.h>
#include <stdatomic.h>
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
#include <wchar.h>

int g_term_w = 0;
int g_term_h = 0;

void term_clear(void)
{
    system("clear");
}

void term_cursor_reset(void)
{
    printf("\033[0;0H");
    fflush(stdout);
}

void term_cursor_hide(void)
{
    printf("\033[?25l");
    fflush(stdout);
}

void term_cursor_show(void)
{
    printf("\033[?25h");
    fflush(stdout);
}

void term_blank(void)
{
    term_cursor_hide();
    term_clear();
}

#define TERM_WIDTH_CAP 55

unsigned short int term_width(void)
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return TERM_WIDTH_CAP - 1; // fallback if detection fails

    unsigned short int col = w.ws_col;
    if (col > TERM_WIDTH_CAP) col = TERM_WIDTH_CAP; // g_term_buffer is fixed size we need to cap it so we never overflow it when writing spaces
    return col;
}

unsigned short int term_height(void)
{
    struct winsize w;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return 10;

    return w.ws_row;
}

void term_init(void)
{
    g_term_w = term_width();
    g_term_h = term_height();
}

int utf8_display_width(const char* s)
{
    mbstate_t ps = {0};
    wchar_t wc;
    int width = 0;

    const char* p = s;

    while (*p) {
        // skip ANSI escape sequences (e.g., "\033[31m")
        if (*p == '\033') {
            if (*(p + 1) == '[') {
                p += 2;
                while (*p && (*p < '@' || *p > '~')) // skip until letter ending sequence
                    p++;
                if (*p) p++; // skip final letter
                continue;
            }
        }

        // convert next multibyte character
        size_t n = mbrtowc(&wc, p, MB_CUR_MAX, &ps);
        if (n == (size_t)-1 || n == (size_t)-2) {
            // invalid UTF-8, skip a byte
            p++;
            continue;
        }
        else if (n == 0) {
            break;
        }

        int w = wcwidth(wc);
        if (w > 0) width += w;
        p += n;
    }

    return width;
}

void escape_quotes(const char* input, char* output)
{
    int i = 0, j = 0;

    // Iterate through each character in the input string
    while (input[i] != '\0') {

        // Skip zero-width space (U+200B in UTF-8: 0xE2 0x80 0x8B)
        if ((unsigned char)input[i] == 0xE2 &&
            (unsigned char)input[i + 1] == 0x80 &&
            (unsigned char)input[i + 2] == 0x8B) {
            i += 3; // Skip all 3 bytes
            continue;
        }

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
    bool underline;
    bool has_style;
} StyleState;

// Convert 256-color index to RGB hex string
static void ansi_256_to_rgb(int color_index, char* result)
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
static void parse_ansi_sequence(const char* seq, StyleState* state)
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
        else if (code == 4) {
            // Underline
            state->underline = true;
            state->has_style = true;
        }
        else if (code == 24) {
            // Underline off
            state->underline = false;
            // Update has_style if other styles exist
            state->has_style = state->bold || state->color[0] || state->bg_color[0];
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

static void generate_style_string(const StyleState* state, char* style_str, size_t max_len)
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

    if (state->underline) {
        strncat(style_str, "text-decoration: underline; ", max_len - strlen(style_str) - 1);
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

    strcpy(result, "<span>");
    result_pos = 6;
    in_span = true;

    size_t i = 0;
    while (i < text_len) {
        // Skip zero-width space (U+200B in UTF-8: 0xE2 0x80 0x8B)
        if ((unsigned char)text[i] == 0xE2 &&
            i + 2 < text_len &&
            (unsigned char)text[i + 1] == 0x80 &&
            (unsigned char)text[i + 2] == 0x8B) {
            i += 3;
            continue;
        }

        // Handle ANSI escape sequence
        if (text[i] == '\033' && text[i + 1] == '[') {
            i += 2;

            // Check for \033[K (clear to end of line) - just skip it
            if (text[i] == 'K') {
                i++;
                continue;
            }

            // Find the end of the escape sequence (marked by 'm')
            const char* seq_start = text + i;
            char* seq_end = strchr(seq_start, 'm');
            if (seq_end != NULL) {
                size_t seq_len = (size_t)(seq_end - seq_start);
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
                            result_pos += (size_t)sprintf(result + result_pos, "<span style=\"%s\">", style_str);
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
