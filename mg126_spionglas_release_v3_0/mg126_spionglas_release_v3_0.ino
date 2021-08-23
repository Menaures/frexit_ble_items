#include <Arduino.h>
#include <stdio.h>
#include <FastLED.h>
#include <MG126_Ble.h>
#include "SPI.h"


#define NUM_LEDS 55
#define DATA_PIN 5
CRGB leds[NUM_LEDS];


MG126_Ble_Class MG126_Ble;


void setup() {
  MG126_Ble.ble_init();
  setupItemAction();
}

void loop() {
  if (GetConnectedStatus()) {
    connectedLoop();
  } else {
    ble_set_adv_enableFlag(1);
  }
}

void setupItemAction() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.delay(10);
  FastLED.clear(true);
}

void itemAction(bool state) {
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

void connectedLoop() {
  itemAction(true);
  FastLED.delay(20000);
  itemAction(false);
  FastLED.delay(10);
}
