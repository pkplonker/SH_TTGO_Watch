#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

extern unsigned long lastInteraction;
extern EventGroupHandle_t eventGroupHandle;