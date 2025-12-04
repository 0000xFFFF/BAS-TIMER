#!/usr/bin/env bash
set -x
pkill konsole
cd ~/BAS-TIMER
git pull
make release
sudo make install
DISPLAY=:0 i3-msg restart
