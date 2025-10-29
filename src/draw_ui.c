#include "colors.h"
#include "debug.h"
#include "globals.h"
#include "request.h"
#include "serve_websocket.h"
#include "spinners.h"
#include "utils.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct bas_info du_info = {0};

#define TERM_BUFFER_SIZE 1024 * 2
char g_term_buffer[TERM_BUFFER_SIZE] = {0};

char* weather[] = {
    CTEXT_FG(196, ""),
    CTEXT_FG(208, ""),
    CTEXT_FG(208, "󰖨"),
    CTEXT_FG(214, ""),
    CTEXT_FG(75, ""),
    CTEXT_FG(81, ""),
    CTEXT_FG(87, "")};

char* temp_to_emoji(double temp)
{
    if (temp > 40.0) return weather[0];  //  super hot
    if (temp >= 30.0) return weather[1]; //  really hot
    if (temp >= 25.0) return weather[2]; // 󰖨 hot
    if (temp >= 15.0) return weather[3]; //  mild
    if (temp >= 5.0) return weather[4];  //  cold
    if (temp >= -5.0) return weather[5]; //  really cold
    return weather[6];                   //  super cold
}

char* clock_hours[] = {"", "", "", "", "", "", "", "", "", "", "", ""};
char* hour_to_clock(int hour)
{
    return clock_hours[hour % 12];
}

char* hour_to_emoji(int hour)
{
    if (hour >= 5 && hour <= 7) return get_frame(&spinner_sunrise, 1); // Sunrise
    if (hour >= 7 && hour < 12) return "";                          // Morning
    if (hour >= 12 && hour < 17) return "󰖙";                        // Afternoon
    if (hour >= 17 && hour < 19) return get_frame(&spinner_sunset, 1); // Sunset
    if (hour >= 19 && hour < 21) return "";                         // Evening
    return "󰖔";                                                     // Night
}

int hour_to_color(int hour)
{
    if (hour >= 5 && hour < 7) return 214;   // Orange (Sunrise)
    if (hour >= 7 && hour < 12) return 220;  // Bright Yellow (Morning)
    if (hour >= 12 && hour < 17) return 208; // Deep Orange (Afternoon)
    if (hour >= 17 && hour < 19) return 202; // Red-Orange (Sunset)
    if (hour >= 19 && hour < 21) return 99;  // Purple-Pink (Evening)
    return 33;                               // Deep Blue (Night)
}

size_t draw_heat(char* buffer, size_t size, int value)
{
    return value ? ctext_fg(buffer, size, COLOR_ON, get_frame(&spinner_heat, 1)) : ctext_fg(buffer, size, COLOR_OFF, "󱪯");
}

size_t draw_regime(char* buffer, size_t size, int value)
{
    return cnum_fg(buffer, size, 192, value);
}

enum PUMP_STATUS {
    PUMP_STATUS_AUTO_OFF = 0,
    PUMP_STATUS_AUTO_ON = 1,
    PUMP_STATUS_MANUAL_OFF = 2,
    PUMP_STATUS_MANUAL_ON = 3
};

int pump_is_on(int value)
{
    return value == PUMP_STATUS_AUTO_ON || value == PUMP_STATUS_MANUAL_ON;
}

size_t draw_pump_bars(char* buffer, size_t size, int value)
{

    switch (value) {

        case PUMP_STATUS_AUTO_ON:    return ctext_fg_con(buffer, size, COLOR_ON_AUTO, get_frame(&spinner_bars, 0)); break;
        case PUMP_STATUS_MANUAL_ON:  return ctext_fg_con(buffer, size, COLOR_ON_MANUAL, get_frame(&spinner_bars, 0)); break;

        default:
        case PUMP_STATUS_AUTO_OFF:   return ctext_fg(buffer, size, COLOR_OFF_AUTO, ""); break;
        case PUMP_STATUS_MANUAL_OFF: return ctext_fg(buffer, size, COLOR_OFF_MANUAL, ""); break;
    }
}

typedef size_t (*func_draw_with_value)(char* buffer, size_t size, int value);
typedef size_t (*func_draw_extra)(char* buffer, size_t size);

size_t bool_to_check(char* buffer, size_t size, int value)
{
    return value ? ctext_fg(buffer, size, COLOR_ON, "") : ctext_fg(buffer, size, COLOR_OFF, "");
}

