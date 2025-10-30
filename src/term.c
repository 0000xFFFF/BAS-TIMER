#include "term.h"
#include <float.h>
#include <libgen.h>
#include <limits.h>
#include <regex.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

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

#define FIXED_TERM_WIDTH 50
#define ENABLE_FIXED_TERM_WIDTH 1

int term_width()
{
#if ENABLE_FIXED_TERM_WIDTH
    return FIXED_TERM_WIDTH;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        return FIXED_TERM_WIDTH; // fallback if detection fails

    return w.ws_col;
#endif
}
