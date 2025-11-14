#include "thread_utils.h"
#include "debug.h"
#include "globals.h"
#include <asm-generic/errno.h>
#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <unistd.h>

void init_thread_data(void)
{
    pthread_mutex_init(&g_mutex, NULL);

    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_cond_init(&g_cond, &attr);
    pthread_condattr_destroy(&attr);
}

void stop_all_threads(void)
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
    clock_gettime(CLOCK_MONOTONIC, &ts);

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
