// #include "Main.h"

// // #define MULTICORE
// #if MULTICORE
// static const BaseType_t NUM_CORES = 2;
// #else
// static const BaseType_t NUM_CORES = 1;
// #endif

// TTGOClass *watch = nullptr;
// SerialLogger *logger = nullptr;
// Screen *currentScreen = nullptr;
// uint32_t IRQFlags = 0;
// EventGroupHandle_t eventGroupHandle;

// bool sleepMode = false;

// void HandlePowerInterupts();
// void Update(void *paramater);
// void LogTime(void *paramater);
// void ClearScreen();
// void SetupPower();
// void SetupBMA();
// void HandleBMAInterupts();
// void LowPowerMode();
// void Wakeup();

// #define IRQ_POWER_FLAG _BV(1)
// #define IRQ_BMA_FLAG _BV(2)
// #define IRQ_TOUCH_FLAG _BV(3)
// void setup()
// {
//     esp_task_wdt_init(30, false);
//     Serial.begin(SERIAL_BAUD_RATE);
//     Serial.println("Begin");

//     esp_reset_reason_t reason = esp_reset_reason();
//     Serial.printf("Reset reason: %d\n", reason);

//     logger = new SerialLogger();
//     logger->SetLogLevel(Trace);
//     watch = TTGOClass::getWatch();

//     watch->begin();
//     watch->tft->init();
//     TFTHelper::Init(watch->tft);
//     watch->openBL();
//     watch->setBrightness(255);
//     watch->bl->adjust(SCREEN_BRIGHTNESS);
//     watch->tft->fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, WHITE);
//     // Setup power

//     eventGroupHandle = xEventGroupCreate();
//     SetupPower();
//     WiFi.disconnect(true);
//     WiFi.mode(WIFI_OFF);

//     watch->motor_begin();
//     // ClearScreen();
//     SetupBMA();
//     pinMode(TOUCH_INT, INPUT);

//     logger->LogTrace("Setup complete");
// }
// const TickType_t xMaxWait = pdMS_TO_TICKS(100); // wait for 100ms

// void loop()
// {
//     EventBits_t uxBits;

//     uxBits = xEventGroupWaitBits(
//         eventGroupHandle,
//         IRQ_POWER_FLAG | IRQ_BMA_FLAG,
//         pdTRUE,
//         pdFALSE,
//         xMaxWait);

//     if ((uxBits & IRQ_POWER_FLAG) == IRQ_POWER_FLAG)
//     {
//         HandlePowerInterupts();
//         xEventGroupClearBits(eventGroupHandle, IRQ_POWER_FLAG);
//     }

//     if ((uxBits & IRQ_BMA_FLAG) == IRQ_BMA_FLAG)
//     {
//         HandleBMAInterupts();
//         xEventGroupClearBits(eventGroupHandle, IRQ_BMA_FLAG);
//     }
// }
// void HandleBMAInterupts()
// {
//     logger->LogTrace("In BMA IRQ Interupt");
//     watch->bma->readInterrupt();
//     watch->tft->setTextColor(BLACK, GREEN);
//     watch->tft->textsize = 3;

//     if (watch->bma->isDoubleClick())
//     {
//         watch->tft->drawString("Doubletap", 0, 120);
//         logger->LogTrace("Doubletap");
//         watch->shake();
//         Wakeup();
//     }
//     if (watch->bma->isTilt())
//     {
//         watch->tft->drawString("Tilt", 0, 150);
//         logger->LogTrace("Tilt");
//         watch->shake();
//     }
//     if (watch->bma->isStepCounter())
//     {
//         watch->tft->drawString("Step", 0, 180);
//         logger->LogTrace("Step");
//         watch->shake();
//     }
// }
// void HandlePowerInterupts()
// {
//     logger->LogTrace("In IRQ Interupt");
//     watch->power->readIRQ();
//     watch->tft->setTextColor(BLACK, GREEN);
//     watch->tft->textsize = 3;

//     if (watch->power->isPEKLongtPressIRQ())
//     {
//         watch->tft->drawString("PowerKey Press Long", 0, 80);
//         logger->LogTrace("LongPress");
//         watch->power->clearIRQ();
//         LowPowerMode();
//     }
//     else if (watch->power->isPEKShortPressIRQ())
//     {
//         watch->tft->drawString("PowerKey Press Short", 0, 40);
//         logger->LogTrace("ShortPress");
//         Wakeup();
//     }

