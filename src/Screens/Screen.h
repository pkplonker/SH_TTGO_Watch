#pragma once
#include <Arduino.h>
#include "LilyGoWatch.h"
#include "Config.h"
#include "Core/Logger.h"

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