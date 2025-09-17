#include "colors.h"
#include "debug.h"
#include "globals.h"
#include "reqworker.h"
#include "spinners.h"
#include "utils.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

char* draw_heat(int isOn)
{
    return isOn ? ctext_fg(COLOR_ON, get_frame(&spinner_heat, 1)) : ctext_fg(COLOR_OFF, "󱪯");
}

enum PUMP_STATUS {
    PUMP_STATUS_AUTO = 0,
    PUMP_STATUS_PASSIVE = 1,
    PUMP_STATUS_CLOSE_STOP = 2,
    PUMP_STATUS_OPEN_START = 3
};

int pump_is_on(int i)
{
    return i == PUMP_STATUS_OPEN_START;
}

char* draw_pump_bars(int value)
{

    switch (value) {
        case PUMP_STATUS_AUTO:       return ctext_fg(COLOR_OFF, "A"); break;
        case PUMP_STATUS_PASSIVE:    return ctext_fg(COLOR_OFF, "P"); break;
        case PUMP_STATUS_OPEN_START: return ctext_fg_con(COLOR_ON, get_frame(&spinner_bars, 0)); break;

        default:
        case PUMP_STATUS_CLOSE_STOP: return ctext_fg(COLOR_OFF, ""); break;
    }
}

char* bool_to_check(int isOn)
{
    return isOn ? ctext_fg(COLOR_ON, "") : ctext_fg(COLOR_OFF, "");
}

#define TERM_BUFFER_SIZE 1024 * 2
char g_term_buffer[TERM_BUFFER_SIZE] = {0};

extern atomic_int g_auto_timer;
extern atomic_int g_auto_gas;
extern atomic_int g_auto_timer_seconds;
static int g_auto_timer_seconds_old = AUTO_TIMER_SECONDS;
extern int g_auto_timer_started;
extern int g_auto_timer_seconds_elapsed;
extern time_t g_history_mode_time_on;
extern char g_auto_timer_status[];
extern char g_auto_gas_status[];

extern double g_temp_solar_min;
extern double g_temp_solar_max;
extern double g_temp_human_min;
extern double g_temp_human_max;
extern double g_temp_buf_min;
extern double g_temp_buf_max;
extern double g_temp_circ_min;
extern double g_temp_circ_max;

extern atomic_int g_ws_conn_count;

