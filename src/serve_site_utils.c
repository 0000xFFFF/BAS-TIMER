#include "serve_site_utils.h"
#include "debug.h"
#include "draw_ui.h"
#include "logger.h"
#include "mongoose.h"
#include "request.h"
#include "schedules.h"
#include "serve_websocket.h"
#include "utils.h"

int mg_str_contains(struct mg_str haystack, const char* needle)
{
    if (haystack.len == 0 || !needle) return 0;
    for (size_t i = 0; i + strlen(needle) <= haystack.len; i++) {
        if (strncmp(haystack.buf + i, needle, strlen(needle)) == 0) {
            return 1;
        }
    }
    return 0;
}

static void get_api_state(struct mg_connection* c, struct mg_http_message* hm)
{
    struct BasInfo info = {0};
    infos_bas_safe_io(&g_infos.bas, &info);
    if (!info.valid) {
        mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't get state\"}");
        return;
    }

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "{"
                  "\"seconds\": %d"
                  ","
                  "\"auto_timer\": %s"
                  ","
                  "\"auto_gas\": %s"
                  "}",
                  info.opt_auto_timer_seconds,
                  bool_to_str(info.opt_auto_timer),
                  bool_to_str(info.opt_auto_gas));
}

static void post_api_set_timer_seconds(struct mg_connection* c, struct mg_http_message* hm)
{

    double value;
    if (!mg_json_get_num(hm->body, "$.seconds", &value)) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\": \"Invalid JSON format\"}");
        return;
    }
    if (value <= 0) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\": \"Invalid timer value\"}");
        return;
    }

    struct BasInfo info = {0};
    infos_bas_safe_io(&g_infos.bas, &info);
    if (!info.valid) {
        mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't get state\"}");
        return;
    }

    info.opt_auto_timer_seconds = (int)value;
    info.opt_auto_timer_status = OPT_STATUS_CHANGED;

    infos_bas_safe_io(&info, &g_infos.bas);

    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"success\": true, \"seconds\": %d}", info.opt_auto_timer_seconds);
    draw_ui_and_front();
    return;
}

static void get_api_toggle_auto_timer(struct mg_connection* c, struct mg_http_message* hm)
{
    struct BasInfo info = {0};
    infos_bas_safe_io(&g_infos.bas, &info);
    if (!info.valid) {
        mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't get state\"}");
        return;
    }

    info.opt_auto_timer = !info.opt_auto_timer;
    infos_bas_safe_io(&info, &g_infos.bas);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"value\": %s}", bool_to_str(info.opt_auto_timer));
    draw_ui_and_front();
}

static void get_api_toggle_auto_gas(struct mg_connection* c, struct mg_http_message* hm)
{
    struct BasInfo info = {0};
    infos_bas_safe_io(&g_infos.bas, &info);
    if (!info.valid) {
        mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't get state\"}");
        return;
    }

    info.opt_auto_gas = !info.opt_auto_gas;
    infos_bas_safe_io(&info, &g_infos.bas);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"value\": %s}", bool_to_str(info.opt_auto_gas));
    draw_ui_and_front();
}

static void get_api_schedules(struct mg_connection* c, struct mg_http_message* hm)
{

    struct BasInfo info = {0};
    infos_bas_safe_io(&g_infos.bas, &info);

    char buf[HEAT_SCHEDULES_COUNT * (64 + 10)];
    size_t len = 0;

    len += snprintf(buf + len, sizeof(buf) - len, "{ \"schedules\": [");

    bool first = true;

    struct HeatScheduleNode* node = gl_schedules;
    while (node != NULL) {
        struct HeatSchedule* s = &node->data;

        if (!first) len += snprintf(buf + len, sizeof(buf) - len, ",");
        first = false;

        len += snprintf(buf + len, sizeof(buf) - len, "{ \"from\": %d, \"to\": %d, \"duration\": %d }", s->from, s->to, s->duration);

        node = node->next;
    }

    len += snprintf(buf + len, sizeof(buf) - len, "] }");

    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%.*s", (int)len, buf);
    return;
}

static void get_api_bas_heat_on(struct mg_connection* c, struct mg_http_message* hm)
{
    enum RequestStatus r = request_send_quick(URL_HEAT_ON);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"bas_heat_on\": \"%s\"}", request_status_to_str(r));
    draw_ui_and_front();
}

static void get_api_bas_heat_off(struct mg_connection* c, struct mg_http_message* hm)
{
    enum RequestStatus r = request_send_quick(URL_HEAT_OFF);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"bas_heat_off\": \"%s\"}", request_status_to_str(r));
    draw_ui_and_front();
}

