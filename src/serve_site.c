#include "debug.h"
#include "mongoose.h"
#include <stdatomic.h>

extern atomic_int g_auto_timer;
extern atomic_int g_auto_gas;
extern atomic_int g_auto_timer_seconds;

void serve_site(struct mg_connection* c, int ev, void* ev_data) {

    if (ev != MG_EV_HTTP_MSG) { return; }

    struct mg_http_message* hm = (struct mg_http_message*)ev_data;

    if (mg_match(hm->uri, mg_str("/api/get_timer_seconds"), NULL)) {
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"seconds\": %f}", atomic_load(&g_auto_timer_seconds));
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/set_timer_seconds"), NULL)) {
        double value;
        if (mg_json_get_num(hm->body, "$.seconds", &value)) {
            if (value > 0) { // Validate that it's a positive integer
                atomic_store(&g_auto_timer_seconds, value);
                mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"success\": true, \"seconds\": %f}", atomic_load(&g_auto_timer_seconds));
            } else {
                mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\": \"Invalid timer value\"}");
            }
        } else {
            mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\": \"Invalid JSON format\"}");
        }
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/state"), NULL)) {
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"auto_timer\": %d, \"auto_gas\": %d}", atomic_load(&g_auto_timer), atomic_load(&g_auto_gas));
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

    // Serve static files from the "static" directory
    struct mg_http_serve_opts opts = {.root_dir = "static"};
    mg_http_serve_dir(c, hm, &opts);
}
