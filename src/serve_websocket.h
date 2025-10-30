#ifndef SERVE_WEBSOCKET_H
#define SERVE_WEBSOCKET_H

#include <stdatomic.h>
#include "mongoose.h"

extern atomic_int g_ws_conn_count;
extern void websocket_emit(const char* data, int len);
extern void serve_websocket(struct mg_connection* c, int ev, void* ev_data);

#endif // SERVE_WEBSOCKET_H
