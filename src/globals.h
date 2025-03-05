#ifndef GLOBALS_H
#define GLOBALS_H

#define POLL_TIME 10

#ifdef DEBUG
#undef POLL_TIME
#define POLL_TIME 1000
#endif

#define S_ADDR_HTTP "http://0.0.0.0:5000"   // HTTP port
#define S_ADDR_HTTPS "https://0.0.0.0:5001" // HTTPS port -- unused
#define S_ADDR_WS "ws://0.0.0.0:8001"       // WebSocket port
#define S_STATIC_DIR "static"

#define WS_MAX_CONN 100

#define REQUEST_SLEEP 3
#define MAIN_WORKER_DRAW_SLEEP 0.25
#define DO_REQUEST_COUNT (REQUEST_SLEEP/MAIN_WORKER_DRAW_SLEEP)

#define PRINT_ONLY_ONE_IP 1

#define ENABLE_AUTO_TIMER 1
#define ENABLE_AUTO_GAS 1

#define STATUS_BUFFER_SIZE 64
#define REQUEST_URL_BUFFER_SIZE 1024

#endif // GLOBALS_H
