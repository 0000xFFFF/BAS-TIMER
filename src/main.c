#include "debug.h"
#include "globals.h"
#include "mongoose.h"
#include "signals.h"
#include "term.h"
#include "thread_print_loop.h"
#include "thread_request_bas.h"
#include "thread_request_wttrin.h"
#include "thread_serve.h"
#include "thread_utils.h"
#include "utils.h"
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <unistd.h>

atomic_bool g_running = true;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

int main()
{
    // init
    DPL("MAIN START");
    change_to_bin_dir();
    signal(SIGINT, signals_sigint);

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

    pthread_t t4;
    assert(!pthread_create(&t4, NULL, th_print_loop, NULL));

    pthread_join(t4, NULL);

    stop_all_threads();

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    DPL("MAIN STOP");
    return 0;
}
