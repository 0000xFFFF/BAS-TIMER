#include "draw_ui.h"
#include "colors.h"
#include "debug.h"
#include "globals.h"
#include "request.h"
#include "serve_websocket.h"
#include "spinners.h"
#include "term.h"
#include "utils.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// must copy these values they update from another thread
static struct bas_info du_info = {0};
static struct wttrin_info du_wttrin = {0};

#define MAX_ROWS 10
#define MAX_COLS 10
#define MAX_CELL 64 // enough for UTF-8 + ANSI color
static char g_screen[MAX_ROWS][MAX_COLS][MAX_CELL];

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

static void print_screen()
{
    for (int r = 0; r < MAX_ROWS; r++) {
        int empty_row = 1;
        for (int c = 0; c < MAX_COLS; c++) {
            if (g_screen[r][c][0]) {
                empty_row = 0;
                fputs(g_screen[r][c], stdout);
            }
            else {
                fputc(' ', stdout); // blank cell
            }
            fputc(' ', stdout); // optional space between columns
        }
        if (!empty_row)
            fputc('\n', stdout);
    }
}

static char* weather[] = {
    CTEXT_FG(196, ""),
    CTEXT_FG(208, ""),
    CTEXT_FG(208, "󰖨"),
    CTEXT_FG(214, ""),
    CTEXT_FG(75, ""),
    CTEXT_FG(81, ""),
    CTEXT_FG(87, "")};

static char* temp_to_emoji(double temp)
{
    if (temp > 40.0) return weather[0];  //  super hot
    if (temp >= 30.0) return weather[1]; //  really hot
    if (temp >= 25.0) return weather[2]; // 󰖨 hot
    if (temp >= 15.0) return weather[3]; //  mild
    if (temp >= 5.0) return weather[4];  //  cold
    if (temp >= -5.0) return weather[5]; //  really cold
    return weather[6];                   //  super cold
}

static char* clock_hours[] = {"", "", "", "", "", "", "", "", "", "", "", ""};
static char* hour_to_clock(int hour)
{
    return clock_hours[hour % 12];
}

static char* hour_to_emoji(int hour)
{
    if (hour >= 5 && hour <= 7) return get_frame(&spinner_sunrise, 1); // Sunrise
    if (hour >= 7 && hour < 12) return "";                          // Morning
    if (hour >= 12 && hour < 17) return "󰖙";                        // Afternoon
    if (hour >= 17 && hour < 19) return get_frame(&spinner_sunset, 1); // Sunset
    if (hour >= 19 && hour < 21) return "";                         // Evening
    return "󰖔";                                                     // Night
}

static int hour_to_color(int hour)
{
    if (hour >= 5 && hour < 7) return 214;   // Orange (Sunrise)
    if (hour >= 7 && hour < 12) return 220;  // Bright Yellow (Morning)
    if (hour >= 12 && hour < 17) return 208; // Deep Orange (Afternoon)
    if (hour >= 17 && hour < 19) return 202; // Red-Orange (Sunset)
    if (hour >= 19 && hour < 21) return 99;  // Purple-Pink (Evening)
    return 33;                               // Deep Blue (Night)
}

enum PUMP_STATUS {
    PUMP_STATUS_AUTO_OFF = 0,
    PUMP_STATUS_AUTO_ON = 1,
    PUMP_STATUS_MANUAL_OFF = 2,
    PUMP_STATUS_MANUAL_ON = 3
};

static size_t weather_to_spinner(char* buffer, size_t size, enum Weather weather)
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

const char* status_to_emoji(enum RequestStatus status)
{
    switch (status) {
        case REQUEST_STATUS_RUNNING:       return CTEXT_FG(211, ""); break;
        case REQUEST_STATUS_DONE:          return CTEXT_FG(82, "󰌘"); break;
        case REQUEST_STATUS_ERROR_TIMEOUT: return CTEXT_FG(202, "󱫎"); break;
        case REQUEST_STATUS_ERROR_CONN:    return CTEXT_FG(196, "󰌙"); break;
    }
    return "";
}

static int g_term_w;
static int g_term_h;

static char g_temp[MAX_CELL];
static size_t g_temp_b = 0;

static char* time_text()
{
    g_temp_b = 0;
    g_temp_b += dt_full(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b);
    return g_temp;
}

