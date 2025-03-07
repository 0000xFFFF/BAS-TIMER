#include "reqworker.h"
#include "debug.h"
#include "drawui.h"
#include "globals.h"
#include "logger.h"
#include "mongoose.h"
#include "reqworker.h"
#include "spinners.h"
#include "utils.h"
#include <float.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define URL_VARS "http://192.168.1.250/isc/get_var_js.aspx?StatusPumpe3=&StatusPumpe4=&StatusPumpe5=&StatusPumpe6=&StatusPumpe7=&Taktualno=&Tfs=&Tmax=&Tmin=&Tsobna=&Tsolar=&Tspv=&Tzad_komf=&Tzad_mraz=&Tzad_red=&Tzadata=&mod_rada=&mod_rezim=&__Time=&__Date=&Jeftina_tarifa=&grejanje_off=&Alarm_tank=&Alarm_solar=&STATE_Preklopka=&SESSIONID=-1"
#define URL_OFF "http://192.168.1.250/isc/set_var.aspx?mod_rada=0,-1&=&SESSIONID=-1"
#define URL_ON "http://192.168.1.250/isc/set_var.aspx?mod_rada=1,-1&=&SESSIONID=-1"
#define URL_GAS_OFF "http://192.168.1.250/isc/set_var.aspx?RezimRadaPumpe4=0,-1&=&SESSIONID=-1"
#define URL_GAS_ON "http://192.168.1.250/isc/set_var.aspx?RezimRadaPumpe4=3,-1&=&SESSIONID=-1"

static const char* s_url = NULL;
static const uint64_t s_timeout_ms = 1500;

#define ERROR_NONE 0
#define ERROR_TIMEOUT 1
#define ERROR_CONN 2
static int s_errors = 0;
static int s_remember_response = 0;
static struct mg_str s_response_body = {0};

struct bas_info g_info = {0};

long long GLOBAL_UNIX_COUNTER = 0;

atomic_int g_auto_timer;
atomic_int g_auto_gas;
atomic_int g_auto_timer_seconds;
int g_auto_timer_started = 0;
int g_auto_timer_seconds_elapsed = 0;
char g_auto_timer_status[STATUS_BUFFER_SIZE] = "...";
char g_auto_gas_status[STATUS_BUFFER_SIZE] = "...";

int g_history_mode = -1;
time_t g_history_mode_time_changed = 0;
time_t g_history_mode_time_on = 0;
time_t g_history_mode_time_off = 0;
int g_history_gas = -1;
time_t g_history_gas_time_changed = 0;
time_t g_history_gas_time_on = 0;
time_t g_history_gas_time_off = 0;

void init_reqworker() {
    // for req
    GLOBAL_UNIX_COUNTER = timestamp();

    atomic_init(&g_auto_timer, ENABLE_AUTO_TIMER);
    atomic_init(&g_auto_gas, ENABLE_AUTO_GAS);
    atomic_init(&g_auto_timer_seconds, AUTO_TIMER_SECONDS);

    // for drawui
    init_spinners();
}

char* sendreq_error_to_str(int e) {
    switch (e) {
    case 0:
        return "no error";
        break;
    case 1:
        return "timeout";
        break;
    case 2:
        return "connection error";
        break;
    }
    return "?";
}

static char* REQUEST_FORMAT =
    "GET %s HTTP/1.0\r\n"
    "Host: %.*s\r\n"
    "Accept: application/json, text/javascript, */*; q=0.01\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: en-US,en;q=0.9\r\n"
    "Connection: keep-alive\r\n"
    "Cookie: i18next=srb\r\n"
    "Referer: http://192.168.1.250/\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36\r\n"
    "X-Requested-With: XMLHttpRequest\r\n"
    "\r\n";

