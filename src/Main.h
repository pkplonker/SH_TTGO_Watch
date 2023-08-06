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

#define IRQ_POWER_FLAG _BV(1)
#define IRQ_BMA_FLAG _BV(2)
#define IRQ_TOUCH_FLAG _BV(3)

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

int period = 1000;
unsigned long lastTime = 0;
SleepState sleepState;

void Sleepmode();
void Wake();
void SetupPower();
void SetupBMA();
void HandlePowerInterupts();
void HandleBMAInterupts();
void SilentWake();
void HandleAwake();

