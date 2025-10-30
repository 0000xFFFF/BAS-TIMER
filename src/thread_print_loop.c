#include "debug.h"
#include "draw_ui.h"
#include "globals.h"
#include "spinners.h"
#include "thread_utils.h"
#include <stdatomic.h>

void* th_print_loop(void* sig)
{
    init_spinners();

    DPL("THREAD START PRINT LOOP");
    UNUSED(sig);

    while (atomic_load(&g_running)) {

        draw_ui_and_front();
        sleep_ms_interruptible(SLEEP_MS_DRAW);
    }

    DPL("THREAD STOP PRINT LOOP");
    return NULL;
}
