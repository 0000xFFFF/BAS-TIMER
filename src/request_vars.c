#include "src/request.h"
#include <sched.h>
#include <stdatomic.h>
#include <stdint.h>

// BAS
const char* URL_VARS = "http://192.168.1.250/isc/get_var_js.aspx?StatusPumpe3=&StatusPumpe4=&StatusPumpe5=&StatusPumpe6=&StatusPumpe7=&Taktualno=&Tfs=&Tmax=&Tmin=&Tsobna=&Tsolar=&Tspv=&Tzad_komf=&Tzad_mraz=&Tzad_red=&Tzadata=&mod_rada=&mod_rezim=&__Time=&__Date=&Jeftina_tarifa=&grejanje_off=&Alarm_tank=&Alarm_solar=&STATE_Preklopka=&SESSIONID=-1";
const char* URL_HEAT_OFF = "http://192.168.1.250/isc/set_var.aspx?mod_rada=0,-1&=&SESSIONID=-1";
const char* URL_HEAT_ON = "http://192.168.1.250/isc/set_var.aspx?mod_rada=1,-1&=&SESSIONID=-1";
const char* URL_GAS_OFF = "http://192.168.1.250/isc/set_var.aspx?RezimRadaPumpe4=0,-1&=&SESSIONID=-1";
const char* URL_GAS_ON = "http://192.168.1.250/isc/set_var.aspx?RezimRadaPumpe4=3,-1&=&SESSIONID=-1";

// wttr.in
const char* URL_WTTRIN = "https://wttr.in/?format=%c%2C%C%2C%x%2C%h%2C%t%2C%f%2C%w%2C%l%2C%m%2C%M%2C%p%2C%P%2C%u%2C%D%2C%S%2C%z%2C%s%2C%d%2C%T%2C%Z";

//    c    Weather condition,
//    C    Weather condition textual name,
//    x    Weather condition, plain-text symbol,
//    h    Humidity,
//    t    Temperature (Actual),
//    f    Temperature (Feels Like),
//    w    Wind,
//    l    Location,
//    m    Moon phase 🌑🌒🌓🌔🌕🌖🌗🌘,
//    M    Moon day,
//    p    Precipitation (mm/3 hours),
//    P    Pressure (hPa),
//    u    UV index (1-12),
//    D    Dawn*,
//    S    Sunrise*,
//    z    Zenith*,
//    s    Sunset*,
//    d    Dusk*,
//    T    Current time*,
//    Z    Local timezone.

const char* REQUEST_FORMAT_BAS =
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

const char* REQUEST_FORMAT_WTTRIN =
    "GET %s HTTP/1.0\r\n"
    "Host: %.*s\r\n"
    "Content-Type: octet-stream\r\n"
    "Content-Length: 0\r\n"
    "\r\n";

const char* weather_keywords[][5] = {
    [WEATHER_SNOW] = {"snow", "blizzard", "ice", "sleet", NULL},
    [WEATHER_THUNDER] = {"thunder", "storm", "lightning", NULL},
    [WEATHER_RAIN] = {"rain", "drizzle", NULL},
    [WEATHER_CLOUD] = {"cloud", "cloudy", "overcast", NULL},
    [WEATHER_FOG] = {"fog", "mist", NULL},
    [WEATHER_CLEAR] = {"clear", "sunny", NULL},
};
