#include "globals.h"
#include "src/debug.h"
#include <asm-generic/errno.h>
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <unistd.h>

void stop_all_threads()
{
    DPL("STOPPING ALL THREADS");
    atomic_store(&g_running, false);
    pthread_mutex_lock(&g_mutex);
    pthread_cond_broadcast(&g_cond); // wake all
    pthread_mutex_unlock(&g_mutex);
}

struct timespec make_timeout_ms(long ms)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec += ms / 1000;
    ts.tv_nsec += (ms % 1000) * 1000000L;

    // Handle nanosecond overflow
    if (ts.tv_nsec >= 1000000000L) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000L;
    }

    return ts;
}

void sleep_ms_interruptible(long ms)
{
    struct timespec ts = make_timeout_ms(ms);
    pthread_mutex_lock(&g_mutex);
    while (atomic_load(&g_running)) {
        int rc = pthread_cond_timedwait(&g_cond, &g_mutex, &ts);
        if (rc == ETIMEDOUT) break;
    }
    pthread_mutex_unlock(&g_mutex);
}
