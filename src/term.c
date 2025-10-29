#include "term.h"
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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
