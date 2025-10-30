#include "thread_utils.h"
#include "term.h"
#include <stdio.h>

void signals_sigint(int sig)
{
    printf("Caught signal: %d\n", sig);
    stop_all_threads();
    term_cursor_show();
}
