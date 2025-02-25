import os
import sys
import logging
from datetime import datetime

# change cwd to scripts dir
os.chdir(os.path.dirname(os.path.realpath(__file__)))

DEBUG = False

# Suppress Flask logging but keep prints
if not DEBUG:
    log_file = "flask.log"
    logging.getLogger("werkzeug").setLevel(logging.ERROR)
    flask_log = open(log_file, "a")
    sys.stderr = flask_log  # Redirect errors to log file


class CustomLogger:
    def __init__(self, log_file):
        """Initialize the logger with a specific log file."""
        self.log_file = log_file

    def write(self, message):
        """Write a log message to the file. Optionally, avoid adding a newline."""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_entry = f"{timestamp} - {message}"
        with open(self.log_file, "a") as f:
            f.write(log_entry)
            f.flush()


# Create separate loggers
requests_logger = CustomLogger("requests.log")
changes_logger = CustomLogger("changes.log")
debug_logger = CustomLogger("debug.log")
