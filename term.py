import os
import re
from utils import strip_ansi


def term_clear():
    os.system("clear")


def term_cursor_reset():
    print("\033[0;0H", end="")


def term_cursor_hide():
    print("\033[?25l")


def term_cursor_show():
    print("\033[?25h")


def term_blank():
    term_cursor_hide()
    term_clear()


def term_show(text: str):
    columns = 40  # Max column width

    clean_len = len(strip_ansi(text))  # Length without escape sequences

    # Ensure the text does not exceed columns
    if clean_len > columns:
        # Trim visible content if it's too long
        visible_chars = []
        visible_count = 0

        i = 0
        while i < len(text) and visible_count < columns:
            if text[i] == "\x1b":  # Start of an ANSI escape sequence
                esc_seq = re.match(r"\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])", text[i:])
                if esc_seq:
                    visible_chars.append(esc_seq.group())  # Keep ANSI sequences
                    i += len(esc_seq.group())  # Move index past ANSI sequence
                    continue
            else:
                visible_chars.append(text[i])
                visible_count += 1  # Count only visible characters

            i += 1

        text = "".join(visible_chars)

    # Calculate padding based on the visible length
    padding = " " * (columns - clean_len)

    print(text + padding)
