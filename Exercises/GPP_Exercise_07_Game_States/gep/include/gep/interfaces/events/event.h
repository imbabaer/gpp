#pragma once

#include "gep/container/hashmap.h"
#include "gep/container/DynamicArray.h"
#include "gep/ReferenceCounting.h"
#include "gep/exception.h"

#include "gep/interfaces/events/eventId.h"
#include "gep/interfaces/events/eventUpdateFramework.h"

#include "gep/interfaces/scripting.h"
#include "gep/math3d/algorithm.h"

namespace gep
{
    struct EventResult
    {
        enum Enum
        {
            Handled,
            Ignored,
            Cancel
        };

        GEP_DISALLOW_CONSTRUCTION(EventResult);
    };

    template< typename T_EventData>
    class Event
    {
    public:
        typedef std::function<EventResult::Enum(T_EventData)> EventType;
        typedef Event<T_EventData> OwnType;
        typedef EventListenerId<OwnType> ListenerIdType;
        typedef DelayedEventId<OwnType> DelayedEventIdType;
        typedef std::function<void(OwnType&)> InitializerType;
        typedef std::function<void(OwnType&)> DestroyerType;

        struct CInfo
        {
            InitializerType initializer;
            DestroyerType destroyer;
            IUpdateFramework* updateFramework;
            IScriptingManager* scriptingManager;
            IAllocator* allocator;

            CInfo() :
                initializer(nullptr),
                destroyer(nullptr),
                updateFramework(nullptr),
                scriptingManager(nullptr),
                allocator(&g_stdAllocator)
            {
            }

            explicit CInfo(IAllocator* pAllocator) :
                initializer(nullptr),
                destroyer(nullptr),
                updateFramework(nullptr),
                scriptingManager(nullptr),
                allocator(pAllocator)
            {
            }
        };

    public:

        explicit Event(const CInfo& cinfo = CInfo()) :
            m_pAllocator(cinfo.allocator),
            m_id(EventId::generate()),
            m_triggerLevel(0),
            m_listeners(m_pAllocator),
            m_delayedEvents(m_pAllocator),
            m_callbackId_update(-1),
            m_onDestroy(cinfo.destroyer),
            m_pUpdateFramework(cinfo.updateFramework),
            m_pScriptingManager(cinfo.scriptingManager)
        {
            if (m_pUpdateFramework == nullptr) { m_pUpdateFramework = &EventUpdateFramework::instance(); }
            if (m_pScriptingManager == nullptr) { m_pScriptingManager = &EventScriptingManager::instance(); }

            if(cinfo.initializer) { cinfo.initializer(*this); }
        }

        ~Event()
        {
            GEP_ASSERT(m_triggerLevel == 0,
                "This event is being destroyed even though it is still triggering!",
                m_triggerLevel);

            if(m_onDestroy) { m_onDestroy(*this); }

            m_onDestroy = nullptr;
            stopUpdating();
            m_pUpdateFramework = nullptr;
            m_pScriptingManager = nullptr;
            m_delayedEvents.clear();
            m_listeners.clear();
        }

        inline ListenerIdType registerListener(const EventType& listener)
        {
            return registerListenerPriority(0, listener);
        }

        inline ListenerIdType registerListenerPriority(int16 priority, const EventType& listener)
        {
            ListenerWrapper wrapper;
            wrapper.id = ListenerIdType::generate();
            wrapper.priority = priority;
            wrapper.listener = listener;
            insertListener(wrapper);
            return wrapper.id;
        }

        inline ListenerIdType registerScriptListener(ScriptFunctionWrapper funcRef)
        {
            return registerScriptListenerPriority(0, funcRef);
        }

        inline ListenerIdType registerScriptListenerPriority(int16 priority, ScriptFunctionWrapper funcRef)
        {
            ListenerWrapper wrapper;
            wrapper.id = ListenerIdType::generate();
            wrapper.priority = priority;
            wrapper.scriptFunc = funcRef;
            insertListener(wrapper);
            return wrapper.id;
        }

        inline Result deregisterListener(ListenerIdType id)
        {
            if (m_triggerLevel > 0)
            {
                throw EventException("Cannot deregister listener while triggering!");
            }
            return removeListener(id);
        }

        inline EventResult::Enum trigger(T_EventData data)
        {
            TriggerCounter counter(m_triggerLevel);
            EventResult::Enum callResult = EventResult::Ignored;

            for (auto listener : m_listeners)
            {
                if(!listener) { continue; }

                callResult = call(listener, data);
                if (callResult == EventResult::Cancel)
                {
                    break;
                }
            }
            return callResult;
        }

        inline DelayedEventIdType delayedTrigger(float delayInSeconds, T_EventData data)
        {
            if (delayInSeconds <= 0.0f)
            {
                trigger(data);
                return DelayedEventIdType::invalidValue();
            }

            startUpdating();
            DelayedEvent delayedEvent;
            delayedEvent.remainingSeconds = delayInSeconds;
            delayedEvent.data = data;
            DelayedEventIdType delayedEventId(DelayedEventIdType::generate());
            m_delayedEvents[delayedEventId] = delayedEvent;
            return delayedEventId;
        }

        inline Result modifyDelayedEventTime(DelayedEventIdType id, float newTime)
        {
            Result result = FAILURE;

            m_delayedEvents.ifExists(id, [&](DelayedEvent& delayedEvent){
                delayedEvent.remainingSeconds = newTime;
                processDelayedEvent(delayedEvent);
                result = SUCCESS;
            });

            return result;
        }

        inline Result modifyDelayedEventData(DelayedEventIdType id, T_EventData newData)
        {
            Result result = FAILURE;
            m_delayedEvents.ifExists(id, [&](DelayedEvent& delayedEvent){
                delayedEvent.data = newData;
                result = SUCCESS;
            });

            return result;
        }

