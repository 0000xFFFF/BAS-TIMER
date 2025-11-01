#include "mongoose.h"
#include "request.h"
#include "utils.h"
#include <float.h>

double extract_json_label(struct mg_str json_body, const char* label, double fallback)
{
    struct mg_str tok = mg_json_get_tok(json_body, label);
    double value = fallback;
    mg_json_get_num(tok, "$.value", &value);
    return value;
}

char* request_status_to_str(enum RequestStatus status)
{
    switch (status) {
        case REQUEST_STATUS_RUNNING:       return "running"; break;
        case REQUEST_STATUS_DONE:          return "done"; break;
        case REQUEST_STATUS_ERROR_TIMEOUT: return "timeout"; break;
        case REQUEST_STATUS_ERROR_CONN:    return "connection error"; break;
    }
    return "?";
}

char* request_status_to_smallstr(enum RequestStatus status)
{
    switch (status) {
        case REQUEST_STATUS_RUNNING:       return ">"; break;
        case REQUEST_STATUS_DONE:          return "."; break;
        case REQUEST_STATUS_ERROR_TIMEOUT: return "t/o"; break;
        case REQUEST_STATUS_ERROR_CONN:    return "con err"; break;
    }
    return "?";
}

bool request_status_failed(enum RequestStatus status)
{
    switch (status) {
        default:
        case REQUEST_STATUS_RUNNING:
        case REQUEST_STATUS_DONE:          return false;

        case REQUEST_STATUS_ERROR_TIMEOUT: return true; break;
        case REQUEST_STATUS_ERROR_CONN:    return true; break;
    }
    return false;
}

void print_bas_info(const struct bas_info* b)
{
    if (!b) return;

    printf("valid: %s\n", b->valid ? "true" : "false");
    printf("status: %d\n", (int)b->status);

    printf("mod_rada: %d\n", b->mod_rada);
    printf("mod_rezim: %d\n", b->mod_rezim);
    printf("StatusPumpe3: %d\n", b->StatusPumpe3);
    printf("StatusPumpe4: %d\n", b->StatusPumpe4);
    printf("StatusPumpe5: %d\n", b->StatusPumpe5);
    printf("StatusPumpe6: %d\n", b->StatusPumpe6);
    printf("StatusPumpe7: %d\n", b->StatusPumpe7);

    printf("Tspv: %.2f\n", b->Tspv);
    printf("Tsolar: %.2f\n", b->Tsolar);
    printf("Tzadata: %.2f\n", b->Tzadata);
    printf("Tfs: %.2f\n", b->Tfs);
    printf("Tmax: %.2f\n", b->Tmax);
    printf("Tmin: %.2f\n", b->Tmin);
    printf("Tsobna: %.2f\n", b->Tsobna);

    printf("Tmid: %.2f\n", b->Tmid);
    printf("TmidGE: %d\n", b->TmidGE);
    printf("TminLT: %d\n", b->TminLT);

    printf("peaks_valid: %s\n", b->peaks_valid ? "true" : "false");
    printf("peak_min_solar: %.2f\n", b->peak_min_solar);
    printf("peak_max_solar: %.2f\n", b->peak_max_solar);
    printf("peak_min_human: %.2f\n", b->peak_min_human);
    printf("peak_max_human: %.2f\n", b->peak_max_human);
    printf("peak_min_buf: %.2f\n", b->peak_min_buf);
    printf("peak_max_buf: %.2f\n", b->peak_max_buf);
    printf("peak_min_circ: %.2f\n", b->peak_min_circ);
    printf("peak_max_circ: %.2f\n", b->peak_max_circ);

    printf("opt_auto_timer: %d\n", b->opt_auto_timer);
    printf("opt_auto_timer_seconds: %d\n", b->opt_auto_timer_seconds);
    printf("opt_auto_timer_seconds_old: %d\n", b->opt_auto_timer_seconds_old);
    printf("opt_auto_timer_started: %d\n", b->opt_auto_timer_started);
    printf("opt_auto_timer_seconds_elapsed: %d\n", b->opt_auto_timer_seconds_elapsed);
    printf("opt_auto_timer_status: %s\n", b->opt_auto_timer_status);
    printf("opt_auto_gas: %d\n", b->opt_auto_gas);
    printf("opt_auto_gas_status: %s\n", b->opt_auto_gas_status);

    printf("history_mode: %d\n", b->history_mode);
    printf("history_mode_time_changed: %d\n", b->history_mode_time_changed);
    printf("history_mode_time_on: %d\n", b->history_mode_time_on);
    printf("history_mode_time_off: %d\n", b->history_mode_time_off);

    printf("history_gas: %d\n", b->history_gas);
    printf("history_gas_time_changed: %d\n", b->history_gas_time_changed);
    printf("history_gas_time_on: %d\n", b->history_gas_time_on);
    printf("history_gas_time_off: %d\n", b->history_gas_time_off);
}

enum Weather detect_weather(const char* text)
{
    for (int w = WEATHER_CLEAR; w <= WEATHER_FOG; w++) {
        for (int k = 0; weather_keywords[w][k] != NULL; k++) {
            if (istrstr(text, weather_keywords[w][k])) {
                return w;
            }
        }
    }
    return WEATHER_UNKNOWN;
}
