#include "stdafx.h"

#include "gep/memory/leakDetection.h"

using namespace gep;

GEP_UNITTEST_GROUP(LeakDetection)
GEP_UNITTEST_TEST(LeakDetection, AllocatorProxy)
{
    LeakDetectorAllocator* test = new LeakDetectorAllocator(&g_stdAllocator);

    void* mem1 = test->allocateMemory(64);
    void* mem2 = test->allocateMemory(32);

    test->freeMemory(mem1);
    GEP_ASSERT(test->hasLeaks() == true, "IAllocator proxy detector did not find the leak");
    test->freeMemory(mem2);
    delete test;
}

GEP_UNITTEST_TEST(LeakDetection, AllocatorStatisticsProxy)
{
    LeakDetectorAllocatorStatistics* test = new LeakDetectorAllocatorStatistics(&g_stdAllocator);

    void* mem1 = test->allocateMemory(64);
    void* mem2 = test->allocateMemory(32);

    test->freeMemory(mem1);
    GEP_ASSERT(test->hasLeaks() == true, "IAllocatorStatistics proxy detector did not find the leak");
    test->freeMemory(mem2);
    delete test;
}
