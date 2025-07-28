#!/usr/bin/bash

cd /home/austin
. .virtualenvs/pimoroni/bin/activate
cd Pimoroni/inky/wx
sleep 30
OW_API_KEY = "CREATE AN API KEY" ./inky-wx.py 2>&1 | tee -a /var/log/wx.log
