#include "debug.h"
#include "globals.h"
#include "src/request.h"
#include "thread_utils.h"
#include <mongoose.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

extern pthread_mutex_t g_mutex;
extern pthread_cond_t g_cond;

void* th_request_wttrin(void* sig)
{
    UNUSED(sig);
    while (atomic_load(&g_running)) {
#if MAKE_REQUEST_WTTRIN
        update_info_wttrin();
#endif
        sleep_ms_interruptible(SLEEP_MS_WTTRIN);
    }
    return NULL;
}
