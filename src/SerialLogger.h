#pragma once
#include <Arduino.h>
#include "Logger.h"

class SerialLogger : public Logger
{
public:
    void Log(String message, LogLevel logLevel = Trace);
    void LogTrace(String message);
    void LogWarning(String message);
    void LogError(String message);
    void LogCritical(String message);
    void SetLogLevel(LogLevel level);

protected:
    LogLevel loglevel = Trace;
};