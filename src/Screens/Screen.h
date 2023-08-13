#pragma once
#include <Arduino.h>
#include <LilyGoWatch.h>
#include "Config.h"
#include "Core/Logger.h"
#include "../../TTGO_TWatch_Library-1.4.3/src/lvgl/lvgl.h"
#include "../Core/InteractionState.h"

class Screen
{

public:
    Screen(TTGOClass *watch, Logger *logger);
    virtual ~Screen() = default;
protected:
    TTGOClass *watch = nullptr;
    Logger *logger = nullptr;
    bool isActive = false;
};