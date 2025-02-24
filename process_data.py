from tabulate import tabulate
from term import term_show, Spinner
from utils import timestamp, get_local_ips, time_to_str
from colors import (
    COLOR_ON,
    COLOR_OFF,
    temp_to_ctext_bg_con,
    bctext_fg,
    ctext_fg,
    ctext_fg_con,
    ctext_bg_con,
)
import colors
import reqworker


spinner_clock = Spinner(
    ["🕛", "🕐", "🕑", "🕒", "🕓", "🕔", "🕕", "🕖", "🕗", "🕘", "🕙", "🕚"]
)
spinner_bars = Spinner(
    ["▁", "▂", "▃", "▄", "▅", "▆", "▇", "█", "▇", "▆", "▅", "▄", "▃", "▂", "▁"]
)
spinner_basic = Spinner(["-", "\\", "|", "/"])
spinner_lights = Spinner(["󱩎", "󱩏", "󱩐", "󱩑", "󱩒", "󱩓", "󱩔", "󱩕", "󱩖", "󰛨"])
spinner_check = Spinner(["", "", "󰄬", "", "", "󰄭", "󰸞", "󰡕"])
spinner_warn = Spinner(["", ""])
spinner_heat = Spinner(["󰐸", "󰫗"])
spinner_eye_left = Spinner(["󰛐", "󱣾"])
spinner_eye_right = Spinner(["󰛐", "󱤀"])
spinner_circle = Spinner(["󰪞", "󰪟", "󰪠", "󰪡", "󰪢", "󰪣", "󰪤", "󰪥"])


def draw_pump_bars(b):
    if b:
        return ctext_fg_con(COLOR_ON, spinner_bars.get(False))
    else:
        return ctext_fg(COLOR_OFF, "")


def bool_to_check(b):
    if b:
        return ctext_bg_con(COLOR_ON, "")
    else:
        return ctext_fg(COLOR_OFF, "")


