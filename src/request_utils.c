#include "debug.h"
#include "mongoose.h"
#include "request.h"
#include "utils.h"
#include <float.h>
#include <inttypes.h>
#include <stdio.h>

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

void print_bas_info(const struct BasInfo* b)
{
    if (!b) return;

    int clm_len = 33;
    printf("===[ info.bas ]=============================\n");
    printf("%*s : %s\n", clm_len, "valid", b->valid ? "true" : "false");
    printf("%*s : %d\n", clm_len, "status", (int)b->status);
    printf("%*s : %d\n", clm_len, "mod_rada", b->mod_rada);
    printf("%*s : %d\n", clm_len, "mod_rezim", b->mod_rezim);
    printf("%*s : %d\n", clm_len, "StatusPumpe3", (int)b->StatusPumpe3);
    printf("%*s : %d\n", clm_len, "StatusPumpe4", (int)b->StatusPumpe4);
    printf("%*s : %d\n", clm_len, "StatusPumpe5", (int)b->StatusPumpe5);
    printf("%*s : %d\n", clm_len, "StatusPumpe6", (int)b->StatusPumpe6);
    printf("%*s : %d\n", clm_len, "StatusPumpe7", (int)b->StatusPumpe7);
    printf("%*s : %.2f\n", clm_len, "Tspv", b->Tspv);
    printf("%*s : %.2f\n", clm_len, "Tsolar", b->Tsolar);
    printf("%*s : %.2f\n", clm_len, "Tzadata", b->Tzadata);
    printf("%*s : %.2f\n", clm_len, "Tfs", b->Tfs);
    printf("%*s : %.2f\n", clm_len, "Tmax", b->Tmax);
    printf("%*s : %.2f\n", clm_len, "Tmin", b->Tmin);
    printf("%*s : %.2f\n", clm_len, "Tsobna", b->Tsobna);
    printf("%*s : %.2f\n", clm_len, "Tmid", b->Tmid);
    printf("%*s : %d\n", clm_len, "TminLT", b->TminLT);
    printf("%*s : %d\n", clm_len, "TmidGE", b->TmidGE);
    printf("%*s : %d\n", clm_len, "TmaxGE", b->TmaxGE);
    printf("%*s : %.2f\n", clm_len, "peak_min_solar", b->peak_min_solar);
    printf("%*s : %.2f\n", clm_len, "peak_max_solar", b->peak_max_solar);
    printf("%*s : %.2f\n", clm_len, "peak_min_human", b->peak_min_human);
    printf("%*s : %.2f\n", clm_len, "peak_max_human", b->peak_max_human);
    printf("%*s : %.2f\n", clm_len, "peak_min_buf", b->peak_min_buf);
    printf("%*s : %.2f\n", clm_len, "peak_max_buf", b->peak_max_buf);
    printf("%*s : %.2f\n", clm_len, "peak_min_circ", b->peak_min_circ);
    printf("%*s : %.2f\n", clm_len, "peak_max_circ", b->peak_max_circ);
    printf("%*s : %s\n", clm_len, "opt_auto_timer", b->opt_auto_timer ? "true" : "false");
    printf("%*s : %d\n", clm_len, "opt_auto_timer_seconds", b->opt_auto_timer_seconds);
    printf("%*s : %d\n", clm_len, "opt_auto_timer_seconds_old", b->opt_auto_timer_seconds_old);
    printf("%*s : %s\n", clm_len, "opt_auto_timer_started", b->opt_auto_timer_started ? "true" : "false");
    printf("%*s : %" PRIu64 "\n", clm_len, "opt_auto_timer_seconds_elapsed", b->opt_auto_timer_seconds_elapsed);
    printf("%*s : %d\n", clm_len, "opt_auto_timer_status", (int)b->opt_auto_timer_status);
    printf("%*s : %" PRId64 "\n", clm_len, "opt_auto_timer_status_changed", (int64_t)b->opt_auto_timer_status_changed);
    printf("%*s : %s\n", clm_len, "opt_auto_gas", b->opt_auto_gas ? "true" : "false");
    printf("%*s : %d\n", clm_len, "opt_auto_gas_status", (int)b->opt_auto_gas_status);
    printf("%*s : %" PRId64 "\n", clm_len, "opt_auto_gas_status_changed", (int64_t)b->opt_auto_gas_status_changed);
    printf("%*s : %d\n", clm_len, "history_mode", b->history_mode);
    printf("%*s : %" PRId64 "\n", clm_len, "history_mode_time_changed", (int64_t)b->history_mode_time_changed);
    printf("%*s : %" PRId64 "\n", clm_len, "history_mode_time_on", (int64_t)b->history_mode_time_on);
    printf("%*s : %" PRId64 "\n", clm_len, "history_mode_time_off", (int64_t)b->history_mode_time_off);
    printf("%*s : %d\n", clm_len, "history_gas", (int)b->history_gas);
    printf("%*s : %" PRId64 "\n", clm_len, "history_gas_time_changed", (int64_t)b->history_gas_time_changed);
    printf("%*s : %" PRId64 "\n", clm_len, "history_gas_time_on", (int64_t)b->history_gas_time_on);
    printf("%*s : %" PRId64 "\n", clm_len, "history_gas_time_off", (int64_t)b->history_gas_time_off);
    printf("%*s : %d\n", clm_len, "radiator_color", b->radiator_color);
    printf("%*s : %d\n", clm_len, "radiator_color_index", b->radiator_color_index);
    printf("%*s : %" PRId64 "\n", clm_len, "radiator_color_last_update", (int64_t)b->radiator_color_last_update);
    printf("%*s : %.3f\n", clm_len, "radiator_color_current_temp_ratio", b->radiator_color_current_temp_ratio);
    printf("%*s : %.2f\n", clm_len, "schedules_t_min", b->schedules_t_min);
    printf("%*s : %d\n", clm_len, "schedules_last_yday", b->schedules_last_yday);
    for (int i = 0; i < HEAT_SCHEDULES_COUNT; i++) {
        if (!b->schedules[i].valid) continue;
        printf("%*s : %d - %d -> %d, dur: %d\n", clm_len, "schedules", i, b->schedules[i].from, b->schedules[i].to, b->schedules[i].duration);
    }
    printf("%*s : %d\n", clm_len, "schedules_last_yday", b->schedules_last_yday);

    printf("============================================\n");
}

