TEMP_MIN = 45
TEMP_MAX = 60
TEMP_COLORS = [51, 45, 39, 38, 33, 32, 27, 26, 21, 190, 226, 220, 214, 208, 202, 124, 160, 196]

COLOR_ON = 40
COLOR_OFF = 226

def contrast_color(color):
    if color < 16:
        return 15 if color == 0 else 0

    if color > 231:
        return 15 if color < 244 else 0

    g = ((color - 16) % 36) // 6
    return 0 if g > 2 else 15


def cbg(color):
    return f"\033[48;5;{color}m"


def ctext(color, text):
    contrast = contrast_color(color)
    text = text if text is not None else color
    return f"\033[48;5;{color}m\033[38;5;{contrast}m{text}\033[0m"


def temperature_to_color(temp):
    global TEMP_MIN
    global TEMP_MAX
    num_colors = len(TEMP_COLORS)

    if temp >= TEMP_MAX:
        TEMP_MAX = temp
        return TEMP_COLORS[-1]
    elif temp <= TEMP_MIN:
        TEMP_MIN = temp
        return TEMP_COLORS[0]

    index = (temp - TEMP_MIN) * (num_colors - 1) // (TEMP_MAX - TEMP_MIN)
    return TEMP_COLORS[int(index)]


def temp_to_ctext(temp):
    color = temperature_to_color(temp)
    pad_float = str(f"{temp:.2f}")
    pad_def = f"{pad_float} " + chr(176) + "C"
    return ctext(color, f"{pad_def:>9}")


def bool_to_ctext(b):
    if b:
        return ctext(COLOR_ON, f" {b} ")
    else:
        return ctext(COLOR_OFF, f" {b} ")
