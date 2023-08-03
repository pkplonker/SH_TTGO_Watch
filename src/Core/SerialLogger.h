#pragma once
#include <Arduino.h>
#include "Logger.h"

class SerialLogger : public Logger
{
public:
    virtual void Log(const String& message, LogLevel logLevel = Trace) override;
    virtual void LogTrace(const String& message) override;
    virtual void LogWarning(const String& message) override;
    virtual void LogError(const String& message) override;
    virtual void LogCritical(const String& message) override;
    virtual void SetLogLevel(LogLevel level) override;

protected:
    LogLevel loglevel = Trace;
};