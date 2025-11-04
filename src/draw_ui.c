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
static struct bas_info du_info = {0};
static struct wttrin_info du_wttrin = {0};

#define MAX_ROWS 11
#define MAX_COLS 11
#define MAX_CELL SMALLBUFF // enough for UTF-8 + ANSI color
static char g_screen[MAX_ROWS][MAX_COLS][MAX_CELL];

static char g_term_buffer[MAX_ROWS * MAX_COLS * MAX_CELL];
static size_t g_term_buffer_b = 0;


static char g_temp[MAX_CELL];
static size_t g_temp_b = 0;

static void clear_screen()
{
    for (int r = 0; r < MAX_ROWS; r++)
        for (int c = 0; c < MAX_COLS; c++)
            g_screen[r][c][0] = '\0';
}

// set cell
static void sc(int r, int c, const char* text)
{
    if (r < MAX_ROWS && c < MAX_COLS)
        snprintf(g_screen[r][c], MAX_CELL, "%s", text);
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

// void print_screen()
// {
//     for (int r = 0; r < MAX_ROWS; r++) {
//
//         // find last used column in this row
//         int last_col = -1;
//         for (int c = MAX_COLS - 1; c >= 0; c--) {
//             if (g_screen[r][c][0]) {
//                 last_col = c;
//                 break;
//             }
//         }
//
//         // entire row empty? skip
//         if (last_col == -1)
//             continue;
//
//         // print only used columns
//         for (int c = 0; c <= last_col; c++) {
//             if (g_screen[r][c][0])
//                 fputs(g_screen[r][c], stdout);
//             else
//                 fputc(' ', stdout);
//             if (c != last_col)
//                 fputc(' ', stdout); // space between columns
//         }
//         fputc('\n', stdout);
//     }
// }

// char* eye_timer()
// {
//     g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_ON, du_info.opt_auto_timer ? get_frame(&spinner_eye_left, 0) : " ");
//     if (du_info.opt_auto_timer) {
//         if (du_info.opt_auto_timer_started)
//             g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_ON, get_frame(&spinner_clock, 0));
//         else
//             g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_OFF, "󱎫");
//     }
//     else {
//         g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_OFF, " ");
//     }
//     return g_temp;
// }

static size_t make_term_buffer()
{
    g_term_buffer_b = 0; // reset

    for (int r = 0; r < MAX_ROWS; r++) {

        // find last used column in this row
        int last_col = -1;
        for (int c = MAX_COLS - 1; c >= 0; c--) {
            if (g_screen[r][c][0]) {
                last_col = c;
                break;
            }
        }

        if (last_col == -1)
            continue;

        for (int c = 0; c <= last_col; c++) {

            if (g_screen[r][c][0]) {
                // append UTF-8 string
                size_t len = strlen(g_screen[r][c]);
                if (g_term_buffer_b + len < sizeof(g_term_buffer)) {
                    memcpy(&g_term_buffer[g_term_buffer_b], g_screen[r][c], len);
                    g_term_buffer_b += len;
                }
            }
            else {
                // append a plain space
                if (g_term_buffer_b + 1 < sizeof(g_term_buffer)) {
                    g_term_buffer[g_term_buffer_b++] = ' ';
                }
            }

            // space between cells
            if (c != last_col) {
                if (g_term_buffer_b + 1 < sizeof(g_term_buffer)) {
                    g_term_buffer[g_term_buffer_b++] = ' ';
                }
            }
        }

        // newline
        if (g_term_buffer_b + 1 < sizeof(g_term_buffer)) {
            g_term_buffer[g_term_buffer_b++] = '\n';
        }
    }

    // null-terminate (safe even if buffer is full)
    if (g_term_buffer_b < sizeof(g_term_buffer))
        g_term_buffer[g_term_buffer_b] = '\0';
    else
        g_term_buffer[sizeof(g_term_buffer) - 1] = '\0';

    return g_term_buffer_b;
}

static char* dut_hour_to_emoji(int hour)
{
    if (hour >= 5 && hour <= 7) return get_frame(&spinner_sunrise, 1); // Sunrise
    if (hour >= 7 && hour < 12) return "";                          // Morning
    if (hour >= 12 && hour < 17) return "󰖙";                        // Afternoon
    if (hour >= 17 && hour < 19) return get_frame(&spinner_sunset, 1); // Sunset
    if (hour >= 19 && hour < 21) return "";                         // Evening
    return "󰖔";                                                     // Night
}

static int dut_hour_to_color(int hour)
{
    if (hour >= 5 && hour < 7) return 214;   // Orange (Sunrise)
    if (hour >= 7 && hour < 12) return 220;  // Bright Yellow (Morning)
    if (hour >= 12 && hour < 17) return 208; // Deep Orange (Afternoon)
    if (hour >= 17 && hour < 19) return 202; // Red-Orange (Sunset)
    if (hour >= 19 && hour < 21) return 99;  // Purple-Pink (Evening)
    return 33;                               // Deep Blue (Night)
}

static size_t dut_weather_to_spinner(char* buffer, size_t size, enum Weather weather)
{
    switch (weather) {
        default:
        case WEATHER_UNKNOWN: return ctext_fg(buffer, size, 255, get_frame(&spinner_qm, 1)); break;
        case WEATHER_CLEAR:   return ctext_fg(buffer, size, 214, get_frame(&spinner_sun, 1)); break;
        case WEATHER_CLOUD:   return ctext_fg(buffer, size, 252, get_frame(&spinner_cloud, 1)); break;
        case WEATHER_RAIN:    return ctext_fg(buffer, size, 45, get_frame(&spinner_rain, 1)); break;
        case WEATHER_THUNDER: return ctext_fg(buffer, size, 226, get_frame(&spinner_thunder, 1)); break;
        case WEATHER_SNOW:    return ctext_fg(buffer, size, 51, get_frame(&spinner_snow, 1)); break;
        case WEATHER_FOG:     return ctext_fg(buffer, size, 231, get_frame(&spinner_fog, 1)); break;
    }
}

const char* dut_status_to_emoji(enum RequestStatus status)
{
    switch (status) {
        case REQUEST_STATUS_RUNNING:       return CTEXT_FG(211, ""); break;
        case REQUEST_STATUS_DONE:          return CTEXT_FG(82, "󰌘"); break;
        case REQUEST_STATUS_ERROR_TIMEOUT: return CTEXT_FG(202, "󱫎"); break;
        case REQUEST_STATUS_ERROR_CONN:    return CTEXT_FG(196, "󰌙"); break;
    }
    return "";
}

static char* dut_time()
{
    g_temp_b = 0;
    g_temp_b += dt_full(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b);
    return g_temp;
}

static char* dut_wttrin_emoji()
{
    g_temp_b = 0;
    g_temp_b += dut_weather_to_spinner(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, du_wttrin.weather);
    return g_temp;
}

static char* dut_ip()
{
    g_temp_b = 0;
    g_temp_b += get_local_ip(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b);
    return g_temp;
}

static char* dut_conns()
{
    g_temp_b = 0;
    g_temp_b += snprintf(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, "%d", atomic_load(&g_ws_conn_count));
    return g_temp;
}

static char* dut_temp_to_color(double value, double min, double max)
{
    g_temp_b = 0;
    g_temp_b += temp_to_ctext_bg_con(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, value, min, max);
    return g_temp;
}

static char* dut_max_check()
{
    g_temp_b = 0;
    ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, 82, du_info.valid && du_info.TmaxGE ? get_frame(&spinner_check, 1) : " ");
    return g_temp;
}

