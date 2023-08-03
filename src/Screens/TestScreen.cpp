#include "TestScreen.h"

TestScreen::TestScreen(TTGOClass *core, Logger *logger) : Screen(core, logger)
{
    core->tft->init();
}

void TestScreen::Update()
{
    if (!isActive)
        return;

    logger->LogTrace("Updating test screen");
}

void TestScreen::SetActive(bool state)
{
    Screen::SetActive(state);
    auto header = 30;
    core->tft->fillRect(0, 0, TFT_WIDTH, header, DARKGRAY);
    core->tft->fillRect(0, header, TFT_WIDTH, TFT_HEIGHT - header, RED);
    TFTHelper::WriteTextWithBackground("Testing123", 0, 0, 4, BLACK);
}


