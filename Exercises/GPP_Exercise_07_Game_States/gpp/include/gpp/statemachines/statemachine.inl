
inline
const char* gpp::StateMachine::enterStateName()
{
    return "__enter";
}

inline
const char* gpp::StateMachine::leaveStateName()
{
    return "__leave";
}


template<typename T_State>
inline
T_State* gpp::StateMachine::create(const std::string& name)
{
    GEP_ASSERT(get<State>(name) == nullptr, "Attempt to add state that already exists", name);

    auto state = GEP_NEW(m_pAllocator, T_State)(name, m_pAllocator);
    m_states[name] = state;
    state->setLogging(m_pLogging);
    return state;
}

template<typename T_State>
inline
T_State* gpp::StateMachine::get(const std::string& name)
{
    State* pState = nullptr;
    if(name == enterStateName())
    {
        pState = this;
    }
    else if(name == leaveStateName())
    {
        pState = m_pLeaveState;
    }
    else
    {
        // Returns nullptr if there is no such state.
        pState = m_states[name];
    }
    GEP_ASSERT(pState == nullptr || dynamic_cast<T_State*>(pState) != nullptr,
               "Requested state is not a state machine!", name);

    return static_cast<T_State*>(pState);
}

inline
void gpp::StateMachine::addTransition(const std::string& from, const std::string& to)
{
    addTransition(from, to, nullptr);
}
