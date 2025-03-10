#!/bin/bash -x
mkdir -p ~/.config/i3
cp configs/i3config ~/.config/i3/config
mkdir -p ~/.config/alacritty
cp configs/alacritty.yml ~/.config/alacritty/alacritty.yml
mkdir -p ~/.local/share/fonts
ln -sfr static/fonts/Gohu ~/.local/share/fonts/Gohu
fc-cache -r
