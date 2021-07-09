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

// Item characteristic variables
bool itemCharacteristicValue = false;

// Item configuration variables
uint8_t storeOpeningTime[3] = {16, 30, 0};  // hours, minutes, seconds
uint8_t storeClosingTime[3] = {19, 0, 0};  // hours, minutes, seconds
uint8_t storeClosingTimeSat[3] = {18, 0, 0};  // hours, minutes, seconds

// Current time  the onboard RTC is running
uint8_t rtcTime[3] = {0, 0, 0};  // hours, minutes, seconds

// Store is open
bool isStoreOpen;

// The current day of the week is saturday
bool isSaturday;


/* ======================================================================================================
 * Main setup and loop
 * =================================================================================================== */

void setup() {
  MG126_Ble.ble_init();
  setupItemAction();
}


void loop() {
  establishConnection();
}


/* ======================================================================================================
 * BLE function definitions
 * =================================================================================================== */

void establishConnection() {
  if (GetConnectedStatus()) {
    connectCallback();

    // connectedLoop();
    simplifiedConnectedLoop();
    
    disconnectCallback();
  }
}


void connectCallback() {
  rtc.detachInterrupt();
}


void disconnectCallback() {
  itemCharacteristicValue = false;
  lightLoop();
}


void readTime() {
  now = rtc.now();
  rtcTime[0] = now.hour();
  rtcTime[1] = now.minute();
  rtcTime[2] = now.second();
}


void writeTime() {
  now = rtc.now();
  now = DateTime(now.year(), now.month(), now.day(), rtcTime[0], rtcTime[1], rtcTime[2]);
  rtc.adjust(now);
}

/* ======================================================================================================
 * Item function definitions
 * =================================================================================================== */

void setupItemAction() {
  // setup pins for light sensor and led strip control
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAIS_PIN, OUTPUT);
  digitalWrite(RELAIS_PIN, LOW);
  setupRtc();

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  FastLED.delay(10);
  FastLED.clear(true);

  // configure callback functions
  readRtcTimeCback = &readTime;
  writeRtcTimeCback = &writeTime;
}


void connectedLoop() {
  while (GetConnectedStatus()) {
    if (itemCharacteristicValue) {
      digitalWrite(RELAIS_PIN, LOW);
      showCode();
      FastLED.delay(2000);
    }
  }
}

// Ignore the characteristic and instantly show the item action for a fixed time
void simplifiedConnectedLoop() {
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
  rtc.begin();
  lightLoop();
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


bool storeOpen() {
  now = rtc.now();

  // Is it saturday?
  if (now.dayOfTheWeek() == 6) {
    isSaturday = true;
  } else {
    isSaturday = false;
  }

  // Has the store already opened?
  if ((TimeSpan(0, now.hour(), now.minute(), now.second()) - TimeSpan(0, storeOpeningTime[0], storeOpeningTime[1], storeOpeningTime[2])).seconds() >= 0) {
    // Is it saturday?
    if (isSaturday) {
      // Has 6 pm already passed?
      if ((TimeSpan(0, now.hour(), now.minute(), now.second()) - TimeSpan(0, storeClosingTimeSat[0], storeClosingTimeSat[1], storeClosingTimeSat[2])).seconds() >= 0) {
        // Store has already closed.
        isStoreOpen = false;
        return false;
      } else {
        // Store has not closed yet.
        isStoreOpen = true;
        return true;
      }
    } else {
      // It is any other day of the week than saturday.
      // Has 7 pm already passed?
      if ((TimeSpan(0, now.hour(), now.minute(), now.second()) - TimeSpan(0, storeClosingTime[0], storeClosingTime[1], storeClosingTime[2])).seconds() >= 0) {
        // Store has already closed.
        isStoreOpen = false;
        return false;
      } else {
        // Store has not closed yet.
        isStoreOpen = true;
        return true;
      }
    }
  } else {
    // The store has not opened yet.
    isStoreOpen = false;
    return false;
  }
}


void setOneTimeAlarm() {
  now = rtc.now();
  
  if (isStoreOpen) {
    // The store is open. Set an alarm for the closing time.
    // Is it saturday?
    if (isSaturday) {
      alarm = DateTime(now.year(), now.month(), now.day(), storeClosingTimeSat[0], storeClosingTimeSat[1], storeClosingTimeSat[2]);
    } else {
      alarm = DateTime(now.year(), now.month(), now.day(), storeClosingTime[0], storeClosingTime[1], storeClosingTime[2]);
    }
  } else {
    // The store is closed. Set an alarm for the opening time.
    // Is it saturday?
    if (isSaturday) {
      alarm = DateTime(now.year(), now.month(), now.day(), storeOpeningTime[0], storeOpeningTime[1], storeOpeningTime[2])
              + TimeSpan(2, 0, 0, 0);
    } else {
      alarm = DateTime(now.year(), now.month(), now.day(), storeOpeningTime[0], storeOpeningTime[1], storeOpeningTime[2])
              + TimeSpan(1, 0, 0, 0);
    }
  }
  rtc.setAlarm(alarm);
  rtc.enableAlarm(rtc.MATCH_DHHMMSS);
  rtc.attachInterrupt(lightLoop);
}


void setPeriodicAlarm() {
    now = rtc.now();
    alarm = now + TimeSpan(LIGHT_MEASUREMENT_PERIOD_SECONDS);
    rtc.setAlarm(alarm);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.attachInterrupt(lightLoop);
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
