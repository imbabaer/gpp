#include "stdafx.h"
#include "gep/weakPtr.h"
#include "gep/memory/allocators.h"

using namespace gep;

namespace
{
    class WeakRefTest : public WeakReferenced<WeakRefTest>
    {
    };
}
DefineWeakRefStaticMembers(WeakRefTest)

GEP_UNITTEST_GROUP(WeakPtr)
GEP_UNITTEST_TEST(WeakPtr, WeakPtr)
{
    WeakPtr<WeakRefTest> ptr1 = new WeakRefTest();
    WeakPtr<WeakRefTest> ptr2(new WeakRefTest());
    WeakPtr<WeakRefTest> ptr3;
    ptr3 = new WeakRefTest();

    GEP_ASSERT(ptr1.get() != nullptr);
    GEP_ASSERT(ptr2.get() != nullptr);
    GEP_ASSERT(ptr3.get() != nullptr);

    //test copy constructor
    WeakPtr<WeakRefTest> ptr4(ptr1);
    GEP_ASSERT(ptr4.get() == ptr1.get());
    GEP_ASSERT(ptr4.get() != nullptr);

    // test assignment of nullptr
    ptr1 = nullptr;

    //test copy assignment
    ptr1 = ptr4;
    GEP_ASSERT(ptr1.get() == ptr4.get());

    //test pointer invalidation
    delete ptr1.get();
    GEP_ASSERT(ptr1.get() == nullptr);
    GEP_ASSERT(ptr1.get() == nullptr);
    GEP_ASSERT(ptr4.get() == nullptr);
    GEP_ASSERT(ptr2.get() != nullptr);
    GEP_ASSERT(ptr3.get() != nullptr);

    //test destructor
    {
        WeakPtr<WeakRefTest> ptr5(ptr2);
        GEP_ASSERT(ptr5.get() != nullptr);
    }
    GEP_ASSERT(ptr2.get() != nullptr);

    //test pointer invalidation
    delete ptr2.get();
    GEP_ASSERT(ptr1.get() == nullptr);
    GEP_ASSERT(ptr1.get() == nullptr);
    GEP_ASSERT(ptr4.get() == nullptr);
    GEP_ASSERT(ptr2.get() == nullptr);
    GEP_ASSERT(ptr3.get() != nullptr);

    delete ptr3.get();
    GEP_ASSERT(ptr1.get() == nullptr);
    GEP_ASSERT(ptr1.get() == nullptr);
    GEP_ASSERT(ptr4.get() == nullptr);
    GEP_ASSERT(ptr2.get() == nullptr);
    GEP_ASSERT(ptr3.get() == nullptr);
}
