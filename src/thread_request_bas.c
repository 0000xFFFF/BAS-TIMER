#include "debug.h"
#include "globals.h"
#include "request.h"
#include "thread_utils.h"
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

extern pthread_mutex_t g_mutex;
extern pthread_cond_t g_cond;

void* th_request_bas(void* sig)
{
    DPL("THREAD START BAS");
    UNUSED(sig);

    while (atomic_load(&g_running)) {
#if MAKE_REQUEST_BAS
        update_info_bas();
#endif
        sleep_ms_interruptible(SLEEP_MS_BAS);
    }

    DPL("THREAD STOP BAS");
    return NULL;
}