size_t draw_col1(char* buffer, size_t size, char* prelabel, char* label, char* icon, int color, char* gap, double value, double min, double max, char* end, func_draw_extra extra)
{
    size_t c = 0;
    c += snprintf(buffer + c, size - c, "%s", prelabel);
    char temp[MIDBUFF] = {0};
    snprintf(temp, MIDBUFF, "%s %s", label, icon);
    c += ctext_fg(buffer + c, size - c, color, temp);
    c += snprintf(buffer + c, size - c, "%s", gap);
    c += temp_to_ctext_bg_con(buffer + c, size - c, value, min, max);
    if (end) c += snprintf(buffer + c, size - c, " %s", end);
    if (extra) c += extra(buffer + c, size - c);
    c += snprintf(buffer + c, size - c, "\n");
    return c;
}

size_t draw_col2(char* buffer, size_t size, char* prelabel, char* label, char* icon, int color, char* gap, int value, func_draw_with_value func, char* end, func_draw_extra extra)
{
    size_t c = 0;
    c += snprintf(buffer + c, size - c, "%s", prelabel); // draw prelabel
    char temp[MIDBUFF] = {0};
    snprintf(temp, MIDBUFF, "%s %s", label, icon);            // make label with color
    c += ctext_fg(buffer + c, size - c, color, temp);         // draw label with color
    c += snprintf(buffer + c, size - c, "%s", gap);           // draw gap
    if (func) c += func(buffer + c, size - c, value);         // draw function pump_bars or heat
    if (end) c += snprintf(buffer + c, size - c, " %s", end); // draw end
    if (extra) c += extra(buffer + c, size);                  // draw extra function (eye)
    c += snprintf(buffer + c, size - c, "\n");                // newline
    return c;
}

size_t draw_extra_eye_timer(char* buffer, size_t size)
{

    size_t b = 0;
    if (atomic_load(&g_auto_timer)) b += ctext_fg(buffer + b, size - b, COLOR_ON, get_frame(&spinner_eye_left, 0));
    if (g_auto_timer) {
        if (g_auto_timer_started)
            b += ctext_fg(buffer + b, size - b, COLOR_ON, get_frame(&spinner_clock, 0));
        else
            b += ctext_fg(buffer + b, size - b, COLOR_OFF, "󱎫");
    }
    return b;
}

size_t draw_extra_eye_gas(char* buffer, size_t size)
{
    size_t b = 0;
    if (atomic_load(&g_auto_gas)) { b += ctext_fg(buffer, size - b, COLOR_ON, get_frame(&spinner_eye_left, 0)); }
    return b;
}

size_t draw_extra_check(char* buffer, size_t size)
{
    return ctext_fg(buffer, size, 82, du_info.valid && du_info.TmidGE ? get_frame(&spinner_check, 1) : " ");
}

size_t draw_extra_warn(char* buffer, size_t size)
{
    return ctext_fg(buffer, size, 51, du_info.valid && du_info.TminLT ? get_frame(&spinner_snow, 1) : " ");
}

static int g_auto_timer_seconds_old = AUTO_TIMER_SECONDS;

