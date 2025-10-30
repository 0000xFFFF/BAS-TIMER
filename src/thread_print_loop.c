#include "debug.h"
#include "draw_ui.h"
#include "globals.h"
#include "serve_websocket.h"
#include "spinners.h"
#include "thread_utils.h"
#include "utils.h"
#ifndef DEBUG
#include "term.h"
#endif
#include <stdatomic.h>

void* th_print_loop(void* sig)
{
    init_spinners();

    DPL("THREAD START PRINT LOOP");
    UNUSED(sig);

    char html_buffer[1024 * 16] = {0};
    char html_buffer_escaped[1024 * 16 * 2] = {0};
    char emit_buffer[1024 * 16 * 2] = {0};

    while (atomic_load(&g_running)) {

#ifndef DEBUG
        term_cursor_reset();
#endif
        draw_ui();
        ansi_to_html(g_term_buffer, html_buffer);
        escape_quotes(html_buffer, html_buffer_escaped);

        int b = snprintf(emit_buffer, 1024 * 8 * 2,
                         "{"
                         "\"term\": \"%s\""
                         ","
                         "\"Tmin\": %f"
                         ","
                         "\"Tmax\": %f"
                         ","
                         "\"mod_rada\": %d"
                         ","
                         "\"StatusPumpe4\": %d"
                         "}",
                         html_buffer_escaped,
                         du_info.Tmin,
                         du_info.Tmax,
                         du_info.mod_rada,    // heat
                         du_info.StatusPumpe4 // gas pump
        );
        websocket_emit(emit_buffer, b);

        sleep_ms_interruptible(SLEEP_MS_DRAW);
    }

    DPL("THREAD STOP PRINT LOOP");
    return NULL;
}