//     watch->power->clearIRQ();
// }

// void LowPowerMode()
// {
//     sleepMode = true;

//     logger->LogTrace("Entering low power mode");
//     Serial.flush();

//     watch->closeBL();
//     watch->bma->enableStepCountInterrupt(false);
//     watch->displaySleep();
//     setCpuFrequencyMhz(CPU_FREQ_MIN);
//     watch->powerOff();
//     watch->power->enableIRQ(AXP202_ALL_IRQ, false);
//     watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
//     watch->power->clearIRQ();

//     gpio_wakeup_enable((gpio_num_t)AXP202_INT, GPIO_INTR_LOW_LEVEL);
//     gpio_wakeup_enable((gpio_num_t)BMA423_INT1, GPIO_INTR_HIGH_LEVEL);
//     esp_sleep_enable_gpio_wakeup();
//     esp_light_sleep_start();
// }

// void Wakeup()
// {
//     logger->LogTrace("W");
//     if (sleepMode)
//     {
//         // logger->LogTrace("WW");
//         // Serial.flush();
//         setCpuFrequencyMhz(CPU_FREQ_MAX);
//         watch->openBL();
//         watch->bma->enableStepCountInterrupt(true);
//         watch->displayWakeup();
//         sleepMode = false;
//     }
// }
// void ClearScreen()
// {
//     logger->LogTrace("Clearing screen");
//     auto header = 30;
//     watch->tft->fillRect(0, 0, TFT_WIDTH, header, DARKGRAY);
//     watch->tft->fillRect(0, header, TFT_WIDTH, TFT_HEIGHT - header, RED);
//     TFTHelper::WriteTextWithBackgroundAlpha("Testing123", 0, 0, 4, BLACK);
// }
// void SetupPower()
// {
//     pinMode(AXP202_INT, INPUT_PULLUP);
//     attachInterrupt(
//         AXP202_INT, [] {
//           BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//         if (xEventGroupSetBitsFromISR(eventGroupHandle, IRQ_POWER_FLAG, &xHigherPriorityTaskWoken) == pdPASS)
//         {
//             if (xHigherPriorityTaskWoken)
//             {
//                 portYIELD_FROM_ISR();
//             }
//         } 
//         }, FALLING);

//     watch->power->setPowerOutPut(AXP202_EXTEN, AXP202_OFF);
//     watch->power->setPowerOutPut(AXP202_DCDC2, AXP202_OFF);
//     watch->power->setPowerOutPut(AXP202_LDO3, AXP202_OFF);
//     watch->power->setPowerOutPut(AXP202_LDO4, AXP202_OFF);

//     watch->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

//     watch->power->enableIRQ(AXP202_ALL_IRQ, false);

//     watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_PEK_LONGPRESS_IRQ, true);
//     watch->power->clearIRQ();
// }

// void SetupBMA()
// {
//     Acfg cfg;
//     cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
//     cfg.range = BMA4_ACCEL_RANGE_2G;
//     cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
//     cfg.perf_mode = BMA4_CONTINUOUS_MODE;

//     watch->bma->accelConfig(cfg);

//     watch->bma->enableAccel();

//     pinMode(BMA423_INT1, INPUT);
//     attachInterrupt(
//         BMA423_INT1, []
//         {BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//     if (xEventGroupSetBitsFromISR(eventGroupHandle, IRQ_BMA_FLAG, &xHigherPriorityTaskWoken) == pdPASS)
//     {
//         if (xHigherPriorityTaskWoken)
//         {
//             portYIELD_FROM_ISR();
//         }
//     } },
//         RISING);

//     watch->bma->enableFeature(BMA423_STEP_CNTR, true);
//     watch->bma->enableFeature(BMA423_TILT, true);
//     watch->bma->enableFeature(BMA423_WAKEUP, true);
//     watch->bma->resetStepCounter();
//     watch->bma->enableStepCountInterrupt();
//     watch->bma->enableTiltInterrupt();
//     watch->bma->enableWakeupInterrupt();
// }