// #include "Main.h"


// #define MULTICORE
// #if MULTICORE
// static const BaseType_t NUM_CORES = 2;
// #else
// static const BaseType_t NUM_CORES = 1;
// #endif

// TTGOClass *watch = nullptr;
// SerialLogger *logger = nullptr;
// Screen *currentScreen = nullptr;
// TaskHandle_t updateLoopHandle;
// //TaskHandle_t logTimeHandle;
// //TimerHandle_t clearScreenTimer;



// volatile bool powerInterruptFlag = false;

// void HandlePowerInterupts();
// void Update(void *paramater);
// void LogTime(void *paramater);
// void ClearScreen();
// //void ClearScreenTimerCallback(TimerHandle_t xTimer);

// void setup()
// {
//     Serial.begin(SERIAL_BAUD_RATE);
//     Serial.println("Begin");
//     logger = new SerialLogger();
//     logger->SetLogLevel(Trace);

//     watch = TTGOClass::getWatch();

//     watch->begin();
//     watch->tft->init();
//     TFTHelper::Init(watch->tft);
//     watch->openBL();
//     watch->setBrightness(255);
//     watch->bl->adjust(SCREEN_BRIGHTNESS);

//     // Setup power
//     watch->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);
//     watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ | AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_CHARGING_IRQ, true);

//     // Setup power interupts
//     pinMode(AXP202_INT, INPUT_PULLUP);
//     attachInterrupt(
//         AXP202_INT,
//         []()
//         { powerInterruptFlag = true; },
//         FALLING);
//     // core->power->setChargeControlCur(1800);
//     watch->power->clearIRQ();

//     watch->motor_begin();
//     // currentScreen = new TestScreen(watch, logger);
//     // currentScreen->SetActive(true);
//     // logger->LogTrace("Creating timer");
//     // clearScreenTimer = xTimerCreate(
//     //     "ClearScreenTimer",
//     //     pdMS_TO_TICKS(5000),
//     //     pdFALSE,
//     //     0,
//     //     ClearScreenTimerCallback);
//     // if (clearScreenTimer == NULL)
//     // {
//     //     logger->LogError("Failed to create ClearScreenTimer");
//     // }
//     // logger->LogTrace("Created timer");
//     logger->LogTrace("heap size" + xPortGetFreeHeapSize());
//     logger->LogTrace("maxPri " + configMAX_PRIORITIES);

//     logger->LogTrace("Creating tasks");

//     // if (xTaskCreatePinnedToCore(LogTime, "LogTime", 4096, NULL, 4, &logTimeHandle, NUM_CORES) != pdPASS)
//     // {
//     //     logger->LogError("Failed to create LogTime task");
//     // }
//     // else
//     // {
//     //     logger->LogError("Logtime created");
//     // }

// #if MULTICORE
//     if (xTaskCreatePinnedToCore(Update, "Update", 1024, NULL, 5, &updateLoopHandle, NUM_CORES) != pdPASS)
// #else
//     if (xTaskCreate(Update, "Update", 1024, NULL, 5, &updateLoopHandle) != pdPASS) // Note the change here
// #endif
//     {
//         logger->LogError("Failed to create Update task");
//     }
//     else
//     {
//         logger->LogError("update created");
//     }
//     logger->LogTrace("Created tasks");

//     ClearScreen();
//     logger->LogTrace("Setup complete");
// }

// void loop()
// {
//     // logger->LogTrace(watch->rtc->formatDateTime(PCF_TIMEFORMAT_HM));
//     // currentScreen->Update();
//     esp_task_wdt_reset();
// }

// void Update(void *paramater)
// {
//     for (;;)
//     {
//         // logger->LogTrace(watch->rtc->formatDateTime(PCF_TIMEFORMAT_HM));
//         if (powerInterruptFlag)
//             HandlePowerInterupts();

//         esp_task_wdt_reset();
//     }
// }
// void LogTime(void *paramter)
// {
//     for (;;)
//     {
//         logger->LogTrace(watch->rtc->formatDateTime(PCF_TIMEFORMAT_HM));
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }
// void HandlePowerInterupts()
// {
//     logger->LogTrace("In IRQ Interupt");
//     powerInterruptFlag = false;
//     watch->power->readIRQ();

//     // ClearScreen();

//     // if (watch->power->isVbusPlugInIRQ())
//     // {
//     //     TFTHelper::WriteTextWithBackgroundAlpha("Power Plug In", 25, 100, 3, BLACK);
//     // }
//     // if (watch->power->isVbusRemoveIRQ())
//     // {
//     //     TFTHelper::WriteTextWithBackgroundAlpha("Power Remove", 25, 100, 3, BLACK);
//     // }
//     // if (watch->power->isPEKShortPressIRQ())
//     // {
//     //     TFTHelper::WriteTextWithBackgroundAlpha("PowerKey Press Short", 25, 100, 3, BLACK);
//     // }
//     // if (watch->power->isPEKLongtPressIRQ())
//     // {
//     //     TFTHelper::WriteTextWithBackgroundAlpha("PowerKey Press Long", 25, 100, 3, BLACK);
//     // }
//     watch->power->clearIRQ();
//     // if (xTimerIsTimerActive(clearScreenTimer) != pdFALSE)
//     // {
//     //     xTimerStop(clearScreenTimer, 0);
//     // }
//     // xTimerStart(clearScreenTimer, 0);
// }

// void ClearScreen()
// {
//     logger->LogTrace("Clearing screen");
//     auto header = 30;
//     watch->tft->fillRect(0, 0, TFT_WIDTH, header, DARKGRAY);
//     watch->tft->fillRect(0, header, TFT_WIDTH, TFT_HEIGHT - header, RED);
//     TFTHelper::WriteTextWithBackgroundAlpha("Testing123", 0, 0, 4, BLACK);
// }

// // void ClearScreenTimerCallback(TimerHandle_t xTimer)
// // {
// //     ClearScreen();
// // }