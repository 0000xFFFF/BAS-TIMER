import requests
import time

from utils import timestamp, time_to_str, elapsed_str
from process_data import process_data_and_draw_ui
from colors import bool_to_ctext_fg, int_to_ctext_fg, ctext_fg, COLOR_ON, COLOR_OFF
from logger_config import requests_logger, changes_logger

GLOBAL_UNIX_COUNTER = int(time.time() * 1000)

AUTO_TIMER = True
AUTO_TIMER_STARTED = False
AUTO_TIMER_SECONDS = 900  # 15 mins
AUTO_TIMER_SECONDS_ELAPSED = 0
AUTO_TIMER_STATUS = ""
AUTO_GAS = True
AUTO_GAS_STATUS = ""

HISTORY_MODE = None
HISTORY_MODE_TIME_CHANGED = None
HISTORY_MODE_TIME_ON = None
HISTORY_MODE_TIME_OFF = None
HISTORY_GAS = None
HISTORY_GAS_TIME_CHANGED = None
HISTORY_GAS_TIME_ON = None
HISTORY_GAS_TIME_OFF = None

# act like firefox
req_headers = {
    "Accept": "application/json, text/javascript, */*; q=0.01",
    "Accept-Encoding": "gzip, deflate",
    "Accept-Language": "en-US,en;q=0.5",
    "Cache-Control": "no-cache",
    "Connection": "keep-alive",
    "Referer": "http://192.168.1.250/",
    "Sec-GPC": "1",
    "User-Agent": "Mozilla/5.0 (X11; Linux x86_64; rv:135.0) Gecko/20100101 Firefox/135.0",
    "X-Requested-With": "XMLHttpRequest",
}

URLS = {
    "VARS": "http://192.168.1.250/isc/get_var_js.aspx?StatusPumpe3=&StatusPumpe4=&StatusPumpe5=&StatusPumpe6=&StatusPumpe7=&Taktualno=&Tfs=&Tmax=&Tmin=&Tsobna=&Tsolar=&Tspv=&Tzad_komf=&Tzad_mraz=&Tzad_red=&Tzadata=&mod_rada=&mod_rezim=&__Time=&__Date=&Jeftina_tarifa=&grejanje_off=&Alarm_tank=&Alarm_solar=&STATE_Preklopka=&SESSIONID=-1",
    "OFF": "http://192.168.1.250/isc/set_var.aspx?mod_rada=0,-1&=&SESSIONID=-1",
    "ON": "http://192.168.1.250/isc/set_var.aspx?mod_rada=1,-1&=&SESSIONID=-1",
    "GAS_OFF": "http://192.168.1.250/isc/set_var.aspx?RezimRadaPumpe4=0,-1&=&SESSIONID=-1",
    "GAS_ON": "http://192.168.1.250/isc/set_var.aspx?RezimRadaPumpe4=3,-1&=&SESSIONID=-1",
}


def prepare(url):
    global GLOBAL_UNIX_COUNTER
    GLOBAL_UNIX_COUNTER += 1
    url_with_timestamp = f"{url}&_={GLOBAL_UNIX_COUNTER}"
    request = requests.Request("GET", url_with_timestamp, headers=req_headers)
    import server

    prepared = server.main_session.prepare_request(request)
    return prepared


def prepared_url(prepared):
    return f"{prepared.method} {prepared.url}"


def send(url):
    prepared = prepare(url)
    requests_logger.write(f"{prepared_url(prepared)} --> ")

    try:
        import server

        response = server.main_session.send(prepared)
        response.raise_for_status()  # Raise an error for HTTP error codes
        requests_logger.write(f"SUCCESS\n")
    except Exception as e:
        requests_logger.write(f"FAILED ({e})\n")
        return None, e

    return response, None


