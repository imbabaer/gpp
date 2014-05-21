#pragma once

#include "gep/container/hashmap.h"
#include "gep/container/DynamicArray.h"
#include "gep/ReferenceCounting.h"

#include "gep/interfaces/events/eventId.h"
#include "gep/interfaces/events/eventListenerId.h"

#include "gep/globalManager.h"
#include "gep/interfaces/updateFramework.h"
#include "gep/interfaces/scripting.h"

namespace gep
{
    template< typename T_EventData, bool B_EventDataIsScriptable = true>
    class Event
    {
    public:
        typedef std::function<void(T_EventData)> EventType;
        typedef Event<T_EventData, B_EventDataIsScriptable> OwnType;
        typedef EventListenerId<OwnType> ListenerIdType;
        typedef std::function<void(OwnType&)> InitializerType;
        typedef std::function<void(OwnType&)> DestroyerType;

        struct CInfo
        {
            InitializerType initializer;
            DestroyerType destroyer;
        };

    public:

        /// \brief Constructor
        explicit Event(const CInfo& cinfo = CInfo()) :
            m_id(EventId::generate()),
            m_listeners(),
            m_scriptListeners(),
            m_delayedEvents(),
            m_delayedEventsDeleteBuffer(),
            m_callbackId_update(-1),
            m_onDestroy(cinfo.destroyer)
        {
            if(cinfo.initializer) { cinfo.initializer(*this); }
        }

        ~Event()
        {
            if(m_onDestroy) { m_onDestroy(*this); }
            m_onDestroy = nullptr;
            m_listeners.clear();
            m_scriptListeners.clear();
            m_delayedEvents.clear();
            m_delayedEventsDeleteBuffer.clear();
            stopUpdating();
        }

        inline ListenerIdType registerListener(const EventType& listener)
        {
            auto id(ListenerIdType::generate());
            m_listeners[id] = listener;
            return id;
        }

        inline ListenerIdType registerScriptListener(ScriptFunctionWrapper funcRef)
        {
            auto id(ListenerIdType::generate());
            m_scriptListeners[id] = funcRef;
            return id;
        }

        inline Result deregisterListener(ListenerIdType id)
        {
            return m_listeners.remove(id);
        }

        inline Result deregisterScriptListener(ListenerIdType id)
        {
            return m_scriptListeners.remove(id);
        }

        inline Result trigger(T_EventData data)
        {
            for (auto listener : m_listeners.values())
            {
                if (listener) { listener(data); }
            }

            for (auto funcRef : m_scriptListeners.values())
            {
                if (funcRef.isValid()) 
                {
                    ScriptCaller<IsBoundToScript<T_EventData>::value>::call(funcRef, data);
                }
            }

            return SUCCESS;
        }

        inline void delayedTrigger(float delayInSeconds, T_EventData data)
        {
            if (epsilonCompare(delayInSeconds, 0.0f)) // if delay is close enough to 0.0
            {
                trigger(data);
            }
            else
            {
                startUpdating();
                DelayedEventData delayedData(delayInSeconds, data);
                m_delayedEvents.append(delayedData);
            }
        }

        inline EventId getId() const { return m_id; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(registerScriptListener, "registerListener")
            LUA_BIND_FUNCTION_NAMED(deregisterScriptListener, "deregisterListener")
            LUA_BIND_FUNCTION(trigger)
            LUA_BIND_FUNCTION(delayedTrigger)
        LUA_BIND_REFERENCE_TYPE_END

    private:

        template<bool isBoundToScript>
        struct ScriptCaller
        {
            inline static void call(ScriptFunctionWrapper funcRef, T_EventData data) { }
        };
        template<>
        struct ScriptCaller<true>
        {
            inline static void call(ScriptFunctionWrapper funcRef, T_EventData data)
            {
                g_globalManager.getScriptingManager()->callFunction<void, T_EventData>(funcRef, data);
            }
        };

        struct DelayedEventData
        {
            union {
                size_t index;
                float remainingSeconds;
            };
            T_EventData data;

            DelayedEventData(float remainingSeconds, T_EventData data) :
                remainingSeconds(remainingSeconds),
                data(data)
            {
            }
        };

        EventId m_id;
        Hashmap<ListenerIdType, EventType> m_listeners;
        Hashmap<ListenerIdType, ScriptFunctionWrapper> m_scriptListeners;
        DynamicArray<DelayedEventData> m_delayedEvents;
        DynamicArray<DelayedEventData> m_delayedEventsDeleteBuffer;
        CallbackId m_callbackId_update;
        DestroyerType m_onDestroy;

        void startUpdating()
        {
            if (m_callbackId_update.id == -1)
            {
                m_callbackId_update = g_globalManager.getUpdateFramework()->registerUpdateCallback(std::bind(&OwnType::updateDelayedEvents, this, std::placeholders::_1));
            }
        }

        void stopUpdating()
        {
            if (m_callbackId_update.id != -1)
            {
                g_globalManager.getUpdateFramework()->deregisterUpdateCallback(m_callbackId_update);
                m_callbackId_update.id = -1;
            }
        }

        void updateDelayedEvents(float elapsedMilliseconds)
        {
            float elapsedSeconds = elapsedMilliseconds / 1000.0f;

            // Collect all delayed events that shall be triggered now.
            size_t index = 0;
            for (auto& delayedEvent : m_delayedEvents)
            {
                delayedEvent.remainingSeconds -= elapsedSeconds;
                if (delayedEvent.remainingSeconds <= 0.0f)
                {
                    delayedEvent.index = index;
                    m_delayedEventsDeleteBuffer.append(delayedEvent);
                }
                ++index;
            }

            // Trigger and remove all delayed events
            for (auto& delayedEvent : m_delayedEventsDeleteBuffer)
            {
                trigger(delayedEvent.data);
                m_delayedEvents.removeAtIndex(delayedEvent.index);
            }
            m_delayedEventsDeleteBuffer.clear();

            // If there are no more delayed events, there is no need to update
            if (m_delayedEvents.length() == 0)
            {
                stopUpdating();
            }
        }
    };

    template<bool B_EventDataIsScriptable>
    class Event<void, B_EventDataIsScriptable>
    {
        // TODO Events of type 'void' are currently not supported!
    };
}
