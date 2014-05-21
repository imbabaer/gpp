#include "stdafx.h"
#include "gepimpl/subsystems/logging.h"
#include <stdarg.h>
#include <stdio.h>

gep::Logging::Logging()
{
}

gep::Logging::~Logging()
{
    // avoid virtual function call in destructor
    this->gep::Logging::logMessage("logging system shutdown");
}

void gep::Logging::logMessage(const char* fmt, ...)
{
    char buffer[2048];
    va_list argptr;
    va_start(argptr, fmt);
    vsprintf_s(buffer, fmt, argptr);
    va_end(argptr);
    ScopedLock<Mutex> lock(m_sinkMutex);
    for(ILogSink* sink : m_sinks)
    {
        sink->take(LogChannel::message, buffer);
    }
}

void gep::Logging::logWarning(const char* fmt, ...)
{
    char buffer[2048];
    va_list argptr;
    va_start(argptr, fmt);
    vsprintf_s(buffer, fmt, argptr);
    va_end(argptr);
    ScopedLock<Mutex> lock(m_sinkMutex);
    for (ILogSink* sink : m_sinks)
    {
        sink->take(LogChannel::warning, buffer);
    }
}

void gep::Logging::logError(const char* fmt, ...)
{
    char buffer[2048];
    va_list argptr;
    va_start(argptr, fmt);
    vsprintf_s(buffer, fmt, argptr);
    va_end(argptr);
    ScopedLock<Mutex> lock(m_sinkMutex);
    for(ILogSink* sink : m_sinks)
    {
        sink->take(LogChannel::error, buffer);
    }
}

void gep::Logging::registerSink(ILogSink* pSink)
{
    ScopedLock<Mutex> lock(m_sinkMutex);
    m_sinks.append(pSink);
}

void gep::Logging::deregisterSink(ILogSink* pSink)
{
    ScopedLock<Mutex> lock(m_sinkMutex);
    size_t i=0;
    for(; i<m_sinks.length(); i++)
    {
        if(m_sinks[i] == pSink)
            break;
    }
    if(i < m_sinks.length())
    {
        m_sinks.removeAtIndex(i);
    }
}

void gep::Logging::logMessageUnformatted(const char* message)
{
    ScopedLock<Mutex> lock(m_sinkMutex);
    for(ILogSink* sink : m_sinks)
    {
        sink->take(LogChannel::message, message);
    }
}

void gep::Logging::logWarningUnformatted(const char* message)
{
    ScopedLock<Mutex> lock(m_sinkMutex);
    for(ILogSink* sink : m_sinks)
    {
        sink->take(LogChannel::message, message);
    }
}

void gep::Logging::logErrorUnformatted(const char* message)
{
    ScopedLock<Mutex> lock(m_sinkMutex);
    for(ILogSink* sink : m_sinks)
    {
        sink->take(LogChannel::message, message);
    }
}

void gep::ConsoleLogSink::take(LogChannel channel, const char* msg)
{
    const char* channelName = "Unkown: ";
    WORD color = 0x07; //white
    switch(channel)
    {
    case LogChannel::message:
        channelName = "";
        break;
    case LogChannel::warning:
        channelName = "Warning: ";
        color = 0x0E; //yellow
        break;
    case LogChannel::error:
        color = 0x0C; // red
        channelName = "Error: ";
        break;
    default:
        GEP_ASSERT(false, "unhandeled log channel value");
        break;
    }
    if(color != 0x07)
        SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), color);
    printf("%s%s\n", channelName, msg);
    if(color != 0x07)
        SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), 0x07); //back to white
}

gep::FileLogSink::FileLogSink(const char* filename, bool autoFlush) :
    m_autoFlush(autoFlush)
{
    m_LogFile.open(filename);
}

gep::FileLogSink::~FileLogSink()
{
    m_LogFile.close();
}

void gep::FileLogSink::take(LogChannel channel, const char* msg)
{
    const char* channelName = "Unkown: ";
    switch(channel)
    {
    case LogChannel::message:
        channelName = "";
        break;
    case LogChannel::warning:
        channelName = "Warning: ";
        break;
    case LogChannel::error:
        channelName = "Error: ";
        break;
    default:
        GEP_ASSERT(false, "unhandeled log channel value");
        break;
    }
    m_LogFile << channelName << msg << '\n';

    if(m_autoFlush)
    {
        // If we don't flush immediately, all logging information is lost when the program crashes!
        m_LogFile.flush();
    }
}

