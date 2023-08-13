#pragma once
#include "config.h"
#include <Arduino.h>
#include "LilyGoWatch.h"
#include "freertos/FreeRTOS.h"

class HandlerBase
{
public:
    HandlerBase(TTGOClass *watch, Logger *logger)
    {
        this->logger = logger;
        this->watch = watch;
    }
    virtual void Loop() = 0;

protected:
    TTGOClass *watch = nullptr;
    Logger *logger = nullptr;
};
