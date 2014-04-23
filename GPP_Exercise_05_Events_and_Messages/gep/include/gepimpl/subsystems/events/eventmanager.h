#pragma once

#include "gep/interfaces/events/eventManager.h"
#include "gep/interfaces/subsystem.h"

namespace gep
{
    class GlobalEventManager : public IEventManager
    {
        Event<float> m_update;
    public:
        GlobalEventManager() :
            m_update()
        {
        }

        virtual void initialize() override
        {
        }

        virtual void destroy() override
        {
        }

        virtual Event<float>* getUpdateEvent() override
        {
            return &m_update;
        }

        virtual void update(float elapsedTime) override
        {
            m_update.trigger(elapsedTime);
        }

    };
}
