/*

   PROJECT: Weather Station Receiver DEBUG
   BOARD: YD-RP2040 (16MB)

*/

#include "config.h"
#include "icon_types.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <RH_ASK.h>
#include <SPI.h>

// Pins and constants moved to config.ino
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);

// --- OBJECTS ---
Adafruit_NeoPixel strip(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
RH_ASK driver(2000, RF_RX_PIN, RF_TX_PIN, RF_PTT_PIN);

// --- VARIABLES ---
bool systemReady = false;
bool systemRunning = false;
bool hasDisplay2 = false;
bool hasDisplay1 = false;
bool hasReceivedData = false;

// Weather data timeout tracking
unsigned long lastDataTime = 0;
bool showingOfflineMessage = false;

// Icon cycling (every 2 seconds)
unsigned long lastIconSwap = 0;
int iconIndex = 0;

// --- Module API Declarations ---
bool detectOLED();
bool detectOLEDOn(TwoWire &bus);
bool detectRFReceiver();
void setStatusColor(int r, int g, int b);
void drawIconAt(int iconNumber, const uint8_t *bitmap = nullptr);
void updateIconCycle();
void listenForWeather();
void checkDisplayStatus();

// ----------------------------------------
// DISPLAY STATUS HELPER
// ----------------------------------------
void checkDisplayStatus() {
  setStatusColor(255, 255, 0); // Yellow scanning I2C
  Serial.println("Scanning I2C for OLED...");
  if (!detectOLED()) {
    Serial.println("ERROR: No OLED found on I2C!");
    setStatusColor(255, 0, 0);
    while (1);
  }

  setStatusColor(0, 255, 255); // Cyan
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println("OLED init fail");
      setStatusColor(255, 0, 0);
      while (1);
    }
  }
  hasDisplay1 = true;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("OLED OK");
  display.println("Press button to start");

  // Try to init second OLED on separate I2C bus; don't block if missing
  bool ok2 = display2.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  if (!ok2) ok2 = display2.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  if (ok2) {
    hasDisplay2 = true;
    display2.clearDisplay();
    display2.setTextSize(1);
    display2.setTextColor(SSD1306_WHITE);
    display2.setCursor(0,0);
    display2.println("OLED OK");
    display2.println("Press button to start");
    display2.setCursor(0,16);
    display2.println(hasDisplay1 ? "OLED1 OK" : "OLED1 MISSING");
    display2.display();
    Serial.println("Second OLED initialized OK");
  } else {
    Serial.println("Second OLED not detected or init failed (non-blocking)");
  }

  // Show OLED2 status on first screen (third line)
  display.setCursor(0,16);
  display.println(hasDisplay2 ? "OLED2 OK" : "OLED2 MISSING");
  display.display();

  Serial.println("OLED initialized OK");
}

// ----------------------------------------
// SETUP
// ----------------------------------------
void setup() {
  Serial.begin(115200);

  Wire.setSDA(I2C_SDA_PIN);
  Wire.setSCL(I2C_SCL_PIN);
  Wire.begin();
  delay(200);

  // Set up second I2C bus on pins SDA=6, SCL=7 (RP2040 Wire1)
  Wire1.setSDA(I2C2_SDA_PIN);
  Wire1.setSCL(I2C2_SCL_PIN);
  Wire1.begin();
  Serial.println("Scanning I2C (Wire1) for OLED...");
  bool oled2DetectedByScan = detectOLEDOn(Wire1);
  Serial.println(oled2DetectedByScan ? "Wire1: OLED address found" : "Wire1: No OLED found");

  strip.begin();
  strip.setBrightness(40);
  strip.show();

  pinMode(USR_BTN_PIN, INPUT_PULLUP);
  setStatusColor(0, 0, 255); // Blue boot
  checkDisplayStatus();

  // --------------------------
  // RF CHECK + INIT
  // --------------------------
  setStatusColor(255, 0, 255); // Purple RF detection
  Serial.println("Checking RF module...");

  if (!detectRFReceiver()) {
    Serial.println("ERROR: RF module not detected!");
    setStatusColor(255, 0, 0); // Red error
    while (1);
  }

  if (!driver.init()) {
    Serial.println("Radio init failed");
    setStatusColor(255, 0, 0);
    while (1);
  }

  Serial.println("RF OK");

  // --------------------------
  // SYSTEM READY
  // --------------------------
  setStatusColor(0, 255, 0); // Green
  systemReady = true;
  Serial.println("System READY");
}

// ----------------------------------------
// LISTEN FOR PACKETS
// ----------------------------------------
void loop() {
  if (systemReady && !systemRunning) {
  
    systemRunning = true;
    hasReceivedData = false;
    setStatusColor(255, 0, 0);

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("System Running...");
    display.println("Waiting for data...");
    display.display();

    if (hasDisplay2) {
      display2.clearDisplay();
      display2.setCursor(0,0);
      display2.println("System Running...");
      display2.println("Waiting for data...");
      display2.display();
    }

    // Start data timeout window immediately upon entering running state
    lastDataTime = millis();
    showingOfflineMessage = false;

    Serial.println("System running, waiting for data...");
  }

  if (systemRunning) {
    listenForWeather();
    // Cycle icons only while waiting for first data, or if timeout is disabled
    if (!hasReceivedData || DATA_TIMEOUT == 0) {
      updateIconCycle();
    }
    
    // Check if data timeout has occurred
    if (lastDataTime > 0 && (millis() - lastDataTime) > DATA_TIMEOUT && !showingOfflineMessage) {
      showingOfflineMessage = true;
      
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 8);
      display.println("Weather module");
      display.println("might be offline");
      display.display();
      
      if (hasDisplay2) {
        display2.clearDisplay();
        display2.setTextSize(1);
        display2.setCursor(0, 8);
        display2.println("Weather module");
        display2.println("might be offline");
        display2.display();
      }
      
      Serial.println("WARNING: No data received for 30+ seconds - module might be offline");
    }
  }
}