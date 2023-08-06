#include "Main.h"

// #define MULTICORE
#if MULTICORE
static const BaseType_t NUM_CORES = 2;
#else
static const BaseType_t NUM_CORES = 1;
#endif

TTGOClass *watch = nullptr;
SerialLogger *logger = nullptr;
Screen *currentScreen = nullptr;
uint32_t IRQFlags = 0;
SemaphoreHandle_t IRQflagMutex;
EventGroupHandle_t isr_group = NULL;

bool sleepMode = false;

void HandlePowerInterupts();
void Update(void *paramater);
void LogTime(void *paramater);
void ClearScreen();
void SetFlag(unsigned int &flags, int bitPosition, bool value, int32_t waitDelay = portMAX_DELAY);
bool GetFlag(const unsigned int &flags, int bitPosition);
void SetupPower();
void SetupBMA();
void HandleBMAInterupts();
// void DeepSleep();
void LowPowerMode();
void Wakeup();
enum IRQFLAGS
{
    IRQ_POWER_FLAG = 0,
    IRQ_BMA_FLAG = 1,
    IRQ_TOUCH_FLAG = 2,
};

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println("Begin");
    esp_reset_reason_t reason = esp_reset_reason();
    Serial.printf("Reset reason: %d\n", reason);

    logger = new SerialLogger();
    logger->SetLogLevel(Trace);

    watch = TTGOClass::getWatch();

    watch->begin();
    watch->tft->init();
    TFTHelper::Init(watch->tft);
    watch->openBL();
    watch->setBrightness(255);
    watch->bl->adjust(SCREEN_BRIGHTNESS);
    watch->tft->fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, WHITE);
    // Setup power

    isr_group = xEventGroupCreate();
    SetupPower();

    watch->motor_begin();
    // ClearScreen();
    IRQflagMutex = xSemaphoreCreateMutex();
    SetupBMA();
    pinMode(TOUCH_INT, INPUT);

    logger->LogTrace("Setup complete");
}

