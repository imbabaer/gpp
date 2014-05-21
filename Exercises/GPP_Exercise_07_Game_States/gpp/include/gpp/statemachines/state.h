#pragma once

#include "gep/interfaces/scripting.h"

#include "gep/interfaces/events.h"
#include "gep/container/hashmap.h"
#include "gep/interfaces/updateFramework.h"
#include "gpp/dummyLogging.h"

namespace gep
{
    class IAllocator;
}

namespace gpp
{
    class StateMachine;

    class GPP_API State
    {
        friend class StateMachine;

    public:
        struct UpdateStepBehavior
        {
            enum Enum
            {
                Continue = 0,
                Leave,
                LeaveWithNoConditionChecks,
            };

            GEP_DISALLOW_CONSTRUCTION(UpdateStepBehavior);
        };
        typedef std::function<bool()> ConditionFunc_t;

        struct Transition
        {
            State* to;
            ConditionFunc_t condition;
        };
        typedef gep::DynamicArray<Transition> TransitionArray_t;

        class GPP_API EnterEventData
        {
            friend class State;
            friend class StateMachine;

        public:
            EnterEventData();

            State* getState();

            LUA_BIND_REFERENCE_TYPE_BEGIN
                LUA_BIND_FUNCTION(getState)
            LUA_BIND_REFERENCE_TYPE_END

        private:
            State* m_pCurrentState;
        };
        typedef gep::Event<EnterEventData*> EnterEvent_t;

        class GPP_API LeaveEventData
        {
            friend class State;
            friend class StateMachine;

        public:
            LeaveEventData();

            State* getCurrentState();
            void setNextState(State* pNext);
            void setNextState(const std::string& next);

            LUA_BIND_REFERENCE_TYPE_BEGIN
                LUA_BIND_FUNCTION(getCurrentState)
                LUA_BIND_FUNCTION_PTR(static_cast<void(LeaveEventData::*)(State*)>(&setNextState), "setNextState")
                LUA_BIND_FUNCTION_PTR(static_cast<void(LeaveEventData::*)(const std::string&)>(&setNextState), "setNextStateByName")
            LUA_BIND_REFERENCE_TYPE_END

        private:
            StateMachine* m_pCurrentStateMachine;
            State* m_pCurrentState;
            State* m_pNextState;
            bool m_noConditionChecks;
            TransitionArray_t* m_pTransitionArray;
        };
        typedef gep::Event<LeaveEventData*> LeaveEvent_t;

        class GPP_API UpdateEventData
        {
            friend class State;
            friend class StateMachine;
        public:

            UpdateEventData();

            State* getCurrentState();
            float getElapsedTime();
            State::UpdateStepBehavior::Enum getUpdateStepBehavior() const;
            void setUpdateStepBehavior(State::UpdateStepBehavior::Enum value);

            LUA_BIND_REFERENCE_TYPE_BEGIN
                LUA_BIND_FUNCTION(getCurrentState)
                LUA_BIND_FUNCTION(getElapsedTime)
                LUA_BIND_FUNCTION(getUpdateStepBehavior)
                LUA_BIND_FUNCTION(setUpdateStepBehavior)
            LUA_BIND_REFERENCE_TYPE_END

        private:
            State* m_pCurrentState;
            float m_elapsedTime;
            State::UpdateStepBehavior::Enum m_behavior;
        };
        typedef gep::Event<UpdateEventData*> UpdateEvent_t;

    public:

        virtual ~State();

        virtual void enter(EnterEventData* pData);

        /// \brief leaves the current state and returns the next state.
        virtual void leave(LeaveEventData* pData);

        virtual void update(UpdateEventData* pData);

        virtual bool shouldLeave() const;

        /// \brief Gets the next state according to the transition conditions.
        ///
        /// \param noConditionChecks If \c true, does not evaluate any transition conditions.
        ///
        /// \remark A state still qualifies as 'next state' if it does not have any condition,
        ///         regardless of the value of \a noConditionChecks.
        virtual void getNextState(State*& pNextState, bool noConditionChecks = false);

        virtual void setLeaveCondition(ConditionFunc_t condition);
        void setLeaveCondition(gep::ScriptFunctionWrapper condition);

        virtual EnterEvent_t* getEnterEvent();
        virtual LeaveEvent_t* getLeaveEvent();
        virtual UpdateEvent_t* getUpdateEvent();

        virtual void setName(const std::string& name);
        virtual       std::string& getName();
        virtual const std::string& getName() const;

        virtual void setLogging(gep::ILogging* pLogging);

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getEnterEvent)
            LUA_BIND_FUNCTION(getLeaveEvent)
            LUA_BIND_FUNCTION(getUpdateEvent)
            LUA_BIND_FUNCTION_NAMED(getNameCopy, "getName")
            LUA_BIND_FUNCTION_PTR(static_cast<void(State::*)(gep::ScriptFunctionWrapper)>(&setLeaveCondition), "setLeaveCondition")
        LUA_BIND_REFERENCE_TYPE_END

    private:
        gep::IAllocator* m_pAllocator;
        gep::ILogging* m_pLogging;
        std::string m_name;
        ConditionFunc_t m_leaveCondition;
        TransitionArray_t m_transitions;
        EnterEvent_t m_onEnter;
        LeaveEvent_t m_onLeave;
        UpdateEvent_t m_onUpdate;

        explicit State(const std::string& name, gep::IAllocator* pAllocator);
        void hookIntoUpdateFramework(gep::IUpdateFramework* ufx);
        virtual void addTransition(State* to, ConditionFunc_t condition = nullptr);
        std::string getNameCopy() const;
    };
}

#include "gpp/stateMachines/state.inl"