// Print HTTP response and signal that we're done
static void fn(struct mg_connection* c, int ev, void* ev_data) {
    if (ev == MG_EV_OPEN) {
        // Connection created. Store connect expiration time in c->data
        *(uint64_t*)c->data = mg_millis() + s_timeout_ms;
        return;
    }

    if (ev == MG_EV_POLL) {
        if (mg_millis() > *(uint64_t*)c->data &&
            (c->is_connecting || c->is_resolving)) {
            mg_error(c, "Connect timeout");
            s_errors = ERROR_TIMEOUT;
        }
        return;
    }

    if (ev == MG_EV_CONNECT) {
        // Connected to server. Extract host name from URL
        struct mg_str host = mg_url_host(s_url);

        if (mg_url_is_ssl(s_url)) {
            struct mg_tls_opts opts = {.ca = mg_unpacked("/certs/ca.pem"),
                                       .name = mg_url_host(s_url)};
            mg_tls_init(c, &opts);
        }

        // Send request
        mg_printf(c, REQUEST_FORMAT, mg_url_uri(s_url), (int)host.len, host.buf);
        return;
    }

    if (ev == MG_EV_HTTP_MSG) {
        if (s_remember_response) {
            // Response received
            struct mg_http_message* hm = (struct mg_http_message*)ev_data;
            s_response_body = mg_strdup(hm->body);
        }

        c->is_draining = 1;        // Tell mongoose to close this connection
        *(bool*)c->fn_data = true; // Tell event loop to stop
        return;
    }

    if (ev == MG_EV_ERROR) {
        *(bool*)c->fn_data = true; // Error, tell event loop to stop
        s_errors = ERROR_CONN;
        return;
    }
}

int sendreq(const char* url, int log, int remember_response) {
    char request_url[REQUEST_URL_BUFFER_SIZE];
    snprintf(request_url, REQUEST_URL_BUFFER_SIZE, "%s&_=%lld", url, GLOBAL_UNIX_COUNTER);
    if (log) { logger_requests_write("%s\n", request_url); }

    s_remember_response = remember_response;
    s_errors = 0;                            // RESET ERRORS
    struct mg_mgr mgr;                       // Event manager
    int done = 0;                            // Event handler flips it to true
    mg_mgr_init(&mgr);                       // Initialise event manager
    s_url = url;                             // set url
    mg_http_connect(&mgr, s_url, fn, &done); // Create client connection
    while (!done) mg_mgr_poll(&mgr, 50);     // Event manager loops until 'done'
    mg_mgr_free(&mgr);                       // Free resources

    if (s_errors) {
        logger_errors_write("%s -- %s\n", request_url, sendreq_error_to_str(s_errors));
    }

    return s_errors == 0;
}

double extract(struct mg_str json_body, const char* label) {
    D(printf("%s", json_body.buf));
    D(printf(" -- extract: %s", label));
    struct mg_str tok = mg_json_get_tok(json_body, label);

    double value = DBL_MIN;
    mg_json_get_num(tok, "$.value", &value);

    D(printf(" -> %f", value));
    return value;
}

void update_history(int mod_rada, int StatusPumpe4) {

    DPL("UPDATE HISTORY");
    time_t current_time;
    time(&current_time);
    char* t = get_current_time();

    if (g_history_mode == -1 || g_history_mode != mod_rada) {
        g_history_mode = mod_rada;
        g_history_mode_time_changed = current_time;

        if (mod_rada) {
            g_history_mode_time_on = g_history_mode_time_changed;
            logger_changes_write("mod_rada = %d\n", mod_rada);
            snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, " %s 󰐸", t);
        } else {
            g_history_mode_time_off = g_history_mode_time_changed;
            char e[100] = "\n";
            char p[100] = "";
            if (g_history_mode_time_on && g_history_mode_time_off) {
                // Assuming elapsed_str() will return a string with the elapsed time
                char* elap = elapsed_str(g_history_mode_time_off, g_history_mode_time_on);
                snprintf(e, sizeof(e), " -- %s\n", elap);
                snprintf(p, sizeof(p), " 󱫐 %s", elap);
                free(elap);
            }

            logger_changes_write("mod_rada = %d%s", mod_rada, e);
            snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, " %s %s", t, p);

            if (g_auto_timer_started) {
                g_auto_timer_started = 0;
                snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, "%s 󰜺", t);
            }
        }
    }

    if (g_history_gas == -1 || g_history_gas != StatusPumpe4) {
        g_history_gas = StatusPumpe4;
        g_history_gas_time_changed = current_time;

        if (StatusPumpe4) {
            g_history_gas_time_on = g_history_gas_time_changed;
            logger_changes_write("StatusPumpe4 = %d\n", StatusPumpe4);
            snprintf(g_auto_gas_status, STATUS_BUFFER_SIZE, " %s ", t);
        } else {
            g_history_gas_time_off = g_history_gas_time_changed;
            char e[100] = "\n";
            char p[100] = "";
            if (g_history_gas_time_on && g_history_gas_time_off) {
                // Assuming elapsed_str() will return a string with the elapsed time
                char* elap = elapsed_str(g_history_gas_time_off, g_history_gas_time_on);
                snprintf(e, sizeof(e), " -- %s\n", elap);
                snprintf(p, sizeof(p), " 󱫐 %s", elap);
                free(elap);
            }

            logger_changes_write("StatusPumpe4 = %d%s", StatusPumpe4, e);
            sprintf(g_auto_gas_status, " %s %s", t, p);
        }
    }

    free(t);
}

