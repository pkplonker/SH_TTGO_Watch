#pragma once
#include <Arduino.h>
#include <LilyGoWatch.h>
#include <WiFi.h>
#include <JPEGDecoder.h>
#include "WiFi.h"
#include "SPIFFS.h"
#include "esp_task_wdt.h"

#include "config.h"
#include "Graphics/Colors.h"
#include "Graphics/TFTHelper.h"
#include "Core/SerialLogger.h"
#include "Screens/TestScreen.h"

#include <NimBLEDevice.h>

#define IRQ_POWER_FLAG _BV(1)
#define IRQ_BMA_FLAG _BV(2)
#define IRQ_TOUCH_FLAG _BV(3)

#define GPIO_POWER ((uint64_t)1 << 35)
#define GPIO_RTC ((uint64_t)1 << 37)
#define GPIO_TOUCH ((uint64_t)1 << 38)
#define GPIO_BMA ((uint64_t)1 << 39)

#define SERVICE_UUID "647a52e4-3279-41ac-9ed2-2a3c768be01A"

#define MESSAGE_UUID "e9c2e910-a206-4d9e-a2d4-49834e1ec45a"
#define BATTERY_PERCENTAGE_UUID "e9c2e910-a206-4d9e-a2d4-49834e1ec45b"

enum SleepState
{
    SleepState_Awake,
    SleepState_SilentAwake,
    SleepState_LightSleep,
    SleepState_DeepSleep
};

const TickType_t xMaxWait = pdMS_TO_TICKS(100);
const uint64_t silentWakeTime = SILENT_WAKE_TIME_SECONDS * 1000000;
TTGOClass *watch;
EventGroupHandle_t eventGroupHandle;
SerialLogger *logger = nullptr;
unsigned long lastInteraction;
int period = 1000;
unsigned long lastTime = 0;
SleepState sleepState;
NimBLECharacteristic *messageChar;
NimBLECharacteristic *batteryPercentChar;

void SetLightSleep();
void SetWake();
void SetupPower();
void SetupBMA();
void HandlePowerInterupts();
void HandleBMAInterupts();
void SetSilentWake();
void HandleAwake();
void DisplayTimeout();
void InitBT();

class MyCallbacks : public NimBLECharacteristicCallbacks
{
    void onWrite(NimBLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0)
        {
            Serial.println("Received a value from the client!");
            Serial.println(value.c_str());
        }
    }
};
