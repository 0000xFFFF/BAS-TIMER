#include "colors.h"
#include "debug.h"
#include "globals.h"
#include "logger.h"
#include "marquee.h"
#include "mongoose.h"
#include "request.h"
#include "term.h"
#include "utils.h"
#include <float.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static pthread_mutex_t s_infos_bas_mutex = PTHREAD_MUTEX_INITIALIZER;

void infos_bas_safe_io(const struct BasInfo* in, struct BasInfo* out)
{
    pthread_mutex_lock(&s_infos_bas_mutex);
    memcpy(out, in, sizeof(struct BasInfo));
    pthread_mutex_unlock(&s_infos_bas_mutex);
}

static long long s_unix_counter = 0;

void infos_bas_init(void)
{
    pthread_mutex_lock(&s_infos_bas_mutex);

    s_unix_counter = timestamp();

    if (!g_infos.bas.valid) {
        g_infos.bas.mod_rada = -1;
        g_infos.bas.mod_rezim = -1;
        g_infos.bas.StatusPumpe3 = -1;
        g_infos.bas.StatusPumpe4 = -1;
        g_infos.bas.StatusPumpe5 = -1;
        g_infos.bas.StatusPumpe6 = -1;
        g_infos.bas.StatusPumpe7 = -1;

        g_infos.bas.peak_min_solar = TEMP_MIN_SOLAR;
        g_infos.bas.peak_max_solar = TEMP_MAX_SOLAR;
        g_infos.bas.peak_min_human = TEMP_MIN_HUMAN;
        g_infos.bas.peak_max_human = TEMP_MAX_HUMAN;
        g_infos.bas.peak_min_buf = TEMP_MIN_BUF;
        g_infos.bas.peak_max_buf = TEMP_MAX_BUF;
        g_infos.bas.peak_min_circ = TEMP_MIN_CIRC;
        g_infos.bas.peak_max_circ = TEMP_MAX_CIRC;

        g_infos.bas.opt_auto_timer = ENABLE_AUTO_TIMER;
        g_infos.bas.opt_auto_gas = ENABLE_AUTO_GAS;
        g_infos.bas.opt_auto_timer_seconds = (int)AUTO_TIMER_SECONDS;
        g_infos.bas.opt_auto_timer_started = 0;
        g_infos.bas.opt_auto_timer_seconds_elapsed = 0;

        g_infos.bas.history_mode = -1;
        g_infos.bas.history_mode_time_changed = 0;
        g_infos.bas.history_mode_time_on = 0;
        g_infos.bas.history_mode_time_off = 0;
        g_infos.bas.history_gas = -1;
        g_infos.bas.history_gas_time_changed = 0;
        g_infos.bas.history_gas_time_on = 0;
        g_infos.bas.history_gas_time_off = 0;

        g_infos.bas.schedules_t_min = 10.0; // 10 󰔄
        g_infos.bas.schedules_last_yday = -1;

        int i = 0;
        g_infos.bas.schedules[i].valid = true;
        g_infos.bas.schedules[i].from = hms_to_today_seconds(1, 0, 0);     // 01:00:00
        g_infos.bas.schedules[i].to = g_infos.bas.schedules[i].from + hms_to_today_seconds(0, 15, 0); // + 15min
        g_infos.bas.schedules[i].duration = hms_to_today_seconds(0, 5, 0); // 5 min
        g_infos.bas.schedules[i].last_yday = -1;
        i++;

        g_infos.bas.schedules[i].valid = true;
        g_infos.bas.schedules[i].from = hms_to_today_seconds(5, 0, 0);     // 05:00:00
        g_infos.bas.schedules[i].to = g_infos.bas.schedules[i].from + hms_to_today_seconds(0, 15, 0); // + 15min
        g_infos.bas.schedules[i].duration = hms_to_today_seconds(0, 5, 0); // 5 min
        g_infos.bas.schedules[i].last_yday = -1;
        i++;

        g_infos.bas.schedules[i].valid = true;
        g_infos.bas.schedules[i].from = hms_to_today_seconds(9, 0, 0);     // 09:00:00
        g_infos.bas.schedules[i].to = g_infos.bas.schedules[i].from + hms_to_today_seconds(0, 15, 0); // + 15min
        g_infos.bas.schedules[i].duration = hms_to_today_seconds(0, 5, 0); // 5 min
        g_infos.bas.schedules[i].last_yday = -1;
        i++;
    }

    pthread_mutex_unlock(&s_infos_bas_mutex);
}

