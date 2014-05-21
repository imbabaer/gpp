#include "stdafx.h"
#include "gpp/stateMachines/stateMachine.h"
#include "gpp/stateMachines/state.h"

#include "gep/globalManager.h"
#include "gep/interfaces/updateFramework.h"


//////////////////////////////////////////////////////////////////////////

gpp::StateMachine::StateMachine(const std::string& name, gep::IAllocator* pAllocator) :
    State(name, pAllocator),
    m_defaultLeaveState(leaveStateName(), pAllocator),
    m_pLeaveState(&m_defaultLeaveState),
    m_pCurrentState(nullptr),
    m_states(m_pAllocator)
{
}

gpp::StateMachine::~StateMachine()
{
    for(auto pFsm : m_states.values())
    {
        GEP_DELETE(m_pAllocator, pFsm);
    }
    m_states.clear();
}

void gpp::StateMachine::run()
{
    // The other run() will take care to get the default update framework.
    run(*g_globalManager.getUpdateFramework());
}

void gpp::StateMachine::run(gep::IUpdateFramework& updateFramework)
{
    EnterEventData data;
    run(updateFramework, data);
}

void gpp::StateMachine::run(gep::IUpdateFramework& updateFramework, EnterEventData& data)
{
    enter(&data);
    hookIntoUpdateFramework(&updateFramework);
}

void gpp::StateMachine::addTransition(const std::string& from, const std::string& to, ConditionFunc_t condition /*= nullptr*/)
{
    auto fromState = get<State>(from);
    GEP_ASSERT(fromState, "The state you want to add a transition to does not exist.", from);

    auto toState = get<State>(to);
    GEP_ASSERT(toState, "The state you want to make a transition to does not exist.", to);

    if(fromState == this)
    {
        State::addTransition(toState, condition);
    }
    else
    {
        fromState->addTransition(toState, condition);
    }
}

void gpp::StateMachine::addTransition(State* to, ConditionFunc_t condition)
{
    m_pLeaveState->addTransition(to, condition);
}

void gpp::StateMachine::addTransition(const std::string& from, const std::string& to, gep::ScriptFunctionWrapper condition)
{
    addTransition(from, to, [=](){
        return g_globalManager.getScriptingManager()->callFunction<bool>(condition);
    });
}


void gpp::StateMachine::enter(EnterEventData* pData)
{
    State::enter(pData);
    if (m_pCurrentState == nullptr)
    {
        State::getNextState(m_pCurrentState);
    }

    GEP_ASSERT(m_pCurrentState, "There is no state to enter to!");

    m_pCurrentState->enter(pData);
}

void gpp::StateMachine::update(UpdateEventData* pData)
{
    GEP_ASSERT(m_pCurrentState);

    //State::update(pData);
    m_pCurrentState->update(pData);

    bool changeState = m_pCurrentState->shouldLeave() || pData->m_behavior != State::UpdateStepBehavior::Continue;
    if(changeState)
    {
        LeaveEventData leaveData;
        leaveData.m_pCurrentStateMachine = this;
        leaveData.m_noConditionChecks = pData->m_behavior == State::UpdateStepBehavior::LeaveWithNoConditionChecks;
        m_pCurrentState->leave(&leaveData);
        GEP_ASSERT(leaveData.m_pCurrentState = m_pCurrentState, "State did not set the current state correctly!");
        m_pCurrentState = leaveData.m_pNextState;
        GEP_ASSERT(m_pCurrentState, "No next state to make a transition to!", m_pCurrentState);

        if(shouldLeave())
        {
            // The outer state machine will call leave()
            return;
        }

        EnterEventData enterData;
        m_pCurrentState->enter(&enterData);
    }
}

bool gpp::StateMachine::shouldLeave() const
{
    return m_pCurrentState == m_pLeaveState;
}

void gpp::StateMachine::getNextState(State*& pNextState, bool noConditionChecks)
{
    m_pLeaveState->getNextState(pNextState, noConditionChecks);
}

void gpp::StateMachine::setLeaveCondition(ConditionFunc_t condition)
{
    g_globalManager.getLogging()->logWarning("Setting the leave condition of a state machine has no effect!");
}
