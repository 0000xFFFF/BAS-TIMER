#include "marquee.h"
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// Helper: compute visible length ignoring ANSI sequences
static int visible_length(const char* str)
{
    int len = 0;
    mbstate_t state;
    memset(&state, 0, sizeof(state));
    const char* p = str;

    while (*p) {
        if (*p == '\033') { // ANSI escape
            while (*p && *p != 'm') p++;
            if (*p) p++;
            continue;
        }
        wchar_t wc;
        size_t bytes = mbrtowc(&wc, p, MB_CUR_MAX, &state);
        if (bytes == (size_t)-1 || bytes == (size_t)-2) {
            bytes = 1;
            memset(&state, 0, sizeof(state));
        }
        else {
            p += bytes;
        }
        len++;
    }
    return len;
}

// Update ANSI state buffer
static void update_ansi_state(Marquee* m, const char* start, int len)
{
    if (len >= (int)sizeof(m->ansi_state)) return;
    strncpy(m->ansi_state, start, len);
    m->ansi_state[len] = '\0';
}

// Print next UTF-8 char or ANSI sequence into buffer, return width (0 for ANSI, 1 for visible)
static int print_next_char_buf(Marquee* m, const char* str, int* idx, char* buf, size_t buf_size, size_t* buf_pos)
{
    if (!str[*idx] || *buf_pos >= buf_size - 1) return 0;

    if (str[*idx] == '\033') {
        int start = *idx;
        while (str[*idx] && str[*idx] != 'm') (*idx)++;
        if (str[*idx]) (*idx)++;
        update_ansi_state(m, &str[start], *idx - start);

        size_t len = *idx - start;
        if (*buf_pos + len >= buf_size - 1) len = buf_size - 1 - *buf_pos;
        memcpy(&buf[*buf_pos], &str[start], len);
        *buf_pos += len;
        buf[*buf_pos] = '\0';
        return 0;
    }

    mbstate_t state;
    memset(&state, 0, sizeof(state));
    wchar_t wc;
    size_t bytes = mbrtowc(&wc, &str[*idx], MB_CUR_MAX, &state);
    if (bytes == (size_t)-1 || bytes == (size_t)-2) {
        buf[(*buf_pos)++] = str[(*idx)++];
        buf[*buf_pos] = '\0';
        memset(&state, 0, sizeof(state));
        return 1;
    }
    else {
        if (*buf_pos + bytes >= buf_size - 1) bytes = buf_size - 1 - *buf_pos;
        memcpy(&buf[*buf_pos], &str[*idx], bytes);
        *buf_pos += bytes;
        buf[*buf_pos] = '\0';
        *idx += bytes;
        return 1;
    }
}

// Initialize marquee
void init_marquee(Marquee* m, const char* text, int width, int scroll_on)
{

    setlocale(LC_ALL, ""); // UTF-8
    m->text = text;
    m->width = width;
    m->pos = 0;
    m->text_len = visible_length(text);
    m->scroll_needed = m->text_len > width;
    m->i = 0;
    m->scroll_on = scroll_on > 0 ? scroll_on : 1;
    m->ansi_state[0] = '\0';

    asm("int3");
}

// Scroll marquee (updates position based on scroll_on)
void scroll_marquee(Marquee* m)
{
    if (!m->scroll_needed) return;
    m->i++;
    if (m->i >= m->scroll_on) {
        m->i = 0;
        m->pos++;
        if (m->pos >= m->text_len) m->pos = 0;
    }
}

// Render current frame into buffer
void render_marquee(Marquee* m, char* buffer, size_t buffer_size)
{
    if (!buffer || buffer_size < 1) return;

    size_t buf_pos = 0;
    buffer[0] = '\0';

    if (!m->scroll_needed) {
        strncpy(buffer, m->text, buffer_size - 1);
        buffer[buffer_size - 1] = '\0';
        return;
    }

    int displayed = 0;
    int vis_idx = 0;
    int byte_idx = 0;
    char last_ansi[128] = {0};

    // Skip to scroll position
    while (vis_idx < m->pos) {
        if (!m->text[byte_idx]) byte_idx = 0;
        if (m->text[byte_idx] == '\033') {
            int start = byte_idx;
            while (m->text[byte_idx] && m->text[byte_idx] != 'm') byte_idx++;
            if (m->text[byte_idx]) byte_idx++;
            strncpy(last_ansi, &m->text[start], byte_idx - start);
            last_ansi[byte_idx - start] = '\0';
        }
        else {
            mbstate_t state;
            memset(&state, 0, sizeof(state));
            wchar_t wc;
            size_t bytes = mbrtowc(&wc, &m->text[byte_idx], MB_CUR_MAX, &state);
            if (bytes == (size_t)-1 || bytes == (size_t)-2) bytes = 1;
            byte_idx += bytes;
            vis_idx++;
        }
    }

    // Prepend active ANSI
    size_t len = strlen(last_ansi);
    if (buf_pos + len < buffer_size - 1) {
        memcpy(&buffer[buf_pos], last_ansi, len);
        buf_pos += len;
        buffer[buf_pos] = '\0';
    }

    // Print visible characters
    while (displayed < m->width && buf_pos < buffer_size - 1) {
        if (!m->text[byte_idx]) byte_idx = 0;
        displayed += print_next_char_buf(m, m->text, &byte_idx, buffer, buffer_size, &buf_pos);
    }

    // Clear remaining part of buffer to remove artifacts
    while (buf_pos < buffer_size - 1) {
        buffer[buf_pos++] = ' ';
    }
    buffer[buf_pos] = '\0';
}
