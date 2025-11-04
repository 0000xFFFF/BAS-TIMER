#include "debug.h"
#include "globals.h"
#include "marquee.h"
#include "mongoose.h"
#include "term.h"
#include "utils.h"
#include <float.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "request.h"

pthread_mutex_t g_update_info_bas_mutex = PTHREAD_MUTEX_INITIALIZER;

void update_info_bas_safe_io(const struct bas_info* in, struct bas_info* out)
{
    pthread_mutex_lock(&g_update_info_bas_mutex);
    memcpy(out, in, sizeof(struct bas_info));
    pthread_mutex_unlock(&g_update_info_bas_mutex);
}

static long long g_global_unix_counter = 0;

struct bas_info g_info = {0};

void update_info_bas_init()
{
    g_global_unix_counter = timestamp();

    struct bas_info info = {0};

    info.peak_min_solar = TEMP_MIN_SOLAR;
    info.peak_max_solar = TEMP_MAX_SOLAR;
    info.peak_min_human = TEMP_MIN_HUMAN;
    info.peak_max_human = TEMP_MAX_HUMAN;
    info.peak_min_buf = TEMP_MIN_BUF;
    info.peak_max_buf = TEMP_MAX_BUF;
    info.peak_min_circ = TEMP_MIN_CIRC;
    info.peak_max_circ = TEMP_MAX_CIRC;

    info.opt_auto_timer = ENABLE_AUTO_TIMER;
    info.opt_auto_gas = ENABLE_AUTO_GAS;
    info.opt_auto_timer_seconds = AUTO_TIMER_SECONDS;
    info.opt_auto_timer_started = 0;
    info.opt_auto_timer_seconds_elapsed = 0;
    snprintf(info.opt_auto_timer_status, SMALLBUFF, "...");
    snprintf(info.opt_auto_gas_status, SMALLBUFF, "...");

    info.history_mode = -1;
    info.history_mode_time_changed = 0;
    info.history_mode_time_on = 0;
    info.history_mode_time_off = 0;
    info.history_gas = -1;
    info.history_gas_time_changed = 0;
    info.history_gas_time_on = 0;
    info.history_gas_time_off = 0;

    update_info_bas_safe_io(&info, &g_info);
}

// must update_info_bas_init before running this
enum RequestStatus update_info_bas()
{
    g_global_unix_counter++;
    char request_url[BIGBUFF];
    snprintf(request_url, sizeof(request_url), "%s&_=%lld", URL_VARS, g_global_unix_counter);

    struct Request request = {0};
    request.status = REQUEST_STATUS_RUNNING;
    request.url = request_url;
    request.request_format = REQUEST_FORMAT_BAS;
    request.timeout_ms = TIMEOUT_BAS;
    request.remember_response = 1;
    request_send(&request);

    struct bas_info info = {0};
    update_info_bas_safe_io(&g_info, &info);
    info.status = request.status;