// must infos_bas_init before running this
enum RequestStatus infos_bas_update(void)
{
    s_unix_counter++;
    char request_url[BIGBUFF];
    snprintf(request_url, sizeof(request_url), "%s&_=%lld", URL_VARS, s_unix_counter);

    struct Request request = {0};
    request.status = REQUEST_STATUS_RUNNING;
    request.url = request_url;
    request.request_format = REQUEST_FORMAT_BAS;
    request.timeout_ms = TIMEOUT_MS_BAS;
    request.remember_response = 1;

    pthread_mutex_lock(&s_infos_bas_mutex);
    g_infos.bas.status = REQUEST_STATUS_RUNNING;
    pthread_mutex_unlock(&s_infos_bas_mutex);

    request_send(&request);

    struct BasInfo info = {0};
    infos_bas_safe_io(&g_infos.bas, &info);
    info.status = request.status;

    if (request.output.buf) {

        info.valid = true;
        info.mod_rada = (int)extract_json_label(request.output, "$.mod_rada", 0);
        info.mod_rezim = (int)extract_json_label(request.output, "$.mod_rezim", 0);
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

        radiator_color_update(&info);

        remember_vars_do_action(&info);

        D(print_bas_info(&info));

        free((void*)request.output.buf);
    }

    infos_bas_safe_io(&info, &g_infos.bas);

    return request.status;
}

enum RequestStatus infos_bas_health(void)
{
    pthread_mutex_lock(&s_infos_bas_mutex);
    enum RequestStatus r = g_infos.bas.status;
    pthread_mutex_unlock(&s_infos_bas_mutex);
    return r;
}

static pthread_mutex_t s_infos_wttrin_mutex = PTHREAD_MUTEX_INITIALIZER;

void infos_wttrin_update_safe_io(const struct WttrinInfo* in, struct WttrinInfo* out)
{
    pthread_mutex_lock(&s_infos_wttrin_mutex);
    memcpy(out, in, sizeof(struct WttrinInfo));
    pthread_mutex_unlock(&s_infos_wttrin_mutex);
}

void infos_wttrin_init(void)
{
    pthread_mutex_lock(&s_infos_wttrin_mutex);
    if (!g_infos.wttrin.valid) {
        snprintf(g_infos.wttrin.marquee_conds.text, sizeof(g_infos.wttrin.marquee_conds.text), "...");
        snprintf(g_infos.wttrin.marquee_times.text, sizeof(g_infos.wttrin.marquee_times.text), "...");
    }
    pthread_mutex_unlock(&s_infos_wttrin_mutex);
}

#define MZWS MARQUEE_ZERO_WIDTH_SPACE

static void make_wttrin_time(struct WttrinInfo* wi)
{
    size_t b = 0;
    b += dt_HM(wi->time + b, sizeof(wi->time) - b); // prepend hour:minute
    b += (size_t)snprintf(wi->time + b, sizeof(wi->time) - b, ":");
}

static int make_wttrin_marquee_conds_width(int term_width, struct WttrinInfo* wi)
{
    int other = utf8_display_width(wi->time)                      // time -- e.g. "12:12:"
                + 1                                               // space
                + utf8_display_width(wi->csv[WTTRIN_CSV_FIELD_c]) // emojis -- e.g "☀ "
                + 1;                                              // space

    int ret = term_width - other - 1;
    return ret;
}