        inline Result removeDelayedEvent(DelayedEventIdType id)
        {
            return m_delayedEvents.remove(id);
        }

        inline EventId getId() const { return m_id; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(registerScriptListener, "registerListener")
            LUA_BIND_FUNCTION_NAMED(registerScriptListenerPriority, "registerListenerPriority")
            LUA_BIND_FUNCTION(deregisterListener)
            LUA_BIND_FUNCTION(trigger)
            LUA_BIND_FUNCTION(delayedTrigger)
            LUA_BIND_FUNCTION(modifyDelayedEventTime)
            LUA_BIND_FUNCTION(modifyDelayedEventData)
            LUA_BIND_FUNCTION(removeDelayedEvent)
            LUA_BIND_FUNCTION(getId)
        LUA_BIND_REFERENCE_TYPE_END

    private:

        template<bool B_isUsableInScript>
        struct ScriptCaller
        {
            inline static EventResult::Enum call(IScriptingManager*,
                                                 ScriptFunctionWrapper,
                                                 T_EventData&)
            {
                GEP_ASSERT(false, "Attempt to call a script function "
                    "with event data that is not usable in a script!",
                    typeid(T_EventData).name());
                return EventResult::Handled;
            }
        };
        template<>
        struct ScriptCaller<true>
        {
            inline static EventResult::Enum call(IScriptingManager* scripting,
                                                 ScriptFunctionWrapper funcRef,
                                                 T_EventData& data)
            {
                try
                {
                    return scripting->callFunction<EventResult::Enum, T_EventData>(funcRef, data);
                }
                catch (ScriptExecutionException& exception)
                {
                    GEP_ASSERT(false, "Error calling event listener in lua.", exception.what());
                    throw exception;
                }
            }
        };

        typedef ScriptCaller<IsBoundToScript<T_EventData>::value> ScriptCallerType;

        struct ListenerWrapper
        {
            ListenerIdType id;
            int16 priority;
            EventType listener;
            ScriptFunctionWrapper scriptFunc;

            operator bool() const { return id != ListenerIdType::invalidValue(); }
        };

        struct DelayedEvent
        {
            float remainingSeconds;
            T_EventData data;

            DelayedEvent() :
                remainingSeconds(-1.0f),
                data()
            {
            }
        };

        struct TriggerCounter
        {
            uint16& count;
            TriggerCounter(uint16& count) : count(count) { ++count; }
            ~TriggerCounter() { --count; }
        };

    private:

        IAllocator* m_pAllocator;
        EventId m_id;
        uint16 m_triggerLevel;
        DynamicArray<ListenerWrapper> m_listeners;
        Hashmap<DelayedEventIdType, DelayedEvent> m_delayedEvents;
        CallbackId m_callbackId_update;
        DestroyerType m_onDestroy;
        IUpdateFramework* m_pUpdateFramework;
        IScriptingManager* m_pScriptingManager;

        inline void startUpdating()
        {
            if (m_callbackId_update.id == -1)
            {
                auto callback = std::bind(&OwnType::updateDelayedEvents, this, std::placeholders::_1);
                m_callbackId_update = m_pUpdateFramework->registerUpdateCallback(callback);
            }
        }

        inline void stopUpdating()
        {
            if (m_callbackId_update.id != -1)
            {
                m_pUpdateFramework->deregisterUpdateCallback(m_callbackId_update);
                m_callbackId_update.id = -1;
            }
        }

        inline void updateDelayedEvents(float elapsedMilliseconds)
        {
            float elapsedSeconds = elapsedMilliseconds / 1000.0f;

            m_delayedEvents.removeWhere([&](DelayedEventIdType& id, DelayedEvent& delayedEvent){
                delayedEvent.remainingSeconds -= elapsedSeconds;
                return processDelayedEvent(delayedEvent);
            });

            // If there are no more delayed events, there is no need to update
            if (m_delayedEvents.count() == 0)
            {
                stopUpdating();
            }
        }

        /// \brief returns \c true if the delayed event is finished, \c false otherwise.
        inline bool processDelayedEvent(DelayedEvent& delayedEvent)
        {
            bool result = false;
            if (delayedEvent.remainingSeconds <= 0.0f)
            {
                trigger(delayedEvent.data);
                result = true;
            }
            return result;
        }

        inline void insertListener(ListenerWrapper& wrapper)
        {
            if (m_listeners.length() == 0)
            {
                m_listeners.append(wrapper);
                return;
            }

            // reverse iteration
            for (size_t index = m_listeners.length(); index > 0; --index)
            {
                auto& listener = m_listeners[index - 1];
                if (listener.priority <= wrapper.priority)
                {
                    m_listeners.insertAtIndex(index, wrapper);
                    return;
                }
            }
            m_listeners.insertAtIndex(0, wrapper);
        }

        inline Result removeListener(ListenerIdType id)
        {
            for (size_t index = 0; index < m_listeners.length(); ++index)
            {
                if (m_listeners[index].id == id)
                {
                    m_listeners.removeAtIndex(index);
                    return SUCCESS;
                }
            }
            return FAILURE;
        }

        inline EventResult::Enum call(ListenerWrapper& wrapper, T_EventData& data)
        {
            if (wrapper.listener)
            {
                return wrapper.listener(data);
            }
            else if(wrapper.scriptFunc.isValid())
            {
                return ScriptCallerType::call(m_pScriptingManager, wrapper.scriptFunc, data);
            }
            else
            {
                GEP_ASSERT(false, "Unexpected listener that has no valid actual listener.");
                return EventResult::Cancel;
            }
        }
    };

    template<>
    class Event<void>
    {
        // TODO Events of type 'void' are currently not supported!
    };
}
