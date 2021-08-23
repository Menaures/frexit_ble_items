# frexit_ble_items

Installation instructions:
- Follow the steps described in https://wiki.seeedstudio.com/Wio-Lite-MG126/#getting-started.
- Replace Arduino/libraries/Seeed_Arduino_MG126/src/MG126_Ble.h with the MG126_Ble.h in this repo.
- Install the RTC library https://github.com/Seeed-Studio/Seeed_Arduino_RTC.

Project structure:
- Main code in mg126_bierhandlung_release_v2.0.ino / mg126_spionglas_release_v2.0.ino
- Ble definitions in app_hogp.c
  - Service definition l.104 - 108

Notes:
- Seeed Board Package v1.7.9
- Microcontroller development board "Seeed Wio Lite MG126" https://wiki.seeedstudio.com/Wio-Lite-MG126/
- An example for BLE communication can be found in Arduino IDE at "file/examples/examples from custom libraries/Seeed Arduino MG126/echo_ble". The application is based on this example.
- The "nRF Connect" app can be used to debug the BLE connection.

References:
- BLE documentation https://www.bluetooth.com/specifications/specs/core-specification/. The chapter of interest is Vol 3: Part G.

Working principle:
- Initially both items made use of a gatt service with a boolean characteristic which represents the item status, i.e. whether the light should be turned on or off.
- The items should be visible as bluetooth devices with names "fbba51782e8f01" for the beer store, resp. "fbba51782e8f02" for the mirror.
- As soon, as a smartphone connected via the frexit app, the aforementioned characteristic would be written to true. In case of disconnect to false.
- Due to an issue which caused the smartphone to disconnect soon after the connection had been established the characteristic became unused and the item action would perform its action for a defined time duration. (simplifiedConnectedLoop in .ino)
- The beer store item also controls the lighting of the shop sign. This is archieved with hardcoded opening hours and the use of the internal RTC. Additionally a phototransistor is attached in the case of the shop sign to monitor the ambient luminosity.
- The RTC does not have a dedicated power supply, meaning the time and date have to be manually set after the microcontroller has been powered off for a long period of time. This is archieved by uploading the sketch "mg126_set_time.ino" once before uploading the actual item code.

Fixed issues:
- The disconnects seem to be caused by interrupts from the RTC. No interrupts occur if the RTC is not used.
- The items turn undiscoverable after some time. This might be due to the microcontroller entering a low power mode if the processor is idle for too long. This is caused by the advertising duration and has been remedied by continously restarting advertising while no device is connected,
- The light schedule for the beer store contains a logic error. If the code is uploaded on a sunday the light will turn on despite the store being closed.
