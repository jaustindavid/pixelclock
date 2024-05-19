# Getting Started

* https://micropython.org/download/ESP32_GENERIC/
* https://docs.espressif.com/projects/esptool/en/latest/esp32/advanced-topics/boot-mode-selection.html#manual-bootloader
* `esptool.py --chip esp32 --port /dev/tty.usbserial-0001 erase_flash`
* press/hold GPIO, tap EN
* esptool.py --chip esp32 --port /dev/tty.usbserial-0001 --baud 460800 write_flash -z 0x1000 ESP32_GENERIC-20231227-v1.22.0.bin
* REPL: `screen /dev/tty.usbserial-0001 115200`
* ampy -p /dev/tty.usbserial-0001 run timer.py
