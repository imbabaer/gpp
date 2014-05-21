#include "stdafx.h"
#include "Test_StateMachine.h"
#include "gpp/stateMachines/state.h"
#include "gpp/stateMachines/stateMachine.h"
#include "gpp/stateMachines/stateMachineFactory.h"
#include "eventTestingUtils.h"
#include "testLog.h"
#include "gpp/dummyLogging.h"

using namespace gpp;

GEP_UNITTEST_TEST(StateMachine, Nested)
{
    GEP_UNITTEST_SETUP_EVENT_GLOBALS;
    auto& logging =
        TestLogging::instance();
    //DummyLogging::instance();

    // set up update framework
    _updateFramework.setElapsedTime(10.0f);

    // Add at least one callback so we know that we are still updating
    size_t frameCount = 0;
    _updateFramework.registerUpdateCallback([&](float){ logging.logWarning("frame %u:", frameCount); });

    StateMachineFactory factory(&g_stdAllocator);

    StateMachine* pMainFsm = nullptr;

    const size_t numTestStages = 12U;
    gep::Hashmap<std::string, size_t, gep::StringHashPolicy> testStages;

    // actual test
    {
        auto pFsm_A = factory.create("fsm_A");
        pFsm_A->setLogging(&logging);

        pMainFsm = pFsm_A;

        // Adding states
        auto pState_A = pFsm_A->create<State>("A");
        auto pFsm_B = pFsm_A->create<StateMachine>("fsm_B");
        auto pState_B_a = pFsm_B->create<State>("a");

        // Adding state transitions
        pFsm_A->addTransition("__enter", "A");
        pFsm_A->addTransition("A", "fsm_B");
        pFsm_A->addTransition("fsm_B", "__leave");

        pFsm_B->addTransition("__enter", "a");
        pFsm_B->addTransition("a", "__leave");

        // Set state leave conditions
        pState_A->setLeaveCondition([&](){
            return frameCount == 3;
        });
        pState_B_a->setLeaveCondition([&](){
            return frameCount == 6;
        });

        // Set some listeners
        pFsm_A->getEnterEvent()->registerListener([&](State::EnterEventData*){
            testStages["fsm_A.enter"]++;
            return gep::EventResult::Handled;
        });
        pFsm_A->getLeaveEvent()->registerListener([&](State::LeaveEventData*){
            testStages["fsm_A.leave"]++;
            return gep::EventResult::Handled;
        });
        pFsm_A->getUpdateEvent()->registerListener([&](State::UpdateEventData*){
            testStages["fsm_A.update"]++;
            return gep::EventResult::Handled;
        });

        pState_A->getEnterEvent()->registerListener([&](State::EnterEventData*){
            testStages["state_A.enter"]++;
            return gep::EventResult::Handled;
        });
        pState_A->getLeaveEvent()->registerListener([&](State::LeaveEventData*){
            testStages["state_A.leave"]++;
            return gep::EventResult::Handled;
        });
        pState_A->getUpdateEvent()->registerListener([&](State::UpdateEventData*){
            testStages["state_A.update"]++;
            return gep::EventResult::Handled;
        });

        pFsm_B->getEnterEvent()->registerListener([&](State::EnterEventData*){
            testStages["fsm_B.enter"]++;
            return gep::EventResult::Handled;
        });
        pFsm_B->getLeaveEvent()->registerListener([&](State::LeaveEventData*){
            testStages["fsm_B.leave"]++;
            return gep::EventResult::Handled;
        });
        pFsm_B->getUpdateEvent()->registerListener([&](State::UpdateEventData*){
            testStages["fsm_B.update"]++;
            return gep::EventResult::Handled;
        });

        pState_B_a->getEnterEvent()->registerListener([&](State::EnterEventData*){
            testStages["state_B.enter"]++;
            return gep::EventResult::Handled;
        });
        pState_B_a->getLeaveEvent()->registerListener([&](State::LeaveEventData*){
            testStages["state_B.leave"]++;
            return gep::EventResult::Handled;
        });
        pState_B_a->getUpdateEvent()->registerListener([&](State::UpdateEventData*){
            testStages["state_B.update"]++;
            return gep::EventResult::Handled;
        });
    }

    GEP_ASSERT(pMainFsm, "pFsm may not be null! There must exist 1 state machine as entry point.");
    pMainFsm->setLogging(&logging);

    // Run the machine
    pMainFsm->run(_updateFramework);

    for(frameCount = 0; frameCount < 20; ++frameCount)
    {
        _updateFramework.run();
    }
}
