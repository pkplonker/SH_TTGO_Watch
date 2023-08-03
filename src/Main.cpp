#include "Main.h"
#include "SerialLogger.h"
#include "TestScreen.h"

TTGOClass *core = nullptr;
SerialLogger *logger = nullptr;
Screen *currentScreen = nullptr;

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println("Begin");
    logger = new SerialLogger();
    logger->SetLogLevel(Trace);

    core = TTGOClass::getWatch();

    core->begin();
    core->openBL();
    core->setBrightness(255);
    core->bl->adjust(SCREEN_BRIGHTNESS);

    core->tft->init();

    core->power->adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);
    core->power->setChargeControlCur(1800);
    core->power->clearIRQ();

    core->motor_begin();
    currentScreen = new TestScreen(core,logger);
    currentScreen->SetActive(true);
}

void loop()
{
    logger->LogTrace(core->rtc->formatDateTime(PCF_TIMEFORMAT_HM));
    currentScreen->Update();
}
