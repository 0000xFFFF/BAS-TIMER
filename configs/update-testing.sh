#!/usr/bin/env bash
set -x
pkill konsole
cd ~/BAS-TIMER
# rm var/infos.bin
git pull
make testing
sudo make install
DISPLAY=:0 i3-msg restart
