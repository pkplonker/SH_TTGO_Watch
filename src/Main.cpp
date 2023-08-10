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
        NIMBLE_PROPERTY::READ 
            );
    batteryPercentChar = btService->createCharacteristic(
        BATTERY_PERCENTAGE_UUID,
        NIMBLE_PROPERTY::READ);

    messageChar->setCallbacks(new MyCallbacks());
    batteryPercentChar->setCallbacks(new MyCallbacks());

    // Start the service
    btService->start();

    // Start advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(btService->getUUID());
    pAdvertising->start();

    Serial.println("Waiting for clients to connect...");
}

void loop()
{
    HandleAwake();
    if (NimBLEDevice::getServer()->getConnectedCount() > 0)
    {
        messageChar->setValue(millis());
        messageChar->notify();
        batteryPercentChar->setValue(watch->power->getBattPercentage());
        batteryPercentChar->notify();
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

    watch->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

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