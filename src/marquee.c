#include "marquee.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>

// Compute visible length (ignore ANSI)
int marquee_visible_length(const char* str)
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

// Update ANSI state
static void update_ansi_state(struct Marquee* m, const char* start, int len)
{
    if (len >= (int)sizeof(m->ansi_state)) return;
    strncpy(m->ansi_state, start, len);
    m->ansi_state[len] = '\0';
}

// Write next visible character or ANSI code to buffer
static int write_next_char(struct Marquee* m, const char* str, int* idx, char** buf_ptr, size_t* remaining)
{
    if (!str[*idx]) return 0;

    if (str[*idx] == '\033') {
        int start = *idx;
        while (str[*idx] && str[*idx] != 'm') (*idx)++;
        if (str[*idx]) (*idx)++;

        int len = *idx - start;
        if ((size_t)len >= *remaining) return -1; // Buffer overflow

        update_ansi_state(m, &str[start], len);
        memcpy(*buf_ptr, &str[start], len);
        *buf_ptr += len;
        *remaining -= len;
        return 0; // ANSI not counted as width
    }

    mbstate_t state;
    memset(&state, 0, sizeof(state));
    wchar_t wc;
    size_t bytes = mbrtowc(&wc, &str[*idx], MB_CUR_MAX, &state);

    if (bytes == (size_t)-1 || bytes == (size_t)-2) {
        if (*remaining < 1) return -1;
        **buf_ptr = str[*idx];
        (*buf_ptr)++;
        (*remaining)--;
        (*idx)++;
        memset(&state, 0, sizeof(state));
        return 1;
    }
    else {
        if (*remaining < bytes) return -1;
        memcpy(*buf_ptr, &str[*idx], bytes);
        *buf_ptr += bytes;
        *remaining -= bytes;
        *idx += bytes;
        return 1;
    }
}

// Initialize marquee
void marquee_init(struct Marquee* m, const char* text, int width, int start_delay, int update_on)
{
    m->text = text; // Store pointer directly, don't copy
    m->width = width;
    m->pos = 0;
    m->text_len = marquee_visible_length(text);
    m->scroll_needed = m->text_len > width;
    m->ansi_state[0] = '\0';
    m->update_on = update_on;
    m->start_delay = start_delay;
}

void marquee_update_width(struct Marquee* m, int width)
{
    m->width = width;
    m->text_len = marquee_visible_length(m->text);
    m->scroll_needed = m->text_len > width;
}

// Render frame to buffer
int marquee_render(struct Marquee* m, char* buffer, size_t size)
{
    if (!buffer || size == 0) return -1;

    char* buf_ptr = buffer;
    size_t remaining = size - 1; // Reserve space for null terminator

    if (!m->scroll_needed) {
        size_t len = strlen(m->text);
        if (len >= remaining) return -1;
        strcpy(buffer, m->text);
        return len;
    }

    int displayed = 0;
    int vis_idx = 0;
    int byte_idx = 0;
    char last_ansi[128] = {0};

    // Skip to scroll position (count visible chars)
    while (vis_idx < m->pos) {
        if (!m->text[byte_idx]) byte_idx = 0;

        if (m->text[byte_idx] == '\033') {
            int start = byte_idx;
            while (m->text[byte_idx] && m->text[byte_idx] != 'm') byte_idx++;
            if (m->text[byte_idx]) byte_idx++;
            int len = byte_idx - start;
            if (len < (int)sizeof(last_ansi)) {
                strncpy(last_ansi, &m->text[start], len);
                last_ansi[len] = '\0';
            }
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
    if (last_ansi[0]) {
        size_t ansi_len = strlen(last_ansi);
        if (ansi_len >= remaining) return -1;
        memcpy(buf_ptr, last_ansi, ansi_len);
        buf_ptr += ansi_len;
        remaining -= ansi_len;
    }

    // Write visible width
    while (displayed < m->width) {
        if (!m->text[byte_idx]) byte_idx = 0;
        int result = write_next_char(m, m->text, &byte_idx, &buf_ptr, &remaining);
        if (result < 0) return -1; // Buffer overflow
        displayed += result;
    }

    *buf_ptr = '\0';
    return buf_ptr - buffer;
}

// Scroll one step
void marquee_scroll(struct Marquee* m)
{
    if (!m->scroll_needed) return;
    m->i++;

    if (m->start_delay != 0 && m->pos == 0 && m->i < m->start_delay) { return; }

    if (m->i >= m->update_on) {
        m->i = 0;
        m->pos++;
        if (m->pos >= m->text_len) m->pos = 0;
    }
}

// pause marquee scroll on zero width space
void marquee_scroll_smart(struct Marquee* m)
{
    if (!m->scroll_needed) return;

    // Find the current first visible character (after skipping to m->pos)
    int vis_idx = 0;
    int byte_idx = 0;

    // Skip to current scroll position
    while (vis_idx < m->pos) {
        if (!m->text[byte_idx]) byte_idx = 0;

        if (m->text[byte_idx] == '\033') {
            // Skip ANSI escape sequence
            while (m->text[byte_idx] && m->text[byte_idx] != 'm') byte_idx++;
            if (m->text[byte_idx]) byte_idx++;
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

    // Now find the first actual visible character at current position
    if (!m->text[byte_idx]) byte_idx = 0;

    // Skip any ANSI sequences at the current position
    while (m->text[byte_idx] == '\033') {
        while (m->text[byte_idx] && m->text[byte_idx] != 'm') byte_idx++;
        if (m->text[byte_idx]) byte_idx++;
        if (!m->text[byte_idx]) byte_idx = 0;
    }

    // Decode next Unicode character
    mbstate_t state;
    memset(&state, 0, sizeof(state));
    wchar_t wc;
    size_t bytes = mbrtowc(&wc, &m->text[byte_idx], MB_CUR_MAX, &state);

    // Pause on zero width space MARQUEE_ZERO_WIDTH_SPACE
    if (bytes == (size_t)-1 || bytes == (size_t)-2) {
        // Invalid — treat as byte, no pause
        bytes = 1;
    }
    else {
        // U+200B Zero Width Space?
        if (wc == 0x200B && m->i < m->start_delay) {
            m->i++;
            return;
        }
    }

    // Normal scrolling behavior
    m->i++;
    if (m->i >= m->update_on) {
        m->i = 0;
        m->pos++;
        if (m->pos >= m->text_len) m->pos = 0;
    }
}
