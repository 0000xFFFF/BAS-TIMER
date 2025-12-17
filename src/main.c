#include "debug.h"
#include "globals.h"
#include "logger.h"
#include "main_utils.h"
#include "mongoose.h"
#include "request.h"
#include "schedules.h"
#include "signals.h"
#include "term.h"
#include "thread_print_loop.h"
#include "thread_request_bas.h"
#include "thread_request_doctor.h"
#include "thread_request_wttrin.h"
#include "thread_restarter.h"
#include "thread_save_infos.h"
#include "thread_serve.h"
#include "thread_utils.h"
#include "utils.h"
#include <assert.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <unistd.h>

int main(void)
{
    // init
    DPL("MAIN START");
    term_init();
    change_to_bin_dir();
    mkdir_safe(VAR_DIR);
    load_env(".env");
    signal(SIGINT, signals_sigint);
    if (ENABLE_SAVE_INFOS) { load_infos(VAR_DIR_FILE_INFOS_BIN, &g_infos); }

    schedules_init();

    // set needed .env vars to globals here
    // ...
    // const char* url_wttrin = getenv("URL_WTTRIN"); // this is removed since we are parsing csv
    // if (url_wttrin != NULL) { URL_WTTRIN = url_wttrin; }
    ENABLE_REQUEST_BAS = env_str_to_bool(getenv("ENABLE_REQUEST_BAS"));
    ENABLE_REQUEST_WTTRIN = env_str_to_bool(getenv("ENABLE_REQUEST_WTTRIN"));
    ENABLE_SAVE_INFOS = env_str_to_bool(getenv("ENABLE_SAVE_INFOS"));
    ENABLE_RESTARTER = env_str_to_bool(getenv("ENABLE_RESTARTER"));
    ENABLE_DOCTOR = env_str_to_bool(getenv("ENABLE_DOCTOR"));
    ENABLE_AUTO_TIMER = env_str_to_bool(getenv("ENABLE_AUTO_TIMER"));
    ENABLE_AUTO_GAS = env_str_to_bool(getenv("ENABLE_AUTO_GAS"));

#ifdef DEBUG
    const char* log_level = getenv("LOG_LEVEL");
    mg_log_set(log_level == NULL ? MG_LL_DEBUG : atoi(log_level));
#else
    mg_log_set(MG_LL_NONE);
    term_clear();
    term_cursor_hide();
#endif

    init_thread_data();

    logger_write_changes("system - program start\n");

    pthread_t t_serve;
    assert(!pthread_create(&t_serve, NULL, th_serve, NULL));

    pthread_t t_request_doctor;
    assert(!pthread_create(&t_request_doctor, NULL, th_request_doctor, NULL));

    pthread_t t_print_loop;
    assert(!pthread_create(&t_print_loop, NULL, th_print_loop, NULL));

    pthread_t t_request_bas;
    assert(!pthread_create(&t_request_bas, NULL, th_request_bas, NULL));

    pthread_t t_request_wttrin;
    assert(!pthread_create(&t_request_wttrin, NULL, th_request_wttrin, NULL));

    pthread_t t_save_infos;
    assert(!pthread_create(&t_save_infos, NULL, th_save_infos, NULL));

    pthread_t t_restarter;
    assert(!pthread_create(&t_restarter, NULL, th_restarter, NULL));

    pthread_join(t_print_loop, NULL);

    stop_all_threads();

    pthread_join(t_serve, NULL);
    pthread_join(t_print_loop, NULL);
    pthread_join(t_request_bas, NULL);
    pthread_join(t_save_infos, NULL);
    pthread_join(t_restarter, NULL);

    schedules_free();

    DPL("MAIN STOP");
    return 0;
}
