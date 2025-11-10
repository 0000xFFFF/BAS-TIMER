#include "debug.h"
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

void print_bas_info(const struct BasInfo* b)
{
    if (!b) return;

    printf("===[ info.bas\n");
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
    printf("opt_auto_timer_status: %d\n", b->opt_auto_timer_status);
    printf("opt_auto_gas: %d\n", b->opt_auto_gas);
    printf("opt_auto_gas_status: %d\n", b->opt_auto_gas_status);

    printf("history_mode: %d\n", b->history_mode);
    printf("history_mode_time_changed: %d\n", b->history_mode_time_changed);
    printf("history_mode_time_on: %d\n", b->history_mode_time_on);
    printf("history_mode_time_off: %d\n", b->history_mode_time_off);

    printf("history_gas: %d\n", b->history_gas);
    printf("history_gas_time_changed: %d\n", b->history_gas_time_changed);
    printf("history_gas_time_on: %d\n", b->history_gas_time_on);
    printf("history_gas_time_off: %d\n", b->history_gas_time_off);
}

void print_wttrin_info(const struct WttrinInfo* info)
{
    if (!info) return;

    printf("===[ info.wttrin\n");
    printf("valid: %s\n", info->valid ? "true" : "false");
    printf("status: %d\n", (int)info->status);
    printf("weather: %d\n", (int)info->weather);
    printf("time: %s\n", info->time);
    printf("marquee_conds.text: %s\n", info->marquee_conds.text);
    printf("marquee_times.text: %s\n", info->marquee_times.text);
    printf("Weather condition                    : %s\n", info->csv[WTTRIN_CSV_FIELD_c]);
    printf("Weather condition textual name       : %s\n", info->csv[WTTRIN_CSV_FIELD_C]);
    printf("Weather condition  plain-text symbol : %s\n", info->csv[WTTRIN_CSV_FIELD_x]);
    printf("Humidity                             : %s\n", info->csv[WTTRIN_CSV_FIELD_h]);
    printf("Temperature (Actual)                 : %s\n", info->csv[WTTRIN_CSV_FIELD_t]);
    printf("Temperature (Feels Like)             : %s\n", info->csv[WTTRIN_CSV_FIELD_f]);
    printf("Wind                                 : %s\n", info->csv[WTTRIN_CSV_FIELD_w]);
    printf("Location                             : %s\n", info->csv[WTTRIN_CSV_FIELD_l]);
    printf("Moon phase 🌑🌒🌓🌔🌕🌖🌗🌘          : %s\n", info->csv[WTTRIN_CSV_FIELD_m]);
    printf("Moon day                             : %s\n", info->csv[WTTRIN_CSV_FIELD_M]);
    printf("Precipitation (mm/3 hours)           : %s\n", info->csv[WTTRIN_CSV_FIELD_p]);
    printf("Pressure (hPa)                       : %s\n", info->csv[WTTRIN_CSV_FIELD_P]);
    printf("UV index (1-12)                      : %s\n", info->csv[WTTRIN_CSV_FIELD_u]);
    printf("Dawn*                                : %s\n", info->csv[WTTRIN_CSV_FIELD_D]);
    printf("Sunrise*                             : %s\n", info->csv[WTTRIN_CSV_FIELD_S]);
    printf("Zenith*                              : %s\n", info->csv[WTTRIN_CSV_FIELD_z]);
    printf("Sunset*                              : %s\n", info->csv[WTTRIN_CSV_FIELD_s]);
    printf("Dusk*                                : %s\n", info->csv[WTTRIN_CSV_FIELD_d]);
    printf("Current time*                        : %s\n", info->csv[WTTRIN_CSV_FIELD_T]);
    printf("Local timezone.                      : %s\n", info->csv[WTTRIN_CSV_FIELD_Z]);
    printf("csv_parsed: %d\n", info->csv_parsed);
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
                return w;
            }
        }
    }
    return WEATHER_UNKNOWN;
}

int parse_csv(const char* input, char sep, int nfields, int field_size, char fields[][field_size])
{
    int field = 0;
    int pos = 0;

    for (const char* p = input; *p && field < nfields; p++) {
        if (*p == sep) {
            fields[field][pos] = '\0';
            field++;
            pos = 0;
        }
        else if (pos < field_size - 1) {
            fields[field][pos++] = *p;
        }
    }

    if (field < nfields) {
        fields[field][pos] = '\0';
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
    int now = now_seconds();
    if      (now < wttrin->dawn)                                      return TIME_OF_DAY_BEFORE_DAWN;
    else if (now < wttrin->sunrise)                                   return TIME_OF_DAY_DAWN;
    else if (now < wttrin->zenith)                                    return TIME_OF_DAY_MORNING;
    else if (now >= wttrin->zenith && now <= wttrin->zenith_duration) return TIME_OF_DAY_ZENITH;
    else if (now < wttrin->sunset)                                    return TIME_OF_DAY_AFTERNOON;
    else if (now < wttrin->dusk)                                      return TIME_OF_DAY_SUNSET;
    else                                                              return TIME_OF_DAY_NIGHT;
    // clang-format on
}

enum TimeOfDay timeofday()
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
        case TIME_OF_DAY_BEFORE_DAWN: return 99;
        case TIME_OF_DAY_DAWN:        return 130;
        case TIME_OF_DAY_MORNING:     return 214;
        case TIME_OF_DAY_ZENITH:      return 226;
        case TIME_OF_DAY_AFTERNOON:   return 220;
        case TIME_OF_DAY_SUNSET:      return 202;
        case TIME_OF_DAY_NIGHT:       return 105;
        default:                      return 255;
    }
    // clang-format on
}

int wttrin_timeofday_color(struct WttrinInfo* wttrin)
{
    return timeofday_to_color(wttrin_to_timeofday(wttrin));
}
