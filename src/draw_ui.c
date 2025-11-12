#define _XOPEN_SOURCE 700 // needed for wcwidth
#include "draw_ui.h"
#include "colors.h"
#include "globals.h"
#include "marquee.h"
#include "request.h"
#include "serve_websocket.h"
#include "spinners.h"
#include "term.h"
#include "utils.h"
#include <locale.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

// must copy these values they update from another thread
static struct Infos s_du_infos = {0};

#define MAX_ROWS 12
#define MAX_COLS 12
#define MAX_CELL MIDBUFF // enough for UTF-8 + ANSI color
static char s_screen[MAX_ROWS][MAX_COLS][MAX_CELL];

static char s_term_buffer[MAX_ROWS * MAX_COLS * MAX_CELL];
static size_t s_term_buffer_b = 0;

static char s_temp[MAX_CELL];
static size_t s_temp_b = 0;

static void clear_screen()
{
    for (int r = 0; r < MAX_ROWS; r++)
        for (int c = 0; c < MAX_COLS; c++)
            s_screen[r][c][0] = '\0';
}

// set cell
static void sc(int r, int c, const char* text)
{
    if (r < MAX_ROWS && c < MAX_COLS) {
        snprintf(s_screen[r][c], MAX_CELL, "%s", text);
    }
}

// set cell color
static void scc(int r, int c, int color, const char* text)
{
    char temp[BIGBUFF] = {0};
    ctext_fg(temp, sizeof(temp), color, text);
    sc(r, c, temp);
}

static char* human_temp_emojis[] = {
    CTEXT_FG(196, ""),
    CTEXT_FG(208, ""),
    CTEXT_FG(208, "󰖨"),
    CTEXT_FG(214, ""),
    CTEXT_FG(75, ""),
    CTEXT_FG(81, ""),
    CTEXT_FG(87, "")};

static char* human_temp_to_emoji(double temp)
{
    if (temp > 40.0) return human_temp_emojis[0];  //  super hot
    if (temp >= 30.0) return human_temp_emojis[1]; //  really hot
    if (temp >= 25.0) return human_temp_emojis[2]; // 󰖨 hot
    if (temp >= 15.0) return human_temp_emojis[3]; //  mild
    if (temp >= 5.0) return human_temp_emojis[4];  //  cold
    if (temp >= -5.0) return human_temp_emojis[5]; //  really cold
    return human_temp_emojis[6];                   //  super cold
}

static char* clock_hours[] = {"", "", "", "", "", "", "", "", "", "", "", ""};
static char* hour_to_clock(int hour)
{
    return clock_hours[hour % 12];
}

static size_t make_term_buffer()
{
    s_term_buffer_b = 0; // reset

    for (int r = 0; r < MAX_ROWS; r++) {

        // find last used column in this row
        int last_col = -1;
        for (int c = MAX_COLS - 1; c >= 0; c--) {
            if (s_screen[r][c][0]) {
                last_col = c;
                break;
            }
        }

        if (last_col == -1)
            continue;

        for (int c = 0; c <= last_col; c++) {

            if (s_screen[r][c][0]) {
                // append UTF-8 string
                size_t len = strlen(s_screen[r][c]);
                if (s_term_buffer_b + len < sizeof(s_term_buffer)) {
                    memcpy(&s_term_buffer[s_term_buffer_b], s_screen[r][c], len);
                    s_term_buffer_b += len;
                }
            }
            else {
                // append a plain space
                if (s_term_buffer_b + 1 < sizeof(s_term_buffer)) {
                    s_term_buffer[s_term_buffer_b++] = ' ';
                }
            }

            // space between cells
            if (c != last_col) {
                if (s_term_buffer_b + 1 < sizeof(s_term_buffer)) {
                    s_term_buffer[s_term_buffer_b++] = ' ';
                }
            }
        }

        // newline
        if (s_term_buffer_b + 1 < sizeof(s_term_buffer)) {
#ifdef DEBUG
            s_term_buffer[s_term_buffer_b++] = '\n';
#else // on relase don't add new line for last line
            if (r != MAX_ROWS - 1) {
                s_term_buffer[s_term_buffer_b++] = '\n';
            }
#endif
        }
    }

    // null-terminate (safe even if buffer is full)
    if (s_term_buffer_b < sizeof(s_term_buffer))
        s_term_buffer[s_term_buffer_b] = '\0';
    else
        s_term_buffer[sizeof(s_term_buffer) - 1] = '\0';

    return s_term_buffer_b;
}

