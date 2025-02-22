#!/usr/bin/env python
import os
import sys
import requests
import time
import logging
import atexit
from threading import Thread

import eventlet

eventlet.monkey_patch()
import random
from flask import Flask, render_template
from flask_socketio import SocketIO


from term import term_cursor_hide, term_cursor_reset, term_cursor_show, term_clear
from req import fetch_info

# Suppress Flask logging but keep prints
log_file = "flask.log"
logging.getLogger("werkzeug").setLevel(logging.ERROR)
flask_log = open(log_file, "a")
sys.stderr = flask_log  # Redirect errors to log file


# exit handler
@atexit.register
def signal_handler():
    term_cursor_show()


running = True
app = Flask(
    __name__,
    static_url_path='',
    static_folder=os.path.join(
        os.path.dirname(os.path.realpath(__file__)), "static"
    ),
    template_folder=os.path.join(
        os.path.dirname(os.path.realpath(__file__)), "templates"
    ),
)
socketio = SocketIO(app, cors_allowed_origins="*")

@app.route("/")
def index():
    return render_template("index.html")


def worker():
    global running
    term_cursor_hide()
    term_clear()

    main_session = requests.Session()
    log_requests = open("requests.log", "a")

    while running:
        term_cursor_reset()
        dic = fetch_info(main_session, log_requests)

        # send data to frontend
        socketio.emit("vars", dic)

        time.sleep(1)

    log_requests.close()


if __name__ == "__main__":

    t = Thread(target=worker)
    t.daemon = True
    t.start()

    try:
        socketio.run(app, debug=True, host="0.0.0.0", port=5000)
    except KeyboardInterrupt:
        print("Quitting...")
        running = False
        t.join()
