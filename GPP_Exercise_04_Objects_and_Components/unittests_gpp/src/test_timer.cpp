#include "stdafx.h"
#include "gep/timer.h"

using namespace gep;

namespace
{
    void sleepNano(unsigned int numNanoSeconds)
    {
        uint64 frequency;
        QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequency));
        uint64 numTicksToPass = (frequency * numNanoSeconds) / 1000000;
        uint64 start, end;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&start));
        do
        {
            QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&end));
        }
        while(end - start < numTicksToPass);
    }
}

GEP_UNITTEST_GROUP(Timer)
GEP_UNITTEST_TEST(Timer, Timer)
{
    Timer timer;
    {
        sleepNano(500);
        auto time = timer.getTime();
        GEP_ASSERT(time > 0, "some time should have passed", time);
        auto ftime = timer.getTimeAsFloat();
        GEP_ASSERT(ftime > 0.4f && ftime < 0.6f, "the time elapsed since the initialization of the timer should be roughly 0.5 ms", ftime);
        auto dtime = timer.getTimeAsDouble();
        GEP_ASSERT(dtime > 0.4 && dtime < 0.6, "the time elapsed since the initialization of the timer should be roughly 0.5 ms", dtime);
    }

    //test pausing
    timer.pause();
    uint64 time = timer.getTime();
    double pauseTime = timer.getTimeAsDouble();
    for(int i=0; i<100; i++)
    {
        GEP_ASSERT(timer.getTime() == time, "pausing does not work correctly");
        GEP_ASSERT(timer.getTimeAsDouble() == pauseTime, "pausing does not work correctly");
    }
    Sleep(10);
    timer.unpause();
    double unpauseTime = timer.getTimeAsDouble();
    GEP_ASSERT(unpauseTime - pauseTime < 0.1, "time has not been paused");
}

GEP_UNITTEST_TEST(Timer, PointInTime)
{
    Timer timer;

    PointInTime t1(timer);
    Sleep(1);
    PointInTime t2(timer);
    // test operator -
    GEP_ASSERT(t2 - t1 > 0.0f, "taking the difference between to points in time does not work");

    //test comparison
    GEP_ASSERT(t1 == t1);
    GEP_ASSERT(t2 == t2);
    GEP_ASSERT(t1 != t2);
    GEP_ASSERT(t1 < t2);
    GEP_ASSERT(!(t2 < t1));
    GEP_ASSERT(!(t1 > t2));
    GEP_ASSERT(t2 > t1);
    GEP_ASSERT(t1 <= t2);
    GEP_ASSERT(!(t2 <= t1));
    GEP_ASSERT(!(t1 >= t2));
    GEP_ASSERT(t2 >= t1);
    GEP_ASSERT(t1 >= t1);
    GEP_ASSERT(t2 >= t2);
    GEP_ASSERT(t1 <= t1);
    GEP_ASSERT(t2 <= t2);
}
