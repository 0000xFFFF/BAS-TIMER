#include "debug.h"
#include "globals.h"
#include "request.h"
#include "src/utils.h"
#include "thread_utils.h"
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

void* th_request_bas(void* sig)
{
    g_global_unix_counter = timestamp();

    DPL("THREAD START BAS");
    UNUSED(sig);

    while (atomic_load(&g_running)) {
        if (MAKE_REQUEST_BAS) update_info_bas();
        sleep_ms_interruptible(SLEEP_MS_BAS);
    }

    DPL("THREAD STOP BAS");
    return NULL;
}
