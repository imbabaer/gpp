#include "stdafx.h"
#include "Test_StateMachine.h"
#include "gpp/stateMachines/state.h"
#include "gpp/stateMachines/stateMachine.h"
#include "gpp/stateMachines/stateMachineFactory.h"
#include "eventTestingUtils.h"
#include "testLog.h"
#include "gpp/dummyLogging.h"

using namespace gpp;

GEP_UNITTEST_TEST(StateMachine, Basics)
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

    // test data
    size_t testData_A = 0;
    size_t testData_B = 0;

    const size_t numTestStages = 9;
    size_t testStages[numTestStages] = { 0 };

    // actual test
    {
        auto pFsm = pMainFsm = factory.create("fsm");
        pFsm->setLogging(&logging);

        // Adding states
        auto pState_A = pFsm->create<State>("A");
        auto state_B =  pFsm->create<State>("B");

        // Adding state transitions
        pFsm->addTransition("__enter", "A");
        pFsm->addTransition("A", "B");
        pFsm->addTransition("B", "__leave");

        // Set state leave conditions
        pState_A->setLeaveCondition([&](){
            return frameCount == 3;
        });
        state_B->setLeaveCondition([&](){
            return frameCount == 6;
        });

        // Set some listeners
        pFsm->getEnterEvent()->registerListener([&](State::EnterEventData*){
            testStages[0]++;
            return gep::EventResult::Handled;
        });
        pFsm->getLeaveEvent()->registerListener([&](State::LeaveEventData*){
            testStages[1]++;
            return gep::EventResult::Handled;
        });
        pFsm->getUpdateEvent()->registerListener([&](State::UpdateEventData*){
            testStages[2]++;
            return gep::EventResult::Handled;
        });

        pState_A->getEnterEvent()->registerListener([&](State::EnterEventData*){
            testStages[3]++;
            return gep::EventResult::Handled;
        });
        pState_A->getLeaveEvent()->registerListener([&](State::LeaveEventData*){
            testStages[4]++;
            return gep::EventResult::Handled;
        });
        pState_A->getUpdateEvent()->registerListener([&](State::UpdateEventData*){
            testStages[5]++;
            testData_A++;
            return gep::EventResult::Handled;
        });

        state_B->getEnterEvent()->registerListener([&](State::EnterEventData*){
            testStages[6]++;
            return gep::EventResult::Handled;
        });
        state_B->getLeaveEvent()->registerListener([&](State::LeaveEventData*){
            testStages[7]++;
            return gep::EventResult::Handled;
        });
        state_B->getUpdateEvent()->registerListener([&](State::UpdateEventData*){
            testStages[8]++;
            testData_B++;
            return gep::EventResult::Handled;
        });
    }

    pMainFsm->setLogging(&logging);

    // Run the machine
    pMainFsm->run(_updateFramework);

    for(frameCount = 0; frameCount < 10; ++frameCount)
    {
        _updateFramework.run();
    }
}


