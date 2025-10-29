#include "debug.h"
#include "globals.h"
#include "mongoose.h"
#include "serve_site.h"
#include "serve_websocket.h"
#include <stdatomic.h>

void* th_serve(void* sig)
{
    UNUSED(sig);

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, S_ADDR_HTTP, serve_site, &mgr);
    mg_http_listen(&mgr, S_ADDR_WS, serve_websocket, &mgr);
    while (atomic_load(&g_running)) { mg_mgr_poll(&mgr, POLL_TIME); }
    mg_mgr_free(&mgr);

    return NULL;
}
