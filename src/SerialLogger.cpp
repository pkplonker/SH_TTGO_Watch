#include "SerialLogger.h"

void SerialLogger::Log(String message, LogLevel logLevel)
{
    if (loglevel >= this->loglevel)
    {
        Serial.println(message);
    }
}

void SerialLogger::LogTrace(String message)
{
    SerialLogger::Log(message, Trace);
}

void SerialLogger::LogError(String message)
{
    SerialLogger::Log(message, Error);
}

void SerialLogger::LogWarning(String message)
{
    SerialLogger::Log(message, Error);
}
void SerialLogger::LogCritical(String message)
{
    SerialLogger::Log(message, Critical);
}
void SerialLogger::SetLogLevel(LogLevel level)
{
    SerialLogger::loglevel = level;
}
