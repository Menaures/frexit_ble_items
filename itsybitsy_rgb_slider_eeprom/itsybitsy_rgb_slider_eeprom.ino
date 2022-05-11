#include <EEPROM.h>
#include <FastLED.h>

#define NUM_LEDS 8
#define DATA_PIN 4
#define UNLOCK_PIN 6
#define DOOR_CLOSED_PIN A5
#define SET_PIN 8

CRGB leds[NUM_LEDS];


int slider_pins[4] = {A0, A1, A2, A3};
const int threshold = 10;
int hue[4];
int hueRef[4] = {255, 255, 255, 255};



void setup() {
  // setup LEDs
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);
  FastLED.delay(10);
  FastLED.show();

  // pin 7 as voltage source for push buttons
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);

  // sliders
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  // door closed push button
  pinMode(A5, INPUT);
  pinMode(UNLOCK_PIN, OUTPUT);
  digitalWrite(UNLOCK_PIN, LOW);

  // led code set
  pinMode(SET_PIN, INPUT);

  // set reference leds
  setReferenceLedsFromEeprom();
}


void loop() {
  // read all sliders
  for (int i=0; i < 4; i++)
  {
    hue[i] = adjustHue(analogRead(slider_pins[i]));
  }

  // set the led hue according to the slider position
  for (int i=0; i < 4; i++)
  {
    leds[i] = CHSV(hue[i], 255, 255);
  }
  
  // check whether the set button is pushed
  if (digitalRead(SET_PIN)) {
    setReferenceLeds();
  }
  
  FastLED.show();

  // check whether sliders are in the right position
  bool unlock = true;
  for (int i=0; i < 4; i++)
  {
    int diff = abs(hue[i] - hueRef[i]);
    if (diff > threshold) {
      unlock = false;
    }
  }
  if (unlock) openLock();
}


int adjustHue(int hue) {
  return floor(hue/215.0 * 255.0);
}


void openLock() {
  digitalWrite(UNLOCK_PIN, HIGH);
  FastLED.delay(1000);
  digitalWrite(UNLOCK_PIN, LOW);
  while (digitalRead(DOOR_CLOSED_PIN)) {
    FastLED.delay(1);
  }
}


void setReferenceLeds() {
  for (int i=0; i < 4; i++) {
    hue[i] = adjustHue(analogRead(slider_pins[i]));
    hueRef[i] = hue[i];
    leds[i+4] = CHSV(hueRef[i], 255, 255);
  }
  EEPROM.put(0, hueRef);
  FastLED.show();
}

void setReferenceLedsFromEeprom()
{
  EEPROM.get(0, hueRef);
  for (int i=0; i < 4; i++) {
    leds[i+4] = CHSV(hueRef[i], 255, 255);
  }
}
