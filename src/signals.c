#include "signals.h"
#include "term.h"
#include "thread_utils.h"
#include <stdio.h>

void signals_sigint(int sig)
{
    printf("\n\n\nCaught signal: %d\n\n\n", sig);
    stop_all_threads();
    term_cursor_show();
}
