#ifndef SERVE_WEBSOCKET
#define SERVE_WEBSOCKET

void websocket_emit(const char* data, int len);
void serve_websocket(struct mg_connection* c, int ev, void* ev_data);

#endif // SERVE_WEBSOCKET