// clang-format off
#define HEADER_BUFFER_SIZE 1024
int draw_ui(struct bas_info info, int is_sending, int errors) {

    if (!info.hasValues) {
        char* ip = get_local_ips();
        int r = printf("@ %-30s\n> %-30s\n> %-30s\n", ip, "no values to draw.", sendreq_error_to_str(errors));
        free(ip);
        return r;
    }

    // make header
    char* emoji_light = ctext_fg(228, get_frame(&spinner_lights, 1));
    char* emoji_send  = is_sending ? CTEXT_FG(211, "") : " ";

    char full_time_header_raw[HEADER_BUFFER_SIZE];
    char* current_time = get_current_time();
    int hour           = get_current_hour();
    char* emoji_clock  = hour_to_clock(hour);
    char* emoji_dayhr  = hour_to_emoji(hour);
    snprintf(full_time_header_raw, HEADER_BUFFER_SIZE, "%s %s %s", emoji_clock, emoji_dayhr, current_time);
    free(current_time);

    char* full_time_header = ctext_fg(hour_to_color(hour), full_time_header_raw);

    char* ip_raw       = get_local_ips();
    char* ip           = ctext_fg(errors ? COLOR_OFF : COLOR_ON, ip_raw);
    free(ip_raw);

    char* moving_emoji_Tsolar  = ctext_fg(230, get_frame(&spinner_solar_panel, 1));
    char* moving_emoji_Tspv    = ctext_fg(213, get_frame(&spinner_window, 1));
    char* moving_emoji_Tsobna  = ctext_fg( 76, get_frame(&spinner_house, 1));
    char* moving_emoji_Tzadata = ctext_fg(154, get_frame(&spinner_cog, 1));
    //char* moving_emoji_Tmax    = ctext_fg(214, get_frame(&spinner_));
    //char* moving_emoji_Tmid    = ctext_fg(220, get_frame(&spinner_));
    //char* moving_emoji_Tmin    = ctext_fg(226, get_frame(&spinner_));
    char* moving_emoji_Tfs     = ctext_fg(110, get_frame(&spinner_recycle, 1));

    char* label_Tsolar   = CTEXT_FG(230 , "Solar ");  char* Tsolar   = temp_to_ctext_bg_con(info.Tsolar, &g_temp_solar_min, &g_temp_solar_max);
    char* label_Tspv     = CTEXT_FG(213 , "  Out ");  char* Tspv     = temp_to_ctext_bg_con(info.Tspv, &g_temp_human_min, &g_temp_human_max);
    char* label_Tsobna   = CTEXT_FG( 76 , " Room ");  char* Tsobna   = temp_to_ctext_bg_con(info.Tsobna, &g_temp_human_min, &g_temp_human_max);
    char* label_Tzadata  = CTEXT_FG(154 , "  Set ");  char* Tzadata  = temp_to_ctext_bg_con(info.Tzadata, &g_temp_human_min, &g_temp_human_max);
    char* label_Tmax     = CTEXT_FG(214 , "  Max "); char* Tmax     = temp_to_ctext_bg_con(info.Tmax, &g_temp_buf_min, &g_temp_buf_max);
    char* label_Tmid     = CTEXT_FG(220 , "  Mid "); char* Tmid     = temp_to_ctext_bg_con(info.Tmid, &g_temp_buf_min, &g_temp_buf_max);
    char* label_Tmin     = CTEXT_FG(226 , "  Min "); char* Tmin     = temp_to_ctext_bg_con(info.Tmin, &g_temp_buf_min, &g_temp_buf_max);
    char* label_Tfs      = CTEXT_FG(110 , "Circ. ");  char* Tfs      = temp_to_ctext_bg_con(info.Tfs, &g_temp_circ_min, &g_temp_circ_max);

    char* moving_emoji_heat  = ctext_fg(212 , pump_is_on(info.StatusPumpe6) ? get_frame(&spinner_heat_pump, 1) : "󱩃");
    char* moving_emoji_gas   = ctext_fg(203 , pump_is_on(info.StatusPumpe4) ? get_frame(&spinner_fire,      1) : "󰙇");
    char* moving_emoji_circ  = ctext_fg(168 , pump_is_on(info.StatusPumpe3) ? get_frame(&spinner_circle,    1) : "");
    char* moving_emoji_solar = ctext_fg(224 , pump_is_on(info.StatusPumpe7) ? get_frame(&spinner_solar,     1) : "");
    char* moving_emoji_elec  = ctext_fg( 78 , pump_is_on(info.StatusPumpe5) ? get_frame(&spinner_lightning, 1) : "󰠠");

    char* label_mode      = CTEXT_FG(222 , "   Mode 󱪯"); char* mode      = draw_heat(info.mod_rada);
    char* label_regime    = CTEXT_FG(192 , " Regime 󱖫"); char* regime    = cnum_fg(192, info.mod_rezim);
    char* label_heat      = CTEXT_FG(212 , "   Heat ");  char* heat      = draw_pump_bars(info.StatusPumpe6);
    char* label_gas       = CTEXT_FG(203 , "    Gas ");  char* gas       = draw_pump_bars(info.StatusPumpe4);
    char* label_circ      = CTEXT_FG(168 , "  Circ. ");  char* circ      = draw_pump_bars(info.StatusPumpe3);
    char* label_solar     = CTEXT_FG(224 , "  Solar ");  char* solar     = draw_pump_bars(info.StatusPumpe7);
    char* label_elec      = CTEXT_FG( 78 , "  Elec. ");  char* elec      = draw_pump_bars(info.StatusPumpe5);

    char* emoji_check = ctext_fg(82, info.TmidGE ? get_frame(&spinner_check, 1) : " ");
    char* emoji_warn  = ctext_fg(51, info.TminLT ? get_frame(&spinner_snow, 1) : " ");

    char* emoji_eye1  = ctext_fg(COLOR_ON, atomic_load(&g_auto_timer) ? get_frame(&spinner_eye_left, 0) : " ");
    char* emoji_timer = g_auto_timer_started ? ctext_fg(COLOR_ON, get_frame(&spinner_clock, 0)) : ctext_fg(COLOR_OFF, atomic_load(&g_auto_timer) ? "󱎫" : " ");
    char* emoji_eye2  = ctext_fg(COLOR_ON, atomic_load(&g_auto_gas)   ? get_frame(&spinner_eye_left, 0) : " ");

    char* emoji_reye1 = atomic_load(&g_auto_timer) ?  ctext_fg(COLOR_ON, get_frame(&spinner_eye_right, 0)) : ctext_fg(COLOR_OFF, "");
    char* emoji_reye2 = atomic_load(&g_auto_gas)   ?  ctext_fg(COLOR_ON, get_frame(&spinner_eye_right, 0)) : ctext_fg(COLOR_OFF, "");

    if (g_auto_timer_started) {
        time_t current_time;
        time(&current_time);
        g_auto_timer_seconds_elapsed = difftime(current_time, g_history_mode_time_on);
        snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, "%d/%d", g_auto_timer_seconds_elapsed, atomic_load(&g_auto_timer_seconds));
    }

    int s = atomic_load(&g_auto_timer_seconds);
    if (s != g_auto_timer_seconds_old) {
        g_auto_timer_seconds_old = s;
        snprintf(g_auto_timer_status, STATUS_BUFFER_SIZE, "changed to: %d", s);
    }


    int bytes = snprintf(g_term_buffer, TERM_BUFFER_SIZE,
             " %s\n"
             " %s %s %3d %s %-16s\n"
             "%s%s  %s %s %s  %s %s%s\n"
             "%s%s  %s %s %s  %s\n"
             "%s%s  %s %s %s%s  %s\n"
             "%s%s  %s %s %s%s  %s %s\n"
             "%s  %s   %s%s  %s\n"
             "%s  %s %s %s%s  %s\n"
             "%s  %s %s %s%s  %s\n"
             "%s%s  %s\n"
             "%s󱪯 %-40s\n"
             "%s󰙇 %-40s\n",
             full_time_header,
             emoji_light, emoji_send, atomic_load(&g_ws_conn_count), ip, errors ? sendreq_error_to_str(errors) : "",
             label_Tsolar,  moving_emoji_Tsolar,  Tsolar,  temp_to_emoji(info.Tsolar),  label_mode     ,                     mode,   emoji_eye1, emoji_timer,
             label_Tspv,    moving_emoji_Tspv,    Tspv,    temp_to_emoji(info.Tspv),    label_regime   ,                     regime,
             label_Tsobna,  moving_emoji_Tsobna,  Tsobna,  temp_to_emoji(info.Tsobna),  label_heat     , moving_emoji_heat,  heat,
             label_Tzadata, moving_emoji_Tzadata, Tzadata, temp_to_emoji(info.Tzadata), label_gas      , moving_emoji_gas,   gas,    emoji_eye2,
             label_Tmax,                          Tmax,                                 label_circ     , moving_emoji_circ,  circ,
             label_Tmid,                          Tmid,    emoji_check,                 label_solar    , moving_emoji_solar, solar,
             label_Tmin,                          Tmin,    emoji_warn,                  label_elec     , moving_emoji_elec,  elec,
             label_Tfs,     moving_emoji_Tfs,     Tfs,
             emoji_reye1, g_auto_timer_status,
             emoji_reye2, g_auto_gas_status
             );


    free(full_time_header);
    free(emoji_light);
    free(ip);
    
    free(moving_emoji_Tsolar);
    free(moving_emoji_Tspv);
    free(moving_emoji_Tsobna);
    free(moving_emoji_Tzadata);
    //moving_emoji_Tmax
    //moving_emoji_Tmid
    //moving_emoji_Tmin
    free(moving_emoji_Tfs);

    
    free(Tspv);                                               free(mode);   free(emoji_eye1); free(emoji_timer);
    free(Tsolar);                                             free(regime);
    free(Tsobna);                  free(moving_emoji_heat);   free(heat);
    free(Tzadata);                 free(moving_emoji_gas);    free(gas);    free(emoji_eye2);
    free(Tmax);                    free(moving_emoji_circ);   free(circ);
    free(Tmid); free(emoji_check); free(moving_emoji_solar);  free(solar);
    free(Tmin); free(emoji_warn);  free(moving_emoji_elec);   free(elec);
    free(Tfs);
    free(emoji_reye1);
    free(emoji_reye2);

    spin_spinner(&spinner_circle);
    spin_spinner(&spinner_eye_left);
    spin_spinner(&spinner_eye_right);
    spin_spinner(&spinner_bars);
    spin_spinner(&spinner_clock);

    DPL("===[ OUTPUT BEGIN ]===");
    printf("%s", g_term_buffer);
    DPL("===[ OUTPUT END ]===");
    D(printf("OUTPUT LEN: %d\n", bytes));
    return bytes;
}
// clang-format on
