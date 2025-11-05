#include "debug.h"
#include "globals.h"
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

static pthread_mutex_t g_update_info_wttrin_mutex = PTHREAD_MUTEX_INITIALIZER;

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
    update_info_wttrin_safe_io(&wttrin, &g_info.wttrin);
}

#define MZWS MARQUEE_ZERO_WIDTH_SPACE

static void make_wttrin_time(struct wttrin_info* wttrin)
{
    size_t b = 0;
    b += dt_HM(wttrin->time + b, sizeof(wttrin->time) - b); // prepend hour:minute
    b += snprintf(wttrin->time + b, sizeof(wttrin->time) - b, ":");
}

static int make_wttrin_marquee_conds_width(int term_width, struct wttrin_info* wttrin)
{
    int other = utf8_display_width(wttrin->time)                      // time -- e.g. "12:12:"
                + 1                                                   // space
                + utf8_display_width(wttrin->csv[WTTRIN_CSV_FIELD_c]) // emojis -- e.g "☀ "
                + 1;                                                  // space

    int ret = term_width - other;
    return ret;
}

static void make_wttrin_marquee_conds(struct wttrin_info* wttrin)
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

static void make_wttrin_marquee_times(struct wttrin_info* wttrin)
{
    size_t b = 0;
    b += snprintf(wttrin->marquee_times_buf + b, sizeof(wttrin->marquee_times_buf) - b,
                  // MZWS "🌄 Dawn %s, " MZWS "🌅 Sunrise %s, " MZWS "🌞 Zenith %s, " MZWS "🌇 Sunset %s, " MZWS "🌆 Dusk %s  ",
                  MZWS "🌄 %s " MZWS "🌅 %s " MZWS "🌞 %s " MZWS "🌇 %s " MZWS "🌆 %s ",
                  wttrin->csv[WTTRIN_CSV_FIELD_D],
                  wttrin->csv[WTTRIN_CSV_FIELD_S],
                  wttrin->csv[WTTRIN_CSV_FIELD_z],
                  wttrin->csv[WTTRIN_CSV_FIELD_s],
                  wttrin->csv[WTTRIN_CSV_FIELD_d]);

    const int marquee_pause = 3000; // 3 sec pause
    const int width = make_wttrin_marquee_times_width(g_term_w);
    marquee_init(&wttrin->marquee_times, wttrin->marquee_times_buf, width, marquee_pause / SLEEP_MS_DRAW, 3);
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
    update_info_wttrin_safe_io(&g_info.wttrin, &wttrin);
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

        // // override weather cond
        // snprintf(wttrin.csv[WTTRIN_CSV_FIELD_C], sizeof(wttrin.csv[WTTRIN_CSV_FIELD_C]), "Moderate or heavy rain in area with thunder");

        // wttrin emoji
        wttrin.weather = detect_weather(wttrin.csv[WTTRIN_CSV_FIELD_C]);

        // make marquees
        make_wttrin_time(&wttrin);
        make_wttrin_marquee_conds(&wttrin);
        make_wttrin_marquee_times(&wttrin);
    }

    update_info_wttrin_safe_io(&wttrin, &g_info.wttrin);

    return request.status;
}

void update_info_wttrin_marquee_conds_scroll()
{
    pthread_mutex_lock(&g_update_info_wttrin_mutex);
    marquee_scroll_smart(&g_info.wttrin.marquee_conds);
    pthread_mutex_unlock(&g_update_info_wttrin_mutex);
}

void update_info_wttrin_marquee_conds_update_width(int term_width)
{
    pthread_mutex_lock(&g_update_info_wttrin_mutex);
    marquee_update_width(&g_info.wttrin.marquee_conds, make_wttrin_marquee_conds_width(term_width, &g_info.wttrin));
    pthread_mutex_unlock(&g_update_info_wttrin_mutex);
}

void update_info_wttrin_marquee_times_scroll()
{
    pthread_mutex_lock(&g_update_info_wttrin_mutex);
    marquee_scroll_smart(&g_info.wttrin.marquee_times);
    pthread_mutex_unlock(&g_update_info_wttrin_mutex);
}

void update_info_wttrin_marquee_times_update_width(int term_width)
{
    pthread_mutex_lock(&g_update_info_wttrin_mutex);
    marquee_update_width(&g_info.wttrin.marquee_times, term_width);
    pthread_mutex_unlock(&g_update_info_wttrin_mutex);
}
