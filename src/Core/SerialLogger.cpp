#include "SerialLogger.h"

void SerialLogger::Log(const String& message, LogLevel logLevel)
{
    if (loglevel >= this->loglevel)
    {
        Serial.println(message);
    }
}

void SerialLogger::LogTrace(const String& message)
{
    SerialLogger::Log(message, Trace);
}

void SerialLogger::LogError(const String& message)
{
    SerialLogger::Log(message, Error);
}

void SerialLogger::LogWarning(const String& message)
{
    SerialLogger::Log(message, Error);
}
void SerialLogger::LogCritical(const String& message)
{
    SerialLogger::Log(message, Critical);
}
void SerialLogger::SetLogLevel(LogLevel level)
{
    SerialLogger::loglevel = level;
}