def drawui(data, last_ret, is_request=False):
    # Collect key-value pairs
    dic = {}

    try:
        for key, value in data.items():
            if isinstance(value, dict) and "value" in value:
                dic[key] = value["value"]
    except Exception:
        return dic

    # fix main values to int
    dic["mod_rada"] = int(dic["mod_rada"])
    dic["StatusPumpe4"] = int(dic["StatusPumpe4"])

    # get temps and pump status
    Tspv = temp_to_ctext_bg_con(dic["Tspv"])
    Tsolar = temp_to_ctext_bg_con(dic["Tsolar"])
    Tzadata = temp_to_ctext_bg_con(dic["Tzadata"])
    Tfs = temp_to_ctext_bg_con(dic["Tfs"])
    Tmax = temp_to_ctext_bg_con(dic["Tmax"])
    dic["Tmid"] = (dic["Tmax"] + dic["Tmin"]) / 2
    Tmid = temp_to_ctext_bg_con(dic["Tmid"])
    Tmin = temp_to_ctext_bg_con(dic["Tmin"])
    Tsobna = temp_to_ctext_bg_con(dic["Tsobna"])
    dic["Thottest"] = colors.TEMP_MAX
    Thottest = temp_to_ctext_bg_con(dic["Thottest"])
    dic["Tcoldest"] = colors.TEMP_MIN
    Tcoldest = temp_to_ctext_bg_con(dic["Tcoldest"])
    temps = []
    temps.append([ctext_fg(213, "Outside 󱇜"), Tspv])
    temps.append([ctext_fg(230, "Solar 󱩳"), Tsolar])
    temps.append([ctext_fg(76, "Room "), Tsobna])
    temps.append([ctext_fg(154, "Set "), Tzadata])
    temps.append([ctext_fg(214, "Max "), Tmax])
    temps.append([ctext_fg(220, "Mid "), Tmid])
    temps.append([ctext_fg(226, "Min "), Tmin])
    temps.append([ctext_fg(110, "Circ. "), Tfs])
    temps.append([ctext_fg(196, "Hottest 󰈸"), Thottest])
    temps.append([ctext_fg(51, "Coldest "), Tcoldest])

    dic["TminLT"] = int(dic["Tmin"] < 45)
    TminLT = bool_to_check(int(dic["TminLT"]))
    dic["TmidGE"] = int(dic["Tmid"] >= 60)
    TmidGE = bool_to_check(int(dic["TmidGE"]))

    emojis = ["   "] * len(temps)
    if dic["TmidGE"]:
        emojis[5] = ctext_fg(82, f" {spinner_check.get()} ")
    if dic["TminLT"]:
        emojis[6] = ctext_fg(196, f" {spinner_warn.get()} ")

    ModRada = bool_to_check(int(dic["mod_rada"]))
    ModRezim = ctext_fg(22, int(dic["mod_rezim"]))
    StatusPumpe3 = draw_pump_bars(int(dic["StatusPumpe3"]))
    StatusPumpe4 = draw_pump_bars(int(dic["StatusPumpe4"]))
    StatusPumpe5 = draw_pump_bars(int(dic["StatusPumpe5"]))
    StatusPumpe6 = draw_pump_bars(int(dic["StatusPumpe6"]))
    StatusPumpe7 = draw_pump_bars(int(dic["StatusPumpe7"]))
    spinner_bars.spin()
    status = []
    status.append([ctext_fg(13, "Mode 󱪯"), ModRada])
    status.append([ctext_fg(22, "Regime 󱖫"), ModRezim])
    status.append([ctext_fg(118, "Heat 󱩃"), StatusPumpe6])
    status.append([ctext_fg(112, "Gas 󰙇"), StatusPumpe4])
    status.append([ctext_fg(168, "Circ. "), StatusPumpe3])
    status.append([ctext_fg(242, "Pump5 "), StatusPumpe5])
    status.append([ctext_fg(242, "Pump7 "), StatusPumpe7])
    status.append([ctext_fg(87, "Min < 45"), TminLT])
    status.append([ctext_fg(208, "Mid >= 60"), TmidGE])

    emojis2 = ["   "] * len(temps)
    if reqworker.AUTO_TIMER:
        eye = ctext_fg(COLOR_ON, spinner_eye_left.get(False))
        clock = ctext_fg(COLOR_OFF, "󱎫")
        if reqworker.AUTO_TIMER_STARTED:
            clock = ctext_fg(COLOR_ON, spinner_clock.get())
        emojis2[0] = f" {eye}{clock}"
    if reqworker.AUTO_GAS:
        eye = ctext_fg(COLOR_ON, spinner_eye_left.get(False))
        emojis2[3] = f" {eye}"

    spinner_eye_left.spin()

    # format tables
    fmt = "plain"
    table1_str = tabulate(temps, tablefmt=fmt, colalign=("right",))
    table2_str = tabulate(status, tablefmt=fmt, colalign=("right",))
    table1_lines = table1_str.splitlines()
    table2_lines = table2_str.splitlines()
    max_lines = max(len(table1_lines), len(table2_lines))
    while len(table1_lines) < max_lines:
        table1_lines.append("")
    while len(table2_lines) < max_lines:
        table2_lines.append("")

    s = ctext_fg(228, f"{spinner_basic.get()}")
    l = ctext_fg(228, f"{spinner_lights.get()}")

    r = " "
    if is_request:
        r = ctext_fg(211, "")

    COLOR_HEAD = colors.COLOR_ON if last_ret else colors.COLOR_OFF
    term_show(
        f"{s}{l}{r} " + ctext_fg(COLOR_HEAD, f"{timestamp()} {spinner_circle.get()} {get_local_ips()}")
    )

    for line1, emoji1, line2, emoji2 in zip(
        table1_lines, emojis, table2_lines, emojis2
    ):
        term_show(f"{line1}{emoji1}{line2}{emoji2}")

    ts = reqworker.AUTO_TIMER_STATUS
    if ts:
        te = ctext_fg(COLOR_ON, f"{spinner_clock.get()}{spinner_heat.get()}")
        term_show(f"{te} {ts}")

    gs = reqworker.AUTO_GAS_STATUS
    if gs:
        ge = ctext_fg(COLOR_ON, f"{spinner_eye_right.get()}󰙇")
        term_show(f"{ge} {gs}")

    return dic
