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
from flask import Flask, render_template, jsonify
from flask_socketio import SocketIO


from term import term_cursor_hide, term_cursor_reset, term_cursor_show, term_clear
import reqworker

# change cwd to scripts dir
os.chdir(os.path.dirname(os.path.realpath(__file__)))

DEBUG = False

# Suppress Flask logging but keep prints
if not DEBUG:
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
    static_url_path="",
    static_folder=os.path.join(os.path.dirname(os.path.realpath(__file__)), "static"),
    template_folder=os.path.join(
        os.path.dirname(os.path.realpath(__file__)), "templates"
    ),
)
socketio = SocketIO(app, cors_allowed_origins="*")


@app.route("/")
def index():
    return render_template(
        "index.html", auto_timer=reqworker.AUTO_TIMER, auto_gas=reqworker.AUTO_GAS
    )


@app.route("/toggle_autotimer", methods=["POST"])
def toggle_autotimer():
    reqworker.AUTO_TIMER = not reqworker.AUTO_TIMER  # Toggle value
    return jsonify({"auto_timer": reqworker.AUTO_TIMER})


@app.route("/toggle_autogas", methods=["POST"])
def toggle_autogas():
    reqworker.AUTO_GAS = not reqworker.AUTO_GAS  # Toggle value
    return jsonify({"auto_gas": reqworker.AUTO_GAS})


def main_worker():
    global running
    if not DEBUG:
        term_cursor_hide()
        term_clear()

    with requests.Session() as main_session, open("requests.log", "a") as log_requests:
        while running:
            if not DEBUG:
                term_cursor_reset()
            dic = reqworker.dowork(main_session, log_requests)

            # send data to frontend
            socketio.emit("vars", dic)

            time.sleep(3)


if __name__ == "__main__":

    t = Thread(target=main_worker)
    t.daemon = True
    t.start()

    try:
        socketio.run(app, debug=True, host="0.0.0.0", port=5000, use_reloader=False)
    except KeyboardInterrupt:
        print("Quitting...")
        running = False
        t.join()
