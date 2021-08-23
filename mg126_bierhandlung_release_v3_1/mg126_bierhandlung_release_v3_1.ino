#include "mg126_ble_item.h"
#include <Arduino.h>
#include <stdio.h>
#include <MG126_Ble.h>
#include <FastLED.h>
#include <RTC_SAMD21.h>
#include <DateTime.h>
#include "SPI.h"


/* ======================================================================================================
 * Global defines
 * =================================================================================================== */
// Pin definitions
#define NUM_LEDS 12
#define DATA_PIN 5
#define RELAIS_PIN 6
#define SENSOR_PIN A0

// Time interval in which the ambient light sensor will be read
#define LIGHT_MEASUREMENT_PERIOD_SECONDS 10


/* ======================================================================================================
 * Global variable declarations
 * =================================================================================================== */
// Global class instances representing the BLE radio stack, the blinking leds, and the real time clock
MG126_Ble_Class MG126_Ble;
CRGB leds[NUM_LEDS];
RTC_SAMD21 rtc;

// Global time variables to hold the current time and the set alarm
DateTime now;
DateTime alarm;

// "Bierhandlung" in reverse, because the leds are wired beginning at the end of the string
const char characters[13] = "g uldnahreib";

// The code that will blink, if the item is activated
char code[5] = "neun";

// Light sensor variables
uint16_t ambientLight;
uint16_t itemLightThreshold = 50;

// Flag that signals whether an RTC interrupt has occured.
volatile bool alarmFlag = false;


/* ======================================================================================================
 * Main setup and loop
 * =================================================================================================== */
void setup() {
  MG126_Ble.ble_init();
  setupItemAction();
  setupRtc();
}

void loop() {
  if (GetConnectedStatus()) {
    connectedLoop();
    lightLoop();
  }
  else {
    if (alarmFlag) {
      lightLoop();
      alarmFlag = false;
    }
    ble_set_adv_enableFlag(1);
  }
}


/* ======================================================================================================
 * Item function definitions
 * =================================================================================================== */
void setupItemAction() {
  // setup pins for light sensor and led strip control
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAIS_PIN, OUTPUT);
  digitalWrite(RELAIS_PIN, LOW);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  FastLED.delay(10);
  FastLED.clear(true);
}

// Show the item action ten times.
void connectedLoop() {
  digitalWrite(RELAIS_PIN, LOW);
  for (int i=0; i<10; i++) {
    showCode();
    FastLED.delay(2000);
  }
}

void showCode() {
  for (int i = 0; i < sizeof(code) - 1; i++) {
    for (int j = 0; j < sizeof(characters) - 1; j++) {
      leds[j] = (code[i] == characters[j]) ? CRGB::Red : CRGB::Black;
        
    }
    FastLED.show();
    FastLED.delay(1500);
    FastLED.clear(true);
    FastLED.delay(300);
  }
  FastLED.clear(true);
}


/* ======================================================================================================
 * Light control function definitions
 * =================================================================================================== */
void setupRtc() {
  // configure callback functions
  rtc.begin();
  lightLoop();
}

void alarmHandler() {
  alarmFlag = true;
}

//
// tue - fr: 13.00 - 19.00
// sa: 11.30 - 18.00
//
bool storeOpen() {
  now = rtc.now();

  return (now.dayOfTheWeek() >= 2 && now.dayOfTheWeek() <= 5 && now.hour() >= 13 && now.hour() <= 19)
          ||
          (now.dayOfTheWeek() == 6 && now.hour() == 11 && now.minute() >= 30)
          ||
          (now.dayOfTheWeek() == 6 && now.hour() >= 12 && now.hour() <= 18);
}

void lightLoop() {
  // Is the store open?
  if (storeOpen()) {
    // Decide whether is is dark enough to turn the light on.
    if (isDark()) {
      // Turn the light on.
      switchLight(true);
      // Set an alarm for when the store closes to turn the light off.
      setOneTimeAlarm();
    } else {
      // It is not yet dark. Check again in a while.
      setPeriodicAlarm();
    }
  } else {
    switchLight(false);
    setOneTimeAlarm();
  }
}

void setOneTimeAlarm() {
  now = rtc.now();
  
  if (storeOpen()) {
    // The store is open. Set an alarm for the closing time.
    if (now.dayOfTheWeek() == 6) {
      alarm = DateTime(now.year(), now.month(), now.day(), 18, 1, 0);
    } else {
      alarm = DateTime(now.year(), now.month(), now.day(), 19, 1, 0);
    }
  } else {
    // The store is closed. Set an alarm for the opening time.
    if (now.dayOfTheWeek() == 0) {
      alarm = DateTime(now.year(), now.month(), now.day(), 13, 0, 0)
        + TimeSpan(2, 0, 0, 0);
    } else if (now.dayOfTheWeek() == 1) {
      alarm = DateTime(now.year(), now.month(), now.day(), 13, 0, 0)
        + TimeSpan(1, 0, 0, 0);
    } else if (now.dayOfTheWeek() == 6) {
      alarm = DateTime(now.year(), now.month(), now.day(), 13, 0, 0)
              + TimeSpan(3, 0, 0, 0);
    } else {
      alarm = DateTime(now.year(), now.month(), now.day(), 13, 0, 0)
              + TimeSpan(1, 0, 0, 0);
    }
  }
  rtc.setAlarm(alarm);
  rtc.enableAlarm(rtc.MATCH_DHHMMSS);
  rtc.attachInterrupt(alarmHandler);
}

void setPeriodicAlarm() {
    now = rtc.now();
    alarm = now + TimeSpan(LIGHT_MEASUREMENT_PERIOD_SECONDS);
    rtc.setAlarm(alarm);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.attachInterrupt(alarmHandler);
}

bool isDark() {
  static uint16_t meanAmbientLight = 0;

  meanAmbientLight = calculateMeanAmbientLight();
  ambientLight = meanAmbientLight;

  // The output stage uses an inverting amplifier, thus a value of 0 means max brightness
  if (meanAmbientLight > itemLightThreshold) {
    return true;
  } else {
    return false;
  }
}

uint16_t calculateMeanAmbientLight() {
  static uint16_t lightValues[10] = { 0 };
  static uint8_t index = 0;

  lightValues[index] = analogRead(SENSOR_PIN);

  uint32_t sum = 0;
  for (uint8_t i=0; i < 10; i++) {
    sum += lightValues[i];
  }

  index ++;
  if (index > 9) index = 0;
  
  if (sum > 0) return (uint16_t) sum / 10;
  else return 0;
}

void switchLight(bool state) {
  digitalWrite(RELAIS_PIN, state);
}