void print_wttrin_info(const struct WttrinInfo* info)
{
    if (!info) return;

    int cl1 = 18;
    printf("===[ info.wttrin\n");
    printf("%*s : %s\n", cl1, "valid", info->valid ? "true" : "false");
    printf("%*s : %d\n", cl1, "status", (int)info->status);
    printf("%*s : %d\n", cl1, "weather", (int)info->weather);
    printf("%*s : %s\n", cl1, "time", info->time);
    printf("%*s : %s\n", cl1, "marquee_conds.text", info->marquee_conds.text);
    printf("%*s : %s\n", cl1, "marquee_times.text", info->marquee_times.text);

    int cl2 = 18;
    // clang-format off
    printf("%*s : %s\n", cl2, "Weather",            info->csv[WTTRIN_CSV_FIELD_c]);
    printf("%*s : %s\n", cl2, "WC textual name",    info->csv[WTTRIN_CSV_FIELD_C]);
    printf("%*s : %s\n", cl2, "WC text symbol",     info->csv[WTTRIN_CSV_FIELD_x]);
    printf("%*s : %s\n", cl2, "Humidity",           info->csv[WTTRIN_CSV_FIELD_h]);
    printf("%*s : %s\n", cl2, "T (Actual)",         info->csv[WTTRIN_CSV_FIELD_t]);
    printf("%*s : %s\n", cl2, "T (Feels Like)",     info->csv[WTTRIN_CSV_FIELD_f]);
    printf("%*s : %s\n", cl2, "Wind",               info->csv[WTTRIN_CSV_FIELD_w]);
    printf("%*s : %s\n", cl2, "Location",           info->csv[WTTRIN_CSV_FIELD_l]);
    printf("%*s : %s\n", cl2, "Moon phase",         info->csv[WTTRIN_CSV_FIELD_m]);
    printf("%*s : %s\n", cl2, "Moon day",           info->csv[WTTRIN_CSV_FIELD_M]);
    printf("%*s : %s\n", cl2, "Prec. (mm/3 hours)", info->csv[WTTRIN_CSV_FIELD_p]);
    printf("%*s : %s\n", cl2, "Pressure (hPa)",     info->csv[WTTRIN_CSV_FIELD_P]);
    printf("%*s : %s\n", cl2, "UV index (1-12)",    info->csv[WTTRIN_CSV_FIELD_u]);
    printf("%*s : %s\n", cl2, "Dawn",               info->csv[WTTRIN_CSV_FIELD_D]);
    printf("%*s : %s\n", cl2, "Sunrise",            info->csv[WTTRIN_CSV_FIELD_S]);
    printf("%*s : %s\n", cl2, "Zenith",             info->csv[WTTRIN_CSV_FIELD_z]);
    printf("%*s : %s\n", cl2, "Sunset",             info->csv[WTTRIN_CSV_FIELD_s]);
    printf("%*s : %s\n", cl2, "Dusk",               info->csv[WTTRIN_CSV_FIELD_d]);
    printf("%*s : %s\n", cl2, "Time",               info->csv[WTTRIN_CSV_FIELD_T]);
    printf("%*s : %s\n", cl2, "Local timezone.",    info->csv[WTTRIN_CSV_FIELD_Z]);
    printf("csv_parsed: %d\n",                      info->csv_parsed);
    // clang-format on
}

