#ifndef SERVE_SITE_UTILS_H
#define SERVE_SITE_UTILS_H

#include "mongoose.h"

extern int mg_str_contains(struct mg_str haystack, const char* needle);

extern void get_api_state(struct mg_connection* c, struct mg_http_message* hm);
extern void get_api_toggle_auto_timer(struct mg_connection* c, struct mg_http_message* hm);
extern void get_api_toggle_auto_gas(struct mg_connection* c, struct mg_http_message* hm);
extern void get_api_schedules(struct mg_connection* c, struct mg_http_message* hm);
extern void get_api_bas_heat_on(struct mg_connection* c, struct mg_http_message* hm);
extern void get_api_bas_heat_off(struct mg_connection* c, struct mg_http_message* hm);
extern void get_api_bas_gas_on(struct mg_connection* c, struct mg_http_message* hm);
extern void get_api_bas_gas_off(struct mg_connection* c, struct mg_http_message* hm);

extern void get_errors(struct mg_connection* c, struct mg_http_message* hm);
extern void get_requests(struct mg_connection* c, struct mg_http_message* hm);
extern void get_changes(struct mg_connection* c, struct mg_http_message* hm);
extern void get_wttrin(struct mg_connection* c, struct mg_http_message* hm);

extern void get_c(struct mg_connection* c, struct mg_http_message* hm);
extern void get_sumtime(struct mg_connection* c, struct mg_http_message* hm);

extern void post_api_set_timer_seconds(struct mg_connection* c, struct mg_http_message* hm);

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

#endif // SERVE_SITE_UTILS_H
