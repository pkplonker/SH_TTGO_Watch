#include "Graphics/TFTHelper.h"
#include "TFTHelper.h"

TFT_eSPI *TFTHelper::tft = nullptr;

void TFTHelper::WriteTextWithBackgroundAlpha(const String &text, int32_t x, int32_t y, uint8_t fontSize, int32_t color)
{
    tft->setTextFont(fontSize);
    tft->textcolor = color;
    tft->setTextColor(color);
    tft->drawString(text, x, y);
}
void TFTHelper::Init(TFT_eSPI *t)
{
    tft = t;
}
