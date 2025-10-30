#ifndef REQUEST_H
#define REQUEST_H

#include "mongoose.h"
#include "src/globals.h"
#include <stdatomic.h>
#include <time.h>

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
    double Thottest;
    double Tcoldest;
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
};


#define TIMEOUT_BAS    1500
#define TIMEOUT_WTTRIN 5000

extern bool update_info_bas();
extern void update_info_bas_safe_swap(const struct bas_info* in, struct bas_info* out);
extern bool update_info_wttrin();
extern void update_info_wttrin_safe_swap(const char in[], char out[]);
extern enum RequestStatus request_send(struct Request* request);
extern enum RequestStatus request_send_quick(const char* url);
extern char* request_status_to_str(enum RequestStatus status);
extern double extract_json_label(struct mg_str json_body, const char* label);
extern void remember_vars_do_action(int mod_rada, int StatusPumpe4, int TminLT, int TmidGE);

// request_vars.c
char* request_status_to_str(enum RequestStatus status);
char* request_status_to_smallstr(enum RequestStatus status);
bool request_status_failed(enum RequestStatus status);

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

extern long long g_global_unix_counter;

extern atomic_int g_auto_timer;
extern atomic_int g_auto_gas;
extern atomic_int g_auto_timer_seconds;
extern int g_auto_timer_started;
extern int g_auto_timer_seconds_elapsed;
extern char g_auto_timer_status[BIGBUFF];
extern char g_auto_gas_status[BIGBUFF];

extern int g_history_mode;
extern time_t g_history_mode_time_changed;
extern time_t g_history_mode_time_on;
extern time_t g_history_mode_time_off;
extern int g_history_gas;
extern time_t g_history_gas_time_changed;
extern time_t g_history_gas_time_on;
extern time_t g_history_gas_time_off;

#endif // REQUEST_H
