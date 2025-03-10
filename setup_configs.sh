#!/bin/bash
mkdir -p ~/.config/i3
cp configs/i3config ~/.config/i3/config
mkdir -p ~/.config/alacritty
cp configs/alacritty.yml ~/.config/alacritty/alacritty.yml
ln -sfr static/fonts/Gohu ~/.local/share/fonts/Gohu
fc-cache -r
