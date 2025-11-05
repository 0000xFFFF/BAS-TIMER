#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <stdatomic.h>

extern const char* ADDR_HTTP;
extern const char* ADDR_HTTPS;
extern const char* ADDR_WS;
extern const char* STATIC_DIR;

extern const char* VAR_DIR;
extern const char* VAR_DIR_FILE_CHANGES_LOG;
extern const char* VAR_DIR_FILE_REQUESTS_LOG;
extern const char* VAR_DIR_FILE_ERRORS_LOG;
extern const char* VAR_DIR_FILE_INFOS_BIN;

extern const int POLL_TIME;

#define WS_MAX_CONN MIDBUFF

extern const int MAKE_REQUEST_BAS;
extern const int MAKE_REQUEST_WTTRIN;

extern const int SLEEP_MS_DRAW;
extern const int SLEEP_MS_BAS;
extern const int SLEEP_MS_WTTRIN;
extern const int SLEEP_MS_WTTRIN_RETRY;
extern const int SLEEP_MS_SAVE_INFOS;

extern int TEMP_SOLAR_MIN;
extern int TEMP_SOLAR_MAX;
extern int TEMP_HUMAN_MIN;
extern int TEMP_HUMAN_MAX;
extern int TEMP_BUF_MIN;
extern int TEMP_BUF_MAX;
extern int TEMP_CIRC_MIN;
extern int TEMP_CIRC_MAX;

extern atomic_bool g_running;
extern pthread_mutex_t g_mutex;
extern pthread_cond_t g_cond;

#define TINYBUFF  32
#define SMALLBUFF 128
#define MIDBUFF   256
#define BIGBUFF   1024

extern const int ENABLE_AUTO_TIMER;
extern const int ENABLE_AUTO_GAS;
extern const int AUTO_TIMER_SECONDS;

#endif // GLOBALS_H
