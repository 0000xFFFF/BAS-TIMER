from tabulate import tabulate

from utils import timestamp, get_local_ips
from colors import temp_to_ctext_fg, bool_to_ctext_fg, bctext_fg, ctext_bg_con
import colors
import worker


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
    Tspv = temp_to_ctext_fg(dic["Tspv"])
    Tsolar = temp_to_ctext_fg(dic["Tsolar"])
    Tzadata = temp_to_ctext_fg(dic["Tzadata"])
    Tfs = temp_to_ctext_fg(dic["Tfs"])
    Tmax = temp_to_ctext_fg(dic["Tmax"])
    dic["Tmid"] = (dic["Tmax"] + dic["Tmin"]) / 2
    Tmid = temp_to_ctext_fg(dic["Tmid"])
    Tmin = temp_to_ctext_fg(dic["Tmin"])
    Tsobna = temp_to_ctext_fg(dic["Tsobna"])
    dic["Thottest"] = colors.TEMP_MAX
    Thottest = temp_to_ctext_fg(dic["Thottest"])
    dic["Tcoldest"] = colors.TEMP_MIN
    Tcoldest = temp_to_ctext_fg(dic["Tcoldest"])
    temps = []
    temps.append(["Outside 󱇜", Tspv])
    temps.append(["Solar 󱩳", Tsolar])
    temps.append(["Room ", Tsobna])
    temps.append(["Set ", Tzadata])
    temps.append(["Max ", Tmax])
    temps.append(["Mid 󰝹", Tmid])
    temps.append(["Min ", Tmin])
    temps.append(["Circ. ", Tfs])
    temps.append(["Hottest 󰈸", Thottest])
    temps.append(["Coldest ", Tcoldest])


    dic["TminLT"] = dic["Tmin"] < 45
    TminLT = bool_to_ctext_fg(int(dic["TminLT"]))
    dic["TmidGE"] = dic["Tmid"] >= 60
    TmidGE = bool_to_ctext_fg(int(dic["TmidGE"]))

    emojis = [" "] * len(temps)
    emojis[5] = bctext_fg(dic["TmidGE"], f"{"󰩐" if dic["TmidGE"] else ""}")

    StatusPumpe3 = bool_to_ctext_fg(int(dic["StatusPumpe3"]))
    StatusPumpe4 = bool_to_ctext_fg(int(dic["StatusPumpe4"]))
    StatusPumpe5 = bool_to_ctext_fg(int(dic["StatusPumpe5"]))
    StatusPumpe6 = bool_to_ctext_fg(int(dic["StatusPumpe6"]))
    StatusPumpe7 = bool_to_ctext_fg(int(dic["StatusPumpe7"]))
    ModRada = bool_to_ctext_fg(int(dic["mod_rada"]))
    ModRezim = bool_to_ctext_fg(int(dic["mod_rezim"]))
    status = []
    status.append(["Mode 󱪯", ModRada])
    status.append(["Regime 󱖫", ModRezim])
    status.append(["Heat 󱩃", StatusPumpe6])
    status.append(["Gas 󰙇", StatusPumpe4])
    status.append(["Circ. ", StatusPumpe3])
    status.append(["Pump5 ", StatusPumpe5])
    status.append(["Pump7 ", StatusPumpe7])
    status.append(["Min < 45", TminLT])
    status.append(["Mid >= 60", TmidGE])


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
    print(ctext_bg_con(COLOR_HEAD, f"{timestamp()} / {get_local_ips()}"))

    for line1, emoji, line2 in zip(table1_lines, emojis, table2_lines):
        print(f"{line1}{emoji}{line2}")

    print(f"󱎫󰐸 {int(worker.AUTO_TIMER)}/{int(worker.AUTO_TIMER_STARTED)}/{worker.AUTO_TIMER_SECONDS_LEFT} {worker.AUTO_TIMER_STATUS}")
    print(f"󰙇󱣽 {int(worker.AUTO_GAS)} {worker.AUTO_GAS_STATUS}")

    return dic
