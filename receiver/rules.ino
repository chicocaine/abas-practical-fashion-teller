#include <Arduino.h>

// check weather

int check_stormy(float temp, float hum, float atp) {
    return hum > 85 and (temp >= 26 and temp <= 30) and atp < 1000;
}

int check_rainy(float temp, float hum, float atp) {
    return hum > 90 and (temp >= 25 and temp <= 30) and (atp >= 1005 and atp <= 1010);
}

int check_cloudy(float temp, float hum, float atp) {
    return (hum >= 75 and hum <= 90) and (temp >= 24 and temp <= 31);
}

int check_sunny(float temp, float hum, float atp) {
    return hum < 80 and (temp > 30 and temp <= 40) and atp > 1012;
}

int check_cool(float temp, float hum, float atp) {
    return temp < 27;   
}

int check_unstable(float temp, float hum, float atp) {
    return 1;
}

// temp and humidity

int low_tropical_temp(float temp) {
    return temp <= 20;
}

int normal_tropical_temp(float temp) {
    return temp >= 20 and temp <= 30;
}

int high_tropical_temp(float temp) {
    return temp > 30;
}

int low_tropical_hum(float hum) {
    return hum <= 50;
}

int normal_tropical_hum(float hum) {
    return hum >= 50 and hum <= 80;
}

int high_tropical_hum(float hum) {
    return hum > 80;
}

// clothing rules

