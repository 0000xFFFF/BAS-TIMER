import requests
import time

from utils import timestamp
from process_data import process_data

GLOBAL_UNIX_COUNTER = int(time.time() * 1000)

AUTO_TIMER = True
AUTO_TIMER_STARTED = False
AUTO_TIMER_SECONDS = 3
AUTO_TIMER_SECONDS_LEFT = AUTO_TIMER_SECONDS
AUTO_TIMER_STATUS = ""
AUTO_TIMER_TIME_STARTED = time.time()
AUTO_TIMER_TIME_FINISHED = time.time()
AUTO_GAS = True
AUTO_GAS_STATUS = ""

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


def prepare(session, url):
    global GLOBAL_UNIX_COUNTER
    GLOBAL_UNIX_COUNTER += 1
    url_with_timestamp = f"{url}&_={GLOBAL_UNIX_COUNTER}"
    request = requests.Request("GET", url_with_timestamp, headers=req_headers)
    prepared = session.prepare_request(request)
    return prepared


def prepared_url(prepared):
    return f"{prepared.method} {prepared.url}"


def send(session, log_requests, url):
    prepared = prepare(session, url)
    log_requests.write(f"[{timestamp()}] {prepared_url(prepared)} --> ")
    log_requests.flush()

    try:
        response = session.send(prepared)
        response.raise_for_status()  # Raise an error for HTTP error codes
        log_requests.write(f"[{timestamp()}] SUCCESS\n")
        log_requests.flush()
    except Exception as e:
        log_requests.write(f"[{timestamp()}] FAILED ({e})\n")
        log_requests.flush()
        return None, e

    return response, None


def action(session, log_requests, dic):
    global AUTO_TIMER
    global AUTO_TIMER_STARTED
    global AUTO_TIMER_SECONDS
    global AUTO_TIMER_SECONDS_LEFT
    global AUTO_TIMER_STATUS
    global AUTO_TIMER_TIME_STARTED
    global AUTO_TIMER_TIME_FINISHED
    global AUTO_GAS
    global AUTO_GAS_STATUS

    if AUTO_TIMER and int(dic["mod_rada"]):
        if AUTO_TIMER_STARTED:
            AUTO_TIMER_SECONDS_LEFT -= 1
            if AUTO_TIMER_SECONDS_LEFT <= 0:
                AUTO_TIMER_TIME_FINISHED = time.time()
                elapsed_time = AUTO_TIMER_TIME_FINISHED - AUTO_TIMER_TIME_STARTED
                formatted_time = time.strftime("%H:%M:%S", time.gmtime(elapsed_time))
                AUTO_TIMER_STATUS = f"{timestamp()} took: {formatted_time}"
                send(session, log_requests, URLS["OFF"])

        else:
            AUTO_TIMER_STARTED = True
            AUTO_TIMER_SECONDS_LEFT = AUTO_TIMER_SECONDS
            AUTO_TIMER_TIME_STARTED = time.time()
            AUTO_TIMER_STATUS = f"{timestamp()} started"

    if AUTO_GAS and int(dic["StatusPumpe4"]) == 0 and dic["TminLT"]:
        AUTO_GAS_STATUS = f"{timestamp()} ON"
        send(session, log_requests, URLS["GAS_ON"])

    if AUTO_GAS and int(dic["StatusPumpe4"]) == 3 and dic["TmidGE"]:
        AUTO_GAS_STATUS = f"{timestamp()} OFF"
        send(session, log_requests, URLS["GAS_OFF"])


last_data = None
last_ret = False


def worker(session, log_requests):

    global last_data
    global last_ret
    last_ret = True

    # Send GET request
    response, err = send(session, log_requests, URLS["VARS"])

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
    action(session, log_requests, dic)

    return dic
