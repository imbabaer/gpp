#pragma once

#include "gep/interfaces/subsystem.h"

#include "gep/interfaces/events/event.h"

namespace gep
{
    class IEventManager : public ISubsystem
    {
    public:
        virtual ~IEventManager() = 0 {}

        virtual Event<float>* getUpdateEvent() = 0;
    };
}
