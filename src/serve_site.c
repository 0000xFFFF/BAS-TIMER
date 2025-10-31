#include "debug.h"
#include "logger.h"
#include "mongoose.h"
#include "request.h"
#include "src/draw_ui.h"
#include <stdatomic.h>

static int mg_str_contains(struct mg_str haystack, const char* needle)
{
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
        struct bas_info info = {0};
        update_info_bas_safe_io(&g_info, &info);
        if (!info.valid) {
            mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't get state\"}");
            return;
        }

        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                      "{"
                      "\"seconds\": %d"
                      ","
                      "\"auto_timer\": %d"
                      ","
                      "\"auto_gas\": %d"
                      "}",
                      info.opt_auto_timer_seconds,
                      info.opt_auto_timer,
                      info.opt_auto_gas);
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/set_timer_seconds"), NULL)) {
        double value;
        if (!mg_json_get_num(hm->body, "$.seconds", &value)) { return mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\": \"Invalid JSON format\"}"); }
        if (value <= 0) { return mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\": \"Invalid timer value\"}"); }

        struct bas_info info = {0};
        update_info_bas_safe_io(&g_info, &info);
        if (!info.valid) { return mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't get state\"}"); }

        info.opt_auto_timer_seconds = value;
        snprintf(info.opt_auto_timer_status, MIDBUFF, "changed to: %d", info.opt_auto_timer_seconds);

        update_info_bas_safe_io(&info, &g_info);

        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"success\": true, \"seconds\": %d}", info.opt_auto_timer_seconds);
        draw_ui_and_front();
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/toggle_auto_timer"), NULL)) {

        struct bas_info info = {0};
        update_info_bas_safe_io(&g_info, &info);
        if (!info.valid) { return mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't get state\"}"); }

        info.opt_auto_timer = !info.opt_auto_timer;
        update_info_bas_safe_io(&info, &g_info);
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"auto_timer\": %d}", info.opt_auto_timer);
        draw_ui_and_front();
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/toggle_auto_gas"), NULL)) {
        struct bas_info info = {0};
        update_info_bas_safe_io(&g_info, &info);
        if (!info.valid) { return mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't get state\"}"); }

        info.opt_auto_gas = !info.opt_auto_gas;
        update_info_bas_safe_io(&info, &g_info);
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"auto_gas\": %d}", info.opt_auto_gas);
        draw_ui_and_front();
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/bas_heat_on"), NULL)) {
        int r = request_send_quick(URL_HEAT_ON);
        mg_http_reply(c, 200, "Content-Type: text/plain", "bas_heat_on - %s", request_status_to_str(r));
        draw_ui_and_front();
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/bas_heat_off"), NULL)) {
        int r = request_send_quick(URL_HEAT_OFF);
        mg_http_reply(c, 200, "Content-Type: text/plain", "bas_heat_off - %s", request_status_to_str(r));
        draw_ui_and_front();
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/bas_gas_on"), NULL)) {
        int r = request_send_quick(URL_GAS_ON);
        mg_http_reply(c, 200, "Content-Type: text/plain", "bas_gas_on - %s", request_status_to_str(r));
        draw_ui_and_front();
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/bas_gas_off"), NULL)) {
        int r = request_send_quick(URL_GAS_OFF);
        mg_http_reply(c, 200, "Content-Type: text/plain", "bas_gas_off - %s", request_status_to_str(r));
        draw_ui_and_front();
        return;
    }

    if (mg_match(hm->uri, mg_str("/errors"), NULL)) {
        struct mg_http_serve_opts opts = {.mime_types = "text/plain"};
        DPL("SERVER errors.txt");
        mg_http_serve_file(c, hm, STATE_DIR_FILE_ERRORS_LOG, &opts);
        return;
    }

    if (mg_match(hm->uri, mg_str("/requests"), NULL)) {
        struct mg_http_serve_opts opts = {.mime_types = "text/plain"};
        DPL("SERVER requests.txt");
        mg_http_serve_file(c, hm, STATE_DIR_FILE_REQUESTS_LOG, &opts);
        return;
    }

    if (mg_match(hm->uri, mg_str("/changes"), NULL)) {
        struct mg_http_serve_opts opts = {.mime_types = "text/plain"};
        DPL("SERVER changes.txt");
        mg_http_serve_file(c, hm, STATE_DIR_FILE_CHANGES_LOG, &opts);
        return;
    }

    if (mg_match(hm->uri, mg_str("/api/sumtime"), NULL)) {

        char sumtime1[BIGBUFF] = {0};
        char sumtime2[BIGBUFF] = {0};

        int r1 = logger_changes_sumtime(sumtime1, sizeof(sumtime1), "mod_rada = 0 -- ");
        int r2 = logger_changes_sumtime(sumtime2, sizeof(sumtime2), "StatusPumpe4 = 0 -- ");
        if (r1 == -1 || r2 == -1) { return mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't sum time\"}"); }

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

    struct mg_http_serve_opts opts = {.root_dir = STATIC_DIR};
    mg_http_serve_dir(c, hm, &opts);
}
