#include "PowerHandler.h"

void PowerHandler::Loop()
{
    HandleAwake();
    if (millis() - lastInteraction > SCREEN_ACTIVE_TIME)
    {
        DisplayTimeout();
    }
}

void PowerHandler::SetupPower()
{
    pinMode(AXP202_INT, INPUT_PULLUP);

    attachInterrupt(
        AXP202_INT, []
        { BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            if (xEventGroupSetBitsFromISR(eventGroupHandle, IRQ_POWER_FLAG, &xHigherPriorityTaskWoken) == pdPASS)
            {
                if (xHigherPriorityTaskWoken)
                {
                    portYIELD_FROM_ISR();
                }
            } },
        FALLING);

    watch->power->setPowerOutPut(AXP202_EXTEN, AXP202_OFF);
    watch->power->setPowerOutPut(AXP202_DCDC2, AXP202_OFF);
    watch->power->setPowerOutPut(AXP202_LDO3, AXP202_OFF);
    watch->power->setPowerOutPut(AXP202_LDO4, AXP202_OFF);

    watch->power->adc1Enable(AXP202_BATT_VOL_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_ACIN_VOL_ADC1 | AXP202_ACIN_CUR_ADC1 |
                                 AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_APS_VOL_ADC1 | AXP202_TS_PIN_ADC1,
                             true);

    watch->power->enableIRQ(AXP202_ALL_IRQ, false);
    watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_PEK_LONGPRESS_IRQ, true);
    watch->power->clearIRQ();
    watch->power->setChargeControlCur(POWER_CHARGE_CURRENT);
}

void PowerHandler::HandlePowerInterupts()
{
    logger->LogTrace("In IRQ Interupt");
    watch->power->readIRQ();

    if (watch->power->isPEKLongtPressIRQ())
    {
        watch->shake();
        logger->LogTrace("LongPress");
        watch->power->clearIRQ();
        SetLightSleep();
    }
    else if (watch->power->isPEKShortPressIRQ())
    {
        watch->shake();
        logger->LogTrace("ShortPress");
        watch->power->clearIRQ();
    }

    watch->power->clearIRQ();
    xEventGroupClearBits(eventGroupHandle, IRQ_POWER_FLAG);
}

void PowerHandler::SetLightSleep()
{
    logger->LogTrace("Light Sleep");
    Serial.flush();
    sleepState = SleepState_LightSleep;
    watch->displaySleep();
    watch->powerOff();
    watch->setBrightness(0);
    watch->bl->off();
    setCpuFrequencyMhz(CPU_FREQ_MIN);
    watch->motor->adjust(0);
    // esp_sleep_enable_ext1_wakeup(GPIO_TOUCH_BMA, ESP_EXT1_WAKEUP_ANY_HIGH); // BMA
    // esp_sleep_enable_ext1_wakeup(GPIO_TOUCH, ESP_EXT1_WAKEUP_ALL_LOW);  // Touch
    // esp_sleep_enable_ext1_wakeup(GPIO_RTC, ESP_EXT1_WAKEUP_ALL_LOW);  // RTC
    xEventGroupClearBits(eventGroupHandle, 0xFF);
    watch->power->clearIRQ();
    esp_sleep_enable_ext1_wakeup(GPIO_POWER, ESP_EXT1_WAKEUP_ALL_LOW);
#ifdef SILENT_WAKE_ENABLE
    esp_sleep_enable_timer_wakeup(silentWakeTime);
#endif
    esp_light_sleep_start();
}

void PowerHandler::SetSilentWake()
{
    sleepState = SleepState_SilentAwake;
    logger->LogTrace("SilentWake");
    logger->LogTrace(watch->rtc->formatDateTime(PCF_TIMEFORMAT_HM));
    // do periodic checks
    Serial.flush();
    SetLightSleep();
}

void PowerHandler::SetWake()
{
    sleepState = SleepState_Awake;
    setCpuFrequencyMhz(CPU_FREQ_MAX);
    watch->power->clearIRQ();
    watch->displayWakeup();
    watch->setBrightness(255);
    logger->LogTrace("Woke");
}

void PowerHandler::HandleAwake()
{
    if (sleepState == SleepState_Awake)
        return;
    lastInteraction = millis();

    esp_sleep_wakeup_cause_t wakeReason = esp_sleep_get_wakeup_cause();
    switch (wakeReason)
    {
    case ESP_SLEEP_WAKEUP_EXT1:
        SetWake();
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        SetSilentWake();
        break;
    default:
        logger->LogWarning("Unhandled wake reason");
        break;
    }
}
void PowerHandler::DisplayTimeout()
{
#if SCREEN_ACTIVE_TIMEOUT_ENABLED
    logger->LogTrace("Screen time out");
    SetLightSleep();
#endif
}

BatteryData PowerHandler::GetBatteryData()
{
    BatteryData data;
    auto power = watch->power;
    data.isCharging = power->isChargeing();
    data.battChargeCurrent = power->getBattChargeCurrent();
    data.battDischargeCurrent = power->getBattDischargeCurrent();
    data.batteryPercentage = power->getBattPercentage();
    return data;
}