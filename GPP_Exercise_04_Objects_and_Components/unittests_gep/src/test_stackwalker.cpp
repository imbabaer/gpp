#include "stdafx.h"
#include "gep/StackWalker.h"

using namespace gep;

GEP_UNITTEST_GROUP(StackWalker)
GEP_UNITTEST_TEST(StackWalker, StackWalker)
{
    StackWalker::address_t addresses[16];

    size_t numFound = StackWalker::getCallstack(1, ArrayPtr<StackWalker::address_t>(addresses));

    GEP_ASSERT(numFound > 0 && numFound <= 16, "stack walker did not find any stack frames");

    //Test a way to small buffer
    {
        StackWalker::address_t addresses2[3];
        size_t numFound2 = StackWalker::getCallstack(0, ArrayPtr<StackWalker::address_t>(addresses2));

        GEP_ASSERT(numFound2 == GEP_ARRAY_SIZE(addresses2), "stack walker did not respect the array size");
    }

    //Test resolving the stack frames
    char results[GEP_ARRAY_SIZE(addresses)][256];
    memset(results, 0, sizeof(results));
    StackWalker::resolveCallstack(ArrayPtr<StackWalker::address_t>(addresses, numFound), (char*)results, 256);

    GEP_ASSERT(results[0][0] != 0, "nothing has been written to the pFunctionNames argument");
    //make sure the StackWalker sticks to the visual studio error formatting:
    char* loc1 = strchr(results[0] + 3, ':');
    GEP_ASSERT(loc1 != nullptr, "the ':' character could not be found");
    GEP_ASSERT(*(loc1 - 1) == ')', "the character before the ':' character has to be a ')'");
    char* loc2 = strchr(results[0], '(');
    char* temp = loc2;
    //Skip all '(' carachters until the last
    while( (temp = strchr(temp + 1,'(')) != nullptr )
        loc2 = temp;
    GEP_ASSERT(loc2 != nullptr && loc2 < (loc1-1), "the '(' character has not be found or is in the wrong place");
    GEP_ASSERT(loc2 < loc1 - 2, "there are no charachters between '(' and ')'");
    *(loc1 - 1) = '\0';
    int lineNumber = atoi(loc2 + 1);
    GEP_ASSERT(lineNumber == 11, "the line number is not correct");
    GEP_ASSERT(strcmp(loc1 + 2, "Unittest_StackWalkerStackWalker::Run") == 0, "the function name is not correct");
}
