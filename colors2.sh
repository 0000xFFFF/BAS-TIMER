#!/bin/bash

for i in $(seq 0 1 20); do
    r=$((255 - i * 12))
    g=0
    b=$((i * 12))
    
    color=$((16 + (r / 51) * 36 + (g / 51) * 6 + (b / 51)))
    printf "\e[48;5;%sm===\e[0m\n" "$color"
done

