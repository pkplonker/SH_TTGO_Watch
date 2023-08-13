#pragma once
#include "config.h"
#include "Logger.h"
#include "HandlerBase.h"
#include "InteractionState.h"

class BMAHandler : public HandlerBase
{
public:
    BMAHandler(TTGOClass *watch, Logger *logger) : HandlerBase(watch, logger)
    {
        SetupBMA();
    }
    void HandleBMAInterupts();
    virtual void Loop() override;
    inline uint GetSteps() { return steps; }

private:
    void SetupBMA();

    uint steps = 0;
};