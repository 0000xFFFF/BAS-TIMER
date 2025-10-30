#include "globals.h"
#include "mongoose.h"
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

void update_info_bas_safe_swap(struct bas_info* in, struct bas_info* out)
{
    pthread_mutex_lock(&g_update_info_bas_mutex);
    memcpy(out, in, sizeof(struct bas_info));
    pthread_mutex_unlock(&g_update_info_bas_mutex);
}

long long g_global_unix_counter = 0;

struct bas_info g_info = {0};

bool update_info_bas()
{
    g_global_unix_counter++;
    char request_url[BIGBUFF];
    snprintf(request_url, BIGBUFF, "%s&_=%lld", URL_VARS, g_global_unix_counter);

    struct Request request = {0};
    request.status = REQUEST_STATUS_RUNNING;
    request.url = request_url;
    request.request_format = REQUEST_FORMAT_BAS;
    request.timeout_ms = TIMEOUT_BAS;
    request.remember_response = 1;
    request_send(&request);

    if (request.output.buf) {

        struct bas_info info = {0};

        info.valid = true;
        info.status = request.status;
        info.mod_rada = extract_json_label(request.output, "$.mod_rada");
        info.mod_rezim = extract_json_label(request.output, "$.mod_rezim");
        info.StatusPumpe3 = extract_json_label(request.output, "$.StatusPumpe3");
        info.StatusPumpe4 = extract_json_label(request.output, "$.StatusPumpe4");
        info.StatusPumpe5 = extract_json_label(request.output, "$.StatusPumpe5");
        info.StatusPumpe6 = extract_json_label(request.output, "$.StatusPumpe6");
        info.StatusPumpe7 = extract_json_label(request.output, "$.StatusPumpe7");
        info.Tspv = extract_json_label(request.output, "$.Tspv");
        info.Tsolar = extract_json_label(request.output, "$.Tsolar");
        info.Tzadata = extract_json_label(request.output, "$.Tzadata");
        info.Tfs = extract_json_label(request.output, "$.Tfs");
        info.Tmax = extract_json_label(request.output, "$.Tmax");
        info.Tmin = extract_json_label(request.output, "$.Tmin");
        info.Tsobna = extract_json_label(request.output, "$.Tsobna");

        // calc other values
        info.Tmid = (info.Tmax + info.Tmin) / 2;
        info.TminLT = g_info.Tmin < 45;
        info.TmidGE = g_info.Tmid >= 60;
        if (info.peaks_valid) {
            info.peak_min_solar = mind(info.peak_min_solar, info.Tsolar);
            info.peak_max_solar = maxd(info.peak_max_solar, info.Tsolar);
            info.peak_min_human = mind(info.peak_min_human, info.Tsobna, info.Tzadata, info.Tspv);
            info.peak_max_human = maxd(info.peak_max_human, info.Tsobna, info.Tzadata, info.Tspv);
            info.peak_min_buf = mind(info.peak_min_buf, info.Tmin);
            info.peak_max_buf = maxd(info.peak_max_buf, info.Tmax);
            info.peak_min_circ = mind(info.peak_min_circ, info.Tfs);
            info.peak_max_circ = maxd(info.peak_max_circ, info.Tfs);
        }
        else {
            info.peaks_valid = true;
            info.peak_min_solar = info.Tsolar;
            info.peak_max_solar = info.Tsolar;
            info.peak_min_human = mind(info.Tsobna, info.Tzadata, info.Tspv);
            info.peak_max_human = maxd(info.Tsobna, info.Tzadata, info.Tspv);
            info.peak_min_buf = info.Tmin;
            info.peak_max_buf = info.Tmax;
            info.peak_min_circ = info.Tfs;
            info.peak_max_circ = info.Tfs;
        }

        update_info_bas_safe_swap(&info, &g_info);

        free((void*)request.output.buf);
    }

    if (g_info.valid) remember_vars_do_action(g_info.mod_rada, g_info.StatusPumpe4, g_info.TminLT, g_info.TmidGE);
    return g_info.valid;
}

static pthread_mutex_t g_update_info_wttrin_mutex = PTHREAD_MUTEX_INITIALIZER;

void update_info_wttrin_safe_swap(const char* in, char* out)
{
    pthread_mutex_lock(&g_update_info_wttrin_mutex);
    memcpy(out, in, sizeof(g_wttrin_buffer));
    pthread_mutex_lock(&g_update_info_wttrin_mutex);
}

char g_wttrin_buffer[BIGBUFF] = {0};

bool update_info_wttrin()
{
    struct Request request = {0};
    request.status = REQUEST_STATUS_RUNNING;
    request.url = URL_WTTRIN;
    request.request_format = REQUEST_FORMAT_WTTRIN;
    request.timeout_ms = TIMEOUT_WTTRIN;
    request.remember_response = 1;
    request_send(&request);

    if (request.output.buf) {

        char response[BIGBUFF] = {0};

        size_t b = 0;
        b += snprintf(response + b, BIGBUFF - b, "%s", request.output.buf); // write response to buffer
        size_t l = strlen(response);
        if (response[l - 1] == '\n') { response[l - 1] = ' '; } // replace newline with space
        b += dt_HM(response + b, BIGBUFF - b);                  // append hour:minute
        free((void*)request.output.buf);

        update_info_wttrin_safe_swap(response, g_wttrin_buffer);
        return true;
    }
    return false;
}
