#include "debug.h"
#include "logger.h"
#include "mongoose.h"
#include "request.h"
#include <stdatomic.h>

extern atomic_int g_auto_timer;
extern atomic_int g_auto_gas;
extern atomic_int g_auto_timer_seconds;

static int mg_str_contains(struct mg_str haystack, const char *needle) {
    if (haystack.len == 0 || !needle) return 0;
    for (size_t i = 0; i + strlen(needle) <= haystack.len; i++) {
        if (strncmp(haystack.buf + i, needle, strlen(needle)) == 0) {
            return 1;
        }
    }
    return 0;
}

void serve_site(struct mg_connection* c, int ev, void* ev_data)
{

    if (ev != MG_EV_HTTP_MSG) { return; }

    struct mg_http_message* hm = (struct mg_http_message*)ev_data;

    if (mg_match(hm->uri, mg_str("/api/state"), NULL)) {
        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                      "{"
                      "\"seconds\": %d"
                      ","
                      "\"auto_timer\": %d"
                      ","
                      "\"auto_gas\": %d"
                      "}",
                      atomic_load(&g_auto_timer_seconds),
                      atomic_load(&g_auto_timer),
                      atomic_load(&g_auto_gas));
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/set_timer_seconds"), NULL)) {
        double value;
        if (mg_json_get_num(hm->body, "$.seconds", &value)) {
            if (value > 0) { // Validate that it's a positive integer
                atomic_store(&g_auto_timer_seconds, value);
                mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"success\": true, \"seconds\": %d}", atomic_load(&g_auto_timer_seconds));
            }
            else {
                mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\": \"Invalid timer value\"}");
            }
        }
        else {
            mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\": \"Invalid JSON format\"}");
        }
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/toggle_auto_timer"), NULL)) {
        g_auto_timer = !g_auto_timer;
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"auto_timer\": %d}", atomic_load(&g_auto_timer));
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/toggle_auto_gas"), NULL)) {
        g_auto_gas = !g_auto_gas;
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"auto_gas\": %d}", atomic_load(&g_auto_gas));
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/bas_heat_on"), NULL)) {
        int r = requests_send_bas(URL_HEAT_ON, 1, 0);
        mg_http_reply(c, 200, "Content-Type: text/plain", "bas_heat_on - %s", request_status_to_str(r));
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/bas_heat_off"), NULL)) {
        int r = requests_send_bas(URL_HEAT_OFF, 1, 0);
        mg_http_reply(c, 200, "Content-Type: text/plain", "bas_heat_off - %s", request_status_to_str(r));
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/bas_gas_on"), NULL)) {
        int r = requests_send_bas(URL_GAS_ON, 1, 0);
        mg_http_reply(c, 200, "Content-Type: text/plain", "bas_gas_on - %s", request_status_to_str(r));
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/bas_gas_off"), NULL)) {
        int r = requests_send_bas(URL_GAS_OFF, 1, 0);
        mg_http_reply(c, 200, "Content-Type: text/plain", "bas_gas_off - %s", request_status_to_str(r));
        return;
    }

    if (mg_match(hm->uri, mg_str("/errors"), NULL)) {
        struct mg_http_serve_opts opts = {.mime_types = "text/plain"};
        DPL("SERVER errors.txt");
        mg_http_serve_file(c, hm, "./errors.log", &opts);
        return;
    }

    if (mg_match(hm->uri, mg_str("/requests"), NULL)) {
        struct mg_http_serve_opts opts = {.mime_types = "text/plain"};
        DPL("SERVER requests.txt");
        mg_http_serve_file(c, hm, "./requests.log", &opts);
        return;
    }

    if (mg_match(hm->uri, mg_str("/changes"), NULL)) {
        struct mg_http_serve_opts opts = {.mime_types = "text/plain"};
        DPL("SERVER changes.txt");
        mg_http_serve_file(c, hm, "./changes.log", &opts);
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/sumtime"), NULL)) {

#define BUFFER_SIZE 1024
        char sumtime1[BUFFER_SIZE] = {0};
        char sumtime2[BUFFER_SIZE] = {0};

        int r1 = logger_sumtime(sumtime1, BUFFER_SIZE, "changes.log", "mod_rada = 0 -- ");
        int r2 = logger_sumtime(sumtime2, BUFFER_SIZE, "changes.log", "StatusPumpe4 = 0 -- ");
        if (r1 == -1 || r2 == -1) {
            mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't sum time\"}");
            return;
        }

        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                      "{"
                      "\"mod_rada\": \"%s\""
                      ","
                      "\"StatusPumpe4\": \"%s\""
                      "}",
                      sumtime1,
                      sumtime2);

        return;
    }

    // if on mobile serve diff index
    if (mg_match(hm->uri, mg_str("/"), NULL)) {
        struct mg_str* ua = mg_http_get_header(hm, "User-Agent");
        if (ua != NULL) {
            if (mg_str_contains(*ua, "Mobi") || mg_str_contains(*ua, "Android")) {
                struct mg_http_serve_opts opts = {0};
                mg_http_serve_file(c, hm, "static/mobile.html", &opts);
                DPL("SERVE MOBILE");
                return;
            }
        }
    }

    struct mg_http_serve_opts opts = {.root_dir = "static"};
    mg_http_serve_dir(c, hm, &opts);
}
