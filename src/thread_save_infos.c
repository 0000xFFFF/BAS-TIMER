#include "thread_save_infos.h"
#include "debug.h"
#include "globals.h"
#include "request.h"
#include "thread_utils.h"
#include <stdatomic.h>

void* th_save_infos(void* sig)
{
    if (!ENABLE_SAVE_INFOS) { return NULL; }

    DPL("THREAD START SAVE INFOS");
    UNUSED(sig);

    while (atomic_load(&g_running)) {
        infos_save();
        sleep_ms_interruptible(SLEEP_MS_SAVE_INFOS);
    }

    DPL("THREAD STOP SAVE INFOS");
    return NULL;
}
