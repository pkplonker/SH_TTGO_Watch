#include "Main.h"

void setup()
{
    Serial.begin(115200);
    logger = new SerialLogger();
    logger->SetLogLevel(Trace);
    logger->LogTrace("Setup started");

    watch = TTGOClass::getWatch();
    watch->begin();
    watch->openBL();
    watch->bl->adjust(SCREEN_BRIGHTNESS);
    watch->lvgl_begin();
    lastInteraction = millis();
    auto currentScreen = new TestScreen(watch, logger);
    powerHandler = new PowerHandler(watch, logger);
    bmaHandler = new BMAHandler(watch, logger);
    bthandler = new BTHandler(watch, logger, powerHandler, bmaHandler);

    watch->motor_begin();
    watch->rtc->check();
    logger->LogTrace("Setup complete");
}

void loop()
{
    powerHandler->Loop();
    bmaHandler->Loop();
    bthandler->Loop();
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
            powerHandler->HandlePowerInterupts();
            // logger->LogTrace("power");
            watch->power->clearIRQ();
        }
        if ((bits & IRQ_BMA_FLAG) == IRQ_BMA_FLAG)
        {
            bmaHandler->HandleBMAInterupts();
            // logger->LogTrace("BMA");
        }
        xEventGroupClearBits(eventGroupHandle, 0xFF);
        lastInteraction = millis();
    }
    lv_task_handler();
}
