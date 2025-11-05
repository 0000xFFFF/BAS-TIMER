#include "debug.h"
#include "globals.h"
#include "marquee.h"
#include "mongoose.h"
#include "request.h"
#include "colors.h"
#include "term.h"
#include "utils.h"
#include <float.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

pthread_mutex_t g_infos_bas_mutex = PTHREAD_MUTEX_INITIALIZER;

void infos_bas_safe_io(const struct BasInfo* in, struct BasInfo* out)
{
    pthread_mutex_lock(&g_infos_bas_mutex);
    memcpy(out, in, sizeof(struct BasInfo));
    pthread_mutex_unlock(&g_infos_bas_mutex);
}

static long long g_global_unix_counter = 0;

void infos_bas_init()
{
    g_global_unix_counter = timestamp();

    struct BasInfo info = {0};

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

    infos_bas_safe_io(&info, &g_infos.bas);
}

// must infos_bas_init before running this
enum RequestStatus infos_bas_update()
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

    pthread_mutex_lock(&g_infos_bas_mutex);
    g_infos.bas.status = REQUEST_STATUS_RUNNING;
    pthread_mutex_unlock(&g_infos_bas_mutex);

    request_send(&request);

    struct BasInfo info = {0};
    infos_bas_safe_io(&g_infos.bas, &info);
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

    infos_bas_safe_io(&info, &g_infos.bas);

    return request.status;
}

static pthread_mutex_t g_infos_wttrin_mutex = PTHREAD_MUTEX_INITIALIZER;

void infos_wttrin_update_safe_io(const struct WttrinInfo* in, struct WttrinInfo* out)
{
    pthread_mutex_lock(&g_infos_wttrin_mutex);
    memcpy(out, in, sizeof(struct WttrinInfo));
    pthread_mutex_unlock(&g_infos_wttrin_mutex);
}

void infos_wttrin_init()
{
    struct WttrinInfo wttrin = {0};
    snprintf(wttrin.marquee_conds_buf, sizeof(wttrin.marquee_conds_buf), "...");
    snprintf(wttrin.marquee_times_buf, sizeof(wttrin.marquee_times_buf), "...");
    infos_wttrin_update_safe_io(&wttrin, &g_infos.wttrin);
}

#define MZWS MARQUEE_ZERO_WIDTH_SPACE

static void make_wttrin_time(struct WttrinInfo* wttrin)
{
    size_t b = 0;
    b += dt_HM(wttrin->time + b, sizeof(wttrin->time) - b); // prepend hour:minute
    b += snprintf(wttrin->time + b, sizeof(wttrin->time) - b, ":");
}

static int make_wttrin_marquee_conds_width(int term_width, struct WttrinInfo* wttrin)
{
    int other = utf8_display_width(wttrin->time)                      // time -- e.g. "12:12:"
                + 1                                                   // space
                + utf8_display_width(wttrin->csv[WTTRIN_CSV_FIELD_c]) // emojis -- e.g "☀ "
                + 1;                                                  // space

    int ret = term_width - other;
    return ret;
}

static void make_wttrin_marquee_conds(struct WttrinInfo* wttrin)
{
    size_t b = 0;
    b += snprintf(wttrin->marquee_conds_buf + b, sizeof(wttrin->marquee_conds_buf) - b, MZWS); // pause on zero width space char
    b += snprintf(wttrin->marquee_conds_buf + b, sizeof(wttrin->marquee_conds_buf) - b, "%s  ", wttrin->csv[WTTRIN_CSV_FIELD_C]);

    const int marquee_pause = 1000; // 1 sec pause
    const int width = make_wttrin_marquee_conds_width(g_term_w, wttrin);
    marquee_init(&wttrin->marquee_conds, wttrin->marquee_conds_buf, width, marquee_pause / SLEEP_MS_DRAW, 2);
}

static int make_wttrin_marquee_times_width(int term_width)
{
    return term_width - 3;
}

static char g_temp[BIGBUFF];
static size_t g_temp_b = 0;

static char* mk_str(const char* format, char* param)
{
    g_temp_b += snprintf(g_temp, sizeof(g_temp) - g_temp_b, format, param);
    return g_temp;
}

