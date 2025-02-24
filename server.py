#!/usr/bin/env python

import eventlet

eventlet.monkey_patch()

import os
import sys
import requests
import time
import atexit
from threading import Thread

import random
from flask import Flask, render_template, jsonify, request, Response
from flask_socketio import SocketIO


from term import term_cursor_hide, term_cursor_reset, term_cursor_show, term_clear
import reqworker


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

main_session = requests.Session()


@app.route("/")
def index():
    return render_template(
        "index.html", auto_timer=reqworker.AUTO_TIMER, auto_gas=reqworker.AUTO_GAS
    )


@app.route("/requests")
def requests():
    try:
        with open("requests.log", "r") as f:
            content = f.read()
        return Response(content, mimetype="text/plain")
    except FileNotFoundError:
        return "Log file not found.", 404


@app.route("/changes")
def changes():
    try:
        with open("changes.log", "r") as f:
            content = f.read()
        return Response(content, mimetype="text/plain")
    except FileNotFoundError:
        return "Log file not found.", 404


@app.route("/toggle_autotimer", methods=["POST"])
def toggle_autotimer():
    reqworker.AUTO_TIMER = not reqworker.AUTO_TIMER  # Toggle value
    return jsonify({"auto_timer": reqworker.AUTO_TIMER})


@app.route("/toggle_autogas", methods=["POST"])
def toggle_autogas():
    reqworker.AUTO_GAS = not reqworker.AUTO_GAS  # Toggle value
    return jsonify({"auto_gas": reqworker.AUTO_GAS})


@app.route("/get_timer_seconds", methods=["GET"])
def get_timer_seconds():
    return jsonify({"AUTO_TIMER_SECONDS": reqworker.AUTO_TIMER_SECONDS})


@app.route("/set_timer_seconds", methods=["POST"])
def set_timer_seconds():
    data = request.get_json()
    try:
        reqworker.AUTO_TIMER_SECONDS = int(data["seconds"])
        reqworker.AUTO_TIMER_STATUS = f"changed to: {reqworker.AUTO_TIMER_SECONDS}"
        return jsonify(
            {"success": True, "AUTO_TIMER_SECONDS": reqworker.AUTO_TIMER_SECONDS}
        )
    except (KeyError, ValueError):
        return jsonify({"success": False, "error": "Invalid input"}), 400


MAIN_WORKER_DRAW_SLEEP = 0.25
def main_worker():
    global running
    term_cursor_hide()
    term_clear()

    while running:
        term_cursor_reset()
        dic = reqworker.do_work()

        # send data to frontend
        socketio.emit("vars", dic)

        time.sleep(MAIN_WORKER_DRAW_SLEEP)


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
