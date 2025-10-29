#ifndef GLOBALS_H
#define GLOBALS_H

#include <pthread.h>
#include <stdatomic.h>

#define POLL_TIME 10

#ifdef DEBUG
#undef POLL_TIME
#define POLL_TIME 1000
#endif

#define S_ADDR_HTTP  "http://0.0.0.0:5000"  // HTTP port
#define S_ADDR_HTTPS "https://0.0.0.0:5001" // HTTPS port -- unused
#define S_ADDR_WS    "ws://0.0.0.0:8001"    // WebSocket port
#define S_STATIC_DIR "static"

#define WS_MAX_CONN 100

#define SLEEP_MS_DRAW   250
#define SLEEP_MS_BAS    3000       // every 3 seconds
#define SLEEP_MS_WTTRIN 60 * 15    // 15 min

#define MAKE_REQUEST_BAS    1
#define MAKE_REQUEST_WTTRIN 1

#define ENABLE_AUTO_TIMER  1
#define ENABLE_AUTO_GAS    1
#define AUTO_TIMER_SECONDS 8 * 60

#define SMALLBUFF 64
#define MIDBUFF   256
#define BIGBUFF   1024

#define TEMP_SOLAR_MIN 0
#define TEMP_SOLAR_MAX 100
#define TEMP_HUMAN_MIN 0
#define TEMP_HUMAN_MAX 30
#define TEMP_BUF_MIN   45
#define TEMP_BUF_MAX   60
#define TEMP_CIRC_MIN  0
#define TEMP_CIRC_MAX  60

extern atomic_bool g_running;
extern pthread_mutex_t g_mutex;
extern pthread_cond_t g_cond;

#endif // GLOBALS_H
