import requests
import time

from utils import timestamp
from status import process_data

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
req_url = "http://192.168.1.250/isc/get_var_js.aspx"
req_params = {
    "StatusPumpe3": "",
    "StatusPumpe4": "",
    "StatusPumpe5": "",
    "StatusPumpe6": "",
    "StatusPumpe7": "",
    "Taktualno": "",
    "Tfs": "",
    "Tmax": "",
    "Tmin": "",
    "Tsobna": "",
    "Tsolar": "",
    "Tspv": "",
    "Tzad_komf": "",
    "Tzad_mraz": "",
    "Tzad_red": "",
    "Tzadata": "",
    "mod_rada": "",
    "mod_rezim": "",
    "__Time": "",
    "__Date": "",
    "Jeftina_tarifa": "",
    "grejanje_off": "",
    "Alarm_tank": "",
    "Alarm_solar": "",
    "STATE_Preklopka": "",
    "SESSIONID": "-1",
    "_": int(time.time() * 1000),  # Generate 13-digit millisecond timestamp
}


# Create a prepared request
def get_prepared(session):
    req_params["_"] += 1
    request = requests.Request("GET", req_url, headers=req_headers, params=req_params)
    prepared = session.prepare_request(request)
    return prepared


def prepared_url(prepared):
    return f"{prepared.method} {prepared.url}"


def prepared_req(prepared):
    return f"{prepared_req(prepared)} HTTP/1.1"


def print_prepared(prepared):
    print(prepared_req(prepared))
    for key, value in prepared.headers.items():
        print(f"{key}: {value}")

    if prepared.body:
        print(
            "\nRaw Body Bytes:",
            prepared.body.encode() if isinstance(prepared.body, str) else prepared.body,
        )


def fetch_info(main_session, last_data, last_ret, log_requests):

    ret = True

    # Send GET request
    try:
        prepared = get_prepared(main_session)
        # print_prepared(prepared)
        log_requests.write(f"[{timestamp()}] {prepared_url(prepared)} --> ")
        log_requests.flush()
        response = main_session.send(prepared)
        response = requests.get(req_url, headers=req_headers, params=req_params)
        response.raise_for_status()  # Raise an error for HTTP error codes
        data = response.json()
        last_data = data
        log_requests.write(f"[{timestamp()}] SUCCESS\n")
        log_requests.flush()
    except Exception as e:
        ret = False
        log_requests.write(f"[{timestamp()}] FAILED ({e})\n")
        log_requests.flush()
        data = last_data
        if last_data is None:
            print(f"[{timestamp()}] {e.__class__.__name__}")
            return ret, None

    dic = process_data(data, ret)

    return ret, data, dic
