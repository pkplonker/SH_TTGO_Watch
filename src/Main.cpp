#include "Main.h"
#include "config.h"

void setup()
{
    Serial.begin(115200);
    logger = new SerialLogger();
    logger->SetLogLevel(Trace);

    logger->LogTrace("Setup started");
    watch = TTGOClass::getWatch();
    eventGroupHandle = xEventGroupCreate();

    watch->begin();
    watch->openBL();
    watch->tft->fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, WHITE);
    SetupPower();
    SetupBMA();
    sleepState = SleepState_Awake;
    watch->rtc->check();
    lastInteraction = millis();
    watch->power->setChargeControlCur(POWER_CHARGE_CURRENT);
    Serial.println("Battery percentage: " + String(watch->power->getBattPercentage()));
    Serial.println("Battery discharge current: " + String(watch->power->getBattDischargeCurrent()));
    Serial.println("Battery charge current: " + String(watch->power->getBattChargeCurrent()));
    InitBT();
    logger->LogTrace("Setup complete");
}
void IRAM_ATTR onTimerInterrupt() {
    logger->Log("Test");
}
void InitBT()
{

    // Initialize BLE and set device name
    NimBLEDevice::init(BT_NAME);
    NimBLEServer *btServer = NimBLEDevice::createServer();

    // Create a BLE service
    NimBLEService *btService = btServer->createService(SERVICE_UUID);

    // Create a writable BLE characteristic
    messageChar = btService->createCharacteristic(
        MESSAGE_UUID,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY);
    batteryChar = btService->createCharacteristic(
        BATTERY_PERCENTAGE_UUID,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY);
    stepsChar = btService->createCharacteristic(
        STEPS_UUID,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY);

    // Start the service
    btService->start();

    // Start advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(btService->getUUID());
    pAdvertising->start();

    Serial.println("Waiting for clients to connect...");
    btServer->setCallbacks(new BTServerCallbacks());
}

void loop()
{
    HandleAwake();
    if (NimBLEDevice::getServer()->getConnectedCount() > 0)
    {
        //logger->LogTrace("notifying");
        messageChar->setValue((String)millis());
        messageChar->notify();
        stepsChar->setValue((String)steps);
        stepsChar->notify();
        SendBatteryData();
    }
    EventBits_t bits = xEventGroupWaitBits(
        eventGroupHandle,
        IRQ_POWER_FLAG | IRQ_BMA_FLAG,
        pdTRUE,
        pdFALSE,
        xMaxWait);

    if (bits)
    {
        if ((bits & IRQ_POWER_FLAG) == IRQ_POWER_FLAG)
        {
            HandlePowerInterupts();
            xEventGroupClearBits(eventGroupHandle, IRQ_POWER_FLAG);
            logger->LogTrace("power");
            watch->power->clearIRQ();
        }
        if ((bits & IRQ_BMA_FLAG) == IRQ_BMA_FLAG)
        {
            HandleBMAInterupts();
            xEventGroupClearBits(eventGroupHandle, IRQ_BMA_FLAG);
            logger->LogTrace("BMA");
        }
        xEventGroupClearBits(eventGroupHandle, 0xFF);
        if (millis() >= lastTime + period)
        {
            lastTime = millis();
            Serial.println("Battery percentage: " + String(watch->power->getBattPercentage()));
            Serial.println("Battery discharge current: " + String(watch->power->getBattDischargeCurrent()));
            Serial.println("Battery charge current: " + String(watch->power->getBattChargeCurrent()));
        }
        lastInteraction = millis();
    }

    if (millis() - lastInteraction > SCREEN_ACTIVE_TIME)
    {
        DisplayTimeout();
    }
}

void SendBatteryData() {
  // Fill the structure
  BatteryData data;
  auto power = watch->power;
  data.isCharging = power->isChargeing();
  data.isBatteryConnected = power->isBatteryConnect();
  data.acinVoltage = power->getAcinVoltage();
  data.acinCurrent = power->getAcinCurrent();
  data.vbusVoltage = power->getVbusVoltage();
  data.vbusCurrent = power->getVbusCurrent();
  data.temp = power->getTemp();
  data.battVoltage = power->getBattVoltage();
  data.battChargeCurrent = power->getBattChargeCurrent();
  data.battDischargeCurrent = power->getBattDischargeCurrent();
  data.battDischargeCurrent = power->getBattPercentage();

//   logger->Log("Charging: " + String(data.isCharging));
//   logger->Log("Battery Connected: " + String(data.isBatteryConnected));
//   logger->Log("ACIN Voltage: " + String(data.acinVoltage));
//   logger->Log("ACIN Current: " + String(data.acinCurrent));
//   logger->Log("VBUS Voltage: " + String(data.vbusVoltage));
//   logger->Log("VBUS Current: " + String(data.vbusCurrent));
//   logger->Log("Temperature: " + String(data.temp));
//   logger->Log("Battery Voltage: " + String(data.battVoltage));
//   logger->Log("Battery Charge Current: " + String(data.battChargeCurrent));
//   logger->Log("Battery Discharge Current: " + String(data.battDischargeCurrent));
//   logger->Log("Battery %: " + String(data.batteryPercentage));

logger->Log(String("Size of BatteryData: ") + sizeof(data));
  logger->Log(String("Offset of isCharging: ") + offsetof(BatteryData, isCharging));
  logger->Log(String("Offset of isBatteryConnected: ") + offsetof(BatteryData, isBatteryConnected));
  logger->Log(String("Offset of acinVoltage: ") + offsetof(BatteryData, acinVoltage));
  logger->Log(String("Offset of acinCurrent: ") + offsetof(BatteryData, acinCurrent));
  logger->Log(String("Offset of vbusVoltage: ") + offsetof(BatteryData, vbusVoltage));
  logger->Log(String("Offset of vbusCurrent: ") + offsetof(BatteryData, vbusCurrent));
  logger->Log(String("Offset of temp: ") + offsetof(BatteryData, temp));
  logger->Log(String("Offset of battVoltage: ") + offsetof(BatteryData, battVoltage));
  logger->Log(String("Offset of battChargeCurrent: ") + offsetof(BatteryData, battChargeCurrent));
  logger->Log(String("Offset of battDischargeCurrent: ") + offsetof(BatteryData, battDischargeCurrent));
  logger->Log(String("Offset of batteryPercentage: ") + offsetof(BatteryData, batteryPercentage));

  uint8_t dataBytes[sizeof(data)];
  memcpy(dataBytes, &data, sizeof(data));
  logger->Log(String("Size of bool: ") + sizeof(bool));
    logger->Log(String("Size of float: ") + sizeof(float));
    logger->Log(String("Size of int: ") + sizeof(int));
    logger->Log(String("Size of BatteryData: ") + sizeof(BatteryData));
  batteryChar->setValue(dataBytes, sizeof(data));
  batteryChar->notify();
}