static char* dut_mid_check()
{
    g_temp_b = 0;
    g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, 82, du_info.valid && du_info.TmidGE ? get_frame(&spinner_check, 1) : " ");
    return g_temp;
}

static char* dut_min_warn()
{
    g_temp_b = 0;
    g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, 51, du_info.valid && du_info.TminLT ? get_frame(&spinner_snow, 1) : " ");
    return g_temp;
}

static char* dut_heat()
{
    return du_info.mod_rada ? get_frame(&spinner_heat, 1) : "󱪯";
}

static char* dut_regime(int value)
{
    g_temp_b = 0;
    g_temp_b += cnum_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, 192, value);
    return g_temp;
}

static int dut_pump_is_on(enum PumpStatus value)
{
    return value == PUMP_STATUS_AUTO_ON || value == PUMP_STATUS_MANUAL_ON;
}

static char* dut_draw_pump_bars(int value)
{

    g_temp_b = 0;
    switch (value) {

        case PUMP_STATUS_AUTO_ON:    g_temp_b += ctext_fg_con(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_ON_AUTO, get_frame(&spinner_bars, 0)); break;
        case PUMP_STATUS_MANUAL_ON:  g_temp_b += ctext_fg_con(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_ON_MANUAL, get_frame(&spinner_bars, 0)); break;

        default:
        case PUMP_STATUS_AUTO_OFF:   g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_OFF_AUTO, ""); break;
        case PUMP_STATUS_MANUAL_OFF: g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_OFF_MANUAL, ""); break;
    }
    return g_temp;
}

static char* dut_lbl_heat()
{
    return dut_pump_is_on(du_info.StatusPumpe6) ? get_frame(&spinner_heat_pump, 1) : "󱩃";
}

static char* dut_lbl_gas()
{
    return dut_pump_is_on(du_info.StatusPumpe4) ? get_frame(&spinner_fire, 1) : "󰙇";
}

static char* dut_lbl_circ()
{
    return dut_pump_is_on(du_info.StatusPumpe3) ? get_frame(&spinner_circle, 1) : "";
}