static char* wttrin_spinner()
{
    g_temp_b = 0;
    g_temp_b += weather_to_spinner(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, du_wttrin.weather);
    return g_temp;
}

static char* ip()
{
    g_temp_b = 0;
    g_temp_b += get_local_ip(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b);
    return g_temp;
}

static char* conns()
{
    g_temp_b = 0;
    g_temp_b += snprintf(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, "%d", atomic_load(&g_ws_conn_count));
    return g_temp;
}

static char* temp_to_clr(double value, double min, double max)
{
    g_temp_b = 0;
    g_temp_b += temp_to_ctext_bg_con(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, value, min, max);
    return g_temp;
}

char* eye_timer()
{
    g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_ON, du_info.opt_auto_timer ? get_frame(&spinner_eye_left, 0) : " ");
    if (du_info.opt_auto_timer) {
        if (du_info.opt_auto_timer_started)
            g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_ON, get_frame(&spinner_clock, 0));
        else
            g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_OFF, "󱎫");
    }
    else {
        g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_OFF, " ");
    }
    return g_temp;
}

static char* draw_max_check()
{
    g_temp_b = 0;
    ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, 82, du_info.valid && du_info.TmaxGE ? get_frame(&spinner_check, 1) : " ");
    return g_temp;
}

static char* draw_mid_check()
{
    g_temp_b = 0;
    g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, 82, du_info.valid && du_info.TmidGE ? get_frame(&spinner_check, 1) : " ");
    return g_temp;
}

static char* draw_min_warn()
{
    g_temp_b = 0;
    g_temp_b += ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, 51, du_info.valid && du_info.TminLT ? get_frame(&spinner_snow, 1) : " ");
    return g_temp;
}

static char* draw_heat(int value)
{
    g_temp_b = 0;
    g_temp_b += value ? ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_ON, get_frame(&spinner_heat, 1)) : ctext_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, COLOR_OFF, "󱪯");
    return g_temp;
}

static char* draw_regime(int value)
{
    g_temp_b = 0;
    g_temp_b += cnum_fg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, 192, value);
    return g_temp;
}

static int pump_is_on(int value)
{
    return value == PUMP_STATUS_AUTO_ON || value == PUMP_STATUS_MANUAL_ON;
}

static char* draw_pump_bars(int value)
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

static char* draw_label_heat()
{
    return pump_is_on(du_info.StatusPumpe6) ? get_frame(&spinner_heat_pump, 1) : "󱩃";
}

static char* draw_label_gas()
{
    return pump_is_on(du_info.StatusPumpe4) ? get_frame(&spinner_fire, 1) : "󰙇";
}

static char* draw_label_circ()
{
    return pump_is_on(du_info.StatusPumpe3) ? get_frame(&spinner_circle, 1) : "";
}

static char* draw_label_solar()
{
    return pump_is_on(du_info.StatusPumpe7) ? get_frame(&spinner_solar, 1) : "";
}

static char* draw_label_elec()
{
    return pump_is_on(du_info.StatusPumpe5) ? get_frame(&spinner_lightning, 1) : "󰠠";
}

static char* draw_selected(char* text, int is_selected)
{
    if (is_selected) {
        g_temp_b = 0;
        g_temp_b += ctext_bg(g_temp + g_temp_b, sizeof(g_temp) - g_temp_b, 236, text);
        return g_temp;
    }
    return text;
}

