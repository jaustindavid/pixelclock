import board
import busio
import digitalio
from adafruit_epd.ssd1680 import Adafruit_SSD1680Z

# Define SPI and pins
spi = busio.SPI(board.SCK, MOSI=board.MOSI, MISO=board.MISO)
ecs = digitalio.DigitalInOut(board.CE0)  # Chip select
dc = digitalio.DigitalInOut(board.D22)   # Data/command
rst = digitalio.DigitalInOut(board.D27)  # Reset
busy = digitalio.DigitalInOut(board.D17) # Busy

# Initialize the display with SSD1680Z driver
display = Adafruit_SSD1680Z(
            122, 250, spi, cs_pin=ecs, dc_pin=dc, sramcs_pin=None, rst_pin=rst, busy_pin=busy
                        )

# Set display to portrait mode
display.rotation = 1

# Clear the display to solid black, then draws a white rectangle over the entire screen.
display.fill(0)
display.fill_rect(1, 1, 248, 120, 1)
display.display()
