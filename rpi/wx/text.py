import board
import busio
import digitalio
from adafruit_epd.ssd1680 import Adafruit_SSD1680Z
from PIL import Image, ImageDraw, ImageFont


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
# RGB Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)

# Set display to portrait mode
display.rotation = 1

# Load a TTF Font
font = ImageFont.truetype(
    # "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 16 
    "/usr/local/share/fonts/custom/Hack-Regular.ttf", 16
    )
image = Image.new("RGB", (display.width, display.height))
draw = ImageDraw.Draw(image)

# Draw a filled box as the background
draw.rectangle((0, 0, display.width - 1, display.height - 1), fill=BLACK)

# Draw a smaller inner foreground rectangle
BORDER = 4
draw.rectangle(
   (BORDER, BORDER, display.width - BORDER - 1, display.height - BORDER - 1),
       fill=WHITE,
       )

# Draw Some Text
TEXT = "Some Text 012345678"
(font_width, font_height), (offset_x, offset_y) = font.font.getsize(TEXT)
draw.text(
    (display.width // 2 - font_width // 2, display.height // 2 - font_height // 2),
        TEXT,
            font=font,
                fill=BLACK,
                 )

# Display image.
display.image(image)
display.display()
