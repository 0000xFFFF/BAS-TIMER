from tabulate import tabulate

from utils import timestamp, get_local_ips
from colors import (
    COLOR_ON,
    COLOR_OFF,
    temp_to_ctext_bg_con,
    bool_to_ctext_bi,
    bctext_fg,
    ctext_fg,
    int_to_ctext_fg,
)
import colors
import reqworker


def process_data(data, last_ret):
    # Collect key-value pairs
    dic = {}

    try:
        for key, value in data.items():
            if isinstance(value, dict) and "value" in value:
                dic[key] = value["value"]
    except Exception:
        return dic

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
    temps.append([ctext_fg(40, "Room "), Tsobna])
    temps.append([ctext_fg(154, "Set "), Tzadata])
    temps.append([ctext_fg(214, "Max "), Tmax])
    temps.append([ctext_fg(220, "Mid 󰝹"), Tmid])
    temps.append([ctext_fg(226, "Min "), Tmin])
    temps.append([ctext_fg(110 , "Circ. "), Tfs])
    temps.append([ctext_fg(196 , "Hottest 󰈸"), Thottest])
    temps.append([ctext_fg(51 , "Coldest "), Tcoldest])

    dic["TminLT"] = dic["Tmin"] < 45
    TminLT = bool_to_ctext_bi(int(dic["TminLT"]))
    dic["TmidGE"] = dic["Tmid"] >= 60
    TmidGE = bool_to_ctext_bi(int(dic["TmidGE"]))

    emojis = ["   "] * len(temps)
    if dic["TmidGE"]:
        emojis[5] = ctext_fg(COLOR_ON, "  ")
    if dic["TminLT"]:
        emojis[4] = ctext_fg(COLOR_OFF, "  ")

    StatusPumpe3 = bool_to_ctext_bi(int(dic["StatusPumpe3"]))
    StatusPumpe4 = bool_to_ctext_bi(int(dic["StatusPumpe4"]))
    StatusPumpe5 = bool_to_ctext_bi(int(dic["StatusPumpe5"]))
    StatusPumpe6 = bool_to_ctext_bi(int(dic["StatusPumpe6"]))
    StatusPumpe7 = bool_to_ctext_bi(int(dic["StatusPumpe7"]))
    ModRada = bool_to_ctext_bi(int(dic["mod_rada"]))
    ModRezim = bool_to_ctext_bi(int(dic["mod_rezim"]))
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
        emojis2[0] = ctext_fg(COLOR_ON, f"󱣽") + bctext_fg(reqworker.AUTO_TIMER_STARTED, f"󱎫")
    
    if reqworker.AUTO_GAS:
        emojis2[3] = ctext_fg(COLOR_ON, f"󱣽")

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

    COLOR_HEAD = colors.COLOR_ON if last_ret else colors.COLOR_OFF
    print(ctext_fg(COLOR_HEAD, f"{timestamp()} / {get_local_ips()}"))

    for line1, emoji1, line2, emoji2 in zip(table1_lines, emojis, table2_lines, emojis2):
        print(f"{line1}{emoji1}{line2}{emoji2}")

    ts = reqworker.AUTO_TIMER_STATUS
    if ts:
        te = ctext_fg(COLOR_ON, f"󱎫󰐸")
        tt = int_to_ctext_fg(
            reqworker.AUTO_TIMER_SECONDS_LEFT,
            0,
            reqworker.AUTO_TIMER_SECONDS,
            reverse_colors=True
        )
        print(f"{te} {tt} {ts}")

    gs = reqworker.AUTO_GAS_STATUS
    if gs:
        ge = ctext_fg(COLOR_ON, f"󱣿󰙇")
        print(f"󱣿󰙇 {ge} {gs}")


    return dic
