#define GPIO_POWER ((uint64_t)1 << 35)
#define GPIO_RTC ((uint64_t)1 << 37)
#define GPIO_TOUCH ((uint64_t)1 << 38)
#define GPIO_BMA ((uint64_t)1 << 39)

#include "config.h"

TTGOClass *watch;
bool irq = false;
void Sleepmode();
void Wake();

void setup()
{
    Serial.begin(115200);
    Serial.println("start");
    // Get TTGOClass instance
    watch = TTGOClass::getWatch();

    watch->begin();
    watch->openBL();

    watch->power->clearIRQ();
    watch->openBL();
    watch->setBrightness(255);
    watch->tft->fillRect(0, 0, 240, 240, TFT_WHITE);
    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(
        AXP202_INT, []
        { irq = true; },
        FALLING);

    watch->power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ,
                            true);
    watch->power->clearIRQ();
}
bool isAsleep = false;
int period = 1000;
unsigned long lastTime = 0;
void loop()
{
    if (irq && !isAsleep)
    {
        watch->power->clearIRQ();
        Sleepmode();
        irq = false;
    }
    else if (isAsleep)
    {
        Wake();
    }
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
    irq = false;

    Serial.println("Woke");
    Serial.flush();
}