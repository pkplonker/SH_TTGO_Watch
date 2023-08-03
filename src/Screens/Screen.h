#pragma once
#include <Arduino.h>
#include "LilyGoWatch.h"
#include "Config.h"
#include "Core/Logger.h"

class Screen
{

public:
    Screen(TTGOClass *core, Logger* logger);
    virtual ~Screen() = default;
    virtual void Update() = 0;
    inline virtual void SetActive(bool state) { isActive = state; }

protected:
    TTGOClass* core = nullptr;
    Logger* logger = nullptr;
    bool isActive = false;
};