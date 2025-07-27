#!/usr/bin/bash

cd /home/austin/wx
. bin/activate
cd src
./wx.py 2>&1 | tee -a /var/log/wx.log