static void get_api_bas_gas_on(struct mg_connection* c, struct mg_http_message* hm)
{
    enum RequestStatus r = request_send_quick(URL_GAS_ON);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"bas_gas_on\": \"%s\"}", request_status_to_str(r));
    draw_ui_and_front();
}

static void get_api_bas_gas_off(struct mg_connection* c, struct mg_http_message* hm)
{
    enum RequestStatus r = request_send_quick(URL_GAS_OFF);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"bas_gas_off\": \"%s\"}", request_status_to_str(r));
    draw_ui_and_front();
}

static void get_errors(struct mg_connection* c, struct mg_http_message* hm)
{
    struct mg_http_serve_opts opts = {.mime_types = "text/plain"};
    mg_http_serve_file(c, hm, VAR_DIR_FILE_ERRORS_LOG, &opts);
}

static void get_requests(struct mg_connection* c, struct mg_http_message* hm)
{
    struct mg_http_serve_opts opts = {.mime_types = "text/plain"};
    mg_http_serve_file(c, hm, VAR_DIR_FILE_REQUESTS_LOG, &opts);
}

static void get_changes(struct mg_connection* c, struct mg_http_message* hm)
{
    struct mg_http_serve_opts opts = {.mime_types = "text/plain"};
    mg_http_serve_file(c, hm, VAR_DIR_FILE_CHANGES_LOG, &opts);
}

static void get_wttrin(struct mg_connection* c, struct mg_http_message* hm)
{
    struct mg_http_serve_opts opts = {.mime_types = "text/plain"};
    mg_http_serve_file(c, hm, VAR_DIR_FILE_WTTRIN_LOG, &opts);
}

static void get_c(struct mg_connection* c, struct mg_http_message* hm)
{
    DPL("SERVER c");
    char buffer[WS_MAX_CONN * 32] = {0};
    write_conn_to_buffer_safe(buffer, sizeof(buffer));
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "%s", buffer);
}

static void get_sumtime(struct mg_connection* c, struct mg_http_message* hm)
{
    char sumtime1[BIGBUFF] = {0};
    char sumtime2[BIGBUFF] = {0};

    size_t r1 = logger_changes_sumtime(sumtime1, sizeof(sumtime1), "mod_rada = 0 -- ");
    size_t r2 = logger_changes_sumtime(sumtime2, sizeof(sumtime2), "StatusPumpe4 = 0 -- ");
    if (r1 == 0 || r2 == 0) {
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

struct Route {
    const char* method; // "GET", "POST", etc.
    const char* path;   // URI string
    void (*handler)(struct mg_connection*, struct mg_http_message*); // function pointer
};

static struct Route routes[] = {
    { "GET",  "/api/state",             (void*)get_api_state },
    { "GET",  "/api/toggle_auto_timer", (void*)get_api_toggle_auto_timer },
    { "GET",  "/api/toggle_auto_gas",   (void*)get_api_toggle_auto_gas },
    { "GET",  "/api/bas_heat_on",       (void*)get_api_bas_heat_on },
    { "GET",  "/api/bas_heat_off",      (void*)get_api_bas_heat_off },
    { "GET",  "/api/bas_gas_on",        (void*)get_api_bas_gas_on },
    { "GET",  "/api/bas_gas_off",       (void*)get_api_bas_gas_off },
    { "GET",  "/errors",                (void*)get_errors },
    { "GET",  "/requests",              (void*)get_requests },
    { "GET",  "/changes",               (void*)get_changes },
    { "GET",  "/wttrin",                (void*)get_wttrin },
    { "GET",  "/c",                     (void*)get_c },
    { "GET",  "/api/sumtime",           (void*)get_sumtime },
    { "GET",  "/api/schedules",         (void*)get_api_schedules },
    { "POST", "/api/set_timer_seconds", (void*)post_api_set_timer_seconds },
};
static const size_t routes_count = sizeof(routes) / sizeof(routes[0]);

int serve_site_handle_route(struct mg_connection* c, struct mg_http_message* hm)
{
    for (size_t i = 0; i < routes_count; i++) {
        if (mg_strcmp(hm->method, mg_str(routes[i].method)) == 0 &&
            mg_match(hm->uri, mg_str(routes[i].path), NULL)) {
            routes[i].handler(c, hm);
            return 1; // handled
        }
    }
    return 0; // not handled
}
