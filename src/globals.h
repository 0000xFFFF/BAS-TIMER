#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <stdatomic.h>

extern char* ADDR_HTTP;
extern char* ADDR_HTTPS;
extern char* ADDR_WS;
extern char* STATIC_DIR;

extern const char* VAR_DIR;
extern const char* VAR_DIR_FILE_CHANGES_LOG;
extern const char* VAR_DIR_FILE_REQUESTS_LOG;
extern const char* VAR_DIR_FILE_ERRORS_LOG;

extern int POLL_TIME;

extern int WS_MAX_CONN;
extern int MAKE_REQUEST_BAS;
extern int MAKE_REQUEST_WTTRIN;

extern int SLEEP_MS_DRAW;
extern int SLEEP_MS_BAS;
extern int SLEEP_MS_WTTRIN;

extern int ENABLE_AUTO_TIMER;
extern int ENABLE_AUTO_GAS;
extern int AUTO_TIMER_SECONDS;

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

#define SMALLBUFF 64
#define MIDBUFF   256
#define BIGBUFF   1024

#define WS_MAX_CONN MIDBUFF

#define ENABLE_AUTO_TIMER  1
#define ENABLE_AUTO_GAS    1
#define AUTO_TIMER_SECONDS 8 * 60

#endif // GLOBALS_H
