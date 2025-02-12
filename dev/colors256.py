#!/usr/bin/env python


#colors = [51, 45, 39, 33, 27, 21, 226, 220, 214, 208, 202, 196]
#colors_old = [51, 45, 39, 38, 33, 32, 27, 26, 21, 190, 226, 220, 214, 208, 202, 196]

#colors = [
#        51, 45, 39, 38, 33, 32, 27, 26, 21,  # Cyan to Blue
#        20, 19, 18, 17, 16, 15, 14, 13, 12,  # Blue to Darker Blue
#        190, 154, 118, 82, 46, 226, 220, 214, 208, 202, 196,  # Yellow to Orange to Red
#        160, 124, 88, 52, 16  # Red to Darker Red
#    ]

TEMP_MIN = 45
TEMP_MAX = 60
colors = [51, 45, 39, 38, 33, 32, 27, 26, 21, 190, 226, 220, 214, 208, 202, 124, 160, 196]

def contrast_color(color):
    if color < 16:
        return 15 if color == 0 else 0

    if color > 231:
        return 15 if color < 244 else 0

    g = ((color - 16) % 36) // 6
    return 0 if g > 2 else 15


def ctext(color, text):
    contrast = contrast_color(color)
    text = text if text is not None else color
    return f"\033[48;5;{color}m\033[38;5;{contrast}m{text}\033[0m"


def temperature_to_color(temp):

    """Map a temperature value to a color."""
    global TEMP_MIN
    global TEMP_MAX
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

def temp_range():
    for temp in range(45, 70):
        print(temp_to_ctext(temp))


def temp_input():
    while True:
        temp = float(input("TEMP: "))
        print(temp_to_ctext(temp))
        print(temp_to_ctext(TEMP_MAX))
        print(temp_to_ctext(TEMP_MIN))


def color_range():
    for i in colors:
        print(ctext(i, i))


color_range()