size_t draw_ui_unsafe()
{
    clear_screen();

    int term_w = term_width();
    int term_h = term_height();
    if (term_w != g_term_w) {
        g_term_w = term_w;
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
    int color_hour = hour_to_color(hour);
    scc(0, 0, color_hour, hour_to_clock(hour));
    scc(0, 1, color_hour, hour_to_emoji(hour));
    scc(0, 2, 182, time_text());
    sc(0, 3, wttrin_spinner());
    scc(0, 4, 228, get_frame(&spinner_lights, 1));
    sc(0, 5, status_to_emoji(du_info.status));

    // row 1
    scc(1, 0, 181, du_wttrin.buffer);

    // row 2
    scc(2, 0, request_status_failed(du_info.status) ? COLOR_OFF : COLOR_ON, ip());
    sc(2, 1, conns());

    // clang-format off
    scc(3, 0, 230, get_frame(&spinner_solar_panel, 1)); sc(3, 1, temp_to_clr(du_info.Tsolar,  du_info.peak_min_solar, du_info.peak_max_solar)); sc(3, 2, temp_to_emoji(du_info.Tsolar));
    scc(4, 0, 213, get_frame(&spinner_window, 1));      sc(4, 1, temp_to_clr(du_info.Tspv,    du_info.peak_min_human, du_info.peak_max_human)); sc(4, 2, temp_to_emoji(du_info.Tspv));
    scc(5, 0,  76, get_frame(&spinner_house, 1));       sc(5, 1, temp_to_clr(du_info.Tsobna,  du_info.peak_min_human, du_info.peak_max_human)); sc(5, 2, temp_to_emoji(du_info.Tsobna));
    scc(6, 0, 154, get_frame(&spinner_cog, 1));         sc(6, 1, temp_to_clr(du_info.Tzadata, du_info.peak_min_human, du_info.peak_max_human)); sc(6, 2, temp_to_emoji(du_info.Tzadata));

    sc(3, 3, " "); scc(3, 4, 214, "");                            sc(3, 5, temp_to_clr(du_info.Tmax,  du_info.peak_min_buf,  du_info.peak_max_buf));  sc(3, 6, draw_max_check());
    sc(4, 3, " "); scc(4, 4, 220, "");                            sc(4, 5, temp_to_clr(du_info.Tmid,  du_info.peak_min_buf,  du_info.peak_max_buf));  sc(4, 6, draw_mid_check());
    sc(5, 3, " "); scc(5, 4, 226, "");                            sc(5, 5, temp_to_clr(du_info.Tmin,  du_info.peak_min_buf,  du_info.peak_max_buf));  sc(5, 6, draw_min_warn());
    sc(6, 3, " "); scc(6, 4, 110, get_frame(&spinner_recycle, 1)); sc(6, 5, temp_to_clr(du_info.Tfs,   du_info.peak_min_circ, du_info.peak_max_circ)); sc(6, 6, " ");

    const char* pad = " ";
    sc(7, 0, pad);
    sc(8, 0, pad);

    scc(8, 1, 222, draw_selected("󱪯", du_info.opt_auto_timer));               sc(7, 1, draw_heat(du_info.mod_rada));
    scc(8, 2, 192, "󱖫");                                                      sc(7, 2, draw_regime(du_info.mod_rada));

    const char* pad2 = "           ";
    sc(7, 3, pad2);
    sc(8, 3, pad2);
    scc(8, 4, 212, draw_label_heat());                                        sc(7, 4, draw_pump_bars(du_info.StatusPumpe6));
    scc(8, 5, 203, draw_selected(draw_label_gas(), du_info.opt_auto_gas));    sc(7, 5, draw_pump_bars(du_info.StatusPumpe4));
    scc(8, 6, 168, draw_label_circ());                                        sc(7, 6, draw_pump_bars(du_info.StatusPumpe3));
    scc(8, 7, 224, draw_label_solar());                                       sc(7, 7, draw_pump_bars(du_info.StatusPumpe7));
    scc(8, 8,  78, draw_label_elec());                                        sc(7, 8, draw_pump_bars(du_info.StatusPumpe5));

    // clang-format on

    spin_spinner(&spinner_circle);
    spin_spinner(&spinner_eye_left);
    spin_spinner(&spinner_eye_right);
    spin_spinner(&spinner_bars);
    spin_spinner(&spinner_clock);

    print_screen();

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
    // char html_buffer[1024 * 16] = {0};
    // ansi_to_html(g_term_buffer, html_buffer);
    // char html_buffer_escaped[1024 * 16 * 2] = {0};
    // escape_quotes(html_buffer, html_buffer_escaped);

    // char emit_buffer[1024 * 16 * 2] = {0};
    // int b = snprintf(emit_buffer, 1024 * 8 * 2,
    //                  "{"
    //                  "\"term\": \"%s\""
    //                  ","
    //                  "\"Tmin\": %f"
    //                  ","
    //                  "\"Tmax\": %f"
    //                  ","
    //                  "\"mod_rada\": %d"
    //                  ","
    //                  "\"StatusPumpe4\": %d"
    //                  "}",
    //                  html_buffer_escaped,
    //                  du_info.Tmin,
    //                  du_info.Tmax,
    //                  du_info.mod_rada,    // heat
    //                  du_info.StatusPumpe4 // gas pump
    // );
    // websocket_emit(emit_buffer, b);
    pthread_mutex_unlock(&s_du_mutex);
    return r;
}
