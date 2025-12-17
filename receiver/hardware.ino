#include "config.h"
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

extern Adafruit_NeoPixel strip;
// RF_RX_PIN is defined as a macro in config.ino

bool detectOLED() {
  byte error, address;
  for (address = 0x03; address < 0x78; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      if (address == 0x3C || address == 0x3D) return true;
    }
  }
  return false;
}

bool detectOLEDOn(TwoWire &bus) {
  byte error, address;
  for (address = 0x03; address < 0x78; address++) {
    bus.beginTransmission(address);
    error = bus.endTransmission();
    if (error == 0) {
      if (address == 0x3C || address == 0x3D) return true;
    }
  }
  return false;
}

bool detectRFReceiver() {
  pinMode(RF_RX_PIN, INPUT);
  int initial = digitalRead(RF_RX_PIN);
  bool changed = false;
  unsigned long t0 = millis();
  while (millis() - t0 < 200) {
    if (digitalRead(RF_RX_PIN) != initial) { changed = true; break; }
  }
  return changed;
}

void setStatusColor(int r, int g, int b) {
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}
