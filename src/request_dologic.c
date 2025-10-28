#include "src/debug.h"
#include "src/logger.h"
#include "src/request.h"
#include "src/utils.h"
#include <stdio.h>
#include <time.h>

void update_history(int mod_rada, int StatusPumpe4)
{
    DPL("UPDATE HISTORY");
    time_t current_time;
    time(&current_time);
    struct tm* timeinfo = localtime(&current_time);
    char time_str[MIDBUFF] = {0};
    strftime_YmdHMS(time_str, MIDBUFF, timeinfo);

    if (g_history_mode == -1 || g_history_mode != mod_rada) {
        g_history_mode = mod_rada;
        g_history_mode_time_changed = current_time;

        if (mod_rada) {
            g_history_mode_time_on = g_history_mode_time_changed;
            logger_changes_write("mod_rada = %d\n", mod_rada);
            snprintf(g_auto_timer_status, BIGBUFF, " %s 󰐸", time_str);
        }
        else {
            g_history_mode_time_off = g_history_mode_time_changed;
            char e[MIDBUFF] = "\n";
            char p[MIDBUFF] = "";
            if (g_history_mode_time_on && g_history_mode_time_off) {
                char elap[SMALLBUFF];
                elapsed_str(elap, SMALLBUFF, g_history_mode_time_off, g_history_mode_time_on);
                snprintf(e, MIDBUFF, " -- %s\n", elap);
                snprintf(p, MIDBUFF, " 󱫐 %s", elap);
            }

            logger_changes_write("mod_rada = %d%s", mod_rada, e);
            snprintf(g_auto_timer_status, BIGBUFF, " %s %s", time_str, p);

            if (g_auto_timer_started) {
                g_auto_timer_started = 0;
                snprintf(g_auto_timer_status, BIGBUFF, "%s 󰜺", time_str);
            }
        }
    }

    if (g_history_gas == -1 || g_history_gas != StatusPumpe4) {
        g_history_gas = StatusPumpe4;
        g_history_gas_time_changed = current_time;

        if (StatusPumpe4) {
            g_history_gas_time_on = g_history_gas_time_changed;
            logger_changes_write("StatusPumpe4 = %d\n", StatusPumpe4);
            snprintf(g_auto_gas_status, BIGBUFF, " %s ", time_str);
        }
        else {
            g_history_gas_time_off = g_history_gas_time_changed;
            char e[MIDBUFF] = "\n";
            char p[MIDBUFF] = "";
            if (g_history_gas_time_on && g_history_gas_time_off) {
                char elap[SMALLBUFF] = {0};
                elapsed_str(elap, SMALLBUFF, g_history_gas_time_off, g_history_gas_time_on);
                snprintf(e, sizeof(e), " -- %s\n", elap);
                snprintf(p, sizeof(p), " 󱫐 %s", elap);
            }

            logger_changes_write("StatusPumpe4 = %d%s", StatusPumpe4, e);
            sprintf(g_auto_gas_status, " %s %s", time_str, p);
        }
    }
}

void do_logic_timer(int mod_rada)
{

    time_t current_time;
    time(&current_time);
    struct tm* timeinfo = localtime(&current_time);
    char time_str[MIDBUFF] = {0};
    strftime_YmdHMS(time_str, MIDBUFF, timeinfo);

    if (g_auto_timer && mod_rada) {
        if (g_auto_timer_started) {
            g_auto_timer_seconds_elapsed = difftime(current_time, g_history_mode_time_on);
            snprintf(g_auto_timer_status, BIGBUFF, "%d/%d", g_auto_timer_seconds_elapsed, atomic_load(&g_auto_timer_seconds));

            if (g_auto_timer_seconds_elapsed >= g_auto_timer_seconds) {
                g_auto_timer_started = 0;
                snprintf(g_auto_timer_status, BIGBUFF, "%s 󱪯", time_str);
                if (g_history_mode_time_on) {
                    char elap[SMALLBUFF] = {0};
                    elapsed_str(elap, SMALLBUFF, current_time, g_history_mode_time_on);
                    snprintf(g_auto_timer_status, BIGBUFF, "󱫐 %s 󱪯", elap);
                }
                requests_send_quick(URL_HEAT_OFF, 1, 0);
            }
        }
        else {
            g_auto_timer_started = 1;
            snprintf(g_auto_timer_status, BIGBUFF, "%s 󱫌", time_str);
        }
    }
}

void do_logic_gas(int StatusPumpe4, int TminLT, int TmidGE)
{

    time_t current_time;
    time(&current_time);
    struct tm* timeinfo = localtime(&current_time);
    char time_str[MIDBUFF] = {0};
    strftime_YmdHMS(time_str, MIDBUFF, timeinfo);

    if (g_auto_gas && StatusPumpe4 == 0 && TminLT) {
        sprintf(g_auto_gas_status, "%s ", time_str);
        requests_send_bas(URL_GAS_ON, 1, 0);
    }

    if (g_auto_gas && StatusPumpe4 == 3 && TmidGE) {
        sprintf(g_auto_gas_status, "%s 󰙇", time_str);
        if (g_history_gas_time_on && g_history_gas_time_off) {
            char elap[SMALLBUFF] = {0};
            elapsed_str(elap, SMALLBUFF, current_time, g_history_gas_time_on);
            sprintf(g_auto_gas_status, "󱫐 %s 󰙇", elap);
        }
        requests_send_bas(URL_GAS_OFF, 1, 0);
    }
}

void remember_vars_do_action(int mod_rada, int StatusPumpe4, int TminLT, int TmidGE)
{
    update_history(mod_rada, StatusPumpe4);
    do_logic_timer(mod_rada);
    do_logic_gas(StatusPumpe4, TminLT, TmidGE);
}
