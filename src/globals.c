#include "globals.h"

const char* ADDR_HTTP = "http://0.0.0.0:5000";   // HTTP port
const char* ADDR_HTTPS = "https://0.0.0.0:5001"; // HTTPS port -- unused
const char* STATIC_DIR = "static";

#define VAR_DIR_ "var"
#define SEP_     "/"
const char* VAR_DIR = VAR_DIR_;
const char* VAR_DIR_FILE_CHANGES_LOG = VAR_DIR_ SEP_ "changes.log";
const char* VAR_DIR_FILE_REQUESTS_LOG = VAR_DIR_ SEP_ "requests.log";
const char* VAR_DIR_FILE_ERRORS_LOG = VAR_DIR_ SEP_ "errors.log";
const char* VAR_DIR_FILE_WTTRIN_LOG = VAR_DIR_ SEP_ "wttrin.log";
const char* VAR_DIR_FILE_INFOS_BIN = VAR_DIR_ SEP_ "infos.bin";
const char* VAR_DIR_FILE_SCHEDULES_BIN = VAR_DIR_ SEP_ "schedules.bin";

const bool ENABLE_REQUEST_BAS = true;
const bool ENABLE_REQUEST_WTTRIN = true;
const bool ENABLE_SAVE_INFOS =
#ifdef DEBUG
    false;
#else
    true;
#endif
const bool ENABLE_RESTARTER = true;
const bool ENABLE_DOCTOR = true;

const bool ENABLE_AUTO_TIMER = true;
const bool ENABLE_AUTO_GAS = true;
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
const int SLEEP_MS_BAS = 3000;                // 3 sec
const int SLEEP_MS_WTTRIN = 10 * 60 * 1000;   // 10 min
const int SLEEP_MS_SAVE_INFOS = 3000;         // 3 sec
const int SLEEP_MS_RESTARTER = 3 * 60 * 1000; // 3 min
const int SLEEP_MS_DOCTOR = 60 * 1000;        // 1 min

const uint64_t TIMEOUT_MS_BAS = 3000;             // 3 sec
const uint64_t TIMEOUT_MS_WTTRIN = 3 * 60 * 1000; // 3 min

const double TEMP_MIN_SOLAR = 0;
const double TEMP_MAX_SOLAR = 100;
const double TEMP_MIN_HUMAN = 0;
const double TEMP_MAX_HUMAN = 30;
const double TEMP_MIN_BUF = 45;
const double TEMP_MAX_BUF = 60;
const double TEMP_MIN_CIRC = 0;
const double TEMP_MAX_CIRC = 60;

const int RADIATOR_WARMUP_SEC = 8 * 60;                    // 8 minutes
const int RADIATOR_COOLDOWN_SEC = 1 * 60 * 60 + (30 * 60); // 1 hour and 30 min

atomic_bool g_running = true;
pthread_mutex_t g_mutex;
pthread_cond_t g_cond;