void do_logic_timer(int mod_rada) {

    time_t current_time;
    time(&current_time);

    char* t = get_current_time();

    if (g_auto_timer && mod_rada) {
        if (g_auto_timer_started) {
            g_auto_timer_seconds_elapsed = difftime(current_time, g_history_mode_time_on);
            snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, "%d/%d", g_auto_timer_seconds_elapsed, atomic_load(&g_auto_timer_seconds));

            if (g_auto_timer_seconds_elapsed >= g_auto_timer_seconds) {
                g_auto_timer_started = 0;
                snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, "%s 󱪯", t);
                if (g_history_mode_time_on) {
                    char* elap = elapsed_str(time(NULL), g_history_mode_time_on);
                    snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, "󱫐 %s 󱪯", elap);
                    free(elap);
                }
                sendreq(URL_OFF, 1, 0);
            }
        } else {
            g_auto_timer_started = 1;
            snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, "%s 󱫌", t);
        }
    }

    free(t);
}

void do_logic_gas(int StatusPumpe4, int TminLT, int TmidGE) {

    char* t = get_current_time();

    if (g_auto_gas && StatusPumpe4 == 0 && TminLT) {
        sprintf(g_auto_gas_status, "%s ", t);
        sendreq(URL_GAS_ON, 1, 0);
    }

    if (g_auto_gas && StatusPumpe4 == 3 && TmidGE) {
        sprintf(g_auto_gas_status, "%s 󰙇", t);
        if (g_history_gas_time_on && g_history_gas_time_off) {
            sprintf(g_auto_gas_status, "󱫐 %s 󰙇", elapsed_str(time(NULL), g_history_gas_time_on));
        }
        sendreq(URL_GAS_OFF, 1, 0);
    }

    free(t);
}

void remember_vars_do_action(int mod_rada, int StatusPumpe4, int TminLT, int TmidGE) {
    update_history(mod_rada, StatusPumpe4);
    do_logic_timer(mod_rada);
    do_logic_gas(StatusPumpe4, TminLT, TmidGE);
}

extern double g_temp_buf_max;
extern double g_temp_buf_min;

void update_info() {
    GLOBAL_UNIX_COUNTER++;

    // get request, parse response
    sendreq(URL_VARS, 0, 1);
    if (s_response_body.buf) {
        g_info.hasValues = 1;
        g_info.mod_rada = extract(s_response_body, "$.mod_rada");
        g_info.mod_rezim = extract(s_response_body, "$.mod_rezim");
        g_info.StatusPumpe3 = extract(s_response_body, "$.StatusPumpe3");
        g_info.StatusPumpe4 = extract(s_response_body, "$.StatusPumpe4");
        g_info.StatusPumpe5 = extract(s_response_body, "$.StatusPumpe5");
        g_info.StatusPumpe6 = extract(s_response_body, "$.StatusPumpe6");
        g_info.StatusPumpe7 = extract(s_response_body, "$.StatusPumpe7");
        g_info.Tspv = extract(s_response_body, "$.Tspv");
        g_info.Tsolar = extract(s_response_body, "$.Tsolar");
        g_info.Tzadata = extract(s_response_body, "$.Tzadata");
        g_info.Tfs = extract(s_response_body, "$.Tfs");
        g_info.Tmax = extract(s_response_body, "$.Tmax");
        g_info.Tmin = extract(s_response_body, "$.Tmin");
        g_info.Tsobna = extract(s_response_body, "$.Tsobna");

        // calc other values
        g_info.Tmid = (g_info.Tmax + g_info.Tmin) / 2;
        g_info.Thottest = g_temp_buf_max;
        g_info.Tcoldest = g_temp_buf_min;
        g_info.TminLT = g_info.Tmin < 45;
        g_info.TmidGE = g_info.Tmid >= 60;

        // free buffer
        free((void*)s_response_body.buf);
        s_response_body.buf = NULL;
    }

    draw_ui(g_info, 1, s_errors);

    remember_vars_do_action(g_info.mod_rada, g_info.StatusPumpe4, g_info.TminLT, g_info.TmidGE);
}

static int request_count = 0;
extern char g_term_buffer[];

void reqworker_do_work() {

    request_count++;

    if (g_term_buffer[0] == 0 || request_count >= DO_REQUEST_COUNT) {
        request_count = 0;
        update_info();
        return;
    }

    draw_ui(g_info, 0, s_errors);
}
