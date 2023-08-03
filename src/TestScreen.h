#pragma once
#include "Screen.h"

class TestScreen : public Screen
{
public:
    TestScreen(TTGOClass *core, Logger* logger) : Screen(core, logger){}
    virtual void Update() override;
    virtual void SetActive(bool state);
};