def update_history(dic):

    global HISTORY_MODE
    global HISTORY_MODE_TIME_CHANGED
    global HISTORY_MODE_TIME_ON
    global HISTORY_MODE_TIME_OFF
    global HISTORY_GAS
    global HISTORY_GAS_TIME_CHANGED
    global HISTORY_GAS_TIME_ON
    global HISTORY_GAS_TIME_OFF

    global AUTO_TIMER_STARTED
    global AUTO_TIMER_STATUS
    global AUTO_GAS_STATUS

    if HISTORY_MODE is None or HISTORY_MODE != dic["mod_rada"]:
        HISTORY_MODE = dic["mod_rada"]
        HISTORY_MODE_TIME_CHANGED = time.time()

        if dic["mod_rada"]:
            HISTORY_MODE_TIME_ON = HISTORY_MODE_TIME_CHANGED
            changes_logger.write(f"mod_rada = {dic['mod_rada']}\n")
            AUTO_TIMER_STATUS = ctext_fg(COLOR_ON, f" {timestamp()} 󰐸")
        else:
            HISTORY_MODE_TIME_OFF = HISTORY_MODE_TIME_CHANGED
            e = "\n"
            p = ""
            if HISTORY_MODE_TIME_ON and HISTORY_MODE_TIME_OFF:
                elap = elapsed_str(HISTORY_MODE_TIME_OFF, HISTORY_MODE_TIME_ON)
                e = f" -- {elap}\n"
                p = f" 󱫐 {elap}\n"

            changes_logger.write(f"mod_rada = {dic['mod_rada']}{e}")
            AUTO_TIMER_STATUS = ctext_fg(COLOR_OFF, f" {timestamp()} {p}")

            if AUTO_TIMER_STARTED:
                AUTO_TIMER_STARTED = False
                AUTO_TIMER_STAUS = ctext_fg(COLOR_OFF, f"{timestamp()} 󰜺")

    if HISTORY_GAS is None or HISTORY_GAS != dic["StatusPumpe4"]:
        HISTORY_GAS = dic["StatusPumpe4"]
        HISTORY_GAS_TIME_CHANGED = time.time()

        if dic["StatusPumpe4"]:
            HISTORY_GAS_TIME_ON = HISTORY_GAS_TIME_CHANGED
            changes_logger.write(f"StatusPumpe4 = {dic['StatusPumpe4']}\n")
            AUTO_GAS_STATUS = ctext_fg(COLOR_ON, f" {timestamp()} ")
        else:
            HISTORY_GAS_TIME_OFF = HISTORY_GAS_TIME_CHANGED
            e = "\n"
            p = ""
            if HISTORY_GAS_TIME_ON and HISTORY_GAS_TIME_OFF:
                elap = elapsed_str(HISTORY_GAS_TIME_OFF, HISTORY_GAS_TIME_ON)
                e = f" -- {elap}\n"
                p = f" 󱫐 {elap}\n"

            changes_logger.write(f"StatusPumpe4 = {dic['StatusPumpe4']}{e}")
            AUTO_GAS_STATUS = ctext_fg(COLOR_OFF, f" {timestamp()} {p}")


def do_logic_timer(dic):
    global AUTO_TIMER
    global AUTO_TIMER_STARTED
    global AUTO_TIMER_SECONDS
    global AUTO_TIMER_SECONDS_ELAPSED
    global AUTO_TIMER_STATUS

    if AUTO_TIMER and int(dic["mod_rada"]):
        if AUTO_TIMER_STARTED:
            AUTO_TIMER_SECONDS_ELAPSED = time.time() - HISTORY_MODE_TIME_ON
            AUTO_TIMER_STATUS = f"{AUTO_TIMER_SECONDS_ELAPSED:.2f}/{AUTO_TIMER_SECONDS}"

            if AUTO_TIMER_SECONDS_ELAPSED >= AUTO_TIMER_SECONDS:
                AUTO_TIMER_STARTED = False
                AUTO_TIMER_STATUS = ctext_fg(COLOR_OFF, f"{timestamp()} 󱪯")
                if HISTORY_MODE_TIME_ON:
                    e = elapsed_str(time.time(), HISTORY_MODE_TIME_ON)
                    AUTO_TIMER_STATUS = ctext_fg(COLOR_OFF, f"󱫐 {e} 󱪯")
                send(URLS["OFF"])

        else:
            AUTO_TIMER_STARTED = True
            AUTO_TIMER_STATUS = ctext_fg(COLOR_ON, f"{timestamp()} 󱫌")


def do_logic_gas(dic):
    if AUTO_GAS and int(dic["StatusPumpe4"]) == 0 and dic["TminLT"]:
        AUTO_GAS_STATUS = ctext_fg(COLOR_ON, f"{timestamp()} ")
        send(URLS["GAS_ON"])

    if AUTO_GAS and int(dic["StatusPumpe4"]) == 3 and dic["TmidGE"]:
        AUTO_GAS_STATUS = ctext_fg(COLOR_OFF, f"{timestamp()} 󰙇")
        if HISTORY_GAS_TIME_ON and HISTORY_GAS_TIME_OFF:
            AUTO_GAS_STATUS = ctext_fg(COLOR_OFF, f"󱫐 {elapsed_str(time.time(), HISTORY_GAS_TIME_ON)} 󰙇")
        send(URLS["GAS_OFF"])


def remember_vars_do_action(dic):
    global AUTO_GAS
    global AUTO_GAS_STATUS

    update_history(dic)
    do_logic_timer(dic)
    do_logic_gas(dic)

    return 0


last_data = None
last_ret = False


def make_request():
    global last_data
    global last_ret

    # Send GET request
    response, err = send(URLS["VARS"])

    if response is None:
        last_ret = False
        data = last_data

        if last_data is None:
            print(f"[{timestamp()}] {err.__class__.__name__}")
            return
    else:
        data = response.json()
        last_data = data

    dic = process_data_and_draw_ui(data, last_ret, is_request=True)

    # send requests based on processed data
    remember_vars_do_action(dic)

    return dic


from server import MAIN_WORKER_DRAW_SLEEP


REQUEST_SLEEP = 3  # on third second to request
do_reqest_on_count = REQUEST_SLEEP / MAIN_WORKER_DRAW_SLEEP
request_count = 0


def do_work():

    global last_data
    global last_ret
    global request_count
    last_ret = True
    request_count += 1

    if request_count >= do_reqest_on_count or last_data is None:
        request_count = 0
        return make_request()

    dic = process_data_and_draw_ui(last_data, last_ret)
    return dic
