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

int term_width()
{
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return 50; // fallback if detection fails

    return w.ws_col - 1; // NOTE: some utf8 chars break term spacing so -1 to dirty fix that
}

int term_height()
{
    struct winsize w;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return 10;

    return w.ws_row;
}

void term_init()
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
