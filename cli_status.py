#!/usr/bin/env python
import requests
import json
import time

# Define the URL
url = "http://192.168.1.250:9001/isc/get_var_js.aspx?AUTOT=&Alarm_solar=&Alarm_tank=&EK=&GK=&Jeftina_tarifa=&MANUALPreklopka=&STATE_Preklopka=&Status3PTReg3=&StatusPumpe3=&StatusPumpe4=&StatusPumpe5=&StatusPumpe6=&StatusPumpe7=&TP=&Taktualno=&Tfs=&Tmax=&Tmin=&Tsobna=&Tsolar=&Tspv=&Tzad_komf=&Tzad_mraz=&Tzad_red=&Tzadata=&__Date=&__Time=&deltaT=&glavni_on=&grejanje_off=&kom=&kom1=&kom2=&mod_rada=&mod_rezim=&mraz=&mraz1=&mraz2=&red=&red1=&red2=&sw1=&sw2=&sw3=&sw4=&rucni_komf=&rucni_red=&SESSIONID=-1"


running = True;

while running:
    try:
        # Send GET request
        response = requests.get(url)
        response.raise_for_status()  # Raise an error for HTTP error codes
        
        # Parse JSON response
        data = response.json()
        
        # Loop through and print key-value pairs
        for key, value in data.items():
            if isinstance(value, dict) and "value" in value:
                print(f"{key}: {value['value']}")

    except requests.exceptions.RequestException as e:
        print(f"Error fetching data: {e}")
    except json.JSONDecodeError:
        print("Error decoding JSON response")

    time.sleep(1)

