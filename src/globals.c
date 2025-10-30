#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

const char* S_ADDR_HTTP = "http://0.0.0.0:5000";   // HTTP port
const char* S_ADDR_HTTPS = "https://0.0.0.0:5001"; // HTTPS port -- unused
const char* S_ADDR_WS = "ws://0.0.0.0:8001";       // WebSocket port
const char* S_STATIC_DIR = "static";

#define STATE_DIR_ "state"
#define SEP_ "/"
const char* STATE_DIR = STATE_DIR_;
const char* STATE_DIR_FILE_CHANGES_LOG = STATE_DIR_ SEP_ "changes.log";
const char* STATE_DIR_FILE_REQUESTS_LOG = STATE_DIR_ SEP_ "requests.log";
const char* STATE_DIR_FILE_ERRORS_LOG = STATE_DIR_ SEP_ "errors.log";

const int POLL_TIME =
#ifdef DEBUG
    1000;
#else
    10;
#endif

const int MAKE_REQUEST_BAS = 1;
const int MAKE_REQUEST_WTTRIN = 1;

const int SLEEP_MS_DRAW = 250;
const int SLEEP_MS_BAS = 3000;       // every 3 seconds
const int SLEEP_MS_WTTRIN = 60 * 15; // 15 min

const int TEMP_SOLAR_MIN = 0;
const int TEMP_SOLAR_MAX = 100;
const int TEMP_HUMAN_MIN = 0;
const int TEMP_HUMAN_MAX = 30;
const int TEMP_BUF_MIN = 45;
const int TEMP_BUF_MAX = 60;
const int TEMP_CIRC_MIN = 0;
const int TEMP_CIRC_MAX = 60;

atomic_bool g_running = true;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t g_mutex_file_changes = PTHREAD_MUTEX_INITIALIZER;

