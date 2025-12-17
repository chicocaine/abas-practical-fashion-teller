#include "config.h"
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <RH_ASK.h>
#include "icon_types.h"

extern Adafruit_SSD1306 display;
extern Adafruit_SSD1306 display2;
extern Adafruit_NeoPixel strip;
extern RH_ASK driver;
extern bool hasDisplay2;
extern unsigned long lastDataTime;
extern bool showingOfflineMessage;
extern bool hasReceivedData;

void drawIconAt(int iconNumber, const uint8_t *bitmap);
void setStatusColor(int r, int g, int b);
const uint8_t* getBitmapByIcon(IconId id); // from bitmaps.ino

// Weather rule prototypes (defined in rules.ino)
int check_stormy(float temp, float hum, float atp);
int check_rainy(float temp, float hum, float atp);
int check_cloudy(float temp, float hum, float atp);
int check_sunny(float temp, float hum, float atp);
int check_cool(float temp, float hum, float atp);
int check_unstable(float temp, float hum, float atp);
int high_tropical_temp(float temp);
int high_tropical_hum(float hum);
int low_tropical_hum(float hum);

static IconId selectClothingIcon(float temp, float hum, float atp) {
  // Rain/Storm -> Raincoat
  if (check_stormy(temp, hum, atp) || check_rainy(temp, hum, atp)) {
    return RAINCOAT;
  }

  // Foggy/Cloudy/Cool -> Jacket (reuse cloudy + cool temp)
  if (check_cloudy(temp, hum, atp) && check_cool(temp, hum, atp)) {
    return JACKET;
  }

  if (check_cool(temp, hum, atp)) {
    return JACKET;
  }

  // High temp + high humidity -> Light Breathable
  if (high_tropical_temp(temp) && high_tropical_hum(hum)) {
    return LIGHT_BREATHABLE;
  }

  // High temp + low humidity -> Light Cover
  if (high_tropical_temp(temp) && low_tropical_hum(hum)) {
    return LIGHT_COVER;
  }

  // If Unstable: Naked (use QUESTION_MARK as placeholder if no naked icon)
  if (check_unstable(temp, hum, atp)) {
    return DEKAN_MAN;
  }

  // Default: Light Cover
  return LIGHT_COVER;
}

static IconId selectWeatherIcon(float temp, float hum, float atp) {
  if (check_stormy(temp, hum, atp)) return STORM;
  if (check_rainy(temp, hum, atp))  return RAINY;
  if (check_cloudy(temp, hum, atp)) return CLOUDY;
  if (check_sunny(temp, hum, atp))  return SUNNY;
  if (check_cool(temp, hum, atp))   return SNOW_FLAKE;
  // Fallbacks
  if (check_unstable(temp, hum, atp)) return QUESTION_MARK;
  return QUESTION_MARK;
}

static IconId selectAccessoryIcon(float temp, float hum, float atp) {
  if (check_stormy(temp, hum, atp)) return BOOTS;

  if (check_rainy(temp, hum, atp)) return UMBRELLA;

  if (high_tropical_temp(temp)) return BOTTLE_O_WATER;

  if (high_tropical_hum(hum)) return TOWEL;

  return UMBRELLA;
}

void listenForWeather() {
  uint8_t buf[50];
  uint8_t buflen = sizeof(buf);

  if (driver.recv(buf, &buflen)) {
    buf[buflen] = '\0';
    String data = String((char*)buf);

    Serial.print("RX: ");
    Serial.println(data);

    if (data.startsWith("<") && data.endsWith(">")) {
      data = data.substring(1, data.length()-1);

      float temp, hum, pres;
      sscanf(data.c_str(), "%f,%f,%f", &temp, &hum, &pres);

      lastDataTime = millis();
      hasReceivedData = true;
      showingOfflineMessage = false;

      // icons 
      display.clearDisplay();

      IconId weather = selectWeatherIcon(temp, hum, pres);
      drawIconAt(1, getBitmapByIcon(weather));

      IconId clothing = selectClothingIcon(temp, hum, pres);
      drawIconAt(2, getBitmapByIcon(clothing));

      IconId accessory = selectAccessoryIcon(temp, hum, pres);
      drawIconAt(3, getBitmapByIcon(accessory));
      display.display();

      if (hasDisplay2) {
        display2.clearDisplay();
        display2.setTextSize(1);
        display2.setTextColor(SSD1306_WHITE);
        display2.setCursor(0, 0);
        display2.print("Temp: ");
        display2.print(temp, 1);
        display2.print("C");
        display2.setCursor(0, 10);
        display2.print("Hum:  ");
        display2.print(hum, 0);
        display2.print("%");
        display2.setCursor(0, 20);
        display2.print("Pres: ");
        display2.print(pres, 0);
        display2.print("hPa");
        display2.display();
      }

      strip.setPixelColor(0, strip.Color(255, 255, 255));
      strip.show();
      delay(50);

      setStatusColor(255, 0, 0);
    }
  }
}
