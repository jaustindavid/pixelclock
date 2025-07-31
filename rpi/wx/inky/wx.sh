#!/usr/bin/bash

cd /home/austin
. .virtualenvs/pimoroni/bin/activate
cd Pimoroni/inky/wx
[ -z "$1" -o "$1" != "now" ] && sleep 30
OW_API_KEY=$(cat OW_API_KEY.txt) ./inky-wx.py 2>&1 | tee -a /var/log/wx.log
