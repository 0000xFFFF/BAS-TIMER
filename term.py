import os
import re
from utils import strip_ansi


TERM_OUTPUT_LINES = ""


def term_clear():
    os.system("clear")


def term_cursor_reset():
    print("\033[0;0H", end="")
    global TERM_OUTPUT_LINES
    TERM_OUTPUT_LINES = ""


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

    output = text + padding + "\n"
    global TERM_OUTPUT_LINES
    TERM_OUTPUT_LINES += output
    print(output, end="")


class Spinner:
    def __init__(self, spinner):
        self.spinner = spinner
        self.index = 0

    def spin(self):
        self.index = (self.index + 1) % len(self.spinner)

    def get(self, also_spin=True):
        ret = self.spinner[self.index]
        if also_spin:
            self.spin()
        return ret


def ansi_to_html(text):
    def ansi_256_to_rgb(color_index):
        if color_index < 16:
            basic_colors = [
                "#000000",
                "#800000",
                "#008000",
                "#808000",
                "#000080",
                "#800080",
                "#008080",
                "#c0c0c0",
                "#808080",
                "#ff0000",
                "#00ff00",
                "#ffff00",
                "#0000ff",
                "#ff00ff",
                "#00ffff",
                "#ffffff",
            ]
            return basic_colors[color_index]
        elif 16 <= color_index <= 231:
            color_index -= 16
            r = (color_index // 36) * 51
            g = ((color_index % 36) // 6) * 51
            b = (color_index % 6) * 51
            return f"#{r:02x}{g:02x}{b:02x}"
        elif 232 <= color_index <= 255:
            shade = (color_index - 232) * 10 + 8
            return f"#{shade:02x}{shade:02x}{shade:02x}"
        return "#ffffff"

    def ansi_256_to_css(match):
        seq = match.group(1)
        parts = seq.split(";")
        color = ""
        bg_color = ""
        bold = False

        i = 0
        while i < len(parts):
            part = int(parts[i])
            if part == 1:
                bold = True
            elif part == 38 and i + 2 < len(parts) and parts[i + 1] == "5":
                color = ansi_256_to_rgb(int(parts[i + 2]))
                i += 2
            elif part == 48 and i + 2 < len(parts) and parts[i + 1] == "5":
                bg_color = ansi_256_to_rgb(int(parts[i + 2]))
                i += 2
            i += 1

        style = ""
        if color:
            style += f"color: {color}; "
        if bg_color:
            style += f"background-color: {bg_color}; "
        if bold:
            style += "font-weight: bold; "

        return f'<span style="{style}">' if style else ""

    text = text.replace("\033[0m", "</span>")  # Properly close spans
    text = re.sub(r"( )", "&nbsp;", text)  # Preserve spaces where needed
    text = re.sub(r"\033\[(.*?)m", ansi_256_to_css, text)  # Convert ANSI to spans
    text = text.replace("\n", "</span><br><span>")  # Handle newlines correctly

    return f"<span>{text}</span>"
