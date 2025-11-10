#include "debug.h"
#include "globals.h"
#include "mongoose.h"
#include <stdatomic.h>

#define MAX_QUEUE_SIZE 256
static atomic_int s_queue_size = 0;

struct queued_msg {
    struct mg_connection* c;
    char* data;
    size_t len;
    int op;
    struct queued_msg* next;
};

static struct queued_msg* g_msg_head = NULL;
static struct queued_msg* g_msg_tail = NULL;
static pthread_mutex_t g_msg_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t s_mutex_ws = PTHREAD_MUTEX_INITIALIZER;    // mutex for connections below
static struct mg_connection* s_ws_connections[WS_MAX_CONN] = {0}; // Array to track WebSocket connections
atomic_size_t g_ws_conn_count = 0;                                // Counter for active connections

// Called from any thread to enqueue message for a ws connection
static void ws_queue_add(struct mg_connection* c, const char* data, size_t len, int op)
{
    if (atomic_load(&s_queue_size) >= MAX_QUEUE_SIZE) {
        DPL("WARNING: WebSocket queue full, dropping message");
        return;
    }

    struct queued_msg* m = malloc(sizeof(*m));
    if (!m) return;
    m->c = c;
    m->data = malloc(len);
    if (!m->data) {
        free(m);
        return;
    }
    memcpy(m->data, data, len);
    m->len = len;
    m->op = op;
    m->next = NULL;

    pthread_mutex_lock(&g_msg_mutex);
    if (g_msg_tail)
        g_msg_tail->next = m;
    else
        g_msg_head = m;
    g_msg_tail = m;
    atomic_fetch_add(&s_queue_size, 1);
    pthread_mutex_unlock(&g_msg_mutex);
}

// Helper function to check if connection is still valid
static bool ws_is_valid_connection(struct mg_connection* c)
{
    pthread_mutex_lock(&s_mutex_ws);
    size_t count = atomic_load(&g_ws_conn_count);
    bool valid = false;

    for (size_t i = 0; i < count; i++) {
        if (s_ws_connections[i] == c) {
            valid = true;
            break;
        }
    }

    pthread_mutex_unlock(&s_mutex_ws);
    return valid;
}

// In the server thread, after mg_mgr_poll, drain the queue and call mg_ws_send
void ws_queue_drain()
{
    struct queued_msg* local_head = NULL;

    pthread_mutex_lock(&g_msg_mutex);
    local_head = g_msg_head;
    g_msg_head = g_msg_tail = NULL;
    atomic_store(&s_queue_size, 0);
    pthread_mutex_unlock(&g_msg_mutex);

    for (struct queued_msg* m = local_head; m != NULL;) {
        struct queued_msg* next = m->next;
        if (m->c && ws_is_valid_connection(m->c) && !m->c->is_closing) {
            mg_ws_send(m->c, m->data, (int)m->len, m->op);
        }
        free(m->data);
        free(m);
        m = next;
    }
}

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

// Helper function to queue a message to all WebSocket
void ws_emit(const char* data, int len)
{
    pthread_mutex_lock(&s_mutex_ws);
    size_t count = atomic_load(&g_ws_conn_count);
    for (size_t i = 0; i < count; i++) {
        struct mg_connection* conn = s_ws_connections[i];
        if (conn != NULL) {
            ws_queue_add(conn, data, len, WEBSOCKET_OP_TEXT);
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

            ws_queue_add(ic, "", 0, WEBSOCKET_OP_CLOSE);

            // CRITICAL: Remove from array immediately
            for (size_t j = i; j < count - 1; j++) {
                s_ws_connections[j] = s_ws_connections[j + 1];
            }
            s_ws_connections[count - 1] = NULL;
            atomic_store(&g_ws_conn_count, count - 1);

            dropped++;
            break;
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
