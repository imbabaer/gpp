#include "stdafx.h"
#include "gep/ReferenceCounting.h"
#include "gep/memory/allocators.h"

using namespace gep;

namespace
{
    bool g_deleted = false;

    class RefCountTest1 : public ReferenceCounted
    {
    public:
        RefCountTest1() {}
        ~RefCountTest1() { g_deleted = true; }

        inline IAllocator* getAllocator() { return m_pAllocator; }
        inline unsigned int getRefCount() { return m_referenceCount; }
    };
};

GEP_UNITTEST_GROUP(RefCounting)
GEP_UNITTEST_TEST(RefCounting, RefCounted)
{
    auto instance1 = GEP_NEW(g_stdAllocator, RefCountTest1)();
    GEP_ASSERT(instance1->getAllocator() == &g_stdAllocator, "Allocator may not be set inside the constructor");
    auto instance2 = GEP_NEW(SimpleLeakCheckingAllocator::instance(), RefCountTest1)();
    GEP_ASSERT(instance2->getAllocator() == &SimpleLeakCheckingAllocator::instance(), "Allocator may not be set inside the constructor");

    g_deleted = false;
    instance1->addReference();
    instance1->removeReference();
    GEP_ASSERT(g_deleted == true, "reference counting is not working correctly");
    int count = 500;
    for(int i=0; i<count; i++)
        instance2->addReference();
    for(int i=0; i<count; i++)
        instance2->removeReference();
    SimpleLeakCheckingAllocator::destroyInstance(); //checks for memory leaks
}

GEP_UNITTEST_TEST(RefCounting, SmartPtr)
{
    {
        // test construction from instance
        SmartPtr<RefCountTest1> ptr1(GEP_NEW(SimpleLeakCheckingAllocator::instance(), RefCountTest1)());
        GEP_ASSERT(ptr1.get() != nullptr);
        {
            //test copy constructor
            SmartPtr<RefCountTest1> ptr2(ptr1);
        }

        {
            //test move constructor
            SmartPtr<RefCountTest1> ptr2(std::move(ptr1));
            GEP_ASSERT(ptr2.get() != nullptr);
            GEP_ASSERT(ptr1.get() == nullptr);
            GEP_ASSERT(ptr2->getRefCount() == 1);

            // test move assignment operator
            ptr1 = std::move(ptr2);
            GEP_ASSERT(ptr2.get() == nullptr);
            GEP_ASSERT(ptr1.get() != nullptr);
            GEP_ASSERT(ptr1->getRefCount() == 1);
        }

        {
            //test assignment operator
            SmartPtr<RefCountTest1> ptr2;
            ptr2 = ptr1;
            GEP_ASSERT(ptr2.get() != nullptr);
            GEP_ASSERT(ptr2->getRefCount() == 2);
        }
        GEP_ASSERT(ptr1->getRefCount() == 1);

        {
            //test assignment of nullptr
            ptr1 = nullptr;
            GEP_ASSERT(ptr1.get() == nullptr);
            SimpleLeakCheckingAllocator::destroyInstance(); //checks for memory leaks
        }

        {
            //test construction from nullptr
            SmartPtr<RefCountTest1> ptr2(nullptr);
            GEP_ASSERT(ptr2.get() == nullptr);
        }

        {
            //test assignment of other instance
            ptr1 = GEP_NEW(SimpleLeakCheckingAllocator::instance(),RefCountTest1)();
            GEP_ASSERT(ptr1.get() != nullptr);
        }
    }

    SimpleLeakCheckingAllocator::destroyInstance(); //checks for memory leaks
}
