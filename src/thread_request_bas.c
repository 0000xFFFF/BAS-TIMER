#include "thread_request_bas.h"
#include "debug.h"
#include "globals.h"
#include "request.h"
#include "thread_utils.h"
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

void* th_request_bas(void* sig)
{
    if (!ENABLE_REQUEST_BAS) { return NULL; }

    DPL("THREAD START BAS");
    UNUSED(sig);

    infos_bas_init();
    while (atomic_load(&g_running)) {
        infos_bas_update();
        sleep_ms_interruptible(SLEEP_MS_BAS);
    }

    DPL("THREAD STOP BAS");
    return NULL;
}
