#!/bin/bash
cd $(dirname $0)

if [ -e pixelclock.py ]
then
  python3 pixelclock.py > /dev/null
else
  python3 main.py > /dev/null
fi