// clang-format off
size_t draw_ui() {

    DPL("DRAW UI");
    update_info_bas_safe_swap(&g_info, &du_info);

    // init draw buffers
    char temp[MIDBUFF] = {0};
    size_t t = 0;
    size_t b = 0;
    memset(g_term_buffer, 0, TERM_BUFFER_SIZE);

    // check if we have values
    if (!du_info.valid) {
        b += snprintf(g_term_buffer+b, TERM_BUFFER_SIZE - b, "@ ");
        get_local_ips(g_term_buffer+b, TERM_BUFFER_SIZE - b);
        b += snprintf(g_term_buffer+b, TERM_BUFFER_SIZE - b, "\n> no values to draw.\n>%s\n", request_status_to_str(du_info.status));
        return printf("%s", g_term_buffer);
    }


    // clock + hour emoji + date-time
    int hour           = localtime_hour();
    char* emoji_clock  = hour_to_clock(hour);
    char* emoji_dayhr  = hour_to_emoji(hour);
    t = 0;
    t += snprintf(temp+t, MIDBUFF-t, "%s %s ", emoji_clock, emoji_dayhr);
    t += dt_full(temp+t, MIDBUFF-t);
    int color_hour = hour_to_color(hour);
    b += ctext_fg(g_term_buffer+b, TERM_BUFFER_SIZE - b, color_hour, temp);
    b += snprintf(g_term_buffer+b, TERM_BUFFER_SIZE - b, "\n");

    // weather
    b += ctext_fg(g_term_buffer+b, TERM_BUFFER_SIZE - b, color_hour, g_wttrin_buffer);
    b += snprintf(g_term_buffer+b, TERM_BUFFER_SIZE - b, "\n");

    // light + send + conn count + ip
    b += ctext_fg(g_term_buffer+b, TERM_BUFFER_SIZE - b, 228, get_frame(&spinner_lights, 1));
    b += snprintf(g_term_buffer+b, TERM_BUFFER_SIZE - b, " %s %3d ", du_info.status == REQUEST_STATUS_RUNNING ? CTEXT_FG(211, "") : " ", atomic_load(&g_ws_conn_count));
    t = 0;
    t += get_local_ip(temp, MIDBUFF-t);
    b += ctext_fg(g_term_buffer+b, TERM_BUFFER_SIZE - b, request_status_failed(du_info.status) ? COLOR_OFF : COLOR_ON, temp);
    if (request_status_failed(du_info.status)) b += snprintf(g_term_buffer+b, TERM_BUFFER_SIZE - b, " %-16s", request_status_to_smallstr(du_info.status));
    b += snprintf(g_term_buffer+b, TERM_BUFFER_SIZE - b, "\n");

    char col1[TERM_BUFFER_SIZE] = {0};
    size_t c1 = 0;
    c1 += draw_col1(col1+c1, TERM_BUFFER_SIZE - c1, "", "Solar", get_frame(&spinner_solar_panel, 1), 230, "  ", du_info.Tsolar,  du_info.peak_min_solar, du_info.peak_max_solar, temp_to_emoji(du_info.Tsolar), NULL);
    c1 += draw_col1(col1+c1, TERM_BUFFER_SIZE - c1, "  ", "Out", get_frame(&spinner_window, 1),      213, "  ", du_info.Tspv,    du_info.peak_min_human, du_info.peak_max_human, temp_to_emoji(du_info.Tspv), NULL);
    c1 += draw_col1(col1+c1, TERM_BUFFER_SIZE - c1, " ", "Room", get_frame(&spinner_house, 1),        76, "  ", du_info.Tsobna,  du_info.peak_min_human, du_info.peak_max_human, temp_to_emoji(du_info.Tsobna), NULL);
    c1 += draw_col1(col1+c1, TERM_BUFFER_SIZE - c1, "  ", "Set", get_frame(&spinner_cog, 1),         154, "  ", du_info.Tzadata, du_info.peak_min_human, du_info.peak_max_human, temp_to_emoji(du_info.Tzadata), NULL);
    c1 += draw_col1(col1+c1, TERM_BUFFER_SIZE - c1, "  ", "Max", "",                                214, "  ", du_info.Tmax,    du_info.peak_min_buf,   du_info.peak_max_buf,   "", NULL);
    c1 += draw_col1(col1+c1, TERM_BUFFER_SIZE - c1, "  ", "Mid", "",                                220, "  ", du_info.Tmid,    du_info.peak_min_buf,   du_info.peak_max_buf,   "", draw_extra_check);
    c1 += draw_col1(col1+c1, TERM_BUFFER_SIZE - c1, "  ", "Min", "",                                226, "  ", du_info.Tmin,    du_info.peak_min_buf,   du_info.peak_max_buf,   "", draw_extra_warn);
    c1 += draw_col1(col1+c1, TERM_BUFFER_SIZE - c1, "", "Circ.", get_frame(&spinner_recycle, 1),     110, "  ", du_info.Tfs,     du_info.peak_min_circ,  du_info.peak_max_circ,  "", NULL);

    DPL("COL1");
    D(printf("%s\n", col1));

    char col2[TERM_BUFFER_SIZE] = {0};
    size_t c2 = 0;
    c2 += draw_col2(col2+c2, TERM_BUFFER_SIZE - c2, "   ", "Mode", "󱪯",                                                                       222, "  ", du_info.mod_rada,     draw_heat,      "", draw_extra_eye_timer);
    c2 += draw_col2(col2+c2, TERM_BUFFER_SIZE - c2, " ", "Regime", "󱖫",                                                                       192, "  ", du_info.mod_rada,     draw_regime,    "", NULL);
    c2 += draw_col2(col2+c2, TERM_BUFFER_SIZE - c2, "   ", "Heat",  pump_is_on(du_info.StatusPumpe6) ? get_frame(&spinner_heat_pump, 1) : "󱩃",   212, "  ", du_info.StatusPumpe6, draw_pump_bars, "", NULL);
    c2 += draw_col2(col2+c2, TERM_BUFFER_SIZE - c2, "    ", "Gas",  pump_is_on(du_info.StatusPumpe4) ? get_frame(&spinner_fire, 1) : "󰙇",        203, "  ", du_info.StatusPumpe4, draw_pump_bars, "", draw_extra_eye_gas);
    c2 += draw_col2(col2+c2, TERM_BUFFER_SIZE - c2, "   ", "Circ.",  pump_is_on(du_info.StatusPumpe3) ? get_frame(&spinner_circle, 1) : "",      168, "  ", du_info.StatusPumpe3, draw_pump_bars, "", NULL);
    c2 += draw_col2(col2+c2, TERM_BUFFER_SIZE - c2, "  ", "Solar",  pump_is_on(du_info.StatusPumpe7) ? get_frame(&spinner_solar, 1) : "",       224, "  ", du_info.StatusPumpe7, draw_pump_bars, "", NULL);
    c2 += draw_col2(col2+c2, TERM_BUFFER_SIZE - c2, "  ", "Elec.",  pump_is_on(du_info.StatusPumpe5) ? get_frame(&spinner_lightning, 1) : "󰠠",    78, "  ", du_info.StatusPumpe5, draw_pump_bars, "", NULL);

    DPL("COL2");
    D(printf("%s\n", col2));

    char line1[BIGBUFF], line2[BIGBUFF];
    const char *p1 = col1;
    const char *p2 = col2;

    // combine 2 columns
    while (*p1 || *p2) {
        int n1 = 0, n2 = 0;

        while (*p1 && *p1 != '\n' && n1 < (int)sizeof(line1) - 1) line1[n1++] = *p1++;
        line1[n1] = '\0';
        if (*p1 == '\n') p1++;

        while (*p2 && *p2 != '\n' && n2 < (int)sizeof(line2) - 1) line2[n2++] = *p2++;
        line2[n2] = '\0';
        if (*p2 == '\n') p2++;

        b += snprintf(g_term_buffer+b, TERM_BUFFER_SIZE - b,"%s%s\n", line1, line2);
    }


    if (g_auto_timer_started) {
        time_t current_time;
        time(&current_time);
        g_auto_timer_seconds_elapsed = difftime(current_time, g_history_mode_time_on);
        snprintf(g_auto_timer_status, BIGBUFF, "%d/%d", g_auto_timer_seconds_elapsed, atomic_load(&g_auto_timer_seconds));
    }

    int s = atomic_load(&g_auto_timer_seconds);
    if (s != g_auto_timer_seconds_old) {
        g_auto_timer_seconds_old = s;
        snprintf(g_auto_timer_status, BIGBUFF, "changed to: %d", s);
    }

    if (atomic_load(&g_auto_timer)) { b += ctext_fg(g_term_buffer+b, TERM_BUFFER_SIZE - b, COLOR_ON, get_frame(&spinner_eye_right, 0)); }
    else                            { b += ctext_fg(g_term_buffer+b, TERM_BUFFER_SIZE - b, COLOR_OFF, ""); }
    b += snprintf(g_term_buffer+b, TERM_BUFFER_SIZE - b,"󱪯 %s\n", g_auto_timer_status);

    if (atomic_load(&g_auto_gas)) { b += ctext_fg(g_term_buffer+b, TERM_BUFFER_SIZE - b, COLOR_ON, get_frame(&spinner_eye_right, 0)); }
    else                            { b += ctext_fg(g_term_buffer+b, TERM_BUFFER_SIZE - b, COLOR_OFF, ""); }
    b += snprintf(g_term_buffer+b, TERM_BUFFER_SIZE - b,"󰙇 %s\n", g_auto_gas_status);

    spin_spinner(&spinner_circle);
    spin_spinner(&spinner_eye_left);
    spin_spinner(&spinner_eye_right);
    spin_spinner(&spinner_bars);
    spin_spinner(&spinner_clock);


    DPL("===[ OUTPUT BEGIN ]===");
    size_t r = printf("%s", g_term_buffer);
    DPL("===[ OUTPUT END ]===");

    return r;
}
// clang-format on
