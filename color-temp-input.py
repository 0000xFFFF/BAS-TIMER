#!/usr/bin/env python

TEMP_MIN = 45
TEMP_MAX = 60


def contrast_color(color):
    """Return a color that contrasts with the given color."""
    if color < 16:
        return 15 if color == 0 else 0

    if color > 231:
        return 15 if color < 244 else 0

    g = ((color - 16) % 36) // 6
    return 0 if g > 2 else 15


def ctext(color, text):
    """Print the color block with contrast text."""
    contrast = contrast_color(color)
    text = text if text is not None else color
    return f"\033[48;5;{color}m\033[38;5;{contrast}m{text}\033[0m"


def temperature_to_color(temp):
    """Map a temperature value to a color."""
    global TEMP_MIN
    global TEMP_MAX
    colors = [51, 45, 39, 33, 27, 21, 226, 220, 214, 208, 202, 196]
    num_colors = len(colors)

    if temp >= TEMP_MAX:
        TEMP_MAX = temp
        return colors[-1]
    elif temp <= TEMP_MIN:
        TEMP_MIN = temp
        return colors[0]

    index = (temp - TEMP_MIN) * (num_colors - 1) // (TEMP_MAX - TEMP_MIN)
    return colors[int(index)]


def temp_to_ctext(temp):
    color = temperature_to_color(temp)
    return ctext(color, temp)


while True:
    temp = float(input("TEMP: "))
    print(temp_to_ctext(temp))
    print(temp_to_ctext(TEMP_MAX))
    print(temp_to_ctext(TEMP_MIN))
