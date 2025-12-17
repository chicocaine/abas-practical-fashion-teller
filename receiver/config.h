#pragma once

// --- PIN CONFIGURATION ---
#define RF_RX_PIN      1
#define RF_TX_PIN      20
#define RF_PTT_PIN     21

#define I2C_SDA_PIN    4
#define I2C_SCL_PIN    5

// Second OLED I2C (for testing) -> SDA=6, SCL=7
#define I2C2_SDA_PIN   6
#define I2C2_SCL_PIN   7

#define NEOPIXEL_PIN   23
#define USR_BTN_PIN    24
#define NUM_PIXELS     1

// --- OLED SETTINGS ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

// Weather data timeout tracking (0 disables timeout)
constexpr unsigned long DATA_TIMEOUT = 30000UL; // 30 seconds
