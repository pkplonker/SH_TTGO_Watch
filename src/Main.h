#pragma once
#include <Arduino.h>
#include <LilyGoWatch.h>

#include "config.h"
#include "Core/SerialLogger.h"
#include "Core/PowerHandler.h"
#include "Core/BMAHandler.h"
#include "Core/BTHandler.h"
#include "Core/InteractionState.h"

TTGOClass *watch;
BTHandler *bthandler;
BMAHandler *bmaHandler;
PowerHandler *powerHandler;
SerialLogger *logger = nullptr;
const TickType_t xMaxWait = pdMS_TO_TICKS(100);


