#include "serve_site_endpoints.h"
#include "debug.h"
#include "draw_ui.h"
#include "logger.h"
#include "mongoose.h"
#include "request.h"
#include "schedules.h"
#include "serve_websocket.h"
#include "utils.h"
#include <float.h>

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
    UNUSED(hm);

    struct BasInfo info = {0};
    infos_bas_safe_io(&g_infos.bas, &info);
    if (!info.valid) {
        mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't get state\"}");
        return;
    }

    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "{"
                  "\"seconds\": %" PRIu64
                  ","
                  "\"auto_timer\": %s"
                  ","
                  "\"auto_gas\": %s"
                  "}",
                  info.opt_auto_timer_seconds,
                  bool_to_str(info.opt_auto_timer),
                  bool_to_str(info.opt_auto_gas));
}

static void get_api_schedules(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

    struct BasInfo info = {0};
    infos_bas_safe_io(&g_infos.bas, &info);

    pthread_mutex_lock(&g_mutex_schedules);

    const uint64_t START_TEXT_LEN = 16;
    const uint64_t TEXT_LEN = 64;
    const uint64_t MAX_INT32_LEN = 10;
    const uint64_t MAX_INT64_LEN = 19;
    const uint64_t ELEMENT_LEN = TEXT_LEN + MAX_INT32_LEN * 2 + MAX_INT64_LEN * 2;
    const uint64_t ELEMENTS_LEN = g_schedules_count * ELEMENT_LEN;
    const uint64_t END_TEXT_LEN = 3;
    const uint64_t PADDING = 8;
    const uint64_t TOTAL_TEXT_SIZE = START_TEXT_LEN + ELEMENTS_LEN + END_TEXT_LEN + PADDING;
    char* buf = (char*)calloc((size_t)sizeof(char), (size_t)TOTAL_TEXT_SIZE);
    size_t len = 0;

    len += (size_t)snprintf(buf + len, (size_t)TOTAL_TEXT_SIZE - len, "{ \"schedules\": [");

    bool first = true;

    struct HeatScheduleNode* node = g_schedules;
    while (node != NULL) {
        struct HeatSchedule* s = &node->data;

        if (!first) len += (size_t)snprintf(buf + len, (size_t)TOTAL_TEXT_SIZE - len, ",");
        first = false;

        len += (size_t)snprintf(buf + len, (size_t)TOTAL_TEXT_SIZE - len, "{ \"id\": %" PRIu64 ", \"from\": %d, \"to\": %d, \"duration\": %" PRIu64 " }", node->id, s->from, s->to, s->duration);

        node = node->next;
    }

    len += (size_t)snprintf(buf + len, (size_t)TOTAL_TEXT_SIZE - len, "] }");

    pthread_mutex_unlock(&g_mutex_schedules);

    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%.*s", (int)len, buf);

    free(buf);
    return;
}

static double get_double(struct mg_connection* c, struct mg_http_message* hm, const char* label)
{
    double from = -1.0;
    if (!mg_json_get_num(hm->body, label, &from)) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\": \"Invalid JSON format\"}");
        return -1;
    }

    if (!(from >= 0 && from <= DBL_MAX)) {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\": \"Invalid value must be positive & under DBL_MAX\"}");
        return -2;
    }

    return from;
}

static void post_api_schedules(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

    double from_ = get_double(c, hm, "$.from");
    if (from_ < 0) { return; }
    double to_ = get_double(c, hm, "$.to");
    if (to_ < 0) { return; }
    double duration_ = get_double(c, hm, "$.duration");
    if (duration_ < 0) { return; }

    int from = (int)from_;
    int to = (int)to_;
    uint64_t duration = (uint64_t)duration_;

    schedules_create(from, to, duration);

    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"success\": true }");
}

static void post_api_schedules_defaults(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

    schedules_defaults();

    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"success\": true }");
}

