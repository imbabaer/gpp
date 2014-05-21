#pragma once

#include "gep/interfaces/scripting.h"

namespace gep
{
    struct EventId
    {
        static EventId invalidValue()
        {
            static EventId invalid(std::numeric_limits<uint16>::max());
            return invalid;
        }

        static EventId generate()
        {
            static uint16 counter(0);
            return EventId(++counter);
        }

        uint16 value;

        EventId()
        {
            *this = invalidValue();
        }

        LUA_BIND_VALUE_TYPE_BEGIN
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(value)
        LUA_BIND_VALUE_TYPE_END

    private:
        EventId(uint16 value) : value(value) {}
    };

    static_assert(sizeof(EventId) == sizeof(uint16), "EventId has an incorrect size.");

    inline bool operator == (EventId lhs, EventId rhs)
    {
        return lhs.value == rhs.value;
    }
}
