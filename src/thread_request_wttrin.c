#include "thread_request_wttrin.h"
#include "debug.h"
#include "globals.h"
#include "request.h"
#include "thread_utils.h"
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

void* th_request_wttrin(void* sig)
{
    if (!ENABLE_REQUEST_WTTRIN) { return NULL; }
    UNUSED(sig);
    DPL("THREAD START WTTRIN");
    infos_wttrin_init();
    while (atomic_load(&g_running)) {
        infos_wttrin_update();
        sleep_ms_interruptible(SLEEP_MS_WTTRIN);
    }
    DPL("THREAD STOP WTTRIN");
    return NULL;
}
