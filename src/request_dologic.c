#include "debug.h"
#include "logger.h"
#include "request.h"
#include "utils.h"
#include <stdio.h>
#include <time.h>

static void update_history(struct BasInfo* info)
{
    DPL("UPDATE HISTORY");
    time_t current_time;
    time(&current_time);

    if (info->history_mode == -1 || info->history_mode != info->mod_rada) {
        info->history_mode = info->mod_rada;
        info->history_mode_time_changed = current_time;

        if (info->mod_rada) {
            info->history_mode_time_on = info->history_mode_time_changed;
            logger_write_changes("mod_rada = %d\n", info->mod_rada);
            info->opt_auto_timer_status = OPT_STATUS_STARTED;
            info->opt_auto_timer_status_changed = info->history_mode_time_changed;
        }
        else {
            info->history_mode_time_off = info->history_mode_time_changed;
            char e[TINYBUFF] = "\n";
            if (info->history_mode_time_on && info->history_mode_time_off) {
                char elap[10];
                elapsed_str(elap, sizeof(elap), info->history_mode_time_off, info->history_mode_time_on);
                snprintf(e, sizeof(e), " -- %s\n", elap);
            }

            logger_write_changes("mod_rada = %d%s", info->mod_rada, e);
            info->opt_auto_timer_status = OPT_STATUS_STOPPED;
            if (info->opt_auto_timer_started) {
                info->opt_auto_timer_started = false;
                info->opt_auto_timer_status = OPT_STATUS_CANCELLED;
            }
            info->opt_auto_timer_status_changed = info->history_mode_time_changed;
        }
    }

    if (info->history_gas == PUMP_STATUS_UNKNOWN || info->history_gas != info->StatusPumpe4) {
        info->history_gas = info->StatusPumpe4;
        info->history_gas_time_changed = current_time;

        if (info->StatusPumpe4) {
            info->history_gas_time_on = info->history_gas_time_changed;
            logger_write_changes("StatusPumpe4 = %d\n", info->StatusPumpe4);
            info->opt_auto_gas_status = OPT_STATUS_STARTED;
            info->opt_auto_gas_status_changed = info->history_gas_time_changed;
        }
        else {
            info->history_gas_time_off = info->history_gas_time_changed;
            char e[TINYBUFF] = "\n";
            if (info->history_gas_time_on && info->history_gas_time_off) {
                char elap[10] = {0};
                elapsed_str(elap, sizeof(elap), info->history_gas_time_off, info->history_gas_time_on);
                snprintf(e, sizeof(e), " -- %s\n", elap);
            }
            logger_write_changes("StatusPumpe4 = %d%s", info->StatusPumpe4, e);
            info->opt_auto_gas_status = OPT_STATUS_STOPPED;
            info->opt_auto_gas_status_changed = info->history_gas_time_changed;
        }
    }
}

static void do_logic_timer(struct BasInfo* info)
{
    time_t current_time;
    time(&current_time);

    // scheduled heat to turn on
    if (info->Tspv < info->schedules_t_min) {
        struct tm local;
        localtime_r(&current_time, &local);
        if (local.tm_yday != info->schedules_last_yday) { // new day -> reset all schedules
            for (int i = 0; i < HEAT_SCHEDULES_COUNT; i++) {
                info->schedules[i].switched = false;
            }
            info->schedules_last_yday = local.tm_yday;
        }
        int sec_today = hms_to_seconds(local.tm_hour, local.tm_min, local.tm_sec);
        for (int i = 0; i < HEAT_SCHEDULES_COUNT; i++) {
            if (!info->schedules[i].valid) continue;

            if (!info->schedules[i].switched && sec_today >= info->schedules[i].time) {
                info->schedules[i].switched = true;
                info->opt_auto_timer_seconds = info->schedules[i].duration;
                info->opt_auto_timer_status = OPT_STATUS_CHANGED;
                request_send_quick(URL_HEAT_ON);
                logger_write_changes("heat schedule - duration: %d\n", info->schedules[i].duration);
            }
        }
    }

    if (info->opt_auto_timer && info->mod_rada) {
        if (info->opt_auto_timer_started) {
            info->opt_auto_timer_seconds_elapsed = current_time - info->history_mode_time_on;

            if (info->opt_auto_timer_seconds_elapsed >= info->opt_auto_timer_seconds) {
                info->opt_auto_timer_started = false;
                info->opt_auto_timer_status = OPT_STATUS_STOPPING;
                info->opt_auto_timer_status_changed = current_time;
                request_send_quick(URL_HEAT_OFF);
            }
        }
        else {
            info->opt_auto_timer_started = true;
            info->opt_auto_timer_status = OPT_STATUS_STARTING;
            info->opt_auto_timer_status_changed = current_time;
        }
    }
}

static void do_logic_gas(struct BasInfo* info)
{
    time_t current_time;
    time(&current_time);

    if (info->opt_auto_gas && info->StatusPumpe4 == 0 && info->TminLT && !info->TmidGE && !info->TmaxGE) {
        info->opt_auto_gas_status = OPT_STATUS_STARTING;
        info->opt_auto_gas_status_changed = current_time;
        request_send_quick(URL_GAS_ON);
    }

    if (info->opt_auto_gas && info->StatusPumpe4 == 3 && (info->TmidGE || info->TmaxGE)) {
        info->opt_auto_gas_status = OPT_STATUS_STOPPING;
        info->opt_auto_gas_status_changed = current_time;
        request_send_quick(URL_GAS_OFF);
    }
}

// TODO: request_send_quick can fail --> print/handle that

void remember_vars_do_action(struct BasInfo* info)
{
    update_history(info);
    do_logic_timer(info);
    do_logic_gas(info);
}
