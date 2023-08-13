#pragma once
#include "Screen.h"
#include "Graphics/Colors.h"
#include "Graphics/TFTHelper.h"

class TestScreen : public Screen
{
public:
    TestScreen(TTGOClass *watch, Logger *logger);
    static void event_handler(lv_obj_t *obj, lv_event_t event);

private:
    static TestScreen *instance;
};