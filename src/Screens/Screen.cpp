#include "screen.h"

Screen::Screen(TTGOClass *watch, Logger *logger)
{
    this->watch = watch;
    this->logger = logger;
    logger->LogTrace("Contructed Screen");
    watch->tft->init();
}