    if (request.output.buf) {

        info.valid = true;
        info.mod_rada = extract_json_label(request.output, "$.mod_rada", 0);
        info.mod_rezim = extract_json_label(request.output, "$.mod_rezim", 0);
        info.StatusPumpe3 = extract_json_label(request.output, "$.StatusPumpe3", 0);
        info.StatusPumpe4 = extract_json_label(request.output, "$.StatusPumpe4", 0);
        info.StatusPumpe5 = extract_json_label(request.output, "$.StatusPumpe5", 0);
        info.StatusPumpe6 = extract_json_label(request.output, "$.StatusPumpe6", 0);
        info.StatusPumpe7 = extract_json_label(request.output, "$.StatusPumpe7", 0);
        info.Tspv = extract_json_label(request.output, "$.Tspv", 0);
        info.Tsolar = extract_json_label(request.output, "$.Tsolar", 0);
        info.Tzadata = extract_json_label(request.output, "$.Tzadata", 0);
        info.Tfs = extract_json_label(request.output, "$.Tfs", 0);
        info.Tmax = extract_json_label(request.output, "$.Tmax", 0);
        info.Tmin = extract_json_label(request.output, "$.Tmin", 0);
        info.Tsobna = extract_json_label(request.output, "$.Tsobna", 0);

        // calc other values
        info.Tmid = (info.Tmax + info.Tmin) / 2;
        info.TminLT = info.Tmin < (double)45.0;
        info.TmidGE = info.Tmid >= (double)60.0;
        info.TmaxGE = info.Tmax >= (double)70.0;
        info.peak_min_solar = min_dv(2, info.peak_min_solar, info.Tsolar);
        info.peak_max_solar = max_dv(2, info.peak_max_solar, info.Tsolar);
        info.peak_min_human = min_dv(4, info.peak_min_human, info.Tsobna, info.Tzadata, info.Tspv);
        info.peak_max_human = max_dv(4, info.peak_max_human, info.Tsobna, info.Tzadata, info.Tspv);
        info.peak_min_buf = min_dv(2, info.peak_min_buf, info.Tmin);
        info.peak_max_buf = max_dv(2, info.peak_max_buf, info.Tmax);
        info.peak_min_circ = min_dv(2, info.peak_min_circ, info.Tfs);
        info.peak_max_circ = max_dv(2, info.peak_max_circ, info.Tfs);

        remember_vars_do_action(&info);

        D(print_bas_info(&info));

        free((void*)request.output.buf);
    }

    update_info_bas_safe_io(&info, &g_info);

    return request.status;
}

static pthread_mutex_t g_update_info_wttrin_mutex = PTHREAD_MUTEX_INITIALIZER;

struct wttrin_info g_wttrin = {0};

void update_info_wttrin_safe_io(const struct wttrin_info* in, struct wttrin_info* out)
{
    pthread_mutex_lock(&g_update_info_wttrin_mutex);
    memcpy(out, in, sizeof(struct wttrin_info));
    pthread_mutex_unlock(&g_update_info_wttrin_mutex);
}

void update_info_wttrin_init()
{
    struct wttrin_info wttrin = {0};
    snprintf(wttrin.marquee_conds_buf, sizeof(wttrin.marquee_conds_buf), "...");
    snprintf(wttrin.marquee_times_buf, sizeof(wttrin.marquee_times_buf), "...");
    update_info_wttrin_safe_io(&wttrin, &g_wttrin);
}

static void make_wttrin_marquee_conds(struct wttrin_info* wttrin)
{
    size_t b = 0;
    b += snprintf(wttrin->marquee_conds_buf + b, sizeof(wttrin->marquee_conds_buf) - b, "@ "); // pause on '@' char
    b += dt_HM(wttrin->marquee_conds_buf + b, sizeof(wttrin->marquee_conds_buf) - b);          // prepend hour:minute
    b += snprintf(wttrin->marquee_conds_buf + b, sizeof(wttrin->marquee_conds_buf) - b, ": ");
    b += snprintf(wttrin->marquee_conds_buf + b, sizeof(wttrin->marquee_conds_buf) - b, "%s %s  ",
                  wttrin->csv[WTTRIN_CSV_FIELD_C],
                  wttrin->csv[WTTRIN_CSV_FIELD_c]);

    const int marquee_pause = 1000; // 1 sec pause
    marquee_init(&wttrin->marquee_conds, wttrin->marquee_conds_buf, g_term_w, marquee_pause / SLEEP_MS_DRAW, 1);
}


static void make_wttrin_marquee_times(struct wttrin_info* wttrin)
{
    size_t b = 0;
    b += snprintf(wttrin->marquee_times_buf + b, sizeof(wttrin->marquee_times_buf) - b, "@ Dawn 🌄 %s, @ Sunrise 🌅 %s, @ Zenith 🌞 %s, @ Sunset 🌇 %s, @ Dusk 🌆 %s  ",
                  wttrin->csv[WTTRIN_CSV_FIELD_D],
                  wttrin->csv[WTTRIN_CSV_FIELD_S],
                  wttrin->csv[WTTRIN_CSV_FIELD_z],
                  wttrin->csv[WTTRIN_CSV_FIELD_s],
                  wttrin->csv[WTTRIN_CSV_FIELD_d]);

    const int marquee_pause = 3000; // 3 sec pause
    marquee_init(&wttrin->marquee_times, wttrin->marquee_times_buf, g_term_w - 5, marquee_pause / SLEEP_MS_DRAW, 1);
}

