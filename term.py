import os


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
