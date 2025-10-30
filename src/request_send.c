#include "logger.h"
#include "mongoose.h"
#include "request.h"
#include <float.h>
#include <pthread.h>
#include <stdatomic.h>

static void fn(struct mg_connection* c, int ev, void* ev_data)
{
    struct Request* request = (struct Request*)c->fn_data;

    if (ev == MG_EV_OPEN) {
        request->timeout_ms_start = mg_millis() + request->timeout_ms;
        return;
    }

    if (ev == MG_EV_POLL) {
        if (mg_millis() > request->timeout_ms_start && (c->is_connecting || c->is_resolving)) {
            mg_error(c, "Connect timeout");
            request->status = REQUEST_STATUS_ERROR_TIMEOUT;
        }
        return;
    }

    if (ev == MG_EV_CONNECT) {
        struct mg_str host = mg_url_host(request->url);

        if (mg_url_is_ssl(request->url)) {
            struct mg_tls_opts opts = {.ca = mg_unpacked("/certs/curl.pem"), .name = mg_url_host(request->url)};
            // struct mg_tls_opts opts = {.name = mg_url_host(s_url)};
            mg_tls_init(c, &opts);
        }

        mg_printf(c, request->request_format, mg_url_uri(request->url), (int)host.len, host.buf);
        return;
    }

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        if (request->remember_response) { request->output = mg_strdup(hm->body); }
        c->is_draining = 1; // Tell mongoose to close this connection
        request->status = REQUEST_STATUS_DONE;
        return;
    }

    if (ev == MG_EV_ERROR) {
        request->status = REQUEST_STATUS_ERROR_CONN;
        return;
    }
}

extern atomic_bool g_running;

enum RequestStatus request_send(struct Request* request)
{
    if (request->log) { logger_requests_write("%s\n", request->url); }

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    //mgr.dns4.url = "udp://8.8.8.8:53";
    mg_http_connect(&mgr, request->url, fn, request);
    while (atomic_load(&g_running) && request->status == REQUEST_STATUS_RUNNING) mg_mgr_poll(&mgr, 50);
    mg_mgr_free(&mgr);

    if (request_status_failed(request->status)) { logger_errors_write("%s -- %s\n", request->url, request_status_to_str(request->status)); }
    return request->status;
}

enum RequestStatus request_send_quick(const char* url)
{
    struct Request request = {0};
    request.status = REQUEST_STATUS_RUNNING;
    request.url = url;
    request.request_format = REQUEST_FORMAT_BAS;
    request.timeout_ms = TIMEOUT_BAS;
    request.remember_response = 0;
    return request_send(&request);
}
