#include "stdafx.h"

#include "gep/ArrayPtr.h"

using namespace gep;

GEP_UNITTEST_GROUP(ArrayPtr)
GEP_UNITTEST_TEST(ArrayPtr, ArrayPtr)
{
    int data1[] = {1, 2, 3, 4, 5};
    int data2[] = {3, 4, 5};
    int data3[] = {3, 4, 5, 4, 5};

    ArrayPtr<int> a1(data1);
    GEP_ASSERT(a1.length() == 5, "taking the length from an static array does not work");
    GEP_ASSERT(a1.getPtr() == data1, "taking the ptr from a static array does not work");

    for(size_t i=0; i<a1.length(); i++)
        GEP_ASSERT(a1[i] == i+1);

    ArrayPtr<int> a2 = a1(2, a1.length());

    GEP_ASSERT(a2 == ArrayPtr<int>(data2), "comparing does not work");
    GEP_ASSERT(a2 != a1, "not equals operator does not work");

    ArrayPtr<int> a3(nullptr, 4);
    GEP_ASSERT(a3.getPtr() == nullptr);
    GEP_ASSERT(a3.length() == 0);

    ArrayPtr<int> a4(data2, 0);
    GEP_ASSERT(a4.getPtr() == nullptr);
    GEP_ASSERT(a4.length() == 0);

    GEP_ASSERT(a4 == a3);

    const ArrayPtr<int> a5(a1);
    GEP_ASSERT(a5(2, a5.length()) == ArrayPtr<int>(data2), "comparing does not work");
    for(size_t i=0; i<a1.length(); i++)
        GEP_ASSERT(a1[i] == i+1);

    a1(0, 3).copyFrom(a2);
    GEP_ASSERT(a1 == ArrayPtr<int>(data3));

    // test const conversions
    ArrayPtr<const int> a6(a4);
    ArrayPtr<const int> a7;
    a6 = a3;
}
