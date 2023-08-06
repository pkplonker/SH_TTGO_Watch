#include "Main.h"
#define GPIO_POWER ((uint64_t)1 << 35)
#define GPIO_RTC ((uint64_t)1 << 37)
#define GPIO_TOUCH ((uint64_t)1 << 38)
#define GPIO_BMA ((uint64_t)1 << 39)

#include "config.h"



void setup()
{
    Serial.begin(115200);
    Serial.println("start");
    watch = TTGOClass::getWatch();
    esp_reset_reason_t reason = esp_reset_reason();
    Serial.printf("Reset reason: %d\n", reason);
    eventGroupHandle = xEventGroupCreate();

    logger = new SerialLogger();
    logger->SetLogLevel(Trace);
    watch->begin();
    watch->openBL();
    watch->tft->fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, WHITE);
    SetupPower();
    SetupBMA();
}
bool isAsleep = false;
int period = 1000;
unsigned long lastTime = 0;
void loop()
{
    if (isAsleep)
    {
        Wake();
    }
    EventBits_t bits;

    bits = xEventGroupWaitBits(
        eventGroupHandle,
        IRQ_POWER_FLAG | IRQ_BMA_FLAG,
        pdTRUE,
        pdFALSE,
        xMaxWait);

    if ((bits & IRQ_POWER_FLAG) == IRQ_POWER_FLAG)
    {
        HandlePowerInterupts();
        xEventGroupClearBits(eventGroupHandle, IRQ_POWER_FLAG);
        Serial.println("power");
        watch->power->clearIRQ();
    }

    if ((bits & IRQ_BMA_FLAG) == IRQ_BMA_FLAG)
    {
        HandleBMAInterupts();
        xEventGroupClearBits(eventGroupHandle, IRQ_BMA_FLAG);
        Serial.println("BMA");
    }
    xEventGroupClearBits(eventGroupHandle, 0xFF);
    if (millis() >= lastTime + period)
    {
        lastTime = millis();
        Serial.println("Loop");
    }
}

void Sleepmode()
{
    Serial.println("Sleeping");
    Serial.flush();
    isAsleep = true;
    watch->displaySleep();
    watch->powerOff();
    watch->bl->off();
    // esp_sleep_enable_ext1_wakeup(GPIO_TOUCH_BMA, ESP_EXT1_WAKEUP_ANY_HIGH); // BMA
    // esp_sleep_enable_ext1_wakeup(GPIO_TOUCH, ESP_EXT1_WAKEUP_ALL_LOW);  // Touch
    // esp_sleep_enable_ext1_wakeup(GPIO_RTC, ESP_EXT1_WAKEUP_ALL_LOW);  // RTC
    esp_sleep_enable_ext1_wakeup(GPIO_POWER, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_light_sleep_start();
}

void Wake()
{
    isAsleep = false;
    watch->power->clearIRQ();
    watch->displayWakeup();
    watch->setBrightness(255);
    watch->tft->fillRect(0, 0, 240, 240, TFT_RED);
    Serial.println("Woke");
    Serial.flush();
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
        Sleepmode();
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