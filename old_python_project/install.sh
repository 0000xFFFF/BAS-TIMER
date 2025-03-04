#!/bin/bash -x
sudo ln -sfr server.py /usr/local/bin/bas-server
sudo setcap 'cap_net_bind_service=+ep' server.py
