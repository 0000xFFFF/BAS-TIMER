#include "debug.h"
#include "globals.h"
#include "mongoose.h"
#include "request.h"
#include "spinners.h"
#include "term.h"
#include "thread_print_loop.h"
#include "thread_request_bas.h"
#include "thread_request_wttrin.h"
#include "thread_serve.h"
#include "utils.h"
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>

atomic_bool g_running = true;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

void init()
{
    g_global_unix_counter = timestamp();

    // for drawui
    init_spinners();
}

void stop() {
    DPL("STOPPING ALL THREADS");
    atomic_store(&g_running, false);
    pthread_mutex_lock(&g_mutex);
    pthread_cond_broadcast(&g_cond); // wake all
    pthread_mutex_unlock(&g_mutex);
}

static void signal_handler(int sig)
{
    printf("Caught signal: %d\n", sig);
    stop();
    term_cursor_show();
}

int main()
{
    // init
    DPL("MAIN START");
    change_to_bin_dir();
    init();
    signal(SIGINT, signal_handler);

#ifdef DEBUG
    const char* log_level = getenv("LOG_LEVEL");
    mg_log_set(log_level == NULL ? MG_LL_DEBUG : atoi(log_level));
#else
    mg_log_set(MG_LL_NONE);
    term_clear();
    term_cursor_hide();
#endif

    pthread_t t1;
    assert(!pthread_create(&t1, NULL, th_serve, NULL));

    pthread_t t2;
    assert(!pthread_create(&t2, NULL, th_request_bas, NULL));

    pthread_t t3;
    assert(!pthread_create(&t3, NULL, th_request_wttrin, NULL));

    th_print_loop(0);

    stop();

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    DPL("main exit");
    return 0;
}
