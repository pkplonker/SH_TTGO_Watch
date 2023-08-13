#pragma once
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "config.h"
#include "Logger.h"
#include "HandlerBase.h"
#include "BatteryData.h"
#include "InteractionState.h"

enum SleepState
{
    SleepState_Awake,
    SleepState_SilentAwake,
    SleepState_LightSleep,
    SleepState_DeepSleep
};

class PowerHandler : public HandlerBase
{
public:
    PowerHandler(TTGOClass *watch, Logger *logger) : HandlerBase(watch, logger)
    {

        SetupPower();
    }
    virtual void Loop() override;
    BatteryData GetBatteryData();
    void HandlePowerInterupts();

private:
    SleepState sleepState;
    void ISRCallback();
    void SetupPower();
    void HandleAwake();
    void SetLightSleep();
    void SetWake();
    void SetSilentWake();
    void DisplayTimeout();
    const uint64_t silentWakeTime = SILENT_WAKE_TIME_SECONDS * 1000000;
};
