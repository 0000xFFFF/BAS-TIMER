#include "mongoose.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include <signal.h>
#include <stdio.h>

#include "debug.h"
#include "globals.h"
#include "reqworker.h"
#include "term.h"

#include "serve_site.h"
#include "serve_websocket.h"
#include "utils.h"

int g_running = 1;

#ifdef QUICK
#define MAX_ITERS 20
int iter_counter = 0;
#endif

extern char g_term_buffer[];

extern double g_temp_max;
extern double g_temp_min;

extern struct bas_info g_info;

static void* main_worker(void* sig)
{

    UNUSED(sig);

    DPL("WORKER START");

    DPL("WORKER INIT");
    init_reqworker();
    DPL("WORKER INIT DONE");

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000000 * MAIN_WORKER_DRAW_SLEEP; // 1 milliseconds in nanoseconds

    char html_buffer[1024 * 8] = {0};
    char html_buffer_escaped[1024 * 8 * 2] = {0};
    char emit_buffer[1024 * 8 * 2] = {0};

    while (g_running) {

#ifndef DEBUG
        term_cursor_reset();
#endif
        reqworker_do_work();

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
    int result_code = pthread_create(&t1, NULL, main_worker, NULL);
    assert(!result_code);

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, S_ADDR_HTTP, serve_site, &mgr);
    mg_http_listen(&mgr, S_ADDR_WS, serve_websocket, &mgr);

    while (g_running) {
        mg_mgr_poll(&mgr, POLL_TIME);
    }
    mg_mgr_free(&mgr);

    DPL("WORKER JOIN");
    pthread_join(t1, NULL);
    DPL("WORKER JOIN DONE");

    DPL("MAIN EXIT");
    return 0;
}
