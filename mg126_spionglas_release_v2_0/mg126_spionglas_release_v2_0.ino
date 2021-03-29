#include <Arduino.h>
#include <stdio.h>
#include <FastLED.h>
#include <MG126_Ble.h>
#include "SPI.h"


#define NUM_LEDS 55
#define DATA_PIN 5
CRGB leds[NUM_LEDS];


MG126_Ble_Class MG126_Ble;

bool bItemCharacteristicValueChanged = false;
bool itemCharacteristicValue = false;


void setup() {
  MG126_Ble.ble_init();
  setupItemAction();
}


void loop() {
  if (GetConnectedStatus()) {
    simplifiedConnected();
  } 
  
  //establishConnection();
}


void setupItemAction() {
#ifdef DEFAULT_ITEM
  // setup default item action
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
#endif  // DEFAULT_ITEM

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.delay(10);
  FastLED.clear(true);
}


void itemAction(bool state) {
#ifdef DEFAULT_ITEM
  // default item action
  digitalWrite(LED_BUILTIN, state);
#endif  // DEFAULT_ITEM

  if (state) {
    fill_solid(leds, NUM_LEDS, CRGB::White);
    for (int brightness = 0; brightness < 256; brightness++) {
      FastLED.setBrightness(brightness);
      FastLED.show();
      FastLED.delay(10);
    }
  } else {
    for (int brightness = 255; brightness >= 0; brightness--) {
      FastLED.setBrightness(brightness);
      FastLED.show();
      FastLED.delay(10);
    }
    FastLED.clear(true);
  }
}


void establishConnection() {
  if (GetConnectedStatus()) {
    Serial.println("Central connected");
    // TODO Verify identity here
    
    while (GetConnectedStatus()) {
      if (bItemCharacteristicValueChanged) {
        itemAction(itemCharacteristicValue);
        bItemCharacteristicValueChanged = false;
      }
    }
    Serial.println("Disconnected from central");
    itemCharacteristicValue = false;
    itemAction(itemCharacteristicValue);
    FastLED.clear(true);
    FastLED.delay(10);
  }
}

void simplifiedConnected() {
  itemAction(true);
  FastLED.delay(20000);
  itemAction(false);
  FastLED.delay(10);
}
