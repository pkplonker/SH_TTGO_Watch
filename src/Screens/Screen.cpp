#include "screen.h"

Screen::Screen(TTGOClass *core, Logger* logger)
{
    this->core = core;
    this->logger = logger;
    logger->LogTrace("Contructed Screen");
}