// wttr.in: time; emoji ; weather marquee ; top weather emoji
static void display_status_weather_conditions(struct WttrinInfo* wi)
{
    make_wttrin_time(&g_infos.wttrin);
    char buf[MIDBUFF];
    size_t b = 0;
    b += (size_t)snprintf(buf + b, sizeof(buf) - b, MZWS); // pause on zero width space char
    b += (size_t)snprintf(buf + b, sizeof(buf) - b, "%s  ", wi->csv[WTTRIN_CSV_FIELD_C]);
    g_infos.wttrin.weather = detect_weather(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_C]); // wttrin emoji

    const int marquee_pause = 1000; // 1 sec pause
    const int width = make_wttrin_marquee_conds_width(g_term_w, wi);
    marquee_init(&wi->marquee_conds, buf, width, marquee_pause / SLEEP_MS_DRAW, 2);
}

static int make_wttrin_marquee_times_width(int term_width)
{
    return term_width;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
static char s_mk_str_temp_buff[SMALLBUFF];
static char* mk_str(const char* format, char* param)
{
    snprintf(s_mk_str_temp_buff, sizeof(s_mk_str_temp_buff), format, param);
    return s_mk_str_temp_buff;
}
#pragma GCC diagnostic pop

// "sunset, sunrise, ..." marquee & moon phase status
static void display_status_times(struct WttrinInfo* wi)
{
    char buf[MIDBUFF];
    size_t b = 0;
    // clang-format on
    b += ctext_fg(buf + b, sizeof(buf) - b, timeofday_to_color(TIME_OF_DAY_DAWN), mk_str(MZWS "🌄%s", wi->csv[WTTRIN_CSV_FIELD_D]));
    b += ctext_fg(buf + b, sizeof(buf) - b, timeofday_to_color(TIME_OF_DAY_MORNING), mk_str(MZWS "🌅%s", wi->csv[WTTRIN_CSV_FIELD_S]));
    b += ctext_fg(buf + b, sizeof(buf) - b, timeofday_to_color(TIME_OF_DAY_ZENITH), mk_str(MZWS "🌞%s", wi->csv[WTTRIN_CSV_FIELD_z]));
    b += ctext_fg(buf + b, sizeof(buf) - b, timeofday_to_color(TIME_OF_DAY_SUNSET), mk_str(MZWS "🌇%s", wi->csv[WTTRIN_CSV_FIELD_s]));
    b += ctext_fg(buf + b, sizeof(buf) - b, timeofday_to_color(TIME_OF_DAY_NIGHT), mk_str(MZWS "🌆%s", wi->csv[WTTRIN_CSV_FIELD_d]));
    // clang-format off

    const int marquee_pause = 3000; // 3 sec pause
    const int width = make_wttrin_marquee_times_width(g_term_w);
    marquee_init(&wi->marquee_times, buf, width, marquee_pause / SLEEP_MS_DRAW, 3);

}

// must infos_wttrin_init before running this
enum RequestStatus infos_wttrin_update(void)
{
    struct Request request = {0};
    request.status = REQUEST_STATUS_RUNNING;
    request.url = URL_WTTRIN;
    request.request_format = REQUEST_FORMAT_WTTRIN;
    request.timeout_ms = TIMEOUT_MS_WTTRIN;
    request.remember_response = 1;

    pthread_mutex_lock(&s_infos_wttrin_mutex);
    g_infos.wttrin.status = REQUEST_STATUS_RUNNING;
    pthread_mutex_unlock(&s_infos_wttrin_mutex);

    request_send(&request);

    pthread_mutex_lock(&s_infos_wttrin_mutex);

    g_infos.wttrin.status = request.status;

    if (request.output.buf) {
        // parse csv
        g_infos.wttrin.csv_parsed = parse_csv(request.output.buf,
                                      URL_WTTRIN_OUTPUT_CSV_SEP,
                                      URL_WTTRIN_OUTPUT_MAX_FIELDS,
                                      URL_WTTRIN_OUTPUT_MAX_FIELD_LEN,
                                      (char*)g_infos.wttrin.csv);

        logger_wttrin_write("%s\n", request.output.buf);
        free((void*)request.output.buf);

        D(printf("WTTRIN PARSED: %d\n", g_infos.wttrin.csv_parsed));

        if (g_infos.wttrin.csv_parsed < URL_WTTRIN_OUTPUT_MAX_FIELDS) {

            pthread_mutex_unlock(&s_infos_wttrin_mutex);
            DPL("FAILED TO PARSE WTTRIN");
            return request.status;
        }

        g_infos.wttrin.valid = true;

        // // override weather cond
        // snprintf(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_C], sizeof(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_C]), "Moderate or heavy rain in area with thunder");


        // parse seconds for wttrin_to_timeofday
        g_infos.wttrin.dawn = hms_to_today_seconds_str(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_D]);
        g_infos.wttrin.sunrise = hms_to_today_seconds_str(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_S]);
        g_infos.wttrin.zenith = hms_to_today_seconds_str(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_z]);
        g_infos.wttrin.zenith_duration = g_infos.wttrin.zenith + 1 * 3600; // duration 1 hour
        g_infos.wttrin.sunset = hms_to_today_seconds_str(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_s]);
        g_infos.wttrin.dusk = hms_to_today_seconds_str(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_d]);

        trim_spaces(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_c]); // trim spaces from emoji

        // // make times smaller '06:09:47' -> '6:09' = remove seconds + remove it if it's padded with 0 on left
        //trim_right(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_D], 3);
        //trim_right(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_S], 3);
        //trim_right(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_z], 3);
        //trim_right(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_s], 3);
        //trim_right(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_d], 3);
        //if (g_infos.wttrin.csv[WTTRIN_CSV_FIELD_D][0] == '0') trim_left(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_D], 1);
        //if (g_infos.wttrin.csv[WTTRIN_CSV_FIELD_S][0] == '0') trim_left(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_S], 1);
        //if (g_infos.wttrin.csv[WTTRIN_CSV_FIELD_z][0] == '0') trim_left(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_z], 1);
        //if (g_infos.wttrin.csv[WTTRIN_CSV_FIELD_s][0] == '0') trim_left(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_s], 1);
        //if (g_infos.wttrin.csv[WTTRIN_CSV_FIELD_d][0] == '0') trim_left(g_infos.wttrin.csv[WTTRIN_CSV_FIELD_d], 1);

        display_status_weather_conditions(&g_infos.wttrin);
        display_status_times(&g_infos.wttrin);

        D(print_wttrin_info(&g_infos.wttrin));
    }


    pthread_mutex_unlock(&s_infos_wttrin_mutex);
    return request.status;
}

