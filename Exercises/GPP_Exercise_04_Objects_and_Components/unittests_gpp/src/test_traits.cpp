#include "stdafx.h"

#include "gep/traits.h"

using namespace gep;

struct nonPod1
{
    virtual void foo() {}
};

struct nonPod2
{
    nonPod2(const nonPod2&){}
};

static_assert(isPod<int>::value == true, "int is not a pod");
static_assert(isPod<float>::value == true, "float is not a pod");
static_assert(isPod<ArrayPtr<int>>::value == true, "ArrayPtr is not a pod");
static_assert(isPod<int*>::value == true, "a pointer is not a pod");
static_assert(isPod<int[4]>::value == true, "a static array is not a pod");
static_assert(isPod<nonPod1>::value == false, "nonPod1 is pod");
static_assert(isPod<nonPod2>::value == false, "nonPod2 is pod");
