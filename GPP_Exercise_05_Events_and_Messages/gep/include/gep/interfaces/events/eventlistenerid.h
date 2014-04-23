#pragma once

#include "gep/interfaces/events/eventId.h"
#include "gep/interfaces/scripting.h"

namespace gep
{
    template<typename T_Event>
    struct EventListenerId
    {
        static EventListenerId invalidValue()
        {
            static EventListenerId invalid(std::numeric_limits<uint16>::max());
            return invalid;
        }

        static EventListenerId generate()
        {
            static uint16 counter(0);
            return EventListenerId(++counter);
        }

        uint16 value;

        EventListenerId()
        {
            *this = invalidValue();
        }

        LUA_BIND_VALUE_TYPE_BEGIN
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(value)
        LUA_BIND_VALUE_TYPE_END

    private:
        EventListenerId(uint16 value) :
            value(value)
        {
        }
    };

    template<typename T_Event>
    inline bool operator == (const EventListenerId<T_Event>& lhs, const EventListenerId<T_Event>& rhs)
    {
        return lhs.value == rhs.value;
    }
}
