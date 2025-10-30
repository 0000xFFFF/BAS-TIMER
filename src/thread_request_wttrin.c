#include "debug.h"
#include "globals.h"
#include "src/request.h"
#include "thread_utils.h"
#include <mongoose.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

void* th_request_wttrin(void* sig)
{
    UNUSED(sig);
    DPL("THREAD START WTTRIN");
    while (atomic_load(&g_running)) {
#if MAKE_REQUEST_WTTRIN
        update_info_wttrin();
#endif
        sleep_ms_interruptible(SLEEP_MS_WTTRIN);
    }
    DPL("THREAD STOP WTTRIN");
    return NULL;
}
