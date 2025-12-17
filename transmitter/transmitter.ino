/*
   PROJECT: Weather Station Transmitter DEBUG
   BOARD: YD-RP2040 (16MB)
   
   WIRING:
   - BME280 SDA -> GP4
   - BME280 SCL -> GP5
   - RF DAT     -> GP0
   - Onboard LED -> GP23
   - Onboard BTN -> GP24
*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>
#include <RH_ASK.h>
#include <SPI.h>

// --- PIN CONFIGURATION ---
#define RF_TX_PIN      0
#define RF_RX_PIN      20
#define RF_PTT_PIN     21

#define I2C_SDA_PIN    4
#define I2C_SCL_PIN    5

#define NEOPIXEL_PIN   23
#define USR_BTN_PIN    24
#define NUM_PIXELS     1

// --- OBJECTS ---
Adafruit_NeoPixel strip(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_BME280 bme;
RH_ASK driver(2000, RF_RX_PIN, RF_TX_PIN, RF_PTT_PIN);

// --- VARIABLES ---
bool systemReady = false;
bool systemRunning = false;

void setup() {
  Serial.begin(115200);

  // I2C setup
  Wire.setSDA(I2C_SDA_PIN);
  Wire.setSCL(I2C_SCL_PIN);
  Wire.begin();

  // LED
  strip.begin();
  strip.setBrightness(40);
  strip.show();

  // Button
  pinMode(USR_BTN_PIN, INPUT_PULLUP);

  // --- POST / TEST ---
  
  // Blue: booting
  setStatusColor(0, 0, 255);
  delay(500);

  // Yellow: check BME280
  setStatusColor(255, 255, 0);
  if (!bme.begin(0x76) && !bme.begin(0x77)) {
    Serial.println("BME280 Error! Check wiring and address.");
    setStatusColor(255, 100, 0); // Orange
    while (1);
  }
  Serial.println("BME280 OK");
  delay(500);

  // Purple: check RF driver
  setStatusColor(255, 0, 255);
  if (!driver.init()) {
    Serial.println("RF Driver Error! Check wiring.");
    setStatusColor(255, 0, 100); // Magenta
    while (1);
  }
  Serial.println("RF Driver OK");
  delay(500);

  // Green: ready
  setStatusColor(0, 255, 0);
  Serial.println("System Ready. Press Button to Start Transmission.");
  systemReady = true;
}

void loop() {

  if (systemReady && !systemRunning) {
      systemRunning = true;
      setStatusColor(255, 0, 0); // RED: Transmitting
      Serial.println("Button pressed. Transmission started.");
  }

  if (systemRunning) {
    transmitWeather();
    delay(2000); // Send every 2 seconds
  }
}

void transmitWeather() {
  // Read sensor
  float temp = bme.readTemperature();
  float hum  = bme.readHumidity();
  float pres = bme.readPressure() / 100.0F;

  // Print to Serial for debug
  Serial.print("Temp: "); Serial.print(temp); Serial.print(" C, ");
  Serial.print("Hum: "); Serial.print(hum); Serial.print("%, ");
  Serial.print("Pres: "); Serial.print(pres); Serial.println(" hPa");

  // Format string: <temp,hum,pres>
  char buffer[32];
  snprintf(buffer, sizeof(buffer), "<%.2f,%.2f,%.2f>", temp, hum, pres);

  // Send via RF
  driver.send((uint8_t*)buffer, strlen(buffer));
  driver.waitPacketSent();
  Serial.print("Sent RF: "); Serial.println(buffer);

  // LED blink
  strip.setPixelColor(0, strip.Color(255, 255, 255));
  strip.show();
  delay(50);
  setStatusColor(255, 0, 0); // Back to RED
}

void setStatusColor(int r, int g, int b) {
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}
