#include "mongoose.h"

#include <assert.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>

#include <signal.h>
#include <stdio.h>

#include "debug.h"
#include "globals.h"
#include "request.h"
#include "src/spinners.h"
#include "term.h"

#include "serve_site.h"
#include "serve_websocket.h"
#include "utils.h"

atomic_bool g_running = true;
pthread_mutex_t g_update_info_bas_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

#ifdef QUICK
#define MAX_ITERS 20
int iter_counter = 0;
#endif

extern char g_term_buffer[];

extern double g_temp_max;
extern double g_temp_min;

extern struct bas_info g_info;

void init() {

    g_global_unix_counter = timestamp();

    // for drawui
    init_spinners();
}

static void* th_print_loop(void* sig)
{

    UNUSED(sig);

    init();

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000 * SLEEP_MS_DRAW;

    char html_buffer[1024 * 16] = {0};
    char html_buffer_escaped[1024 * 16 * 2] = {0};
    char emit_buffer[1024 * 16 * 2] = {0};

    while (atomic_load(&g_running)) {

#ifndef DEBUG
        term_cursor_reset();
#endif
        draw_ui();
        ansi_to_html(g_term_buffer, html_buffer);
        escape_quotes(html_buffer, html_buffer_escaped);

        int b = snprintf(emit_buffer, 1024 * 8 * 2,
                         "{"
                         "\"term\": \"%s\""
                         ","
                         "\"Tmin\": %f"
                         ","
                         "\"Tmax\": %f"
                         ","
                         "\"mod_rada\": %d"
                         ","
                         "\"StatusPumpe4\": %d"
                         "}",
                         html_buffer_escaped,
                         g_info.Tmin,
                         g_info.Tmax,
                         g_info.mod_rada,    // heat
                         g_info.StatusPumpe4 // gas pump
        );
        websocket_emit(emit_buffer, b);

        nanosleep(&ts, NULL);

#ifdef QUICK
        iter_counter++;
        if (iter_counter >= MAX_ITERS) { g_running = 0; }
#endif
    }

    DPL("WORKER EXIT (CLEANUP)");
    return NULL;
}

static void* th_requests_bas(void* sig)
{

    UNUSED(sig);

    while (atomic_load(&g_running)) {

#if MAKE_REQUEST_BAS
        update_info_bas();
#endif

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += SLEEP_MS_REQUESTS_WORKER_WTTRIN;

        pthread_mutex_lock(&g_update_info_bas_mutex);
        while (atomic_load(&g_running)) {
            int rc = pthread_cond_timedwait(&g_cond, &g_update_info_bas_mutex, &ts);
            if (rc == ETIMEDOUT) break;
        }
        pthread_mutex_unlock(&g_update_info_bas_mutex);
    }

    return NULL;
}

static void* th_requests_wttrin(void* sig)
{

    UNUSED(sig);

    while (atomic_load(&g_running)) {

#if MAKE_REQUEST_WTTRIN
        update_info_wttrin();
#endif

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += SLEEP_MS_REQUESTS_WORKER_WTTRIN;

        pthread_mutex_lock(&g_update_info_bas_mutex);
        while (atomic_load(&g_running)) {
            int rc = pthread_cond_timedwait(&g_cond, &g_update_info_bas_mutex, &ts);
            if (rc == ETIMEDOUT) break;
        }
        pthread_mutex_unlock(&g_update_info_bas_mutex);
    }

    return NULL;
}

static void signal_handler(int sig)
{
    printf("Caught signal: %d\n", sig);
    g_running = 0;
    term_cursor_show();
}

int main()
{

    DPL("MAIN START");

    change_to_bin_dir();

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
    assert(!pthread_create(&t1, NULL, th_print_loop, NULL));

    pthread_t t2;
    assert(!pthread_create(&t2, NULL, th_requests_bas, NULL));

    pthread_t t3;
    assert(!pthread_create(&t3, NULL, th_requests_wttrin, NULL));

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, S_ADDR_HTTP, serve_site, &mgr);
    mg_http_listen(&mgr, S_ADDR_WS, serve_websocket, &mgr);
    while (atomic_load(&g_running)) { mg_mgr_poll(&mgr, POLL_TIME); }
    mg_mgr_free(&mgr);

    atomic_store(&g_running, false);
    pthread_mutex_lock(&g_update_info_bas_mutex);
    pthread_cond_broadcast(&g_cond);
    pthread_mutex_unlock(&g_update_info_bas_mutex);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    DPL("MAIN EXIT");
    return 0;
}