void loop()
{
    if (GetFlag(IRQFlags, IRQ_POWER_FLAG))
        HandlePowerInterupts();
    if (GetFlag(IRQFlags, IRQ_BMA_FLAG))
        HandleBMAInterupts();
    vTaskDelay(pdMS_TO_TICKS(1));

    int16_t x, y;
    if (digitalRead(TOUCH_INT) == LOW)
    {
        Serial.println("PRESSED");
        if (watch->getTouch(x, y))
        {
            watch->tft->drawString("Touched", 0, 210);
        }
    }
}
void HandleBMAInterupts()
{
    logger->LogTrace("In BMA IRQ Interupt");
    SetFlag(IRQFlags, IRQ_BMA_FLAG, false);
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
void HandlePowerInterupts()
{
    logger->LogTrace("In IRQ Interupt");
    SetFlag(IRQFlags, IRQ_POWER_FLAG, false);
    watch->power->readIRQ();
    watch->tft->setTextColor(BLACK, GREEN);
    watch->tft->textsize = 3;

    if (watch->power->isPEKLongtPressIRQ())
    {
        watch->tft->drawString("PowerKey Press Long", 0, 80);
        logger->LogTrace("LongPress");
        watch->power->clearIRQ();
        LowPowerMode();
    }
    else if (watch->power->isPEKShortPressIRQ())
    {
        watch->tft->drawString("PowerKey Press Short", 0, 40);
        logger->LogTrace("ShortPress");
        Wakeup();
    }

    watch->power->clearIRQ();
}

void ClearScreen()
{
    logger->LogTrace("Clearing screen");
    auto header = 30;
    watch->tft->fillRect(0, 0, TFT_WIDTH, header, DARKGRAY);
    watch->tft->fillRect(0, header, TFT_WIDTH, TFT_HEIGHT - header, RED);
    TFTHelper::WriteTextWithBackgroundAlpha("Testing123", 0, 0, 4, BLACK);
}

void SetFlag(unsigned int &flags, int bitPosition, bool value, int32_t waitDelay)
{
    // logger->LogTrace("Attempting to set flag");
    if (xSemaphoreTake(IRQflagMutex, waitDelay) == pdTRUE)
    {
        if (value)
        {
            flags |= (1 << bitPosition);
        }
        else
        {
            flags &= ~(1 << bitPosition);
        }
        xSemaphoreGive(IRQflagMutex);
        // logger->LogTrace("set flag");
        return;
    }
    // logger->LogTrace("Failed to SetFlag");
}

bool GetFlag(const unsigned int &flags, int bitPosition)
{
    // logger->LogTrace("Attempting to get flag");
    bool value = false;
    if (xSemaphoreTake(IRQflagMutex, portMAX_DELAY) == pdTRUE)
    {
        value = (flags & (1 << bitPosition)) != 0;
        xSemaphoreGive(IRQflagMutex);
    }
    return value;
}

void SetupPower()
{
    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(
        AXP202_INT,
        []()
        { SetFlag(IRQFlags, IRQ_POWER_FLAG, true); },
        FALLING);
    // core->power->setChargeControlCur(1800);
    watch->power->clearIRQ();

    watch->power->setPowerOutPut(AXP202_EXTEN, AXP202_OFF);
    watch->power->setPowerOutPut(AXP202_DCDC2, AXP202_OFF);
    watch->power->setPowerOutPut(AXP202_LDO3, AXP202_OFF);
    watch->power->setPowerOutPut(AXP202_LDO4, AXP202_OFF);

    watch->power->adc1Enable(AXP202_VBUS_VOL_ADC1 |
                                 AXP202_VBUS_CUR_ADC1 |
                                 AXP202_BATT_CUR_ADC1 |
                                 AXP202_BATT_VOL_ADC1,
                             true);

    // RESET ALL TO AVOID ISSUES
    watch->power->enableIRQ(AXP202_ALL_IRQ, false);

    watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ
                                // | AXP202_VBUS_REMOVED_IRQ
                                // | AXP202_VBUS_CONNECT_IRQ
                                // | AXP202_CHARGING_IRQ
                                | AXP202_PEK_LONGPRESS_IRQ
                            // | AXP202_CHARGING_FINISHED_IRQ
                            // | AXP202_VBUS_CONNECT_IRQ
                            ,
                            true);
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
        { SetFlag(IRQFlags, IRQ_BMA_FLAG, true); },
        RISING);

    watch->bma->enableFeature(BMA423_STEP_CNTR, true);
    watch->bma->enableFeature(BMA423_TILT, true);
    watch->bma->enableFeature(BMA423_WAKEUP, true);
    watch->bma->resetStepCounter();
    watch->bma->enableStepCountInterrupt();
    watch->bma->enableTiltInterrupt();
    watch->bma->enableWakeupInterrupt();
}

// void DeepSleep()
// {
//     sleepMode = true;
//     logger->LogTrace("Sleeping");
//     watch->displaySleep();
//     watch->powerOff();
//     esp_sleep_enable_ext1_wakeup(GPIO_SEL_38, ESP_EXT1_WAKEUP_ALL_LOW); // Touch
//     // esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW); // Key
//     esp_deep_sleep_start();
// }

void LowPowerMode()
{
    sleepMode = true;
    logger->LogTrace("Entering low power mode");
    Serial.flush();
    watch->closeBL();
    watch->bma->enableStepCountInterrupt(false);
    watch->displaySleep();
    setCpuFrequencyMhz(CPU_FREQ_MIN);
    watch->powerOff();
    watch->power->enableIRQ(AXP202_ALL_IRQ, false);
    watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
    watch->power->clearIRQ();

    gpio_wakeup_enable((gpio_num_t)AXP202_INT, GPIO_INTR_LOW_LEVEL);
    gpio_wakeup_enable((gpio_num_t)BMA423_INT1, GPIO_INTR_HIGH_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    esp_light_sleep_start();
}

void Wakeup()
{
    if (sleepMode)
    {
        logger->LogTrace("Waking up");
        Serial.flush();
        setCpuFrequencyMhz(CPU_FREQ_MAX);
        watch->openBL();
        watch->bma->enableStepCountInterrupt(true);
        watch->displayWakeup();
        gpio_wakeup_disable((gpio_num_t)AXP202_INT);
        gpio_wakeup_disable((gpio_num_t)BMA423_INT1);
        sleepMode = false;
    }
}