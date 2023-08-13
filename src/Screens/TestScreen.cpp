#include "TestScreen.h"

TestScreen* TestScreen::instance = nullptr;

void TestScreen::event_handler(lv_obj_t *obj, lv_event_t event)
{
    if (event == LV_EVENT_CLICKED)
    {
        Serial.printf("Clicked\n");
    }
    else if (event == LV_EVENT_VALUE_CHANGED)
    {
        Serial.printf("Toggled\n");
    }
    instance->watch->motor->onec();
    lastInteraction = millis();
}

TestScreen::TestScreen(TTGOClass *watch, Logger *logger) : Screen(watch, logger)
{
    instance = this;
    lv_obj_t *btn = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_pos(btn, 10, 10);
    lv_obj_set_size(btn, 120, 50);
    lv_obj_set_event_cb(btn, event_handler);
    lv_obj_t *label = lv_label_create(btn, NULL);
    lv_label_set_text(label, "Button");
    // lv_obj_center(label);
    lv_obj_align_mid(btn, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
}
