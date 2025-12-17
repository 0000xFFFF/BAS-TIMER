#ifndef REQUEST_H
#define REQUEST_H

#include "globals.h"
#include "marquee.h"
#include "mongoose.h"
#include <stdatomic.h>

extern const char* URL_VARS;
extern const char* URL_HEAT_OFF;
extern const char* URL_HEAT_ON;
extern const char* URL_GAS_OFF;
extern const char* URL_GAS_ON;
extern const char* REQUEST_FORMAT_BAS;
extern const char* REQUEST_FORMAT_WTTRIN;

#define URL_WTTRIN_OUTPUT_CSV_SEP       '|' // can't use ',' --- WTTRIN_CSV_FIELD_l has ',' inside it
#define URL_WTTRIN_OUTPUT_MAX_FIELDS    20
#define URL_WTTRIN_OUTPUT_MAX_FIELD_LEN 64
enum WttrinCsvField {
    WTTRIN_CSV_FIELD_c, // c -- Weather condition,
    WTTRIN_CSV_FIELD_C, // C -- Weather condition textual name,
    WTTRIN_CSV_FIELD_x, // x -- Weather condition, plain-text symbol,
    WTTRIN_CSV_FIELD_h, // h -- Humidity,
    WTTRIN_CSV_FIELD_t, // t -- Temperature (Actual),
    WTTRIN_CSV_FIELD_f, // f -- Temperature (Feels Like),
    WTTRIN_CSV_FIELD_w, // w -- Wind,
    WTTRIN_CSV_FIELD_l, // l -- Location,
    WTTRIN_CSV_FIELD_m, // m -- Moon phase 🌑🌒🌓🌔🌕🌖🌗🌘,
    WTTRIN_CSV_FIELD_M, // M -- Moon day,
    WTTRIN_CSV_FIELD_p, // p -- Precipitation (mm/3 hours),
    WTTRIN_CSV_FIELD_P, // P -- Pressure (hPa),
    WTTRIN_CSV_FIELD_u, // u -- UV index (1-12),
    WTTRIN_CSV_FIELD_D, // D -- Dawn*,
    WTTRIN_CSV_FIELD_S, // S -- Sunrise*,
    WTTRIN_CSV_FIELD_z, // z -- Zenith*,
    WTTRIN_CSV_FIELD_s, // s -- Sunset*,
    WTTRIN_CSV_FIELD_d, // d -- Dusk*,
    WTTRIN_CSV_FIELD_T, // T -- Current time*,
    WTTRIN_CSV_FIELD_Z  // Z -- Local timezone.
};

extern const char* URL_WTTRIN;

enum RequestStatus {
    REQUEST_STATUS_DISABLED,
    REQUEST_STATUS_RUNNING,
    REQUEST_STATUS_DONE,
    REQUEST_STATUS_ERROR_TIMEOUT,
    REQUEST_STATUS_ERROR_CONN,
};

struct Request {
    bool log;
    enum RequestStatus status;
    const char* url;
    const char* request_format;
    uint64_t timeout_ms;
    uint64_t timeout_ms_deadline;
    int remember_response;
    struct mg_str output;
};

enum PumpStatus {
    PUMP_STATUS_UNKNOWN = -1,
    PUMP_STATUS_AUTO_OFF = 0,
    PUMP_STATUS_AUTO_ON = 1,
    PUMP_STATUS_MANUAL_OFF = 2,
    PUMP_STATUS_MANUAL_ON = 3
};

enum OptStatus {
    OPT_STATUS_UNKNOWN,
    OPT_STATUS_STARTING,
    OPT_STATUS_STARTED,
    OPT_STATUS_STOPPING,
    OPT_STATUS_STOPPED,
    OPT_STATUS_CHANGED,
    OPT_STATUS_CANCELLED
};

struct HeatSchedule {
    int from;          // seconds since midnight (0–86399)
    int to;            // same as above, use hms_to_seconds
    uint64_t duration; // timer duration that turns heat off
    int last_yday;     // day-of-year when this schedule last ran
};

#define HEAT_SCHEDULES_COUNT 12

struct BasInfo {

    bool valid; // is not empty
    enum RequestStatus status;

    // statuses
    int mod_rada;
    int mod_rezim;
    enum PumpStatus StatusPumpe3;
    enum PumpStatus StatusPumpe4;
    enum PumpStatus StatusPumpe5;
    enum PumpStatus StatusPumpe6;
    enum PumpStatus StatusPumpe7;

    // temps
    double Tspv;
    double Tsolar;
    double Tzadata;
    double Tfs;
    double Tmax;
    double Tmin;
    double Tsobna;

    // other calced values
    double Tmid;
    int TminLT;
    int TmidGE;
    int TmaxGE;

