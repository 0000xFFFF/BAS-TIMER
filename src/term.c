#include "term.h"
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void term_clear()
{
    system("clear");
}

void term_cursor_reset()
{
    printf("\033[0;0H");
    fflush(stdout);
}

void term_cursor_hide()
{
    printf("\033[?25l");
    fflush(stdout);
}

void term_cursor_show()
{
    printf("\033[?25h");
    fflush(stdout);
}

void term_blank()
{
    term_cursor_hide();
    term_clear();
}

// Structure to hold the current style state
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

// Generate CSS style string based on current state
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

// Convert ANSI escape sequences to HTML
int ansi_to_html(const char* text, char* result)
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
