#include "stdafx.h"

#include "gep/memory/leakDetection.h"

using namespace gep;

struct ReferencesSomethingElse
{
    char* ref;
};

GEP_UNITTEST_GROUP(Basic)
GEP_UNITTEST_TEST(Basic, LeakDetection)
{
    auto p = new ReferencesSomethingElse;
    p->ref = new char[256];

    delete[] p->ref;
    delete p;
}