enum RequestStatus infos_wttrin_health(void)
{
    pthread_mutex_lock(&s_infos_wttrin_mutex);
    enum RequestStatus r = g_infos.wttrin.status;
    pthread_mutex_unlock(&s_infos_wttrin_mutex);
    return r;
}

void infos_wttrin_marquee_conds_scroll(void)
{
    pthread_mutex_lock(&s_infos_wttrin_mutex);
    marquee_scroll_smart(&g_infos.wttrin.marquee_conds);
    pthread_mutex_unlock(&s_infos_wttrin_mutex);
}

void infos_wttrin_marquee_conds_update_width(int term_width)
{
    pthread_mutex_lock(&s_infos_wttrin_mutex);
    marquee_update_width(&g_infos.wttrin.marquee_conds, make_wttrin_marquee_conds_width(term_width, &g_infos.wttrin));
    pthread_mutex_unlock(&s_infos_wttrin_mutex);
}

void infos_wttrin_marquee_times_scroll(void)
{
    pthread_mutex_lock(&s_infos_wttrin_mutex);
    marquee_scroll_smart(&g_infos.wttrin.marquee_times);
    pthread_mutex_unlock(&s_infos_wttrin_mutex);
}

void infos_wttrin_marquee_times_update_width(int term_width)
{
    pthread_mutex_lock(&s_infos_wttrin_mutex);
    marquee_update_width(&g_infos.wttrin.marquee_times, term_width);
    pthread_mutex_unlock(&s_infos_wttrin_mutex);
}

void infos_save(void)
{
    pthread_mutex_lock(&s_infos_bas_mutex);
    pthread_mutex_lock(&s_infos_wttrin_mutex);

    save_infos(VAR_DIR_FILE_INFOS_BIN, &g_infos);

    pthread_mutex_unlock(&s_infos_bas_mutex);
    pthread_mutex_unlock(&s_infos_wttrin_mutex);
}

