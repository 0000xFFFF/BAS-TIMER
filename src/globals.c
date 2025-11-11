#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

const char* ADDR_HTTP = "http://0.0.0.0:5000";   // HTTP port
const char* ADDR_HTTPS = "https://0.0.0.0:5001"; // HTTPS port -- unused
const char* STATIC_DIR = "static";

#define VAR_DIR_ "var"
#define SEP_     "/"
const char* VAR_DIR = VAR_DIR_;
const char* VAR_DIR_FILE_CHANGES_LOG = VAR_DIR_ SEP_ "changes.log";
const char* VAR_DIR_FILE_REQUESTS_LOG = VAR_DIR_ SEP_ "requests.log";
const char* VAR_DIR_FILE_ERRORS_LOG = VAR_DIR_ SEP_ "errors.log";
const char* VAR_DIR_FILE_INFOS_BIN = VAR_DIR_ SEP_ "infos.bin";

const int ENABLE_REQUEST_BAS = 1;
const int ENABLE_REQUEST_WTTRIN = 1;
const int ENABLE_SAVE_INFOS = 1;
const int ENABLE_AUTO_TIMER = 1;
const int ENABLE_AUTO_GAS = 1;
const int AUTO_TIMER_SECONDS = 8 * 60;


const int POLL_TIME =
#ifdef DEBUG
    1000;
#else
    10;
#endif
const int SLEEP_MS_DRAW =
#ifdef DEBUG
    1000;
#else
    250;
#endif
const int SLEEP_MS_BAS = 3000;             // 3 sec
const int SLEEP_MS_WTTRIN = 60 * 5 * 1000; // 5 min
const int SLEEP_MS_WTTRIN_RETRY = 5000;    // 5 sec
const int SLEEP_MS_SAVE_INFOS = 60 * 1000; // 1 min
                                           //
const int TIMEOUT_MS_BAS = 1500;
const int TIMEOUT_MS_WTTRIN = 10000;

const double TEMP_MIN_SOLAR = 0;
const double TEMP_MAX_SOLAR = 100;
const double TEMP_MIN_HUMAN = 0;
const double TEMP_MAX_HUMAN = 30;
const double TEMP_MIN_BUF  = 45;
const double TEMP_MAX_BUF =  60;
const double TEMP_MIN_CIRC = 0;
const double TEMP_MAX_CIRC = 60;

atomic_bool g_running = true;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;