static char* dut_lbl_solar()
{
    return dut_pump_is_on(du_info.StatusPumpe7) ? get_frame(&spinner_solar, 1) : "";
}

static char* dut_lbl_elec()
{
    return dut_pump_is_on(du_info.StatusPumpe5) ? get_frame(&spinner_lightning, 1) : "󰠠";
}

static char* dut_selected(char* text, int is_selected)
{
    if (is_selected) {
        g_temp_b = 0;
        g_temp_b += ctext_u(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, text);
        return g_temp;
    }
    return text;
}

static char* dut_label_auto_timer_status()
{
    g_temp_b = 0;
    if (du_info.opt_auto_timer) { g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_ON, get_frame(&spinner_eye_right, 0)); }
    else {
        g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_OFF, "");
    }
    g_temp_b += snprintf(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, "󱪯");
    return g_temp;
}

static char* dut_label_auto_gas_status()
{
    g_temp_b = 0;
    if (du_info.opt_auto_gas) { g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_ON, get_frame(&spinner_eye_right, 0)); }
    else {
        g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_OFF, "");
    }
    g_temp_b += snprintf(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, "󰙇");
    return g_temp;
}

static int utf8_display_width(const char* s)
{
    mbstate_t ps = {0};
    wchar_t wc;
    int width = 0;

    const char* p = s;

    while (*p) {
        // skip ANSI escape sequences (e.g., "\033[31m")
        if (*p == '\033') {
            if (*(p + 1) == '[') {
                p += 2;
                while (*p && (*p < '@' || *p > '~')) // skip until letter ending sequence
                    p++;
                if (*p) p++; // skip final letter
                continue;
            }
        }

        // convert next multibyte character
        size_t n = mbrtowc(&wc, p, MB_CUR_MAX, &ps);
        if (n == (size_t)-1 || n == (size_t)-2) {
            // invalid UTF-8, skip a byte
            p++;
            continue;
        }
        else if (n == 0) {
            break;
        }

        int w = wcwidth(wc);
        if (w > 0) width += w;
        p += n;
    }

    return width;
}

static void print_buffer_padded()
{
    setlocale(LC_ALL, ""); // ensure correct UTF-8 width

    const char* p = g_term_buffer;
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
            fflush(stdout);
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

        fputc('\n', stdout);
    }
}

static char* dut_wttrin()
{
    if (du_wttrin.valid) {
        char temp[sizeof(g_temp) - 5];
        marquee_render(&du_wttrin.marquee, temp, sizeof(temp));
        snprintf(g_temp, sizeof(g_temp), "\r%s\033[K", temp);
        update_info_wttrin_marquee_scroll();
        return g_temp;
    }
    return du_wttrin.buffer;
}