static int dut_weather_to_color(enum Weather weather)
{
    switch (weather) {
        default:
        case WEATHER_UNKNOWN: return 255;
        case WEATHER_CLEAR:   return 214;
        case WEATHER_CLOUD:   return 252;
        case WEATHER_RAIN:    return 45;
        case WEATHER_THUNDER: return 226;
        case WEATHER_SNOW:    return 51;
        case WEATHER_FOG:     return 231;
    }
}

static size_t dut_weather_to_spinner(char* buffer, size_t size, int color, enum Weather weather)
{
    switch (weather) {
        default:
        case WEATHER_UNKNOWN: return ctext_fg(buffer, size, color, get_frame(&spinner_qm, 1)); break;
        case WEATHER_CLEAR:   return ctext_fg(buffer, size, color, get_frame(&spinner_sun, 1)); break;
        case WEATHER_CLOUD:   return ctext_fg(buffer, size, color, get_frame(&spinner_cloud, 1)); break;
        case WEATHER_RAIN:    return ctext_fg(buffer, size, color, get_frame(&spinner_rain, 1)); break;
        case WEATHER_THUNDER: return ctext_fg(buffer, size, color, get_frame(&spinner_thunder, 1)); break;
        case WEATHER_SNOW:    return ctext_fg(buffer, size, color, get_frame(&spinner_snow, 1)); break;
        case WEATHER_FOG:     return ctext_fg(buffer, size, color, get_frame(&spinner_fog, 1)); break;
    }
}

const char* dut_status_to_emoji(enum RequestStatus status)
{
    switch (status) {
        case REQUEST_STATUS_RUNNING:       return CTEXT_FG(211, ""); break;
        case REQUEST_STATUS_DONE:          return CTEXT_FG(82, "󰌘"); break;
        case REQUEST_STATUS_ERROR_TIMEOUT: return CTEXT_FG(197, "󱫎"); break;
        case REQUEST_STATUS_ERROR_CONN:    return CTEXT_FG(196, "󰌙"); break;
    }
    return "";
}

static char* dut_time()
{
    s_temp_b = 0;
    s_temp_b += dt_full(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b);
    return s_temp;
}

static char* dut_wttrin_emoji(int color)
{
    s_temp_b = 0;
    s_temp_b += dut_weather_to_spinner(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, color, s_du_infos.wttrin.weather);
    return s_temp;
}

static char* dut_ip()
{
    s_temp_b = 0;
    s_temp_b += get_local_ip(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b);
    return s_temp;
}

static char* dut_conns()
{
    s_temp_b = 0;
    s_temp_b += snprintf(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, "%d", atomic_load(&g_ws_conn_count));
    return s_temp;
}

static char* dut_temperature(double value, double min, double max)
{
    s_temp_b = 0;
    s_temp_b += temp_to_ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, value, min, max, "%7.2f");
    return s_temp;
}

static char* dut_wttrin_temp_to_color(char* s, double max, double min)
{
    char* end;
    double value = strtod(s, &end);

    s_temp_b = 0;
    s_temp_b += temp_to_ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, value, min, max, "%.0f");
    return s_temp;
}

static char* dut_max_check()
{
    s_temp_b = 0;
    ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, 82, s_du_infos.bas.valid && s_du_infos.bas.TmaxGE ? get_frame(&spinner_check, 1) : " ");
    return s_temp;
}

static char* dut_mid_check()
{
    s_temp_b = 0;
    s_temp_b += ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, 82, s_du_infos.bas.valid && s_du_infos.bas.TmidGE ? get_frame(&spinner_check, 1) : " ");
    return s_temp;
}