static void delete_api_schedules(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

    double id_ = get_double(c, hm, "$.id");
    if (id_ < 0) { return; }

    uint64_t id = (uint64_t)id_;
    schedules_delete(id);

    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"success\": true, \"deleted\": %d}", id);
}

static void get_api_bas_heat_on(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

    enum RequestStatus r = request_send_quick(URL_HEAT_ON);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"bas_heat_on\": \"%s\"}", request_status_to_str(r));
}

static void get_api_bas_heat_off(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

    enum RequestStatus r = request_send_quick(URL_HEAT_OFF);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"bas_heat_off\": \"%s\"}", request_status_to_str(r));
}

static void get_api_bas_gas_on(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

    enum RequestStatus r = request_send_quick(URL_GAS_ON);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"bas_gas_on\": \"%s\"}", request_status_to_str(r));
}

static void get_api_bas_gas_off(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

    enum RequestStatus r = request_send_quick(URL_GAS_OFF);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"bas_gas_off\": \"%s\"}", request_status_to_str(r));
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
    UNUSED(hm);
    DPL("SERVER c");
    char buffer[WS_MAX_CONN * 32] = {0};
    write_conn_to_buffer_safe(buffer, sizeof(buffer));
    mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "%s", buffer);
}

static void get_sumtime(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

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

static void post_api_set_timer_seconds(struct mg_connection* c, struct mg_http_message* hm)
{

    double seconds_ = get_double(c, hm, "$.seconds");
    if (seconds_ < 0) { return; }

    struct BasInfo info = {0};
    infos_bas_safe_io(&g_infos.bas, &info);
    if (!info.valid) {
        mg_http_reply(c, 500, "Content-Type: application/json\r\n", "{\"error\": \"Can't get state\"}");
        return;
    }

    info.opt_auto_timer_seconds = (uint64_t)seconds_;
    info.opt_auto_timer_status = OPT_STATUS_CHANGED;

    infos_bas_safe_io(&info, &g_infos.bas);

    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"success\": true, \"seconds\": %ld}", info.opt_auto_timer_seconds);
    draw_ui_and_front();
    return;
}

static void post_api_toggle_auto_timer(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

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

static void post_api_toggle_auto_gas(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

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

static void get_api_times(struct mg_connection* c, struct mg_http_message* hm)
{
    UNUSED(hm);

    char* times = logger_get_mod_rada_intervals_today_json();
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", times);
    free(times);
}

typedef void (*route_handler_t)(struct mg_connection*, struct mg_http_message*);

struct Route {
    const char* method; // "GET", "POST", etc.
    const char* path;   // URI string
    route_handler_t handler;
};

// clang-format off
static struct Route routes[] = {
    { "GET",      "/api/state",                get_api_state },
    { "GET",      "/api/bas_heat_on",          get_api_bas_heat_on },
    { "GET",      "/api/bas_heat_off",         get_api_bas_heat_off },
    { "GET",      "/api/bas_gas_on",           get_api_bas_gas_on },
    { "GET",      "/api/bas_gas_off",          get_api_bas_gas_off },
    { "GET",      "/errors",                   get_errors },
    { "GET",      "/requests",                 get_requests },
    { "GET",      "/changes",                  get_changes },
    { "GET",      "/wttrin",                   get_wttrin },
    { "GET",      "/c",                        get_c },
    { "GET",      "/api/sumtime",              get_sumtime },
    { "GET",      "/api/schedules",            get_api_schedules },
    { "POST",     "/api/schedules",            post_api_schedules },
    { "POST",     "/api/schedules/defaults",   post_api_schedules_defaults },
    { "DELETE",   "/api/schedules",            delete_api_schedules },
    { "POST",     "/api/set_timer_seconds",    post_api_set_timer_seconds },
    { "POST",     "/api/toggle_auto_timer",    post_api_toggle_auto_timer },
    { "POST",     "/api/toggle_auto_gas",      post_api_toggle_auto_gas },
    { "GET",      "/api/times",                get_api_times },
};
// clang-format on

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
