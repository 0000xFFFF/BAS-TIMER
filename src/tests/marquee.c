#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <wchar.h>

typedef struct {
    char* text;
    int width;
    int pos;            // Scroll position in visible chars
    int text_len;       // Visible length
    int scroll_needed;
    char ansi_state[128]; // Active ANSI codes
} Marquee;

// Compute visible length (ignore ANSI)
int visible_length(const char* str) {
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
        } else {
            p += bytes;
        }
        len++;
    }
    return len;
}

// Update ANSI state
void update_ansi_state(Marquee* m, const char* start, int len) {
    if (len >= (int)sizeof(m->ansi_state)) return;
    strncpy(m->ansi_state, start, len);
    m->ansi_state[len] = '\0';
}

// Print next visible character or ANSI code
int print_next_char(Marquee* m, const char* str, int* idx) {
    if (!str[*idx]) return 0;

    if (str[*idx] == '\033') {
        int start = *idx;
        while (str[*idx] && str[*idx] != 'm') (*idx)++;
        if (str[*idx]) (*idx)++;
        update_ansi_state(m, &str[start], *idx - start);
        fwrite(&str[start], 1, *idx - start, stdout);
        return 0; // ANSI not counted as width
    }

    mbstate_t state;
    memset(&state, 0, sizeof(state));
    wchar_t wc;
    size_t bytes = mbrtowc(&wc, &str[*idx], MB_CUR_MAX, &state);
    if (bytes == (size_t)-1 || bytes == (size_t)-2) {
        putchar(str[(*idx)++]);
        memset(&state, 0, sizeof(state));
        return 1;
    } else {
        fwrite(&str[*idx], 1, bytes, stdout);
        *idx += bytes;
        return 1;
    }
}

// Initialize marquee
void init_marquee(Marquee* m, const char* text, int width) {
    m->text = strdup(text);
    m->width = width;
    m->pos = 0;
    m->text_len = visible_length(text);
    m->scroll_needed = m->text_len > width;
    m->ansi_state[0] = '\0';
}

// Render frame
void render_marquee(Marquee* m) {
    printf("\r"); // go to start of line

    if (!m->scroll_needed) {
        printf("%s", m->text);
        printf("\033[K"); // clear rest of line
        fflush(stdout);
        return;
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
            strncpy(last_ansi, &m->text[start], byte_idx - start);
            last_ansi[byte_idx - start] = '\0';
        } else {
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
    if (last_ansi[0]) fputs(last_ansi, stdout);

    // Print visible width
    while (displayed < m->width) {
        if (!m->text[byte_idx]) byte_idx = 0;
        displayed += print_next_char(m, m->text, &byte_idx);
    }

    printf("\033[K"); // clear leftover characters
    fflush(stdout);
}

// Scroll one step
void scroll_marquee(Marquee* m) {
    if (!m->scroll_needed) return;
    m->pos++;
    if (m->pos >= m->text_len) m->pos = 0;
}

int main() {
    setlocale(LC_ALL, "");
    Marquee m;
    int term_width = 30;

    init_marquee(&m, "\033[31mHello 🌍! This is a \033[32mmarquee\033[0m scroll demo!", term_width);

    while (1) {
        render_marquee(&m);
        scroll_marquee(&m);
        usleep(200000);
    }

    free(m.text);
    return 0;
}