void DisplayTimeout()
{
#if SCREEN_ACTIVE_TIMEOUT_ENABLED
    logger->LogTrace("Screen time out");
    Serial.println("Battery percentage: " + String(watch->power->getBattPercentage()));
    Serial.println("Battery discharge current: " + String(watch->power->getBattDischargeCurrent()));
    Serial.println("Battery charge current: " + String(watch->power->getBattChargeCurrent()));
    SetLightSleep();
#endif
}

void SetLightSleep()
{
    logger->LogTrace("Light Sleep");
    Serial.flush();
    sleepState = SleepState_LightSleep;
    watch->displaySleep();
    watch->powerOff();
    watch->setBrightness(0);
    watch->bl->off();
    setCpuFrequencyMhz(CPU_FREQ_MIN);

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

void SetSilentWake()
{
    sleepState = SleepState_SilentAwake;
    logger->LogTrace("SilentWake");
    logger->LogTrace(watch->rtc->formatDateTime(PCF_TIMEFORMAT_HM));
    // do periodic checks
    Serial.flush();
    SetLightSleep();
}

void SetWake()
{
    sleepState = SleepState_Awake;
    setCpuFrequencyMhz(CPU_FREQ_MAX);

    watch->power->clearIRQ();
    watch->displayWakeup();
    watch->setBrightness(255);
    watch->tft->fillRect(0, 0, 240, 240, TFT_RED);
    logger->LogTrace("Woke");
}

void SetupBMA()
{
    Acfg cfg;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;

    watch->bma->accelConfig(cfg);
    watch->bma->enableAccel();

    pinMode(BMA423_INT1, INPUT);

    attachInterrupt(
        BMA423_INT1, []
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            if (xEventGroupSetBitsFromISR(eventGroupHandle, IRQ_BMA_FLAG, &xHigherPriorityTaskWoken) == pdPASS)
            {
                if (xHigherPriorityTaskWoken)
                {
                    portYIELD_FROM_ISR();
                }
            } },
        RISING);

    watch->bma->enableFeature(BMA423_STEP_CNTR, true);
    watch->bma->enableFeature(BMA423_TILT, true);
    watch->bma->enableFeature(BMA423_WAKEUP, true);
    watch->bma->resetStepCounter();
    watch->bma->enableStepCountInterrupt();
    watch->bma->enableTiltInterrupt();
    watch->bma->enableWakeupInterrupt();
}

void SetupPower()
{
    pinMode(AXP202_INT, INPUT_PULLUP);

    attachInterrupt(
        AXP202_INT, []
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
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
    AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_APS_VOL_ADC1 | AXP202_TS_PIN_ADC1, true);
    watch->power->adc2Enable(AXP202_TEMP_MONITORING_ADC2 | AXP202_GPIO1_FUNC_ADC2 | AXP202_GPIO0_FUNC_ADC2, true);

    watch->power->enableIRQ(AXP202_ALL_IRQ, false);
    watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_PEK_LONGPRESS_IRQ, true);
    watch->power->clearIRQ();
}
void HandlePowerInterupts()
{
    logger->LogTrace("In IRQ Interupt");
    watch->power->readIRQ();
    watch->tft->setTextColor(BLACK, GREEN);
    watch->tft->textsize = 3;

    if (watch->power->isPEKLongtPressIRQ())
    {
        watch->shake();
        watch->tft->drawString("PowerKey Press Long", 0, 80);
        logger->LogTrace("LongPress");
        watch->power->clearIRQ();
        SetLightSleep();
    }
    else if (watch->power->isPEKShortPressIRQ())
    {
        watch->shake();
        watch->tft->drawString("PowerKey Press Short", 0, 40);
        logger->LogTrace("ShortPress");
        watch->power->clearIRQ();
    }

    watch->power->clearIRQ();
}

void HandleBMAInterupts()
{
    logger->LogTrace("In BMA IRQ Interupt");
    watch->bma->readInterrupt();
    watch->tft->setTextColor(BLACK, GREEN);
    watch->tft->textsize = 3;

    if (watch->bma->isDoubleClick())
    {
        watch->tft->drawString("Doubletap", 0, 120);
        logger->LogTrace("Doubletap");
        watch->shake();
    }
    if (watch->bma->isTilt())
    {
        watch->tft->drawString("Tilt", 0, 150);
        logger->LogTrace("Tilt");
        watch->shake();
    }
    if (watch->bma->isStepCounter())
    {
        watch->tft->drawString("Step", 0, 180);
        logger->LogTrace("Step");
        watch->shake();
        steps++;
    }
}

void HandleAwake()
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
        logger->LogWarning("Unhandle wake reason");
        break;
    }
}