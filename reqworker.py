import requests
import time

from utils import timestamp, time_to_str, elapsed_str
from process_data import process_data
from colors import bool_to_ctext_fg, int_to_ctext_fg
from logger_config import requests_logger, changes_logger

GLOBAL_UNIX_COUNTER = int(time.time() * 1000)

AUTO_TIMER = True
AUTO_TIMER_STARTED = False
AUTO_TIMER_SECONDS = 900  # 15 mins
AUTO_TIMER_SECONDS_ELAPSED = 0
AUTO_TIMER_STATUS = ""
AUTO_TIMER_TIME_STARTED = 0
AUTO_TIMER_TIME_FINISHED = 0
AUTO_GAS = True
AUTO_GAS_STATUS = ""
AUTO_GAS_TIME_STARTED = 0
AUTO_GAS_TIME_FINISHED = 0

HISTORY_MODE = None
HISTORY_MODE_TIME_CHANGED = None
HISTORY_MODE_TIME_STARTED = None
HISTORY_MODE_TIME_FINISHED = None
HISTORY_GAS = None
HISTORY_GAS_TIME_CHANGED = None
HISTORY_GAS_TIME_STARTED = None
HISTORY_GAS_TIME_FINISHED = None

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


def action(dic):
    global AUTO_TIMER
    global AUTO_TIMER_STARTED
    global AUTO_TIMER_SECONDS
    global AUTO_TIMER_SECONDS_ELAPSED
    global AUTO_TIMER_STATUS
    global AUTO_TIMER_TIME_STARTED
    global AUTO_TIMER_TIME_FINISHED
    global AUTO_GAS
    global AUTO_GAS_STATUS
    global AUTO_GAS_TIME_STARTED
    global AUTO_GAS_TIME_FINISHED

    global HISTORY_MODE
    global HISTORY_MODE_TIME_CHANGED
    global HISTORY_MODE_TIME_STARTED
    global HISTORY_MODE_TIME_FINISHED
    global HISTORY_GAS
    global HISTORY_GAS_TIME_CHANGED
    global HISTORY_GAS_TIME_STARTED
    global HISTORY_GAS_TIME_FINISHED

    if HISTORY_MODE is None or HISTORY_MODE != dic["mod_rada"]:
        HISTORY_MODE = dic["mod_rada"]
        HISTORY_MODE_TIME_CHANGED = time.time()

        if dic["mod_rada"]:
            HISTORY_MODE_TIME_STARTED = HISTORY_MODE_TIME_CHANGED
            changes_logger.write(f"mod_rada = {dic["mod_rada"]}\n")
        else:
            HISTORY_MODE_TIME_FINISHED = HISTORY_MODE_TIME_CHANGED
            e = "\n"
            if HISTORY_MODE_TIME_STARTED and HISTORY_MODE_TIME_FINISHED:
                e = f" -- {elapsed_str(HISTORY_MODE_TIME_FINISHED, HISTORY_MODE_TIME_STARTED)}\n"

            changes_logger.write(f"mod_rada = {dic["mod_rada"]}{e}")

            if AUTO_TIMER_STARTED:
                AUTO_TIMER_STARTED = False
                AUTO_TIMER_TIME_FINISHED = time.time()
                AUTO_TIMER_STATUS = f"{timestamp()} 󰜺"

    if HISTORY_GAS is None or HISTORY_GAS != dic["StatusPumpe4"]:
        HISTORY_GAS = dic["StatusPumpe4"]
        HISTORY_GAS_TIME_CHANGED = time.time()

        if dic["StatusPumpe4"]:
            HISTORY_GAS_TIME_STARTED = HISTORY_GAS_TIME_CHANGED
            t = time_to_str(HISTORY_GAS_TIME_STARTED)
            changes_logger.write(f"{t}  StatusPumpe4 = {dic["StatusPumpe4"]}\n")
        else:
            HISTORY_GAS_TIME_FINISHED = HISTORY_GAS_TIME_CHANGED
            e = "\n"
            if HISTORY_GAS_TIME_STARTED and HISTORY_GAS_TIME_FINISHED:
                changes_logger.write(
                    f" -- {elapsed_str(HISTORY_GAS_TIME_FINISHED, HISTORY_GAS_TIME_STARTED)}\n"
                )

            changes_logger.write(f"StatusPumpe4 = {dic["StatusPumpe4"]}{e}")

    if AUTO_TIMER and int(dic["mod_rada"]):
        if AUTO_TIMER_STARTED:
            AUTO_TIMER_SECONDS_ELAPSED = time.time() - HISTORY_MODE_TIME_CHANGED
            AUTO_TIMER_STATUS = f"{AUTO_TIMER_SECONDS_ELAPSED:.2f}/{AUTO_TIMER_SECONDS}"

            if AUTO_TIMER_SECONDS_ELAPSED >= AUTO_TIMER_SECONDS:
                AUTO_TIMER_STARTED = False
                AUTO_TIMER_TIME_FINISHED = time.time()
                AUTO_TIMER_STATUS = f"󱫓 {elapsed_str(AUTO_TIMER_TIME_FINISHED, AUTO_TIMER_TIME_STARTED)} 󱪯"
                send(URLS["OFF"])

        else:
            AUTO_TIMER_STARTED = True
            AUTO_TIMER_TIME_STARTED = time.time()
            AUTO_TIMER_STATUS = f"{timestamp()} 󱫌"

    if AUTO_GAS and int(dic["StatusPumpe4"]) == 0 and dic["TminLT"]:
        AUTO_GAS_TIME_STARTED = time.time()
        AUTO_GAS_STATUS = f"{timestamp()} "
        send(URLS["GAS_ON"])

    if AUTO_GAS and int(dic["StatusPumpe4"]) == 3 and dic["TmidGE"]:
        if AUTO_GAS_TIME_STARTED and AUTO_GAS_TIME_FINISHED:
            AUTO_GAS_STATUS = (
                f"󱫓 {elapsed_str(AUTO_GAS_TIME_FINISHED, AUTO_GAS_TIME_STARTED)} 󰙇"
            )
        else:
            AUTO_GAS_STATUS = f"{timestamp()} 󰙇"
        send(URLS["GAS_OFF"])


last_data = None
last_ret = False


def dowork():

    global last_data
    global last_ret
    last_ret = True

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

    dic = process_data(data, last_ret)

    # send requests based on processed data
    action(dic)

    return dic
