#!/bin/bash
pkill konsole
cd ~/BAS-TIMER
rm var/infos.bin
git pull
make testing
DISPLAY=:0 i3-msg restart
