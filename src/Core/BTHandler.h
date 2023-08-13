#pragma once
#include <NimBLEDevice.h>
#include "Logger.h"

#include "config.h"
#include "HandlerBase.h"
#include "BMAHandler.h"
#include "BatteryData.h"

#define SERVICE_UUID "647a52e4-3279-41ac-9ed2-2a3c768be01A"
#define MESSAGE_UUID "e9c2e910-a206-4d9e-a2d4-49834e1ec45a"
#define BATTERY_PERCENTAGE_UUID "e9c2e910-a206-4d9e-a2d4-49834e1ec45b"
#define STEPS_UUID "e9c2e910-a206-4d9e-a2d4-49834e1ec45c"

class PowerHandler;
class BMAHandler;

class BTHandler : public HandlerBase
{
public:
    BTHandler(TTGOClass *watch, Logger *logger, PowerHandler *powerHandler, BMAHandler *bmaHandler);
    void InitBT();
    virtual void Loop() override;
    void SendBatteryData();

private:
    NimBLECharacteristic *messageChar = nullptr;
    NimBLECharacteristic *batteryChar = nullptr;
    NimBLECharacteristic *stepsChar = nullptr;
    PowerHandler *powerHandler = nullptr;
    BMAHandler *bmaHandler = nullptr;

    class BTServerCallbacks : public NimBLEServerCallbacks
    {
    public:
        BTServerCallbacks(Logger *logger){
            this->logger = logger;
        }
        void onConnect(NimBLEServer *pServer)
        {
            logger->LogTrace("Client connected");
        };

        void onDisconnect(NimBLEServer *pServer)
        {
            logger->LogTrace("Client disconnected");
        };

    private:
        Logger *logger = nullptr;
    };
};
