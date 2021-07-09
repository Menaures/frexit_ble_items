#include <Arduino.h>
#include <RTC_SAMD21.h>
#include <DateTime.h>

RTC_SAMD21 rtc;
DateTime now;

void setup() {
  rtc.begin();
  now = DateTime(F(__DATE__), F(__TIME__));
  rtc.adjust(now);
}

void loop() { }
