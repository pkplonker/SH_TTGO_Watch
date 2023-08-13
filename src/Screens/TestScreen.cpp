#include "TestScreen.h"

TestScreen::TestScreen(TTGOClass *watch, Logger *logger) : Screen(watch, logger)
{
    logger->Log("Testscreen contructor");
    lv_obj_t *text = lv_label_create(lv_scr_act(), NULL);
    
    lv_label_set_text(text, "T-Watch");
    lv_obj_align(text, NULL, LV_ALIGN_CENTER, 0, 0);
}
