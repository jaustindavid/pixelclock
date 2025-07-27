from PIL import Image, ImageDraw, ImageFont
import open_weather

small_font = ImageFont.truetype(
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 16
)
medium_font = ImageFont.truetype(
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 20
)
large_font = ImageFont.truetype(
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24
)
time_font = ImageFont.truetype(
    "/usr/share/fonts/truetype/hack/Hack-Bold.ttf", 48
)

ICON_MAP = {
    "01d": "B",
    "01n": "C",
    "02d": "H",
    "02n": "I",
    "03d": "N",
    "03n": "N",
    "04d": "Y",
    "04n": "Y",
    "09d": "Q",
    "09n": "Q",
    "10d": "R",
    "10n": "R",
    "11d": "Z",
    "11n": "Z",
    "13d": "W",
    "13n": "W",
    "50d": "J",
    "50n": "K",
}

# RGB Colors
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)


_MAIN_Y = 76
_TEMP_X = 175
_TEMP_Y = _MAIN_Y


class WeatherGraphics:
    def __init__(self, inky_display=None, adafruit_display=None):
        assert(inky_display or adafruit_display)

        self.small_font = small_font
        self.medium_font = medium_font
        self.large_font = large_font
        self.time_font = time_font
        self.icon_font = ImageFont.truetype("./meteocons.ttf", 74)

        self.inky_display = inky_display
        self.adafruit_display = adafruit_display
        self.any_display = inky_display or adafruit_display

        self._weather_icon = None
        self._city_name = None
        self._main_text = None
        self._feels_like = None
        self._temp_range = None
        self._description = None
        self._time_hh = 0
        self._time_mm = 0

    def update_weather(self, ow):
        self._min_temp, self._max_temp = ow.range(0, 11)
        self._temp_range = f"{self._min_temp:2.1f}-{self._max_temp:2.1f} °C"
        self._feels_like = f"{ow.f(ow.feels_like()):2.1f}"

        # set the icon/background
        self._weather_icon = ICON_MAP[ow.forecast()['weather'][0]['icon']]
        self._city_name = ow.city
        print(f'{self._city_name} : {self._temp_range}')

        self._main_text = ow.forecast()["weather"][0]["main"]
        print(f'{self._main_text} : {self._feels_like}')

        description = ow.forecast()["weather"][0]["description"]
        description = description[0].upper() + description[1:]

        next_rain = ow.pop(0.40, 0, 11)
        if not next_rain:
            upcoming = "no rain"
        else:
            upcoming = f"{100*next_rain[1]:d}% in {next_rain[0]} hrs"

        self._description = f"{description}; {upcoming}"
        print(self._description)
        # "thunderstorm with heavy drizzle"

    def update_time(self, hh, mm):
        self._time_hh = f'{hh:02d}'
        self._time_mm = f'{mm:02d}'

    def clear(self):
        image = Image.new("RGB", 
                          (self.any_display.width, 
                           self.any_display.height))
        draw = ImageDraw.Draw(image)
        draw.rectangle((0, 0, 
                        self.any_display.width - 1, 
                        self.any_display.height - 1), 
                       fill=WHITE)
        self.inky_display.set_image(image)

    def show_grid(self, draw):
        # vertical guides
        draw.rectangle((width//2, 0, width//2, height), fill=BLACK)
        draw.rectangle((width//5, 0, width//5, height), fill=BLACK)
        draw.rectangle((width*4//5, 0, width*4//5, height), fill=BLACK)
        # horizontal guides
        draw.rectangle((0, height//2, width, height//2), fill=BLACK)
        draw.rectangle((0, height//4, width, height//4), fill=BLACK)
        draw.rectangle((0, height*3//4, width, height*3//4), fill=BLACK)

    def display(self):
        width = self.any_display.width
        height = self.any_display.height

        # clear -- big white rectangle
        image = Image.new("RGB", 
                          (width, self.inky_display.height), 
                          color=WHITE)
        draw = ImageDraw.Draw(image)
        # self.show_grid(draw)

        # Draw the Icon, centered
        x = self.halign_center(width//2, self.icon_font, self._weather_icon)
        y = self.valign_center(height//2, self.icon_font, self._weather_icon)
        draw.text((x, y), self._weather_icon, font=self.icon_font, fill=BLACK)

        # Draw the city -- top left
        draw.text(
            (0, 0), self._city_name, font=self.small_font, fill=BLACK,
        )

        # draw the range, top right
        x = self.halign_right(width, self.small_font, self._temp_range)
        draw.text(
            (x,0), self._temp_range, font=self.small_font, fill=BLACK,
        )

        # Draw the time
        hh_x = self.halign_center(width//6, self.time_font, self._time_hh)
        hh_y = self.valign_center(height//3, self.time_font, self._time_hh)
        draw.text(
            (hh_x, hh_y),
            self._time_hh,
            font=self.time_font,
            fill=BLACK,
        )
        mm_x = self.halign_center(width*5//6, self.time_font, self._time_hh)
        draw.text(
            (mm_x, hh_y),
            self._time_mm,
            font=self.time_font,
            fill=BLACK,
        )

        # Draw the main text
        draw.text(
            (0, _MAIN_Y),
            self._main_text,
            font=self.large_font,
            fill=BLACK,
        )

        # Draw the "feels like" temperature, right of main
        x = self.halign_right(width, self.large_font, self._feels_like)
        draw.text(
            (x, _MAIN_Y),
            self._feels_like,
            font=self.large_font,
            fill=BLACK,
        )

        # Draw the description, centered bottom
        x = self.halign_center(width//2, self.small_font, self._description)
        y = self.valign_bottom(height, self.small_font, self._description)
        draw.text((x, y), self._description, font=self.small_font, fill=BLACK)
        
        # show the image
        self.inky_display.set_image(image)
        self.inky_display.show()

    def halign_right(self, right_x, font, text):
        """returns the proper X to right-justify text against x"""
        (text_width, _), (_,_) = font.font.getsize(text)
        return right_x - text_width

    def halign_center(self, center_x, font, text):
        """returns the proper X to center text around x"""
        (text_width, _), (_,_) = font.font.getsize(text)
        return center_x - text_width//2

    def valign_center(self, center_y, font, text):
        """returns the proper Y to center-orient text against y"""
        _, _, _, text_height = font.getbbox(text)
        return center_y - text_height//2

    def valign_bottom(self, bottom_y, font, text):
        """returns the proper Y to bottom-orient text against y"""
        _, _, _, text_height = font.getbbox(text)
        return bottom_y - text_height

    def update(self, ow, hh, mm):
        self.update_weather(ow)
        self.update_time(hh, mm)
        self.display()