enum RequestStatus update_info_wttrin()
{
    struct Request request = {0};
    request.status = REQUEST_STATUS_RUNNING;
    request.url = URL_WTTRIN;
    request.request_format = REQUEST_FORMAT_WTTRIN;
    request.timeout_ms = TIMEOUT_WTTRIN;
    request.remember_response = 1;
    request_send(&request);

    struct wttrin_info wttrin = {0};
    update_info_wttrin_safe_io(&g_wttrin, &wttrin);
    wttrin.status = request.status;

    if (request.output.buf) {

        wttrin.valid = true;

        // parse csv
        wttrin.csv_parsed = parse_csv(request.output.buf,
                                      URL_WTTRIN_OUTPUT_CSV_SEP,
                                      URL_WTTRIN_OUTPUT_MAX_FIELDS,
                                      URL_WTTRIN_OUTPUT_MAX_FIELD_LEN,
                                      wttrin.csv);
        free((void*)request.output.buf);

        D(printf("WTTRIN PARSED: %d\n", wttrin.csv_parsed));

        D(
            printf("Weather condition                    : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_c]);
            printf("Weather condition textual name       : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_C]);
            printf("Weather condition  plain-text symbol : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_x]);
            printf("Humidity                             : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_h]);
            printf("Temperature (Actual)                 : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_t]);
            printf("Temperature (Feels Like)             : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_f]);
            printf("Wind                                 : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_w]);
            printf("Location                             : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_l]);
            printf("Moon phase 🌑🌒🌓🌔🌕🌖🌗🌘          : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_m]);
            printf("Moon day                             : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_M]);
            printf("Precipitation (mm/3 hours)           : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_p]);
            printf("Pressure (hPa)                       : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_P]);
            printf("UV index (1-12)                      : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_u]);
            printf("Dawn*                                : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_D]);
            printf("Sunrise*                             : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_S]);
            printf("Zenith*                              : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_z]);
            printf("Sunset*                              : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_s]);
            printf("Dusk*                                : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_d]);
            printf("Current time*                        : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_T]);
            printf("Local timezone.                      : %s\n", wttrin.csv[WTTRIN_CSV_FIELD_Z]););

        if (wttrin.csv_parsed < URL_WTTRIN_OUTPUT_MAX_FIELDS) {
            DPL("FAILED TO PARSE WTTRIN");
            return request.status;
        }

        // wttrin emoji
        wttrin.weather = detect_weather(wttrin.csv[WTTRIN_CSV_FIELD_C]);

        // make marquees
        make_wttrin_marquee_conds(&wttrin);
        make_wttrin_marquee_times(&wttrin);
    }

    update_info_wttrin_safe_io(&wttrin, &g_wttrin);

    return request.status;
}

void update_info_wttrin_marquee_conds_scroll()
{
    pthread_mutex_lock(&g_update_info_wttrin_mutex);
    marquee_scroll_smart(&g_wttrin.marquee_conds);
    pthread_mutex_unlock(&g_update_info_wttrin_mutex);
}

void update_info_wttrin_marquee_conds_update_width(int term_width)
{
    pthread_mutex_lock(&g_update_info_wttrin_mutex);
    marquee_update_width(&g_wttrin.marquee_conds, term_width);
    pthread_mutex_unlock(&g_update_info_wttrin_mutex);
}

void update_info_wttrin_marquee_times_scroll()
{
    pthread_mutex_lock(&g_update_info_wttrin_mutex);
    marquee_scroll_smart(&g_wttrin.marquee_times);
    pthread_mutex_unlock(&g_update_info_wttrin_mutex);
}

void update_info_wttrin_marquee_times_update_width(int term_width)
{
    pthread_mutex_lock(&g_update_info_wttrin_mutex);
    marquee_update_width(&g_wttrin.marquee_times, term_width);
    pthread_mutex_unlock(&g_update_info_wttrin_mutex);
}
