#ifndef MG126_BLE_ITEM_H_
#define MG126_BLE_ITEM_H_

#include <stdint.h>

extern uint16_t itemLightThreshold;
extern uint16_t ambientLight;

extern void (*readRtcTimeCback)();
extern void (*writeRtcTimeCback)();

void establishConnection();
void connectCallback();
void disconnectCallback();
void readTime();
void writeTime();
void setupItemAction();
void connectedLoop();
void showCode();
void setupRtc();
void lightLoop();
bool storeOpen();
void alarmHandler();
void setOneTimeAlarm();
void setPeriodicAlarm();
bool isDark();
uint16_t calculateMeanAmbientLight();
void switchLight(bool state);

#endif  // MG126_BLE_ITEM_H_
