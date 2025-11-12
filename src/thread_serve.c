#include "thread_serve.h"
#include "debug.h"
#include "globals.h"
#include "mongoose.h"
#include "serve.h"
#include "serve_websocket.h"
#include <stdatomic.h>

void* th_serve(void* sig)
{
    UNUSED(sig);
    DPL("THREAD START SERVE");

    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, ADDR_HTTP, serve, &mgr);
    while (atomic_load(&g_running)) {
        mg_mgr_poll(&mgr, POLL_TIME);
        ws_queue_drain();
    }
    mg_mgr_free(&mgr);

    DPL("THREAD STOP SERVE");
    return NULL;
}
