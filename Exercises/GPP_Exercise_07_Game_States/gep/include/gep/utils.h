#pragma once

#include <string>
#include <varargs.h>

namespace gep
{
    GEP_API std::string format(GEP_PRINTF_FORMAT_STRING const char* fmt, ...);
}
