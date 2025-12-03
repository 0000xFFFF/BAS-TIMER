#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

extern const char* ADDR_HTTP;
extern const char* ADDR_HTTPS;
extern const char* ADDR_WS;
extern const char* STATIC_DIR;

extern const char* VAR_DIR;
extern const char* VAR_DIR_FILE_CHANGES_LOG;
extern const char* VAR_DIR_FILE_REQUESTS_LOG;
extern const char* VAR_DIR_FILE_ERRORS_LOG;
extern const char* VAR_DIR_FILE_INFOS_BIN;

extern const bool ENABLE_REQUEST_BAS;
extern const bool ENABLE_REQUEST_WTTRIN;
extern const bool ENABLE_SAVE_INFOS;
extern const bool ENABLE_RESTARTER;
extern const bool ENABLE_AUTO_TIMER;
extern const bool ENABLE_AUTO_GAS;
extern const int AUTO_TIMER_SECONDS;

extern const int POLL_TIME;
extern const int SLEEP_MS_DRAW;
extern const int SLEEP_MS_BAS;
extern const int SLEEP_MS_WTTRIN;
extern const int SLEEP_MS_WTTRIN_RETRY;
extern const int SLEEP_MS_SAVE_INFOS;
extern const int SLEEP_MS_RESTARTER;

extern const uint64_t TIMEOUT_MS_BAS;
extern const uint64_t TIMEOUT_MS_WTTRIN;

extern const double TEMP_MIN_SOLAR;
extern const double TEMP_MAX_SOLAR;
extern const double TEMP_MIN_HUMAN;
extern const double TEMP_MAX_HUMAN;
extern const double TEMP_MIN_BUF;
extern const double TEMP_MAX_BUF;
extern const double TEMP_MIN_CIRC;
extern const double TEMP_MAX_CIRC;

extern const int RADIATOR_WARMUP_SEC;
extern const int RADIATOR_COOLDOWN_SEC;

extern atomic_bool g_running;
extern pthread_mutex_t g_mutex;
extern pthread_cond_t g_cond;

#define TINYBUFF  32
#define SMALLBUFF 128
#define MIDBUFF   256
#define BIGBUFF   1024

#define WS_MAX_CONN MIDBUFF

#endif // GLOBALS_H