static void make_wttrin_marquee_times(struct WttrinInfo* wttrin)
{
    size_t b = 0;
    // clang-format on
    b += ctext_fg(wttrin->marquee_times_buf + b, sizeof(wttrin->marquee_times_buf) - b, timeofday_to_color(TIME_OF_DAY_DAWN),      mk_str(MZWS "🌄 %s ", wttrin->csv[WTTRIN_CSV_FIELD_D]));
    b += ctext_fg(wttrin->marquee_times_buf + b, sizeof(wttrin->marquee_times_buf) - b, timeofday_to_color(TIME_OF_DAY_MORNING),   mk_str(MZWS "🌅 %s ", wttrin->csv[WTTRIN_CSV_FIELD_S]));
    b += ctext_fg(wttrin->marquee_times_buf + b, sizeof(wttrin->marquee_times_buf) - b, timeofday_to_color(TIME_OF_DAY_AFTERNOON), mk_str(MZWS "🌞 %s ", wttrin->csv[WTTRIN_CSV_FIELD_z]));
    b += ctext_fg(wttrin->marquee_times_buf + b, sizeof(wttrin->marquee_times_buf) - b, timeofday_to_color(TIME_OF_DAY_SUNSET),    mk_str(MZWS "🌇 %s ", wttrin->csv[WTTRIN_CSV_FIELD_s]));
    b += ctext_fg(wttrin->marquee_times_buf + b, sizeof(wttrin->marquee_times_buf) - b, timeofday_to_color(TIME_OF_DAY_NIGHT),     mk_str(MZWS "🌆 %s ", wttrin->csv[WTTRIN_CSV_FIELD_d]));
    // clang-format off

    const int marquee_pause = 3000; // 3 sec pause
    const int width = make_wttrin_marquee_times_width(g_term_w);
    marquee_init(&wttrin->marquee_times, wttrin->marquee_times_buf, width, marquee_pause / SLEEP_MS_DRAW, 3);
}

// must infos_wttrin_init before running this
enum RequestStatus infos_wttrin_update()
{
    struct Request request = {0};
    request.status = REQUEST_STATUS_RUNNING;
    request.url = URL_WTTRIN;
    request.request_format = REQUEST_FORMAT_WTTRIN;
    request.timeout_ms = TIMEOUT_WTTRIN;
    request.remember_response = 1;

    pthread_mutex_lock(&g_infos_wttrin_mutex);
    g_infos.wttrin.status = REQUEST_STATUS_RUNNING;
    pthread_mutex_unlock(&g_infos_wttrin_mutex);

    request_send(&request);

    struct WttrinInfo wttrin = {0};
    infos_wttrin_update_safe_io(&g_infos.wttrin, &wttrin);
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

        if (wttrin.csv_parsed < URL_WTTRIN_OUTPUT_MAX_FIELDS) {
            DPL("FAILED TO PARSE WTTRIN");
            return request.status;
        }

        D(print_wttrin_info(&wttrin));

        // // override weather cond
        // snprintf(wttrin.csv[WTTRIN_CSV_FIELD_C], sizeof(wttrin.csv[WTTRIN_CSV_FIELD_C]), "Moderate or heavy rain in area with thunder");

        // wttrin emoji
        wttrin.weather = detect_weather(wttrin.csv[WTTRIN_CSV_FIELD_C]);

        // parse seconds for TimeOfDay
        wttrin.dawn = hms_to_seconds(wttrin.csv[WTTRIN_CSV_FIELD_D]);
        wttrin.sunrise = hms_to_seconds(wttrin.csv[WTTRIN_CSV_FIELD_S]);
        wttrin.zenith = hms_to_seconds(wttrin.csv[WTTRIN_CSV_FIELD_z]);
        wttrin.sunset = hms_to_seconds(wttrin.csv[WTTRIN_CSV_FIELD_s]);
        wttrin.dusk = hms_to_seconds(wttrin.csv[WTTRIN_CSV_FIELD_d]);

        // make marquees
        make_wttrin_time(&wttrin);
        make_wttrin_marquee_conds(&wttrin);
        make_wttrin_marquee_times(&wttrin);
    }

    infos_wttrin_update_safe_io(&wttrin, &g_infos.wttrin);

    return request.status;
}

void infos_wttrin_marquee_conds_scroll()
{
    pthread_mutex_lock(&g_infos_wttrin_mutex);
    marquee_scroll_smart(&g_infos.wttrin.marquee_conds);
    pthread_mutex_unlock(&g_infos_wttrin_mutex);
}

void infos_wttrin_marquee_conds_update_width(int term_width)
{
    pthread_mutex_lock(&g_infos_wttrin_mutex);
    marquee_update_width(&g_infos.wttrin.marquee_conds, make_wttrin_marquee_conds_width(term_width, &g_infos.wttrin));
    pthread_mutex_unlock(&g_infos_wttrin_mutex);
}

void infos_wttrin_marquee_times_scroll()
{
    pthread_mutex_lock(&g_infos_wttrin_mutex);
    marquee_scroll_smart(&g_infos.wttrin.marquee_times);
    pthread_mutex_unlock(&g_infos_wttrin_mutex);
}

void infos_wttrin_marquee_times_update_width(int term_width)
{
    pthread_mutex_lock(&g_infos_wttrin_mutex);
    marquee_update_width(&g_infos.wttrin.marquee_times, term_width);
    pthread_mutex_unlock(&g_infos_wttrin_mutex);
}

void infos_save()
{
    pthread_mutex_lock(&g_infos_bas_mutex);
    pthread_mutex_lock(&g_infos_wttrin_mutex);

    save_infos(VAR_DIR_FILE_INFOS_BIN, &g_infos);

    pthread_mutex_unlock(&g_infos_bas_mutex);
    pthread_mutex_unlock(&g_infos_wttrin_mutex);
}
