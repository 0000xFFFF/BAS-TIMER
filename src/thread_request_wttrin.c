#include "debug.h"
#include "globals.h"
#include "request.h"
#include "thread_utils.h"
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

void* th_request_wttrin(void* sig)
{
    UNUSED(sig);
    DPL("THREAD START WTTRIN");

    long sleep = SLEEP_MS_WTTRIN;

    update_info_wttrin_init();
    while (atomic_load(&g_running)) {
        if (MAKE_REQUEST_WTTRIN) {
            sleep = request_status_failed(update_info_wttrin()) ? SLEEP_MS_WTTRIN_RETRY : SLEEP_MS_WTTRIN;
        }
        sleep_ms_interruptible(sleep);
    }
    DPL("THREAD STOP WTTRIN");
    return NULL;
}
