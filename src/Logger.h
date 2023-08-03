#pragma once
#include <Arduino.h>
enum LogLevel
{
    Trace,
    Warning,
    Error,
    Critical
};

class Logger
{
public:
    virtual void Log(String message, LogLevel logLevel = Trace) = 0;
    virtual void LogTrace(String message) = 0;
    virtual void LogWarning(String message) = 0;
    virtual void LogError(String message) = 0;
    virtual void LogCritical(String message) = 0;
    virtual void SetLogLevel(LogLevel level) = 0;

private:
    LogLevel loglevel;
};