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
#include <unistd.h>

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

    return w.ws_col;
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
