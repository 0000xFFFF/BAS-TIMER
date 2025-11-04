#!/bin/bash
mkdir -p ~/.config
mkdir -p ~/.config/i3
mkdir -p ~/.local/share
ln -sfr .xinitrc ~/.xinitrc
ln -sfr .bashrc_start ~/.bashrc_start
ln -sfr .config/i3/config ~/.config/i3/config
ln -sfr .config/alacritty ~/.config/.
ln -sfr .config/konsolerc ~/.config/.
ln -sfr .local/share/konsole ~/.local/share/.
ln -sfr update.sh ~/.