void print_infos(const struct Infos* info)
{
    print_bas_info(&info->bas);
    print_wttrin_info(&info->wttrin);
}

enum Weather detect_weather(const char* text)
{
    for (int w = WEATHER_CLEAR; w <= WEATHER_SNOW; w++) {
        for (int k = 0; weather_keywords[w][k] != NULL; k++) {
            if (istrstr(text, weather_keywords[w][k])) {
                return (enum Weather)w;
            }
        }
    }
    return WEATHER_UNKNOWN;
}

int parse_csv(const char* input, char sep, int nfields, int field_size, char* fields)
{
    int field = 0;
    int pos = 0;

    for (const char* p = input; *p && field < nfields; p++) {
        if (*p == sep) {
            fields[field * field_size + pos] = '\0';
            field++;
            pos = 0;
        }
        else if (pos < field_size - 1) {
            fields[field * field_size + pos++] = *p;
        }
    }

    if (field < nfields) {
        fields[field * field_size + pos] = '\0';
        field++;
    }

    return field;
}

int save_infos(const char* filename, const struct Infos* info)
{
    FILE* fp = fopen(filename, "wb");
    if (!fp) return 0;

    DPL("SAVED INFOS TO DISK");
    D(print_infos(info));

    size_t written = fwrite(info, sizeof(struct Infos), 1, fp);
    fclose(fp);

    return written == 1;
}

int load_infos(const char* filename, struct Infos* info)
{
    FILE* fp = fopen(filename, "rb");
    if (!fp) return 0;

    size_t read = fread(info, sizeof(struct Infos), 1, fp);
    fclose(fp);

    DPL("LOADED INFOS FROM DISK");
    D(print_infos(info));

    return read == 1;
}

enum TimeOfDay wttrin_to_timeofday(struct WttrinInfo* wttrin)
{
    if (!wttrin->valid) return TIME_OF_DAY_UNKNOWN;

    // clang-format off
    int now = now_to_today_seconds();
    if      (now < wttrin->dawn)                                      return TIME_OF_DAY_BEFORE_DAWN;
    else if (now < wttrin->sunrise)                                   return TIME_OF_DAY_DAWN;
    else if (now < wttrin->zenith)                                    return TIME_OF_DAY_MORNING;
    else if (now >= wttrin->zenith && now <= wttrin->zenith_duration) return TIME_OF_DAY_ZENITH;
    else if (now < wttrin->sunset)                                    return TIME_OF_DAY_AFTERNOON;
    else if (now < wttrin->dusk)                                      return TIME_OF_DAY_SUNSET;
    else                                                              return TIME_OF_DAY_NIGHT;
    // clang-format on
}

enum TimeOfDay timeofday(void)
{
    // clang-format off
    int hour = localtime_hour();
    if      (hour >= 4 && hour < 6)   return TIME_OF_DAY_BEFORE_DAWN;
    else if (hour >= 6 && hour < 7)   return TIME_OF_DAY_DAWN;
    else if (hour >= 7 && hour < 12)  return TIME_OF_DAY_MORNING;
    else if (hour >= 12 && hour < 13) return TIME_OF_DAY_ZENITH;
    else if (hour >= 13 && hour < 17) return TIME_OF_DAY_AFTERNOON;
    else if (hour >= 17 && hour < 19) return TIME_OF_DAY_SUNSET;
    else                              return TIME_OF_DAY_NIGHT;
    // clang-format on
}

int timeofday_to_color(enum TimeOfDay tod)
{
    // clang-format off
    switch (tod) {
        default:
        case TIME_OF_DAY_UNKNOWN:     return 255;
        case TIME_OF_DAY_BEFORE_DAWN: return 99;
        case TIME_OF_DAY_DAWN:        return 130;
        case TIME_OF_DAY_MORNING:     return 214;
        case TIME_OF_DAY_ZENITH:      return 226;
        case TIME_OF_DAY_AFTERNOON:   return 220;
        case TIME_OF_DAY_SUNSET:      return 202;
        case TIME_OF_DAY_NIGHT:       return 105;
    }
    // clang-format on
}

int wttrin_timeofday_color(struct WttrinInfo* wttrin)
{
    return timeofday_to_color(wttrin_to_timeofday(wttrin));
}
