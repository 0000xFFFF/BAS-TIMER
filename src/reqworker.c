#include "reqworker.h"
#include "colors.h"
#include "debug.h"
#include "drawui.h"
#include "globals.h"
#include "logger.h"
#include "mongoose.h"
#include "reqworker.h"
#include "utils.h"
#include <json-c/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define URL_VARS "http://192.168.1.250/isc/get_var_js.aspx?StatusPumpe3=&StatusPumpe4=&StatusPumpe5=&StatusPumpe6=&StatusPumpe7=&Taktualno=&Tfs=&Tmax=&Tmin=&Tsobna=&Tsolar=&Tspv=&Tzad_komf=&Tzad_mraz=&Tzad_red=&Tzadata=&mod_rada=&mod_rezim=&__Time=&__Date=&Jeftina_tarifa=&grejanje_off=&Alarm_tank=&Alarm_solar=&STATE_Preklopka=&SESSIONID=-1"
#define URL_OFF "http://192.168.1.250/isc/set_var.aspx?mod_rada=0,-1&=&SESSIONID=-1"
#define URL_ON "http://192.168.1.250/isc/set_var.aspx?mod_rada=1,-1&=&SESSIONID=-1"
#define URL_GAS_OFF "http://192.168.1.250/isc/set_var.aspx?RezimRadaPumpe4=0,-1&=&SESSIONID=-1"
#define URL_GAS_ON "http://192.168.1.250/isc/set_var.aspx?RezimRadaPumpe4=3,-1&=&SESSIONID=-1"

// The very first web page in history. You can replace it from command line
static const char* s_url = NULL;
static const uint64_t s_timeout_ms = 1500; // Connect timeout in milliseconds

#define ERROR_NONE 0
#define ERROR_TIMEOUT 1
#define ERROR_OTHER 2
static int errors = 0;
static char* response = NULL;

char* error_to_str(int e) {
    switch (e) {
        case 0: return "no error";      break;
        case 1: return "timeout";       break;
        case 2: return "unknown error"; break;
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
            errors = ERROR_TIMEOUT;
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

        D(
            printf("===[ BEGIN REQ ]===\n");
            printf(REQUEST_FORMAT, mg_url_uri(s_url), (int)host.len, host.buf);
            printf("===[ END REQ ]===\n"));

        // Send request
        mg_printf(c, REQUEST_FORMAT, mg_url_uri(s_url), (int)host.len, host.buf);
        return;
    }

    if (ev == MG_EV_HTTP_MSG) {
        // Response is received. Print it
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        D(
            printf("===[ BEGIN RECV ]===\n");
            printf("%.*s\n", (int)hm->message.len, hm->message.buf);
            printf("===[ END RECV ]===\n"););

        // response = std::string(hm->message.buf, hm->message.len);
        response = strdup(hm->message.buf);

        c->is_draining = 1;        // Tell mongoose to close this connection
        *(bool*)c->fn_data = true; // Tell event loop to stop
        return;
    }

    if (ev == MG_EV_ERROR) {
        *(bool*)c->fn_data = true; // Error, tell event loop to stop
        errors = ERROR_OTHER;
        return;
    }
}

int get_request(const char* url) {
    errors = 0; // RESET ERRORS
    struct mg_mgr mgr;                       // Event manager
    int done = 0;                            // Event handler flips it to true
    mg_mgr_init(&mgr);                       // Initialise event manager
    s_url = url;                             // set url
    mg_http_connect(&mgr, s_url, fn, &done); // Create client connection
    while (!done) mg_mgr_poll(&mgr, 50);     // Event manager loops until 'done'
    mg_mgr_free(&mgr);                       // Free resources
    return errors == 0;
}

// Function to extract JSON from HTTP response
// TODO: FIX USE mongoose way to extract HTML BODY
char* extract_json(const char* http_response) {
    const char* json_start = strstr(http_response, "\r\n\r\n");
    if (!json_start) return NULL;  // No JSON found
    return strdup(json_start + 4); // Skip "\r\n\r\n" to get the JSON part
}

double extract_d(struct json_object* parsed_json, const char* label) {
    // Extract Tmax
    struct json_object *obj, *value, *status;
    if (json_object_object_get_ex(parsed_json, label, &obj)) {
        json_object_object_get_ex(obj, "value", &value);
        json_object_object_get_ex(obj, "status", &status);
        double ret = json_object_get_double(value);
        D(printf("EXTRACT: %s - Value: %f, Status: %d\n", label, ret, json_object_get_int(status)));
        return ret;
    }
    return -1;
}

int extract_i(struct json_object* parsed_json, const char* label) {
    // Extract Tmax
    struct json_object *obj, *value, *status;
    if (json_object_object_get_ex(parsed_json, label, &obj)) {
        json_object_object_get_ex(obj, "value", &value);
        json_object_object_get_ex(obj, "status", &status);
        int ret = json_object_get_int(value);
        D(printf("EXTRACT: %s - Value: %d, Status: %d\n", label, ret, json_object_get_int(status)));
        return ret;
    }
    return -1;
}

struct bas_info info = {0};

long long GLOBAL_UNIX_COUNTER = 0;

void init_unix_global() {
    GLOBAL_UNIX_COUNTER = timestamp();
}

