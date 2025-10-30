#ifndef REQUEST_H
#define REQUEST_H

#include "mongoose.h"
#include "src/globals.h"
#include <stdatomic.h>

extern const char* const URL_VARS;
extern const char* const URL_HEAT_OFF;
extern const char* const URL_HEAT_ON;
extern const char* const URL_GAS_OFF;
extern const char* const URL_GAS_ON;

enum RequestStatus {
    REQUEST_STATUS_RUNNING = 0,
    REQUEST_STATUS_DONE = 1,
    REQUEST_STATUS_ERROR_TIMEOUT = 2,
    REQUEST_STATUS_ERROR_CONN = 4
};

struct Request {
    bool valid;
    bool log;
    enum RequestStatus status;
    const char* url;
    const char* request_format;
    uint64_t timeout_ms;
    uint64_t timeout_ms_start;
    int remember_response;
    struct mg_str output;
};

struct bas_info {

    bool valid; // is not empty
    enum RequestStatus status;

    // statuses
    int mod_rada;
    int mod_rezim;
    int StatusPumpe3;
    int StatusPumpe4;
    int StatusPumpe5;
    int StatusPumpe6;
    int StatusPumpe7;

    // temps
    double Tspv;
    double Tsolar;
    double Tzadata;
    double Tfs;
    double Tmax;
    double Tmin;
    double Tsobna;

    // other calced values
    double Tmid;
    int TmidGE;
    int TminLT;

    bool peaks_valid;
    double peak_min_solar;
    double peak_max_solar;
    double peak_min_human;
    double peak_max_human;
    double peak_min_buf;
    double peak_max_buf;
    double peak_min_circ;
    double peak_max_circ;

    int opt_auto_timer;
    int opt_auto_timer_seconds;
    int opt_auto_timer_seconds_old;
    int opt_auto_timer_started;
    int opt_auto_timer_seconds_elapsed;
    char opt_auto_timer_status[BIGBUFF];
    int opt_auto_gas;
    char opt_auto_gas_status[BIGBUFF];

    int history_mode;
    int history_mode_time_changed;
    int history_mode_time_on;
    int history_mode_time_off;
    int history_gas;
    int history_gas_time_changed;
    int history_gas_time_on;
    int history_gas_time_off;
};

#define TEMP_MIN_SOLAR 10
#define TEMP_MAX_SOLAR 100
#define TEMP_MIN_HUMAN 0
#define TEMP_MAX_HUMAN 30
#define TEMP_MIN_BUF 45
#define TEMP_MAX_BUF 70
#define TEMP_MIN_CIRC 0
#define TEMP_MAX_CIRC 30

#define TIMEOUT_BAS    1500
#define TIMEOUT_WTTRIN 5000

extern void update_info_bas_init();
extern bool update_info_bas();
extern void update_info_bas_safe_io(const struct bas_info* in, struct bas_info* out);
extern bool update_info_wttrin();
extern void update_info_wttrin_safe_io(const char in[], char out[]);
extern enum RequestStatus request_send(struct Request* request);
extern enum RequestStatus request_send_quick(const char* url);
extern char* request_status_to_str(enum RequestStatus status);
extern double extract_json_label(struct mg_str json_body, const char* label, double fallback);
extern void remember_vars_do_action(struct bas_info* info);

// request_vars.c
extern char* request_status_to_str(enum RequestStatus status);
extern char* request_status_to_smallstr(enum RequestStatus status);
extern bool request_status_failed(enum RequestStatus status);
extern void print_bas_info(const struct bas_info* b);

extern const char* const URL_VARS;
extern const char* const URL_HEAT_OFF;
extern const char* const URL_HEAT_ON;
extern const char* const URL_GAS_OFF;
extern const char* const URL_GAS_ON;

extern const char* const URL_WTTRIN;
extern const char* const REQUEST_FORMAT_BAS;
extern const char* const REQUEST_FORMAT_WTTRIN;

#define ERROR_NONE    0
#define ERROR_TIMEOUT 1
#define ERROR_CONN    2

extern struct bas_info g_info;

extern char g_wttrin_buffer[BIGBUFF];

#endif // REQUEST_H
