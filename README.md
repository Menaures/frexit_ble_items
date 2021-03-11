# frexit_ble_items

Programmcode in "mg126_spionglas"

BLE code in "mg126_spionglas/app_hogp.c":
  Item service declaration -> l.104-108

Verwendeter Microcontroller https://wiki.seeedstudio.com/Wio-Lite-MG126/

Arduino IDE für wio-lite-mg126 aufsetzen: https://wiki.seeedstudio.com/Wio-Lite-MG126/#software

Beispiel für BLE auf wio-lite-mg126 in Arduino IDE "file/examples/examples from custom libraries/Seeed Arduino MG126/echo_ble". Darauf basiert der Code für die Items.

BLE Dokumentation: https://www.bluetooth.com/specifications/specs/core-specification/
Das Wichtigste findet sich in Vol 3: Part G. Könnte hilfreich sein, die Characteristic Definition anzuschauen, um zu verstehen, wie die Characteristics in "app_hogp.c" definiert wurden.