size_t draw_ui_unsafe()
{
    clear_screen();

    int term_w = term_width();
    int term_h = term_height();
    if (term_w != g_term_w) {
        g_term_w = term_w;
        if (du_wttrin.valid) {
            update_info_wttrin_marquee_update_width(g_term_w);
        }
        term_clear();
    }
    if (term_h != g_term_h) {
        g_term_h = term_h;
        term_clear();
    }

    update_info_bas_safe_io(&g_info, &du_info);
    update_info_wttrin_safe_io(&g_wttrin, &du_wttrin);

    // row 0
    int hour = localtime_hour();
    int color_hour = dut_hour_to_color(hour);
    scc(0, 0, color_hour, hour_to_clock(hour));
    scc(0, 1, color_hour, dut_hour_to_emoji(hour));
    scc(0, 2, 182, dut_time());
    sc(0, 3, dut_wttrin_emoji());
    scc(0, 4, 228, get_frame(&spinner_lights, 1));
    sc(0, 5, dut_status_to_emoji(du_info.status));
    sc(0, 6, dut_status_to_emoji(du_wttrin.status));

    // row 1
    scc(1, 0, 181, dut_wttrin());

    // row 2
    scc(2, 0, request_status_failed(du_info.status) ? COLOR_OFF : COLOR_ON, dut_ip());
    sc(2, 1, dut_conns());

    scc(3, 0, 214, "");
    scc(4, 0, 220, "");
    scc(5, 0, 226, "");
    scc(6, 0, 110, get_frame(&spinner_recycle, 1));

    sc(3, 1, dut_temp_to_color(du_info.Tmax, du_info.peak_min_buf, du_info.peak_max_buf));
    sc(4, 1, dut_temp_to_color(du_info.Tmid, du_info.peak_min_buf, du_info.peak_max_buf));
    sc(5, 1, dut_temp_to_color(du_info.Tmin, du_info.peak_min_buf, du_info.peak_max_buf));
    sc(6, 1, dut_temp_to_color(du_info.Tfs, du_info.peak_min_circ, du_info.peak_max_circ));

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

    sc(3, 5, dut_temp_to_color(du_info.Tsolar, du_info.peak_min_solar, du_info.peak_max_solar));
    sc(4, 5, dut_temp_to_color(du_info.Tspv, du_info.peak_min_human, du_info.peak_max_human));
    sc(5, 5, dut_temp_to_color(du_info.Tsobna, du_info.peak_min_human, du_info.peak_max_human));
    sc(6, 5, dut_temp_to_color(du_info.Tzadata, du_info.peak_min_human, du_info.peak_max_human));

    sc(3, 6, human_temp_to_emoji(du_info.Tsolar));
    sc(4, 6, human_temp_to_emoji(du_info.Tspv));
    sc(5, 6, human_temp_to_emoji(du_info.Tsobna));
    sc(6, 6, human_temp_to_emoji(du_info.Tzadata));

    scc(7, 0, du_info.mod_rada ? COLOR_ON : COLOR_OFF, dut_selected(dut_heat(), du_info.opt_auto_timer));
    sc(8, 0, dut_regime(du_info.mod_rada));

    sc(7, 1, dut_draw_pump_bars(du_info.StatusPumpe6));
    sc(7, 2, dut_draw_pump_bars(du_info.StatusPumpe4));
    sc(7, 3, dut_draw_pump_bars(du_info.StatusPumpe3));
    sc(7, 4, dut_draw_pump_bars(du_info.StatusPumpe7));
    sc(7, 5, dut_draw_pump_bars(du_info.StatusPumpe5));

    scc(8, 1, 212, dut_lbl_heat());
    scc(8, 2, 203, dut_selected(dut_lbl_gas(), du_info.opt_auto_gas));
    scc(8, 3, 168, dut_lbl_circ());
    scc(8, 4, 224, dut_lbl_solar());
    scc(8, 5, 78, dut_lbl_elec());


    if (du_wttrin.csv_parsed >= WTTRIN_CSV_FIELD_h+1) sc(7, 6, du_wttrin.csv[WTTRIN_CSV_FIELD_h]);
    if (du_wttrin.csv_parsed >= WTTRIN_CSV_FIELD_u+1) sc(8, 6, du_wttrin.csv[WTTRIN_CSV_FIELD_u]);

    if (du_wttrin.csv_parsed >= WTTRIN_CSV_FIELD_w+1) sc(7, 7, du_wttrin.csv[WTTRIN_CSV_FIELD_w]);
    if (du_wttrin.csv_parsed >= WTTRIN_CSV_FIELD_p+1) sc(8, 7, du_wttrin.csv[WTTRIN_CSV_FIELD_p]);

    if (du_wttrin.csv_parsed >= WTTRIN_CSV_FIELD_m+1) sc(7, 8, du_wttrin.csv[WTTRIN_CSV_FIELD_m]);
    if (du_wttrin.csv_parsed >= WTTRIN_CSV_FIELD_M+1) sc(7, 9, du_wttrin.csv[WTTRIN_CSV_FIELD_M]);
    if (du_wttrin.csv_parsed >= WTTRIN_CSV_FIELD_P+1) sc(8, 8, du_wttrin.csv[WTTRIN_CSV_FIELD_P]);

    if (du_info.opt_auto_timer_started) {
        time_t current_time;
        time(&current_time);
        du_info.opt_auto_timer_seconds_elapsed = difftime(current_time, du_info.history_mode_time_on);
        snprintf(du_info.opt_auto_timer_status, sizeof(du_info.opt_auto_timer_status), "%d/%d", du_info.opt_auto_timer_seconds_elapsed, du_info.opt_auto_timer_seconds);
    }

    // statuses
    sc(9, 0, dut_label_auto_timer_status());
    scc(9, 1, 255, du_info.opt_auto_timer_status);
    sc(10, 0, dut_label_auto_gas_status());
    scc(10, 1, 255, du_info.opt_auto_gas_status);

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

size_t draw_ui_and_front()
{
    pthread_mutex_lock(&s_du_mutex);

#ifndef DEBUG
    term_cursor_reset();
#endif
    size_t r = draw_ui_unsafe();
    char html_buffer[1024 * 16] = {0};
    ansi_to_html(g_term_buffer, html_buffer);
    char html_buffer_escaped[1024 * 16 * 2] = {0};
    escape_quotes(html_buffer, html_buffer_escaped);

    char emit_buffer[1024 * 16 * 2] = {0};
    int b = snprintf(emit_buffer, 1024 * 8 * 2,
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
                     du_info.Tmin,
                     du_info.Tmax,
                     du_info.mod_rada,    // heat
                     du_info.StatusPumpe4 // gas pump
    );
    websocket_emit(emit_buffer, b);
    pthread_mutex_unlock(&s_du_mutex);
    return r;
}
