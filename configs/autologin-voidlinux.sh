#!/usr/bin/env bash

set -e

USER_NAME="$(whoami)"
SRC_SERVICE="/etc/sv/agetty-tty1"
DST_SERVICE="/etc/sv/agetty-autologin-tty1"
CONF_FILE="$DST_SERVICE/conf"
ACTIVE_SERVICE="/var/service/agetty-tty1"
ACTIVE_LINK="/var/service/agetty-autologin-tty1"

echo "Enabling tty1 autologin for user: $USER_NAME"

# Copy default service if it doesn't already exist
if [ ! -d "$DST_SERVICE" ]; then
  sudo cp -R "$SRC_SERVICE" "$DST_SERVICE"
fi

# Write config
sudo tee "$CONF_FILE" > /dev/null <<EOF
GETTY_ARGS="--autologin $USER_NAME --noclear"
BAUD_RATE=38400
TERM_NAME=linux
EOF

# Disable default tty1 getty
if [ -e "$ACTIVE_SERVICE" ]; then
  sudo rm -f "$ACTIVE_SERVICE"
fi

# Enable autologin service
if [ ! -L "$ACTIVE_LINK" ]; then
  sudo ln -s "$DST_SERVICE" "$ACTIVE_LINK"
fi

echo "Done. Autologin enabled on tty1."
echo "Switch with Ctrl+Alt+F1 or reboot."
