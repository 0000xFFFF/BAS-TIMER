#ifndef SERVE_WEBSOCKET_H
#define SERVE_WEBSOCKET_H

#include <stdatomic.h>
#include "mongoose.h"

extern atomic_int g_ws_conn_count;
extern void ws_emit(const char* data, int len);
extern void ws_queue_drain();
extern size_t write_conn_to_buffer_safe(char* buffer, size_t size);
extern void serve_websocket(struct mg_connection* c, int ev, void* ev_data);

#endif // SERVE_WEBSOCKET_H
