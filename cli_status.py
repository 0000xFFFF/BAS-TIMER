#!/usr/bin/env python
import requests
import json
import time
import os
from tabulate import tabulate
from colorama import Fore, Back, Style

# Define the URL
#url = "http://192.168.1.250:9001/isc/get_var_js.aspx?AUTOT=&Alarm_solar=&Alarm_tank=&EK=&GK=&Jeftina_tarifa=&MANUALPreklopka=&STATE_Preklopka=&Status3PTReg3=&StatusPumpe3=&StatusPumpe4=&StatusPumpe5=&StatusPumpe6=&StatusPumpe7=&TP=&Taktualno=&Tfs=&Tmax=&Tmin=&Tsobna=&Tsolar=&Tspv=&Tzad_komf=&Tzad_mraz=&Tzad_red=&Tzadata=&__Date=&__Time=&deltaT=&glavni_on=&grejanje_off=&kom=&kom1=&kom2=&mod_rada=&mod_rezim=&mraz=&mraz1=&mraz2=&red=&red1=&red2=&sw1=&sw2=&sw3=&sw4=&rucni_komf=&rucni_red=&SESSIONID=-1"
url = "http://192.168.1.250:9001/isc/get_var_js.aspx?StatusPumpe3=&StatusPumpe4=&StatusPumpe5=&StatusPumpe6=&StatusPumpe7=&Taktualno=&Tfs=&Tmax=&Tmin=&Tsobna=&Tsolar=&Tspv=&Tzad_komf=&Tzad_mraz=&Tzad_red=&Tzadata=&mod_rada=&mod_rezim=&SESSIONID=-1"

running = True

term_show_count = 0
g_tc = os.get_terminal_size().columns
g_tl = os.get_terminal_size().lines
def term_show_count_reset():
    global term_show_count
    term_show_count = 0
def term_clear():
    os.system("clear")
    term_show_count_reset()
def term_cursor_reset():
    print("\033[0;0H", end='')
    term_show_count_reset()
def term_cursor_hide():
    print("\033[?25l")
def term_cursor_show():
    print("\033[?25h")
def term_blank():
    term_cursor_hide()
    term_clear()
def term_show_print(printStr, color = ""):
    global term_show_count
    tl = os.get_terminal_size().lines
    if term_show_count >= (tl-1): return
    print(f"{color}{printStr}{Style.RESET_ALL}")
    term_show_count += 1
def term_show(s, color = ""):
    global g_tc
    global g_tl
    strings = s.split("\n")
    tc = os.get_terminal_size().columns
    tl = os.get_terminal_size().lines
    if g_tc != tc or g_tl != tl:
        g_tc = tc
        g_tl = tl
        term_clear()
    for i in strings:
        il = len(i)
        if il > tc: term_show_print(i[:tc], color)
        else:       term_show_print(i + str(" " * int(int(tc)-int(il))), color)

term_clear()

while running:
    term_cursor_reset()

    try:
        # Send GET request
        response = requests.get(url)
        response.raise_for_status()  # Raise an error for HTTP error codes
        
        data = response.json()

        # Collect key-value pairs
        dic = {}
        for key, value in data.items():
            if isinstance(value, dict) and "value" in value:
                dic[key] = value['value']
        
        output = []

        output.append(["Tspv",         dic["Tspv"]])
        output.append(["Tsolar",       dic["Tsolar"]])
        output.append(["Taktualno",    dic["Taktualno"]])
        output.append(["Tzadata",      dic["Tzadata"]])
        output.append(["Tzad_red",     dic["Tzad_red"]])
        output.append(["Tzad_komf",    dic["Tzad_komf"]])
        output.append(["Tfs",          dic["Tfs"]])
        output.append(["Tmax",         dic["Tmax"]])
        output.append(["Tmid",         (dic["Tmax"]+dic["Tmin"])/2])
        output.append(["Tmin",         dic["Tmin"]])
        output.append(["Tsobna",       dic["Tsobna"]])
        output.append(["StatusPumpe3", dic["StatusPumpe3"]])
        output.append(["StatusPumpe4", dic["StatusPumpe4"]])
        output.append(["StatusPumpe5", dic["StatusPumpe5"]])
        output.append(["StatusPumpe6", dic["StatusPumpe6"]])
        output.append(["StatusPumpe7", dic["StatusPumpe7"]])
        output.append(["mod_rada",     dic["mod_rada"]])
        output.append(["mod_rezim",    dic["mod_rezim"]])

        # Print output as a table
        term_show(tabulate(output, headers=["Variable", "Value"]))

    except requests.exceptions.RequestException as e:
        print(f"Error fetching data: {e}")
    except json.JSONDecodeError:
        print("Error decoding JSON response")

    time.sleep(1)
