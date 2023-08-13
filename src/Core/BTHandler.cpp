#include "BTHandler.h"
#include "PowerHandler.h"
#include "BMAHandler.h"

BTHandler::BTHandler(TTGOClass *watch, Logger *logger, PowerHandler *powerHandler, BMAHandler *bmaHandler) : HandlerBase(watch, logger)
{
    this->powerHandler = powerHandler;
    this->bmaHandler = bmaHandler;
    InitBT();
}
void BTHandler::InitBT()
{
    // Initialize BLE and set device name
    NimBLEDevice::init(BT_NAME);
    NimBLEServer *btServer = NimBLEDevice::createServer();

    // Create a BLE service
    NimBLEService *btService = btServer->createService(SERVICE_UUID);

    // Create a writable BLE characteristic
    messageChar = btService->createCharacteristic(
        MESSAGE_UUID,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY);
    batteryChar = btService->createCharacteristic(
        BATTERY_PERCENTAGE_UUID,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY);
    stepsChar = btService->createCharacteristic(
        STEPS_UUID,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY);

    // Start the service
    btService->start();

    // Start advertising
    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(btService->getUUID());
    pAdvertising->start();

    Serial.println("Waiting for clients to connect...");
    btServer->setCallbacks(new BTServerCallbacks(logger));
}

void BTHandler::SendBatteryData()
{
    BatteryData data = powerHandler->GetBatteryData();

    uint8_t dataBytes[sizeof(data)];
    memcpy(dataBytes, &data, sizeof(data));
    batteryChar->setValue(dataBytes, sizeof(data));
    batteryChar->notify();
}

void BTHandler::Loop()
{
    if (NimBLEDevice::getServer()->getConnectedCount() > 0)
    {
        // logger->LogTrace("notifying");
        messageChar->setValue((String)millis());
        messageChar->notify();
        stepsChar->setValue((String)bmaHandler->GetSteps());
        stepsChar->notify();
        SendBatteryData();
    }
}