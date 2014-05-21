#include "stdafx.h"
#include "gpp/stateMachines/state.h"
#include "gep/memory/allocator.h"
#include "gep/globalManager.h"
#include "gep/interfaces/scripting.h"
#include "gpp/stateMachines/stateMachine.h"

gpp::State::State(const std::string& name, gep::IAllocator* pAllocator) :
    m_pAllocator(pAllocator),
    m_pLogging(&DummyLogging::instance()),
    m_name(name),
    m_leaveCondition(nullptr),
    m_transitions(pAllocator),
    m_onEnter(EnterEvent_t::CInfo(pAllocator)),
    m_onLeave(LeaveEvent_t::CInfo(pAllocator)),
    m_onUpdate(UpdateEvent_t::CInfo(pAllocator))
{
}

gpp::State::~State()
{
    m_pLogging = nullptr;
    m_pAllocator = nullptr;
}

void gpp::State::enter(EnterEventData* pData)
{
    m_pLogging->logMessage(">> %s", getName().c_str());
    pData->m_pCurrentState = this;
    getEnterEvent()->trigger(pData);
}

void gpp::State::leave(LeaveEventData* pData)
{
    m_pLogging->logMessage("<< %s", getName().c_str());
    pData->m_pCurrentState = this;
    pData->m_pNextState = nullptr;
    getNextState(pData->m_pNextState, pData->m_noConditionChecks);
    pData->m_pTransitionArray = &m_transitions;
    getLeaveEvent()->trigger(pData);
}

void gpp::State::update(UpdateEventData* pData)
{
    //m_pLogging->logMessage("++ %s", getName().c_str());
    pData->m_pCurrentState = this;
    getUpdateEvent()->trigger(pData);
}

bool gpp::State::shouldLeave() const
{
    return m_leaveCondition ? m_leaveCondition() : false;
}

void gpp::State::getNextState(State*& pNextState, bool noConditionChecks)
{
    for(auto& transition : m_transitions)
    {
        // If there is no transition condition, the state qualifies as
        // 'next state'.
        if (!transition.condition)
        {
            pNextState = transition.to;
            continue;
        }
        
        // If we are checking for conditions AND the condition is true,
        // the state qualifies as 'next state'.
        if(!noConditionChecks && transition.condition())
        {
            pNextState = transition.to;
        }
    }
}

void gpp::State::setLeaveCondition(ConditionFunc_t condition)
{
    m_leaveCondition = condition;
}

void gpp::State::setLeaveCondition(gep::ScriptFunctionWrapper condition)
{
    m_leaveCondition = [=](){
        return g_globalManager.getScriptingManager()->callFunction<bool>(condition);
    };
}

gpp::State::EnterEvent_t* gpp::State::getEnterEvent()
{
    return &m_onEnter;
}

gpp::State::LeaveEvent_t* gpp::State::getLeaveEvent()
{
    return &m_onLeave;
}

gpp::State::UpdateEvent_t* gpp::State::getUpdateEvent()
{
    return &m_onUpdate;
}

void gpp::State::hookIntoUpdateFramework(gep::IUpdateFramework* ufx)
{
    GEP_ASSERT(ufx, "Update framework is a nullptr.");
    // register update()
    auto id = ufx->registerUpdateCallback([=](float dt){
        UpdateEventData data;
        data.m_elapsedTime = dt;
        update(&data);
        if(shouldLeave())
        {
            LeaveEventData data;
            leave(&data);
        }
    });
    // deregister update() once we are leaving this state machine
    getLeaveEvent()->registerListener([=](LeaveEventData*){
        ufx->deregisterUpdateCallback(id);
        return gep::EventResult::Handled;
    });
}

void gpp::State::setName(const std::string& name)
{
    m_name = name;
}

const std::string& gpp::State::getName() const
{
    return m_name;
}

std::string& gpp::State::getName()
{
    return m_name;
}

void gpp::State::setLogging(gep::ILogging* pLogging)
{
    m_pLogging = pLogging ? pLogging : &DummyLogging::instance();
}

void gpp::State::addTransition(State* to, ConditionFunc_t condition /*= nullptr*/)
{
    GEP_ASSERT(to, "State transition target is invalid.");

    Transition transition;
    transition.to = to;
    transition.condition = condition;

    m_transitions.append(transition);
}

//////////////////////////////////////////////////////////////////////////

gpp::State::EnterEventData::EnterEventData() :
    m_pCurrentState(nullptr)
{

}

gpp::State* gpp::State::EnterEventData::getState()
{
    return m_pCurrentState;
}

//////////////////////////////////////////////////////////////////////////

gpp::State::LeaveEventData::LeaveEventData() :
    m_pCurrentStateMachine(nullptr),
    m_pCurrentState(nullptr),
    m_pNextState(nullptr),
    m_noConditionChecks(false),
    m_pTransitionArray(nullptr)
{

}

gpp::State* gpp::State::LeaveEventData::getCurrentState()
{
    return m_pCurrentState;
}

void gpp::State::LeaveEventData::setNextState(State* pNext)
{
    for (auto& transition : *m_pTransitionArray)
    {
        if(transition.to == pNext)
        {
            m_pNextState = pNext;
            return;
        }
    }
    
    GEP_ASSERT(false, "The next state you want to make a transition to is invalid!", pNext->getName());
    g_globalManager.getLogging()->logError("The next state you want to make a transition to is invalid!");
}

void gpp::State::LeaveEventData::setNextState(const std::string& next)
{
    setNextState(m_pCurrentStateMachine->get<State>(next));
}

//////////////////////////////////////////////////////////////////////////

gpp::State::UpdateEventData::UpdateEventData() :
    m_pCurrentState(nullptr),
    m_elapsedTime(0.0f),
    m_behavior(UpdateStepBehavior::Continue)
{

}

gpp::State* gpp::State::UpdateEventData::getCurrentState()
{
    return m_pCurrentState;
}

float gpp::State::UpdateEventData::getElapsedTime()
{
    return m_elapsedTime;
}

gpp::State::UpdateStepBehavior::Enum gpp::State::UpdateEventData::getUpdateStepBehavior() const
{
    return m_behavior;
}

void gpp::State::UpdateEventData::setUpdateStepBehavior(State::UpdateStepBehavior::Enum value)
{
    m_behavior = value;
}