static char* dut_min_warn()
{
    s_temp_b = 0;
    s_temp_b += ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, 51, s_du_infos.bas.valid && s_du_infos.bas.TminLT ? get_frame(&spinner_snow, 1) : " ");
    return s_temp;
}

static char* dut_heat()
{
    return s_du_infos.bas.mod_rada ? get_frame(&spinner_heat, 1) : "󱪯";
}

static char* dut_regime(int value)
{
    s_temp_b = 0;
    s_temp_b += cnum_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, 192, value);
    return s_temp;
}

static int dut_pump_is_on(enum PumpStatus value)
{
    return value == PUMP_STATUS_AUTO_ON || value == PUMP_STATUS_MANUAL_ON;
}

static char* dut_draw_pump_bars(int value)
{

    s_temp_b = 0;
    switch (value) {

        case PUMP_STATUS_AUTO_ON:    s_temp_b += ctext_fg_con(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, COLOR_ON_AUTO, get_frame(&spinner_bars, 0)); break;
        case PUMP_STATUS_MANUAL_ON:  s_temp_b += ctext_fg_con(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, COLOR_ON_MANUAL, get_frame(&spinner_bars, 0)); break;

        default:
        case PUMP_STATUS_AUTO_OFF:   s_temp_b += ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, COLOR_OFF_AUTO, ""); break;
        case PUMP_STATUS_MANUAL_OFF: s_temp_b += ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, COLOR_OFF_MANUAL, ""); break;
    }
    return s_temp;
}

static char* dut_lbl_heat()
{
    return dut_pump_is_on(s_du_infos.bas.StatusPumpe6) ? get_frame(&spinner_heat_pump, 1) : "󱩃";
}

static char* dut_lbl_gas()
{
    return dut_pump_is_on(s_du_infos.bas.StatusPumpe4) ? get_frame(&spinner_fire, 1) : "󰙇";
}

static char* dut_lbl_circ()
{
    return dut_pump_is_on(s_du_infos.bas.StatusPumpe3) ? get_frame(&spinner_circle, 1) : "";
}

static char* dut_lbl_solar()
{
    return dut_pump_is_on(s_du_infos.bas.StatusPumpe7) ? get_frame(&spinner_solar, 1) : "";
}

static char* dut_lbl_elec()
{
    return dut_pump_is_on(s_du_infos.bas.StatusPumpe5) ? get_frame(&spinner_lightning, 1) : "󰠠";
}

static char* dut_selected(char* text, int is_selected)
{
    if (is_selected) {
        s_temp_b = 0;
        s_temp_b += ctext_u(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, text);
        return s_temp;
    }
    return text;
}

static char* dut_label_auto_timer_status(int color)
{
    s_temp_b = 0;
    if (s_du_infos.bas.opt_auto_timer) { s_temp_b += ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, COLOR_ON, get_frame(&spinner_eye_right, 0)); }
    else {
        s_temp_b += ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, COLOR_OFF, "");
    }
    s_temp_b += ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, color, "󱪯");
    return s_temp;
}

static char* dut_label_auto_gas_status()
{
    s_temp_b = 0;
    if (s_du_infos.bas.opt_auto_gas) { s_temp_b += ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, COLOR_ON, get_frame(&spinner_eye_right, 0)); }
    else {
        s_temp_b += ctext_fg(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, COLOR_OFF, "");
    }
    s_temp_b += snprintf(s_temp + s_temp_b, sizeof(s_temp) - s_temp_b, "󰙇");
    return s_temp;
}

static void print_buffer_padded()
{
    setlocale(LC_ALL, ""); // ensure correct UTF-8 width

    const char* p = s_term_buffer;
    const char* line_start = p;

    while (*p) {
        if (*p == '\n') {
            int line_bytes = p - line_start;

            char line[4096]; // enough per line
            memcpy(line, line_start, line_bytes);
            line[line_bytes] = '\0';

            // print line
            fputs(line, stdout);

            // measure display width
            int w = utf8_display_width(line);

            // pad spaces
            for (; w < g_term_w; w++)
                fputc(' ', stdout);

            fputc('\n', stdout);

            p++;
            line_start = p;
        }
        else {
            p++;
        }
    }

    // last line without trailing newline
    if (line_start != p) {
        int line_bytes = p - line_start;

        char line[4096];
        memcpy(line, line_start, line_bytes);
        line[line_bytes] = '\0';

        fputs(line, stdout);

        int w = utf8_display_width(line);
        for (; w < g_term_w; w++)
            fputc(' ', stdout);

#ifdef DEBUG // last newline only in debug
        fputc('\n', stdout);
#endif
    }

    fflush(stdout);
}

