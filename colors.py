TEMP_MIN = 45
TEMP_MAX = 60
TEMP_COLORS = [
    51,  # COLDEST
    45,
    39,
    38,
    33,
    32,
    27,
    26,
    21,
    190,
    226,
    220,
    214,
    208,
    202,
    124,
    160,
    196,  # HOTTEST
]

APPEAR_COLORS = [
    232,  # DARKEST
    233,
    234,
    235,
    236,
    237,
    238,
    239,
    240,
    241,
    242,
    243,
    244,
    245,
    246,
    247,
    248,
    249,
    250,
    251,
    252,
    253,
    254,
    255,  # BRIGHTEST
]

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


# color text fg
def ctext_fg(color, text):
    text = text if text is not None else color
    return f"\033[38;5;{color}m{text}\033[0m"


# color text background
def ctext_bg(color, text):
    text = text if text is not None else color
    return f"\033[48;5;{color}m{text}\033[0m"


# color text ; contrast background
def ctext_fg_con(color, text):
    contrast = contrast_color(color)
    text = text if text is not None else color
    return f"\033[48;5;{contrast}m\033[38;5;{color}m{text}\033[0m"


# color bg ; contrast text
def ctext_bg_con(color, text):
    contrast = contrast_color(color)
    text = text if text is not None else color
    return f"\033[48;5;{color}m\033[38;5;{contrast}m{text}\033[0m"


def int_to_color(val, min, max, reverse_colors=False):

    colors = APPEAR_COLORS.copy()
    if reverse_colors:
        colors.reverse()

    num_colors = len(colors)

    if val >= min:
        return colors[-1]
    elif val <= max:
        return colors[0]

    index = (val - min) * (num_colors - 1) // (max - min)
    return colors[int(index)]


def int_to_ctext_fg(val, min, max, reverse_colors=False):
    color = int_to_color(val, min, max, reverse_colors)
    return ctext_fg(color, val)


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


def format_temp_color_text(temp):
    color = temperature_to_color(temp)
    pad_float = str(f"{temp:.2f}")
    pad_def = f"{pad_float} " + chr(176) + "C"
    return color, f"{pad_def:>9}"


def temp_to_ctext_fg(temp):
    color, text = format_temp_color_text(temp)
    return ctext_fg(color, text)


def temp_to_ctext_bg(temp):
    color, text = format_temp_color_text(temp)
    return ctext_bg(color, text)


def temp_to_ctext_fg_con(temp):
    color, text = format_temp_color_text(temp)
    return ctext_fg_con(color, text)


def temp_to_ctext_bg_con(temp):
    color, text = format_temp_color_text(temp)
    return ctext_bg_con(color, text)


def bctext_fg(b, text):
    if b:
        return ctext_fg(COLOR_ON, f"{text}")
    else:
        return ctext_fg(COLOR_OFF, f"{text}")


def bool_to_ctext_fg(b):
    if b:
        return ctext_fg(COLOR_ON, f" {b} ")
    else:
        return ctext_fg(COLOR_OFF, f" {b} ")


def bool_to_ctext_bg(b):
    if b:
        return ctext_bg(COLOR_ON, f" {b} ")
    else:
        return ctext_bg(COLOR_OFF, f" {b} ")


def bool_to_ctext_bg_con(b):
    if b:
        return ctext_bg_con(COLOR_ON, f" {b} ")
    else:
        return ctext_bg_con(COLOR_OFF, f" {b} ")


def bool_to_ctext_fg_con(b):
    if b:
        return ctext_fg_con(COLOR_ON, f" {b} ")
    else:
        return ctext_fg_con(COLOR_OFF, f" {b} ")


def bool_to_ctext_bi(b):
    if b:
        return ctext_bg_con(COLOR_ON, f" {b} ")
    else:
        return ctext_fg(COLOR_OFF, f" {b} ")
