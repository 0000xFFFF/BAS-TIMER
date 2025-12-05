#include "thread_request_doctor.h"
#include "debug.h"
#include "globals.h"
#include "thread_utils.h"
#include "utils.h"
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

atomic_bool g_online = false;

void* th_request_doctor(void* sig)
{
    if (!ENABLE_DOCTOR) { return NULL; }

    DPL("THREAD START DOCTOR");
    UNUSED(sig);

    while (atomic_load(&g_running)) {
        atomic_store(&g_online, is_connection_healthy());
        sleep_ms_interruptible(SLEEP_MS_DOCTOR);
    }

    DPL("THREAD STOP DOCTOR");
    return NULL;
}