static const char* dut_wttrin_marquee_conds()
{
    if (s_du_infos.wttrin.marquee_conds.valid) {
        char temp[sizeof(s_temp) - 5];
        marquee_render(&s_du_infos.wttrin.marquee_conds, temp, sizeof(temp));
        snprintf(s_temp, sizeof(s_temp), "%s\033[K", temp);
        infos_wttrin_marquee_conds_scroll();
        return s_temp;
    }
    return s_du_infos.wttrin.marquee_conds.text;
}

static char* dut_wttrin_marquee_times()
{
    if (s_du_infos.wttrin.marquee_times.valid) {
        char temp[sizeof(s_temp) - 5];
        marquee_render(&s_du_infos.wttrin.marquee_times, temp, sizeof(temp));
        snprintf(s_temp, sizeof(s_temp), "%s\033[K", temp);
        infos_wttrin_marquee_times_scroll();
        return s_temp;
    }
    return s_du_infos.wttrin.marquee_times.text;
}

enum TimeOfDay get_tod()
{
    return s_du_infos.wttrin.valid ? wttrin_to_timeofday(&s_du_infos.wttrin) : timeofday(); // fallback with timeofday if can't get wttrin
}

static char* dut_timeofday_emoji()
{
    enum TimeOfDay tod = get_tod();

    // clang-format off
    switch (tod) {
        case TIME_OF_DAY_BEFORE_DAWN: return "";
        case TIME_OF_DAY_DAWN:        return get_frame(&spinner_sunrise, 1);
        case TIME_OF_DAY_MORNING:     return "";
        case TIME_OF_DAY_ZENITH:      return "󰖙";
        case TIME_OF_DAY_AFTERNOON:   return "";
        case TIME_OF_DAY_SUNSET:      return get_frame(&spinner_sunset, 1);
        case TIME_OF_DAY_NIGHT:       return "󰖔";
        default:                      return "?";
    }
    // clang-format on
}

static const char* dut_progressbar()
{
    time_t current_time;
    time(&current_time);
    s_du_infos.bas.opt_auto_timer_seconds_elapsed = difftime(current_time, s_du_infos.bas.history_mode_time_on);
    float percent = s_du_infos.bas.opt_auto_timer_seconds >= 0 ? ((float)s_du_infos.bas.opt_auto_timer_seconds_elapsed / (float)s_du_infos.bas.opt_auto_timer_seconds) * 100
                                                               : 0;

    // Progress bar configuration
    const int bar_width = 24;
    const int filled_color = 28; // green
    const int empty_color = 235; // dark gray

    // Create the text content first
    char text_content[32];
    int text_len = snprintf(text_content, sizeof(text_content),
                            "%d/%d %.2f%%",
                            s_du_infos.bas.opt_auto_timer_seconds_elapsed,
                            s_du_infos.bas.opt_auto_timer_seconds,
                            percent);

    // Calculate filled portion
    int filled = (int)((percent / 100.0f) * bar_width);
    if (filled > bar_width) filled = bar_width;

    // Build progress bar - only 2 sections maximum
    char bar[MIDBUFF - 3] = {0};
    char filled_section[32];
    char empty_section[32];
    int offset = 0;

    // Filled section
    if (filled > 0) {
        int filled_text_len = (text_len < filled) ? text_len : filled;
        strncpy(filled_section, text_content, filled_text_len);
        filled_section[filled_text_len] = '\0';
        // Pad with spaces if needed
        for (int i = filled_text_len; i < filled; i++) {
            filled_section[i] = ' ';
        }
        filled_section[filled] = '\0';

        offset += ctext_bg(bar + offset, sizeof(bar) - offset, filled_color, filled_section);
    }

    // Empty section
    if (filled < bar_width) {
        int empty_start = filled;
        int empty_len = bar_width - filled;

        // Get remaining text or spaces
        for (int i = 0; i < empty_len; i++) {
            int text_idx = empty_start + i;
            empty_section[i] = (text_idx < text_len) ? text_content[text_idx] : ' ';
        }
        empty_section[empty_len] = '\0';

        offset += ctext_bg(bar + offset, sizeof(bar) - offset, empty_color, empty_section);
    }

    snprintf(s_temp, sizeof(s_temp), "[%s]", bar);
    return s_temp;
}

