#include "serve_site.h"
#include "debug.h"
#include "globals.h"
#include "mongoose.h"
#include "serve_site_utils.h"
#include <stdatomic.h>

static int handle_route(struct mg_connection* c, struct mg_http_message* hm) {
    for (size_t i = 0; i < routes_count; i++) {
        if (mg_strcmp(hm->method, mg_str(routes[i].method)) == 0 &&
            mg_match(hm->uri, mg_str(routes[i].path), NULL)) {
            routes[i].handler(c, hm);
            return 1; // handled
        }
    }
    return 0; // not handled
}

void serve_site(struct mg_connection* c, int ev, void* ev_data) {
    if (ev != MG_EV_HTTP_MSG) return;

    struct mg_http_message* hm = (struct mg_http_message*)ev_data;

    // Try route table first
    if (handle_route(c, hm)) return;

    // Mobile index check
    if (mg_match(hm->uri, mg_str("/"), NULL)) {
        struct mg_str* ua = mg_http_get_header(hm, "User-Agent");
        if (ua && (mg_str_contains(*ua, "Mobi") || mg_str_contains(*ua, "Android"))) {
            struct mg_http_serve_opts opts = {0};
            mg_http_serve_file(c, hm, "static/mobile.html", &opts);
            DPL("SERVE MOBILE");
            return;
        }
    }

    // Default: serve static dir
    struct mg_http_serve_opts opts = {.root_dir = STATIC_DIR};
    mg_http_serve_dir(c, hm, &opts);
}
