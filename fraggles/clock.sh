#!/bin/bash
cd $(dirname $0)

if [ -e clock.py ]
then
  python3 clock.py > /dev/null
else
  python3 main.py > /dev/null
fi
