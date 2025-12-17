#include <Arduino.h>
#include <Adafruit_SSD1306.h>

extern Adafruit_SSD1306 display;
extern unsigned long lastIconSwap;
extern int iconIndex;

void drawIconAt(int iconNumber, const uint8_t *bitmap) {
  int x = 0;
  if (iconNumber == 1) x = 8;
  else if (iconNumber == 2) x = 48;
  else if (iconNumber == 3) x = 88;
  else return;

  if (bitmap) {
    display.drawBitmap(x, 0, bitmap, 32, 32, SSD1306_WHITE);
  } else {
    display.drawRect(x, 0, 32, 32, SSD1306_WHITE);
  }
}

void updateIconCycle() {
  const unsigned long ICON_PERIOD = 2000;
  if (millis() - lastIconSwap < ICON_PERIOD) return;
  lastIconSwap = millis();

  extern const uint8_t* getBitmapByIndex(int index);
  extern const size_t BITMAP_COUNT;

  display.clearDisplay();
  for (int slot = 0; slot < 3; ++slot) {
    int idx = (iconIndex + slot) % (int)BITMAP_COUNT;
    const uint8_t* bmp = getBitmapByIndex(idx);
    drawIconAt(slot + 1, bmp);
  }
  display.display();

  iconIndex = (iconIndex + 1) % (int)BITMAP_COUNT;
}
