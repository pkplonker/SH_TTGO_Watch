#pragma once

#pragma pack(push, 1)
struct BatteryData
{
    bool isCharging;            // 1
    float battChargeCurrent;    // 4
    float battDischargeCurrent; // 4
    int batteryPercentage;      // 4
};
#pragma pack(pop)