#include "src/globals.h"
#include "src/request.h"
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>

const char* const URL_VARS = "http://192.168.1.250/isc/get_var_js.aspx?StatusPumpe3=&StatusPumpe4=&StatusPumpe5=&StatusPumpe6=&StatusPumpe7=&Taktualno=&Tfs=&Tmax=&Tmin=&Tsobna=&Tsolar=&Tspv=&Tzad_komf=&Tzad_mraz=&Tzad_red=&Tzadata=&mod_rada=&mod_rezim=&__Time=&__Date=&Jeftina_tarifa=&grejanje_off=&Alarm_tank=&Alarm_solar=&STATE_Preklopka=&SESSIONID=-1";
const char* const URL_HEAT_OFF = "http://192.168.1.250/isc/set_var.aspx?mod_rada=0,-1&=&SESSIONID=-1";
const char* const URL_HEAT_ON = "http://192.168.1.250/isc/set_var.aspx?mod_rada=1,-1&=&SESSIONID=-1";
const char* const URL_GAS_OFF = "http://192.168.1.250/isc/set_var.aspx?RezimRadaPumpe4=0,-1&=&SESSIONID=-1";
const char* const URL_GAS_ON = "http://192.168.1.250/isc/set_var.aspx?RezimRadaPumpe4=3,-1&=&SESSIONID=-1";

const char* const URL_WTTRIN = "https://wttr.in/?format=4";

const char* const REQUEST_FORMAT_BAS =
    "GET %s HTTP/1.0\r\n"
    "Host: %.*s\r\n"
    "Accept: application/json, text/javascript, */*; q=0.01\r\n"
    "Accept-Encoding: gzip, deflate\r\n"
    "Accept-Language: en-US,en;q=0.9\r\n"
    "Connection: keep-alive\r\n"
    "Cookie: i18next=srb\r\n"
    "Referer: http://192.168.1.250/\r\n"
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/133.0.0.0 Safari/537.36\r\n"
    "X-Requested-With: XMLHttpRequest\r\n"
    "\r\n";

const char* const REQUEST_FORMAT_WTTRIN =
    "GET %s HTTP/1.0\r\n"
    "Host: %.*s\r\n"
    "Content-Type: octet-stream\r\n"
    "Content-Length: 0\r\n"
    "\r\n";


atomic_int g_auto_timer = ENABLE_AUTO_TIMER;
atomic_int g_auto_gas = ENABLE_AUTO_GAS;
atomic_int g_auto_timer_seconds = AUTO_TIMER_SECONDS;
int g_auto_timer_started = 0;
int g_auto_timer_seconds_elapsed = 0;
char g_auto_timer_status[BIGBUFF] = "...";
char g_auto_gas_status[BIGBUFF] = "...";

int g_history_mode = -1;
time_t g_history_mode_time_changed = 0;
time_t g_history_mode_time_on = 0;
time_t g_history_mode_time_off = 0;
int g_history_gas = -1;
time_t g_history_gas_time_changed = 0;
time_t g_history_gas_time_on = 0;
time_t g_history_gas_time_off = 0;

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