static const char* dut_opt_status_timer(struct BasInfo* info)
{
    struct tm* timeinfo = localtime(&info->opt_auto_timer_status_changed);
    char time_str[10] = {0};
    strftime_HMS(time_str, sizeof(time_str), timeinfo);

    char p[TINYBUFF] = "";
    if (info->history_mode_time_on && info->history_mode_time_off) {
        char elap[10];
        elapsed_str(elap, sizeof(elap), info->history_mode_time_off, info->history_mode_time_on);
        snprintf(p, sizeof(p), " 󱫐 %s", elap);
    }

    switch (info->opt_auto_timer_status) {
        default:
        case OPT_STATUS_UNKNOWN:   snprintf(s_temp, sizeof(s_temp), " %s ?", time_str); break;
        case OPT_STATUS_STARTING:  snprintf(s_temp, sizeof(s_temp), "󱫌 %s", time_str); break;
        case OPT_STATUS_STARTED:   snprintf(s_temp, sizeof(s_temp), " %s 󰐸", time_str); break;
        case OPT_STATUS_STOPPING:  snprintf(s_temp, sizeof(s_temp), " %s", time_str); break;
        case OPT_STATUS_STOPPED:   snprintf(s_temp, sizeof(s_temp), " %s %s", time_str, p); break;
        case OPT_STATUS_CHANGED:   snprintf(s_temp, sizeof(s_temp), "changed to: %d", info->opt_auto_timer_seconds); break;
        case OPT_STATUS_CANCELLED: snprintf(s_temp, sizeof(s_temp), "󰜺 %s%s", time_str, p); break;
    }

    return s_temp;
}

static const char* dut_opt_status_gas(struct BasInfo* info)
{
    struct tm* timeinfo = localtime(&info->opt_auto_gas_status_changed);
    char time_str[10] = {0};
    strftime_HMS(time_str, sizeof(time_str), timeinfo);

    char p[TINYBUFF] = "";
    if (info->history_gas_time_on && info->history_gas_time_off) {
        char elap[10];
        elapsed_str(elap, sizeof(elap), info->history_gas_time_off, info->history_gas_time_on);
        snprintf(p, sizeof(p), " 󱫐 %s", elap);
    }

    switch (info->opt_auto_gas_status) {
        default:
        case OPT_STATUS_UNKNOWN:
        case OPT_STATUS_CANCELLED:
        case OPT_STATUS_CHANGED:   snprintf(s_temp, sizeof(s_temp), " %s ?", time_str); break;
        case OPT_STATUS_STARTING:  snprintf(s_temp, sizeof(s_temp), " %s", time_str); break;
        case OPT_STATUS_STARTED:   snprintf(s_temp, sizeof(s_temp), " %s ", time_str); break;
        case OPT_STATUS_STOPPING:  snprintf(s_temp, sizeof(s_temp), " %s", time_str); break;
        case OPT_STATUS_STOPPED:   snprintf(s_temp, sizeof(s_temp), " %s %s", time_str, p); break;
    }

    return s_temp;
}

