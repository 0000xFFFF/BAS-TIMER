#!/bin/bash
pkill konsole
cd ~/BAS-TIMER
git pull
make testing
DISPLAY=:0 i3-msg restart
