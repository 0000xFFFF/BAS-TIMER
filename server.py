#!/usr/bin/env python
import requests
import time
import atexit

from term import term_cursor_hide, term_cursor_reset, term_cursor_show, term_clear
from req import fetch_info


# exit handler
@atexit.register
def signal_handler():
    term_cursor_show()


if __name__ == "__main__":

    term_cursor_hide()
    term_clear()

    main_session = requests.Session()
    log_requests = open("requests.log", "a")

    running = True
    last_ret = False
    last_data = None

    try:
        while running:
            term_cursor_reset()
            last_ret, last_data = fetch_info(
                main_session, last_ret, last_data, log_requests
            )
            time.sleep(1)
    except KeyboardInterrupt:
        print("Quitting...")
        running = False

    log_requests.close()
