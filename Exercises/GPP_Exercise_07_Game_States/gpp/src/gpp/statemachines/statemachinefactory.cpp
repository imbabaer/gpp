#include "stdafx.h"
#include "gpp/stateMachines/state.h"
#include "gpp/stateMachines/stateMachine.h"
#include "gpp/stateMachines/stateMachineFactory.h"

#include "gep/memory/allocator.h"

gpp::StateMachineFactory::StateMachineFactory(gep::IAllocator* pAllocator) :
    m_pAllocator(pAllocator),
    m_stateMachines(m_pAllocator)
{
}

gpp::StateMachineFactory::~StateMachineFactory()
{

}

gpp::StateMachine* gpp::StateMachineFactory::create(const std::string& name)
{
    GEP_ASSERT(m_stateMachines.exists(name) == false, "A top-level state machine of the given name already exists!", name);
    auto pFsm = GEP_NEW(m_pAllocator, StateMachine)(name, m_pAllocator);
    m_stateMachines[name] = pFsm;
    return pFsm;
}

void gpp::StateMachineFactory::initialize()
{

}

void gpp::StateMachineFactory::destroy()
{
    for (auto pFsm : m_stateMachines.values())
    {
        GEP_DELETE(m_pAllocator, pFsm);
    }
    m_stateMachines.clear();
}
