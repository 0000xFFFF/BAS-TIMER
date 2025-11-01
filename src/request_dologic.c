#include "src/debug.h"
#include "src/logger.h"
#include "src/request.h"
#include "src/utils.h"
#include <stdio.h>
#include <time.h>

static void update_history(struct bas_info* info)
{
    DPL("UPDATE HISTORY");
    time_t current_time;
    time(&current_time);
    struct tm* timeinfo = localtime(&current_time);
    char time_str[MIDBUFF] = {0};
    strftime_YmdHMS(time_str, sizeof(time_str), timeinfo);

    if (info->history_mode == -1 || info->history_mode != info->mod_rada) {
        info->history_mode = info->mod_rada;
        info->history_mode_time_changed = current_time;

        if (info->mod_rada) {
            info->history_mode_time_on = info->history_mode_time_changed;
            logger_changes_write("mod_rada = %d\n", info->mod_rada);
            snprintf(info->opt_auto_timer_status, sizeof(info->opt_auto_timer_status), " %s 󰐸", time_str);
        }
        else {
            info->history_mode_time_off = info->history_mode_time_changed;
            char e[MIDBUFF] = "\n";
            char p[MIDBUFF] = "";
            if (info->history_mode_time_on && info->history_mode_time_off) {
                char elap[SMALLBUFF];
                elapsed_str(elap, sizeof(elap), info->history_mode_time_off, info->history_mode_time_on);
                snprintf(e, sizeof(e), " -- %s\n", elap);
                snprintf(p, sizeof(p), " 󱫐 %s", elap);
            }

            logger_changes_write("mod_rada = %d%s", info->mod_rada, e);
            snprintf(info->opt_auto_timer_status, sizeof(info->opt_auto_timer_status), " %s %s", time_str, p);

            if (info->opt_auto_timer_started) {
                info->opt_auto_timer_started = 0;
                snprintf(info->opt_auto_timer_status, sizeof(info->opt_auto_timer_status), "%s 󰜺", time_str);
            }
        }
    }

    if (info->history_gas == -1 || info->history_gas != info->StatusPumpe4) {
        info->history_gas = info->StatusPumpe4;
        info->history_gas_time_changed = current_time;

        if (info->StatusPumpe4) {
            info->history_gas_time_on = info->history_gas_time_changed;
            logger_changes_write("StatusPumpe4 = %d\n", info->StatusPumpe4);
            snprintf(info->opt_auto_gas_status, sizeof(info->opt_auto_gas_status), " %s ", time_str);
        }
        else {
            info->history_gas_time_off = info->history_gas_time_changed;
            char e[MIDBUFF] = "\n";
            char p[MIDBUFF] = "";
            if (info->history_gas_time_on && info->history_gas_time_off) {
                char elap[SMALLBUFF] = {0};
                elapsed_str(elap, sizeof(elap), info->history_gas_time_off, info->history_gas_time_on);
                snprintf(e, sizeof(e), " -- %s\n", elap);
                snprintf(p, sizeof(p), " 󱫐 %s", elap);
            }

            logger_changes_write("StatusPumpe4 = %d%s", info->StatusPumpe4, e);
            snprintf(info->opt_auto_gas_status, sizeof(info->opt_auto_gas_status), " %s %s", time_str, p);
        }
    }
}

static void do_logic_timer(struct bas_info* info)
{
    time_t current_time;
    time(&current_time);
    struct tm* timeinfo = localtime(&current_time);
    char time_str[MIDBUFF] = {0};
    strftime_YmdHMS(time_str, sizeof(time_str), timeinfo);

    if (info->opt_auto_timer && info->mod_rada) {
        if (info->opt_auto_timer_started) {
            info->opt_auto_timer_seconds_elapsed = difftime(current_time, info->history_mode_time_on);
            snprintf(info->opt_auto_timer_status, sizeof(info->opt_auto_timer_status), "%d/%d", info->opt_auto_timer_seconds_elapsed, info->opt_auto_timer_seconds);

            if (info->opt_auto_timer_seconds_elapsed >= info->opt_auto_timer_seconds) {
                info->opt_auto_timer_started = 0;
                snprintf(info->opt_auto_timer_status, sizeof(info->opt_auto_timer_status), "%s 󱪯", time_str);
                if (info->history_mode_time_on) {
                    char elap[SMALLBUFF] = {0};
                    elapsed_str(elap, SMALLBUFF, current_time, info->history_mode_time_on);
                    snprintf(info->opt_auto_timer_status, sizeof(info->opt_auto_timer_status), "󱫐 %s 󱪯", elap);
                }
                request_send_quick(URL_HEAT_OFF);
            }
        }
        else {
            info->opt_auto_timer_started = 1;
            snprintf(info->opt_auto_timer_status, sizeof(info->opt_auto_timer_status), "%s 󱫌", time_str);
        }
    }
}

static void do_logic_gas(struct bas_info* info)
{
    time_t current_time;
    time(&current_time);
    struct tm* timeinfo = localtime(&current_time);
    char time_str[MIDBUFF] = {0};
    strftime_YmdHMS(time_str, sizeof(time_str), timeinfo);

    if (info->opt_auto_gas && info->StatusPumpe4 == 0 && info->TminLT && !info->TmidGE && !info->TmaxGE) {
        snprintf(info->opt_auto_gas_status, sizeof(info->opt_auto_gas_status), "%s ", time_str);
        request_send_quick(URL_GAS_ON);
    }

    if (info->opt_auto_gas && info->StatusPumpe4 == 3 && (info->TmidGE || info->TmaxGE)) {
        snprintf(info->opt_auto_gas_status, sizeof(info->opt_auto_gas_status), "%s 󰙇", time_str);
        if (info->history_gas_time_on && info->history_gas_time_off) {
            char elap[SMALLBUFF] = {0};
            elapsed_str(elap, sizeof(elap), current_time, info->history_gas_time_on);
            snprintf(info->opt_auto_gas_status, sizeof(info->opt_auto_gas_status), "󱫐 %s 󰙇", elap);
        }
        request_send_quick(URL_GAS_OFF);
    }
}

// TODO: request_send_quick can fail --> print/handle that

void remember_vars_do_action(struct bas_info* info)
{
    update_history(info);
    do_logic_timer(info);
    do_logic_gas(info);
}