    double peak_min_solar;
    double peak_max_solar;
    double peak_min_human;
    double peak_max_human;
    double peak_min_buf;
    double peak_max_buf;
    double peak_min_circ;
    double peak_max_circ;

    bool opt_auto_timer;
    uint64_t opt_auto_timer_seconds;
    uint64_t opt_auto_timer_seconds_old;
    bool opt_auto_timer_started;
    uint64_t opt_auto_timer_seconds_elapsed;
    enum OptStatus opt_auto_timer_status;
    time_t opt_auto_timer_status_changed;
    bool opt_auto_gas;
    enum OptStatus opt_auto_gas_status;
    time_t opt_auto_gas_status_changed;

    int history_mode;
    time_t history_mode_time_changed;
    time_t history_mode_time_on;
    time_t history_mode_time_off;
    enum PumpStatus history_gas;
    time_t history_gas_time_changed;
    time_t history_gas_time_on;
    time_t history_gas_time_off;

    int radiator_color;
    int radiator_color_index;
    time_t radiator_color_last_update;
    double radiator_color_current_temp_ratio; // 0.0 = cold, 1.0 = fully hot

    double schedules_t_min;  // must be under this temperature to run
    int schedules_last_yday; // day of year [0–365], to reset schedules each day
};

enum Weather {
    WEATHER_UNKNOWN,
    WEATHER_CLEAR,
    WEATHER_CLOUD,
    WEATHER_FOG,
    WEATHER_RAIN,
    WEATHER_THUNDER,
    WEATHER_SNOW,
};

enum TimeOfDay {
    TIME_OF_DAY_UNKNOWN,
    TIME_OF_DAY_BEFORE_DAWN,
    TIME_OF_DAY_DAWN,
    TIME_OF_DAY_MORNING,
    TIME_OF_DAY_ZENITH,
    TIME_OF_DAY_AFTERNOON,
    TIME_OF_DAY_SUNSET,
    TIME_OF_DAY_NIGHT
};

extern const char* weather_keywords[][5];

struct WttrinInfo {
    bool valid;

    enum RequestStatus status;

    char csv[URL_WTTRIN_OUTPUT_MAX_FIELDS][URL_WTTRIN_OUTPUT_MAX_FIELD_LEN];
    int csv_parsed;

    char time[TINYBUFF];
    enum Weather weather;
    int dawn;
    int sunrise;
    int zenith;
    int zenith_duration; // zenith + 1 hour
    int sunset;
    int dusk;

    struct Marquee marquee_conds; // c -- Weather condition, C -- Weather condition textual name
    struct Marquee marquee_times; // D -- Dawn*, S -- Sunrise*, z -- Zenith*, s -- Sunset*, d -- Dusk*
};

struct Infos {
    struct BasInfo bas;
    struct WttrinInfo wttrin;
};

// request_infos.c
extern void infos_bas_init(void);
extern enum RequestStatus infos_bas_update(void);
extern enum RequestStatus infos_bas_health(void);
extern void infos_bas_safe_io(const struct BasInfo* in, struct BasInfo* out);
extern void infos_wttrin_init(void);
extern enum RequestStatus infos_wttrin_update(void);
extern enum RequestStatus infos_wttrin_health(void);
extern void infos_wttrin_update_safe_io(const struct WttrinInfo* in, struct WttrinInfo* out);
extern void infos_wttrin_marquee_conds_scroll(void);
extern void infos_wttrin_marquee_conds_update_width(int term_width);
extern void infos_wttrin_marquee_times_scroll(void);
extern void infos_wttrin_marquee_times_update_width(int term_width);
extern void infos_save(void);

// request_send.c
extern enum RequestStatus request_send(struct Request* request);
extern enum RequestStatus request_send_quick(const char* url);

// request_dologic.c
extern void remember_vars_do_action(struct BasInfo* info);

// request_utils.c
extern double extract_json_label(struct mg_str json_body, const char* label, double fallback);
extern char* request_status_to_str(enum RequestStatus status);
extern char* request_status_to_smallstr(enum RequestStatus status);
extern bool request_status_failed(enum RequestStatus status);
extern void print_bas_info(const struct BasInfo* b);
extern void print_wttrin_info(const struct WttrinInfo* info);
extern void print_infos(const struct Infos* info);
extern enum Weather detect_weather(const char* text);
extern int parse_csv(const char* input, char sep, int nfields, int field_size, char* fields);
extern int save_infos(const char* filename, const struct Infos* info);
extern int load_infos(const char* filename, struct Infos* info);
extern enum TimeOfDay wttrin_to_timeofday(struct WttrinInfo* wttrin);
extern int timeofday_to_color(enum TimeOfDay tod);
extern int wttrin_timeofday_color(struct WttrinInfo* wttrin);
extern enum TimeOfDay timeofday(void);

extern struct Infos g_infos;

#endif // REQUEST_H