void sendreq(const char* url, int log) {

    char request_url[REQUEST_URL_BUFFER_SIZE];
    snprintf(request_url, REQUEST_URL_BUFFER_SIZE, "%s&_=%lld", url, GLOBAL_UNIX_COUNTER);
    if (log) { logger_requests_write("%s\n", request_url); }
    int res = get_request(request_url);
    if (!res) {
        logger_errors_write("%s -- %s\n", request_url, error_to_str(errors));
    }
}

int g_auto_timer = 1;
int g_auto_timer_started = 0;
double g_auto_timer_seconds = 900; // 15 mins;
double g_auto_timer_seconds_elapsed = 0;
char g_auto_timer_status[STATUS_BUFFER_SIZE] = "...";
int g_auto_gas = 1;
char g_auto_gas_status[STATUS_BUFFER_SIZE] = "...";

int g_history_mode = -1;
time_t g_history_mode_time_changed = 0;
time_t g_history_mode_time_on = 0;
time_t g_history_mode_time_off = 0;
int g_history_gas = -1;
time_t g_history_gas_time_changed = 0;
time_t g_history_gas_time_on = 0;
time_t g_history_gas_time_off = 0;

int get_auto_timer() { return g_auto_timer; }
void set_auto_timer(int val) { g_auto_timer = val; }
double get_auto_timer_seconds() { printf("GET_AUTO_TIMER_SECONDS: %f\n", g_auto_timer_seconds); return g_auto_timer_seconds; }
void set_auto_timer_seconds(double val) { g_auto_timer_seconds = val; }
int get_auto_gas() { return g_auto_gas; }
void set_auto_gas(int val) { g_auto_gas = val; }
char* get_auto_timer_status() { return g_auto_timer_status; }
char* get_auto_gas_status() { return g_auto_gas_status; }

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
            snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, "%.0f/%.0f", g_auto_timer_seconds_elapsed, g_auto_timer_seconds);

            if (g_auto_timer_seconds_elapsed >= g_auto_timer_seconds) {
                g_auto_timer_started = 0;
                snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, "%s 󱪯", t);
                if (g_history_mode_time_on) {
                    char* elap = elapsed_str(time(NULL), g_history_mode_time_on);
                    snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, "󱫐 %s 󱪯", elap);
                    free(elap);
                }
                sendreq(URL_OFF, 1);
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
        sendreq(URL_GAS_ON, 1);
    }

    if (g_auto_gas && StatusPumpe4 == 3 && TmidGE) {
        sprintf(g_auto_gas_status, "%s 󰙇", t);
        if (g_history_gas_time_on && g_history_gas_time_off) {
            sprintf(g_auto_gas_status, "󱫐 %s 󰙇", elapsed_str(time(NULL), g_history_gas_time_on));
        }
        sendreq(URL_GAS_OFF, 1);
    }

    free(t);
}

void remember_vars_do_action(int mod_rada, int StatusPumpe4, int TminLT, int TmidGE) {
    update_history(mod_rada, StatusPumpe4);
    do_logic_timer(mod_rada);
    do_logic_gas(StatusPumpe4, TminLT, TmidGE);
}

extern double g_temp_max;
extern double g_temp_min;

void update_info() {
    GLOBAL_UNIX_COUNTER++;

    // get request, parse response
    sendreq(URL_VARS, 0);
    if (errors) { return; }
    if (!response) { return; }

    char* json_str = extract_json(response);
    if (!json_str) {
        DPL("failed to extract JSON from HTTP response");
        return;
    }
    free(response);

    // Parse the JSON string
    struct json_object* parsed_json = json_tokener_parse(json_str);
    if (!parsed_json) {
        DPL("Error parsing JSON");
        return;
    }

    info.mod_rada = extract_i(parsed_json, "mod_rada");
    info.mod_rezim = extract_i(parsed_json, "mod_rezim");
    info.StatusPumpe3 = extract_i(parsed_json, "StatusPumpe3");
    info.StatusPumpe4 = extract_i(parsed_json, "StatusPumpe4");
    info.StatusPumpe5 = extract_i(parsed_json, "StatusPumpe5");
    info.StatusPumpe6 = extract_i(parsed_json, "StatusPumpe6");
    info.StatusPumpe7 = extract_i(parsed_json, "StatusPumpe7");
    info.Tspv = extract_d(parsed_json, "Tspv");
    info.Tsolar = extract_d(parsed_json, "Tsolar");
    info.Tzadata = extract_d(parsed_json, "Tzadata");
    info.Tfs = extract_d(parsed_json, "Tfs");
    info.Tmax = extract_d(parsed_json, "Tmax");
    info.Tmin = extract_d(parsed_json, "Tmin");
    info.Tsobna = extract_d(parsed_json, "Tsobna");

    // cal other values
    info.Tmid = (info.Tmax + info.Tmin) / 2;
    info.Thottest = g_temp_max;
    info.Tcoldest = g_temp_min;
    info.TminLT = info.Tmin < 45;
    info.TmidGE = info.Tmid >= 60;

    draw_ui(info, 1, errors);

    remember_vars_do_action(info.mod_rada, info.StatusPumpe4, info.TminLT, info.TmidGE);

    // Clean up JSON object
    json_object_put(parsed_json);
    free(json_str);
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

    draw_ui(info, 0, errors);
}
