#pragma once
#pragma once
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Colors.h"

class TFTHelper
{
public:
    static void WriteTextWithBackground(const String& text, int32_t x, int32_t y, uint8_t fontSize, int32_t color);
    static void Init(TFT_eSPI *t);
    static TFT_eSPI *tft;
};