size_t draw_ui_unsafe()
{
    clear_screen();

    int term_w = term_width();
    int term_h = term_height();
    if (term_w != g_term_w) {
        g_term_w = term_w;
        if (s_du_infos.wttrin.valid) {
            infos_wttrin_marquee_conds_update_width(g_term_w);
            infos_wttrin_marquee_times_update_width(g_term_w);
        }
        term_clear();
    }
    if (term_h != g_term_h) {
        g_term_h = term_h;
        term_clear();
    }

    infos_bas_safe_io(&g_infos.bas, &s_du_infos.bas);
    infos_wttrin_update_safe_io(&g_infos.wttrin, &s_du_infos.wttrin);

    time_t current_time;
    time(&current_time);

    int color_radiator = radiator_color_update(s_du_infos.bas.mod_rada);

    // row 0
    enum TimeOfDay tod = get_tod();
    int hour = localtime_hour();
    int tod_color = timeofday_to_color(tod);

    scc(0, 0, tod_color, hour_to_clock(hour));
    scc(0, 1, tod_color, dut_timeofday_emoji());
    scc(0, 2, tod_color, dut_time());
    sc(0, 3, dut_status_to_emoji(s_du_infos.bas.status));
    sc(0, 4, dut_status_to_emoji(s_du_infos.wttrin.status));
    int wttrin_color = dut_weather_to_color(s_du_infos.wttrin.weather);
    sc(0, 5, dut_wttrin_emoji(wttrin_color));
    sc(0, 6, s_du_infos.wttrin.csv[WTTRIN_CSV_FIELD_m]);

    // row 1
    scc(1, 0, wttrin_color, s_du_infos.wttrin.time);
    scc(1, 1, wttrin_color, s_du_infos.wttrin.csv[WTTRIN_CSV_FIELD_c]);
    scc(1, 2, wttrin_color, dut_wttrin_marquee_conds());

    // row 2
    scc(2, 0, request_status_failed(s_du_infos.bas.status) ? COLOR_OFF : COLOR_ON, dut_ip());
    sc(2, 1, dut_conns());

    scc(3, 0, 214, "");
    scc(4, 0, 220, "");
    scc(5, 0, 226, "");
    scc(6, 0, 110, get_frame(&spinner_recycle, 1));

    sc(3, 1, dut_temperature(s_du_infos.bas.Tmax, s_du_infos.bas.peak_min_buf, s_du_infos.bas.peak_max_buf));
    sc(4, 1, dut_temperature(s_du_infos.bas.Tmid, s_du_infos.bas.peak_min_buf, s_du_infos.bas.peak_max_buf));
    sc(5, 1, dut_temperature(s_du_infos.bas.Tmin, s_du_infos.bas.peak_min_buf, s_du_infos.bas.peak_max_buf));
    sc(6, 1, dut_temperature(s_du_infos.bas.Tfs, s_du_infos.bas.peak_min_circ, s_du_infos.bas.peak_max_circ));

    sc(3, 2, dut_max_check());
    sc(4, 2, dut_mid_check());
    sc(5, 2, dut_min_warn());
    sc(6, 2, " ");

    sc(3, 3, " ");
    sc(4, 3, " ");
    sc(5, 3, " ");
    sc(6, 3, " ");

    scc(3, 4, 230, get_frame(&spinner_solar_panel, 1));
    scc(4, 4, 213, get_frame(&spinner_window, 1));
    scc(5, 4, 76, get_frame(&spinner_house, 1));
    scc(6, 4, 154, get_frame(&spinner_cog, 1));

    sc(3, 5, dut_temperature(s_du_infos.bas.Tsolar, s_du_infos.bas.peak_min_solar, s_du_infos.bas.peak_max_solar));
    sc(4, 5, dut_temperature(s_du_infos.bas.Tspv, s_du_infos.bas.peak_min_human, s_du_infos.bas.peak_max_human));
    sc(5, 5, dut_temperature(s_du_infos.bas.Tsobna, s_du_infos.bas.peak_min_human, s_du_infos.bas.peak_max_human));
    sc(6, 5, dut_temperature(s_du_infos.bas.Tzadata, s_du_infos.bas.peak_min_human, s_du_infos.bas.peak_max_human));

    sc(3, 6, human_temp_to_emoji(s_du_infos.bas.Tsolar));
    sc(4, 6, human_temp_to_emoji(s_du_infos.bas.Tspv));
    sc(5, 6, human_temp_to_emoji(s_du_infos.bas.Tsobna));
    sc(6, 6, human_temp_to_emoji(s_du_infos.bas.Tzadata));

    scc(7, 0, color_radiator, dut_selected(dut_heat(), s_du_infos.bas.opt_auto_timer));
    sc(8, 0, dut_regime(s_du_infos.bas.mod_rada));

    sc(7, 1, dut_draw_pump_bars(s_du_infos.bas.StatusPumpe6));
    sc(7, 2, dut_draw_pump_bars(s_du_infos.bas.StatusPumpe4));
    sc(7, 3, dut_draw_pump_bars(s_du_infos.bas.StatusPumpe3));
    sc(7, 4, dut_draw_pump_bars(s_du_infos.bas.StatusPumpe7));
    sc(7, 5, dut_draw_pump_bars(s_du_infos.bas.StatusPumpe5));

    scc(8, 1, 212, dut_lbl_heat());
    scc(8, 2, 203, dut_selected(dut_lbl_gas(), s_du_infos.bas.opt_auto_gas));
    scc(8, 3, 168, dut_lbl_circ());
    scc(8, 4, 224, dut_lbl_solar());
    scc(8, 5, 78, dut_lbl_elec());

    sc(7, 6, dut_wttrin_temp_to_color(s_du_infos.wttrin.csv[WTTRIN_CSV_FIELD_t], s_du_infos.bas.peak_min_human, s_du_infos.bas.peak_max_human));
    sc(7, 7, dut_wttrin_temp_to_color(s_du_infos.wttrin.csv[WTTRIN_CSV_FIELD_f], s_du_infos.bas.peak_min_human, s_du_infos.bas.peak_max_human));
    scc(7, 8, 226, s_du_infos.wttrin.csv[WTTRIN_CSV_FIELD_u]);
    scc(7, 9, 195, s_du_infos.wttrin.csv[WTTRIN_CSV_FIELD_w]);

    scc(8, 6, 123, s_du_infos.wttrin.csv[WTTRIN_CSV_FIELD_h]);
    scc(8, 7, 111, s_du_infos.wttrin.csv[WTTRIN_CSV_FIELD_p]);
    scc(8, 8, 177, s_du_infos.wttrin.csv[WTTRIN_CSV_FIELD_P]);

    // statuses
    sc(9, 0, dut_label_auto_timer_status(color_radiator));
    sc(10, 0, dut_label_auto_gas_status());

    if (s_du_infos.bas.opt_auto_timer_started) {
        sc(9, 1, dut_progressbar());
    }
    else {
        scc(9, 1, color_radiator, dut_opt_status_timer(&s_du_infos.bas));
    }

    scc(10, 1, 255, dut_opt_status_gas(&s_du_infos.bas));

    // times marquee
    sc(11, 0, dut_wttrin_marquee_times());

    spin_spinner(&spinner_circle);
    spin_spinner(&spinner_eye_left);
    spin_spinner(&spinner_eye_right);
    spin_spinner(&spinner_bars);
    spin_spinner(&spinner_clock);

    make_term_buffer();
    print_buffer_padded();

    return 0;
}

static pthread_mutex_t s_du_mutex = PTHREAD_MUTEX_INITIALIZER;

#define HTML_BUFFER_SIZE 1024 * 6
size_t draw_ui_and_front()
{
    pthread_mutex_lock(&s_du_mutex);

#ifndef DEBUG
    term_cursor_reset();
#endif
    size_t r = draw_ui_unsafe();
    char html_buffer[HTML_BUFFER_SIZE] = {0};
    ansi_to_html(s_term_buffer, html_buffer);
    char html_buffer_escaped[HTML_BUFFER_SIZE] = {0};
    escape_quotes(html_buffer, html_buffer_escaped);

    char emit_buffer[HTML_BUFFER_SIZE] = {0};
    int b = snprintf(emit_buffer, HTML_BUFFER_SIZE,
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
                     s_du_infos.bas.Tmin,
                     s_du_infos.bas.Tmax,
                     s_du_infos.bas.mod_rada,    // heat
                     s_du_infos.bas.StatusPumpe4 // gas pump
    );
    ws_emit(emit_buffer, b);
    pthread_mutex_unlock(&s_du_mutex);
    return r;
}
