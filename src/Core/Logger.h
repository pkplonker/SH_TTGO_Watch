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
    virtual void Log(const String& message, LogLevel logLevel = Trace) = 0;
    virtual void LogTrace(const String& message) = 0;
    virtual void LogWarning(const String& message) = 0;
    virtual void LogError(const String& message) = 0;
    virtual void LogCritical(const String& message) = 0;
    virtual void SetLogLevel(LogLevel level) = 0;

private:
    LogLevel loglevel;
};