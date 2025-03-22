#include "debug.h"
#include "globals.h"
#include "mongoose.h"
#include <netinet/tcp.h> // Required for TCP_NODELAY
#include <stdatomic.h>

static struct mg_connection* ws_connections[WS_MAX_CONN]; // Array to track WebSocket connections
atomic_int g_ws_conn_count = 0;                           // Counter for active connections

// Helper function to emit a message to all WebSocket clients
void websocket_emit(const char* data, int len) {
    // Send to all active WebSocket connections
    for (int i = 0; i < atomic_load(&g_ws_conn_count); i++) {
        if (ws_connections[i] != NULL) {
            mg_ws_send(ws_connections[i], data, len, WEBSOCKET_OP_TEXT);
        }
    }
}

void serve_websocket(struct mg_connection* c, int ev, void* ev_data) {

    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message* hm = (struct mg_http_message*)ev_data;

        // Check if this is a request to the WebSocket endpoint
        if (mg_match(hm->uri, mg_str("/ws"), NULL)) {
            // Upgrade the connection to WebSocket
            DPL("WS UPGRADE\n");
            mg_ws_upgrade(c, hm, NULL);
        }

    } else if (ev == MG_EV_WS_OPEN) {
        // WebSocket handshake completed
        DPL("WebSocket connection established\n");

        D(printf("new ip: %d.%d.%d.%d\n", c->rem.ip[0], c->rem.ip[1], c->rem.ip[2], c->rem.ip[3]));

        // // Disable Nagle's Algorithm (TCP_NODELAY)
        // int fd = (int)(intptr_t)c->fd;
        // int flag = 1;
        // setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
        int count = atomic_load(&g_ws_conn_count);

        for (int i = 0; i < count; i++) {
            if (memcmp(ws_connections[i]->rem.ip, c->rem.ip, 16) == 0) {
                D(printf("drop existing ip: %d.%d.%d.%d\n", c->rem.ip[0], c->rem.ip[1], c->rem.ip[2], c->rem.ip[3]));
                // Send a WebSocket close frame
                mg_ws_send(c, "", 0, WEBSOCKET_OP_CLOSE);

                // Close the connection
                c->is_closing = 1;      // Mark for closure
                mg_mgr_poll(c->mgr, 0); // Process the closure
                return;
            }
        }

        // Store the connection (same code as in websocket_handler)
        if (count < WS_MAX_CONN) {

            ws_connections[count] = c;
            D(printf("added new ip: %d.%d.%d.%d\n", c->rem.ip[0], c->rem.ip[1], c->rem.ip[2], c->rem.ip[3]));

            count++;
            atomic_store(&g_ws_conn_count, count);
        }
    } else if (ev == MG_EV_WS_MSG) {
        // WebSocket message handling
        struct mg_ws_message* wm = (struct mg_ws_message*)ev_data;
        D(printf("ECHO BACK WS RECV: %.*s\n", (int)wm->data.len, wm->data.buf));
        mg_ws_send(c, wm->data.buf, wm->data.len, WEBSOCKET_OP_TEXT); // echo back

    } else if (ev == MG_EV_CLOSE) {
        // Connection closed handling
        DPL("Connection closed\n");

        // Remove from our connection array
        int count = atomic_load(&g_ws_conn_count);
        for (int i = 0; i < count; i++) {
            if (memcmp(ws_connections[i]->rem.ip, c->rem.ip, 16) == 0) {
                D(printf("removed ip: %d.%d.%d.%d\n", c->rem.ip[0], c->rem.ip[1], c->rem.ip[2], c->rem.ip[3]));
                // Shift all remaining connections
                for (int j = i; j < count - 1; j++) {
                    ws_connections[j] = ws_connections[j + 1];
                }
                ws_connections[count - 1] = NULL;
                count--;
                atomic_store(&g_ws_conn_count, count);
                break;
            }
        }
    }
}
