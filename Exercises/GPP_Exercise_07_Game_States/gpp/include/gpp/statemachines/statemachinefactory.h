#pragma once
#include "gep/interfaces/scripting.h"

namespace gep
{
    class IAllocator;
}

namespace gpp
{
    class State;
    class StateMachine;

    class GPP_API StateMachineFactory
    {
    public:
        StateMachineFactory(gep::IAllocator* pAllocator);
        ~StateMachineFactory();
        /// \brief Releases all allocated buffers.
        void initialize();
        void destroy();

        StateMachine* create(const std::string& name);

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(create)
        LUA_BIND_REFERENCE_TYPE_END

    private:
        gep::IAllocator* m_pAllocator;
        gep::Hashmap<std::string, StateMachine*, gep::StringHashPolicy> m_stateMachines;

        GEP_DISALLOW_COPY_AND_ASSIGNMENT(StateMachineFactory);
    };
}

#include "gpp/stateMachines/stateMachineFactory.inl"
