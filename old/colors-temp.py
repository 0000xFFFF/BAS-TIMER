#!/usr/bin/env python


def contrast_colour(colour):
    """Return a colour that contrasts with the given colour."""
    if colour < 16:
        return 15 if colour == 0 else 0

    if colour > 231:
        return 15 if colour < 244 else 0

    g = ((colour - 16) % 36) // 6
    return 0 if g > 2 else 15


def print_colour(colour, text=None):
    """Print the colour block with contrast text."""
    contrast = contrast_colour(colour)
    text = text if text is not None else colour
    print(f"\033[48;5;{colour}m\033[38;5;{contrast}m{text}\033[0m")


def temperature_to_colour(temp, min_temp=45, max_temp=60):
    """Map a temperature value to a colour."""
    colors = [51, 45, 39, 33, 27, 21, 226, 220, 214, 208, 202, 196]
    num_colors = len(colors)

    if temp >= max_temp:
        return colors[-1]
    elif temp <= min_temp:
        return colors[0]

    index = (temp - min_temp) * (num_colors - 1) // (max_temp - min_temp)
    return colors[index]


# Print temperature colours
for temp in range(45, 61):
    color = temperature_to_colour(temp)
    print_colour(color, temp)
