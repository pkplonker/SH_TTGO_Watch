#include "BMAHandler.h"

void BMAHandler::Loop()
{
}
void BMAHandler::SetupBMA()
{
    Acfg cfg;
    cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
    cfg.range = BMA4_ACCEL_RANGE_2G;
    cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;
    cfg.perf_mode = BMA4_CONTINUOUS_MODE;

    watch->bma->accelConfig(cfg);
    watch->bma->enableAccel();

    pinMode(BMA423_INT1, INPUT);

    attachInterrupt(
        BMA423_INT1, []
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            if (xEventGroupSetBitsFromISR(eventGroupHandle, IRQ_BMA_FLAG, &xHigherPriorityTaskWoken) == pdPASS)
            {
                if (xHigherPriorityTaskWoken)
                {
                    portYIELD_FROM_ISR();
                }
            } },
        RISING);

    watch->bma->enableFeature(BMA423_STEP_CNTR, true);
    watch->bma->enableFeature(BMA423_TILT, true);
    watch->bma->enableFeature(BMA423_WAKEUP, true);
    watch->bma->resetStepCounter();
    watch->bma->enableStepCountInterrupt();
    watch->bma->enableTiltInterrupt();
    watch->bma->enableWakeupInterrupt();
}

void BMAHandler::HandleBMAInterupts()
{
    logger->LogTrace("In BMA IRQ Interupt");
    watch->bma->readInterrupt();

    if (watch->bma->isDoubleClick())
    {
        logger->LogTrace("Doubletap");
        watch->shake();
    }
    if (watch->bma->isTilt())
    {
        logger->LogTrace("Tilt");
        watch->shake();
    }
    if (watch->bma->isStepCounter())
    {
        logger->LogTrace("Step");
        watch->shake();
        steps++;
    }
    xEventGroupClearBits(eventGroupHandle, IRQ_BMA_FLAG);
}
