#include "debug.h"
#include "globals.h"
#include "mongoose.h"
#include <stdatomic.h>

static struct mg_connection* s_ws_connections[WS_MAX_CONN] = {0}; // Array to track WebSocket connections
atomic_size_t g_ws_conn_count = 0;                                // Counter for active connections

static pthread_mutex_t s_mutex_ws = PTHREAD_MUTEX_INITIALIZER;

size_t write_conn_to_buffer_safe(char* buffer, size_t size)
{
    pthread_mutex_lock(&s_mutex_ws);
    size_t count = atomic_load(&g_ws_conn_count);
    size_t b = 0;
    for (size_t i = 0; i < count; i++) {
        struct mg_connection* c = s_ws_connections[i];
        if (b >= size) break;
        size_t written = snprintf(buffer + b, size - b, "%d.%d.%d.%d\n", c->rem.ip[0], c->rem.ip[1], c->rem.ip[2], c->rem.ip[3]);
        if (written <= 0 || written >= size - b) break;
        b += written;
    }
    pthread_mutex_unlock(&s_mutex_ws);
    return b;
}

// Helper function to emit a message to all WebSocket clients
void websocket_emit(const char* data, int len)
{
    pthread_mutex_lock(&s_mutex_ws);
    size_t count = atomic_load(&g_ws_conn_count);
    for (size_t i = 0; i < count; i++) {
        if (s_ws_connections[i] != NULL) {
            mg_ws_send(s_ws_connections[i], data, len, WEBSOCKET_OP_TEXT);
        }
    }
    pthread_mutex_unlock(&s_mutex_ws);
}

static void ws_remove(struct mg_connection* c)
{
    int count = atomic_load(&g_ws_conn_count);
    for (int i = 0; i < count; i++) {
        if (memcmp(s_ws_connections[i]->rem.ip, c->rem.ip, 16) == 0) {
            D(printf("removed ip: %d.%d.%d.%d\n", c->rem.ip[0], c->rem.ip[1], c->rem.ip[2], c->rem.ip[3]));
            // Shift all remaining connections
            for (int j = i; j < count - 1; j++) {
                s_ws_connections[j] = s_ws_connections[j + 1];
            }
            s_ws_connections[count - 1] = NULL;
            count--;
            atomic_store(&g_ws_conn_count, count);
            break;
        }
    }
}

static size_t ws_drop_if_exist(struct mg_connection* c)
{
    size_t dropped = 0;
    size_t count = atomic_load(&g_ws_conn_count);
    for (size_t i = 0; i < count; i++) {
        struct mg_connection* ic = s_ws_connections[i];
        if (memcmp(ic->rem.ip, c->rem.ip, 16) == 0) {
            D(printf("drop existing ip: %d.%d.%d.%d\n", ic->rem.ip[0], ic->rem.ip[1], ic->rem.ip[2], ic->rem.ip[3]));
            mg_ws_send(ic, "", 0, WEBSOCKET_OP_CLOSE); // Send a WebSocket close frame
            ic->is_closing = 1;                        // Mark for closure
            // mg_mgr_poll(ic->mgr, 0);                   // Process the closure
            return dropped;
        }
    }
    return dropped;
}

static void ws_add(struct mg_connection* c)
{
    int count = atomic_load(&g_ws_conn_count);
    if (count < WS_MAX_CONN) {
        s_ws_connections[count] = c;
        D(printf("added new ip: %d.%d.%d.%d\n", c->rem.ip[0], c->rem.ip[1], c->rem.ip[2], c->rem.ip[3]));
        atomic_store(&g_ws_conn_count, count + 1);
    }
}

static void ws_addex(struct mg_connection* c)
{
    if (ws_drop_if_exist(c)) { return; }
    ws_add(c);
}

static void ws_addex_safe(struct mg_connection* c)
{
    pthread_mutex_lock(&s_mutex_ws);
    ws_addex(c);
    pthread_mutex_unlock(&s_mutex_ws);
}

static void ws_remove_safe(struct mg_connection* c)
{
    pthread_mutex_lock(&s_mutex_ws);
    ws_remove(c);
    pthread_mutex_unlock(&s_mutex_ws);
}

void serve_websocket(struct mg_connection* c, int ev, void* ev_data)
{
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;
        if (mg_match(hm->uri, mg_str("/ws"), NULL)) { return mg_ws_upgrade(c, hm, NULL); }
        return;
    }

    if (ev == MG_EV_WS_OPEN) {
        D(printf("WS new ip: %d.%d.%d.%d\n", c->rem.ip[0], c->rem.ip[1], c->rem.ip[2], c->rem.ip[3]));
        return ws_addex_safe(c);
    }

    if (ev == MG_EV_WS_MSG) {
        struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
        D(printf("ECHO BACK WS RECV: %.*s\n", (int)wm->data.len, wm->data.buf));
        mg_ws_send(c, wm->data.buf, wm->data.len, WEBSOCKET_OP_TEXT); // echo back
        return;
    }

    if (ev == MG_EV_CLOSE) {
        DPL("WS connection closed\n");
        ws_remove_safe(c);
        return;
    }
}
