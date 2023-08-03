#pragma once
#include "Screen.h"
#include "Graphics/Colors.h"
#include "Graphics/TFTHelper.h"

class TestScreen : public Screen
{
public:
    TestScreen(TTGOClass *core, Logger *logger);
    virtual void Update() override;
    virtual void SetActive(bool state);
};