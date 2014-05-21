#include "stdafx.h"
#include "gep/utils.h"

std::string gep::format(const char* fmt, ...)
{
    char buffer[1024];
    va_list argptr;
    va_start(argptr, fmt);
    int charsNeeded = vsnprintf_s(buffer, GEP_ARRAY_SIZE(buffer)-1, fmt, argptr);
    if(charsNeeded > GEP_ARRAY_SIZE(buffer)-1)
    {
        char* largeBuffer = new char[charsNeeded+1];
        vsprintf_s(largeBuffer, charsNeeded+1, fmt, argptr);
        va_end(argptr);
        auto result = std::string(largeBuffer);
        delete(largeBuffer);
        return result;
    }
    return std::string(buffer);
}
