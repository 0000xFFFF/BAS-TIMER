#include "serve.h"
#include "mongoose.h"
#include "serve_site.h"
#include "serve_websocket.h"

void serve(struct mg_connection* c, int ev, void* ev_data)
{
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        if (mg_match(hm->uri, mg_str("/ws"), NULL)) {
            serve_websocket(c, ev, ev_data);
            return;
        }

        serve_site(c, ev, ev_data);
        return;
    }

    if (ev == MG_EV_WS_OPEN || ev == MG_EV_WS_MSG || ev == MG_EV_CLOSE) {
        serve_websocket(c, ev, ev_data);
        return;
    }
}
