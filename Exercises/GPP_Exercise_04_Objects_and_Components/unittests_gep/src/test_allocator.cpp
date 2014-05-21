#include "stdafx.h"

#include "gep/memory/allocators.h"

using namespace gep;

GEP_UNITTEST_GROUP(Allocator)
GEP_UNITTEST_TEST(Allocator, PoolAllocator)
{
    {
        // default parent allocator
        const size_t chunkSize = 1024;
        const size_t numChunks = 2;
        PoolAllocator poolAllocator(chunkSize, numChunks);
        GEP_ASSERT(poolAllocator.getParentAllocator() == &StdAllocator::globalInstance(), "getParentAllocator default is wrong");
    }

    {
        const size_t chunkSize = 1024;
        const size_t numChunks = 2;
        PoolAllocator poolAllocator(chunkSize, numChunks, &SimpleLeakCheckingAllocator::instance());

        // initial statistics
        GEP_ASSERT(poolAllocator.getNumAllocations() == 0, "getNumAllocations is not 0");
        GEP_ASSERT(poolAllocator.getNumFrees() == 0, "getNumFrees is not 0");
        GEP_ASSERT(poolAllocator.getFreeListSize() != std::numeric_limits<size_t>::max(), "getFreeListSize must return a valid value");
        GEP_ASSERT(poolAllocator.getNumBytesReserved() == (chunkSize*numChunks) + poolAllocator.getFreeListSize(), "getNumBytesReserved is wrong");
        GEP_ASSERT(poolAllocator.getNumBytesUsed() == 0*chunkSize, "getNumBytesUsed is not %d", 0*chunkSize);
        GEP_ASSERT(poolAllocator.getParentAllocator() == &SimpleLeakCheckingAllocator::instance(), "getParentAllocator is wrong");

        // allocate p0
        void* p0 = poolAllocator.allocateMemory(chunkSize);
        GEP_ASSERT(p0!=nullptr, "allocateMemory must not return nullptr");
        GEP_ASSERT(poolAllocator.getNumAllocations()==1, "getNumAllocations is not 1");
        GEP_ASSERT(poolAllocator.getNumFrees()==0, "getNumFrees is not 0");
        GEP_ASSERT(poolAllocator.getNumBytesUsed()==1*chunkSize, "getNumBytesUsed is not %d", chunkSize);

        // allocate p1 (with smaller size)
        void* p1 = poolAllocator.allocateMemory(chunkSize>>1);
        GEP_ASSERT(p1!=nullptr, "allocateMemory must not return nullptr");
        GEP_ASSERT(poolAllocator.getNumAllocations()==2, "getNumAllocations is not 2");
        GEP_ASSERT(poolAllocator.getNumFrees()==0, "getNumFrees is not 0");
        GEP_ASSERT(poolAllocator.getNumBytesUsed()==2*chunkSize, "getNumBytesUsed is not %d", 2*chunkSize);

        // allocate p2 (list is full)
        void* p2 = poolAllocator.allocateMemory(chunkSize);
        GEP_ASSERT(p2==nullptr, "allocateMemory must return nullptr when the list is full");
        GEP_ASSERT(poolAllocator.getNumAllocations()==2, "getNumAllocations is not 2");
        GEP_ASSERT(poolAllocator.getNumFrees()==0, "getNumFrees is not 0");
        GEP_ASSERT(poolAllocator.getNumBytesUsed()==2*chunkSize, "getNumBytesUsed is not %d", 2*chunkSize);

        // freeing p1
        poolAllocator.freeMemory(p1);
        GEP_ASSERT(poolAllocator.getNumAllocations()==2, "getNumAllocations is not 2");
        GEP_ASSERT(poolAllocator.getNumFrees()==1, "getNumFrees is not 1");
        GEP_ASSERT(poolAllocator.getNumBytesUsed()==1*chunkSize, "getNumBytesUsed is not %d", 1*chunkSize);

        // freeing p0
        poolAllocator.freeMemory(p0);
        GEP_ASSERT(poolAllocator.getNumAllocations()==2, "getNumAllocations is not 2");
        GEP_ASSERT(poolAllocator.getNumFrees()==2, "getNumFrees is not 2");
        GEP_ASSERT(poolAllocator.getNumBytesUsed()==0*chunkSize, "getNumBytesUsed is not %d", 0*chunkSize);

        // freeing nullptr
        poolAllocator.freeMemory(nullptr);
        GEP_ASSERT(poolAllocator.getNumAllocations()==2, "getNumAllocations is not 2");
        GEP_ASSERT(poolAllocator.getNumFrees()==2, "getNumFrees is not 2");
        GEP_ASSERT(poolAllocator.getNumBytesUsed()==0*chunkSize, "getNumBytesUsed is not %d", 0*chunkSize);
    }
    SimpleLeakCheckingAllocator::destroyInstance(); // causes a memory leak check

    {
        // free list efficiency

        const size_t chunkSize = 512;
        const size_t numChunks = 128;
        PoolAllocator poolAllocator(chunkSize, numChunks);

        void* p0 = poolAllocator.allocateMemory(chunkSize);
        void* p1 = poolAllocator.allocateMemory(chunkSize);

        void* p1Old = p1;
        poolAllocator.freeMemory(p1);
        p1 = poolAllocator.allocateMemory(chunkSize);
        GEP_ASSERT(p1==p1Old, "free list is not working correctly, expected index 1");

        void* p2 = poolAllocator.allocateMemory(chunkSize);
        void* p3 = poolAllocator.allocateMemory(chunkSize);

        poolAllocator.freeMemory(p1);
        poolAllocator.freeMemory(p3);
        poolAllocator.freeMemory(p0);
        poolAllocator.freeMemory(p2);

        GEP_ASSERT(p2==poolAllocator.allocateMemory(chunkSize), "free list is not working correctly, expected index 2");
        GEP_ASSERT(p0==poolAllocator.allocateMemory(chunkSize), "free list is not working correctly, expected index 0");
        GEP_ASSERT(p3==poolAllocator.allocateMemory(chunkSize), "free list is not working correctly, expected index 3");
        GEP_ASSERT(p1==poolAllocator.allocateMemory(chunkSize), "free list is not working correctly, expected index 1");

        poolAllocator.freeMemory(p0);
        poolAllocator.freeMemory(p1);
        poolAllocator.freeMemory(p2);
        poolAllocator.freeMemory(p3);

        // massive allocations
        void* pointers[numChunks];
        for (size_t p=0; p<numChunks; ++p)
        {
            pointers[p] = poolAllocator.allocateMemory(chunkSize);
            GEP_ASSERT(pointers[p]!=nullptr, "allocateMemory must not return nullptr");
        }

        // massive frees (different order)
        for (size_t p=3; p<numChunks; p+=4)
            poolAllocator.freeMemory(pointers[p]);
        for (size_t p=2; p<numChunks; p+=4)
            poolAllocator.freeMemory(pointers[p]);
        for (size_t p=1; p<numChunks; p+=4)
            poolAllocator.freeMemory(pointers[p]);
        for (size_t p=0; p<numChunks; p+=4)
            poolAllocator.freeMemory(pointers[p]);
        GEP_ASSERT(poolAllocator.getNumBytesUsed()==0*chunkSize, "getNumBytesUsed is not %d", 0*chunkSize);
    }

    {
        // misaligned chunk size

        const size_t chunkSize = 1027;
        const size_t chunkSizeAligned = sizeof(void*) == 8 ? 1032 : 1028;
        const size_t numChunks = 2;
        PoolAllocator poolAllocator(chunkSize, numChunks, &SimpleLeakCheckingAllocator::instance());

        void* p0 = poolAllocator.allocateMemory(chunkSize);
        GEP_ASSERT((reinterpret_cast<uintptr_t>(p0)%sizeof(void*))==0, "wrong alignment");
        GEP_ASSERT(poolAllocator.getNumBytesUsed()==1*chunkSizeAligned, "getNumBytesUsed is not %d", 1*chunkSizeAligned);

        void* p1 = poolAllocator.allocateMemory(chunkSize);
        GEP_ASSERT((reinterpret_cast<uintptr_t>(p1)%sizeof(void*))==0, "wrong alignment");
        GEP_ASSERT(poolAllocator.getNumBytesUsed()==2*chunkSizeAligned, "getNumBytesUsed is not %d", 2*chunkSizeAligned);

        GEP_ASSERT(poolAllocator.getNumBytesReserved() == (chunkSizeAligned*numChunks) + poolAllocator.getFreeListSize(), "getNumBytesReserved is wrong");
        GEP_ASSERT(poolAllocator.getNumBytesUsed()==2*chunkSizeAligned, "getNumBytesUsed is not %d", 2*chunkSizeAligned);

        poolAllocator.freeMemory(p0);
        poolAllocator.freeMemory(p1);
    }
    SimpleLeakCheckingAllocator::destroyInstance(); // causes a memory leak check
}

GEP_UNITTEST_TEST(Allocator, StackAllocator)
{
    for (int i=0; i<2; ++i) // front and back
    {
        // default parent allocator front
        const size_t size = 4096;
        StackAllocator stackAllocator(i==0, size);
        GEP_ASSERT(stackAllocator.getParentAllocator() == &StdAllocator::globalInstance(), "getParentAllocator default is wrong");
    }

    for (int i=0; i<2; ++i) // front and back
    {
        const size_t size = 4096;
        StackAllocator stackAllocator(i==0, size, &SimpleLeakCheckingAllocator::instance());

        // initial statistics front
        GEP_ASSERT(stackAllocator.getNumAllocations() == 0, "getNumAllocations is not 0");
        GEP_ASSERT(stackAllocator.getNumFrees() == 0, "getNumFrees is not 0");
        GEP_ASSERT(stackAllocator.getNumBytesReserved() == size + stackAllocator.getDynamicArraySize(), "getNumBytesReserved is wrong");
        GEP_ASSERT(stackAllocator.getNumBytesUsed() == 0, "getNumBytesUsed is not %d", 0);
        GEP_ASSERT(stackAllocator.getParentAllocator() == &SimpleLeakCheckingAllocator::instance(), "getParentAllocator is wrong");

        // allocate p0
        void* p0 = stackAllocator.allocateMemory(1024);
        GEP_ASSERT(p0!=nullptr, "allocateMemory must not return nullptr");
        GEP_ASSERT(stackAllocator.getNumAllocations()==1, "getNumAllocations is not 1");
        GEP_ASSERT(stackAllocator.getNumFrees()==0, "getNumFrees is not 0");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==1024, "getNumBytesUsed is not %d", 1024);

        // now the internally used dynamic array must have reserved some memory, too
        GEP_ASSERT(stackAllocator.getDynamicArraySize()>0, "the dynamic array needs some memory, too!");
        GEP_ASSERT(stackAllocator.getNumBytesReserved() == size + stackAllocator.getDynamicArraySize(), "getNumBytesReserved is wrong");

        // allocate p1
        void* p1 = stackAllocator.allocateMemory(1024);
        GEP_ASSERT(p1!=nullptr, "allocateMemory must not return nullptr");
        GEP_ASSERT(i==0 ? p0<p1 : p0>p1, "front / back is not handled correctly");
        GEP_ASSERT(stackAllocator.getNumAllocations()==2, "getNumAllocations is not 2");
        GEP_ASSERT(stackAllocator.getNumFrees()==0, "getNumFrees is not 0");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==2048, "getNumBytesUsed is not %d", 2048);

        // free p1
        stackAllocator.freeMemory(p1);
        GEP_ASSERT(stackAllocator.getNumAllocations()==2, "getNumAllocations is not 2");
        GEP_ASSERT(stackAllocator.getNumFrees()==1, "getNumFrees is not 1");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==1024, "getNumBytesUsed is not %d", 1024);

        // allocate p1 again (must be the same address)
        void* p1Old = p1;
        p1 = stackAllocator.allocateMemory(1024);
        GEP_ASSERT(p1==p1Old, "stack pointer does not work correctly");
        GEP_ASSERT(stackAllocator.getNumAllocations()==3, "getNumAllocations is not 3");
        GEP_ASSERT(stackAllocator.getNumFrees()==1, "getNumFrees is not 1");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==2048, "getNumBytesUsed is not %d", 2048);

        // free p1 and p0
        stackAllocator.freeMemory(p1);
        stackAllocator.freeMemory(p0);
        GEP_ASSERT(stackAllocator.getNumAllocations()==3, "getNumAllocations is not 3");
        GEP_ASSERT(stackAllocator.getNumFrees()==3, "getNumFrees is not 3");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==0, "getNumBytesUsed is not %d", 0);

        // fill to maximum
        p0 = stackAllocator.allocateMemory(size>>1);
        p1 = stackAllocator.allocateMemory(size>>1);
        GEP_ASSERT(stackAllocator.getNumAllocations()==5, "getNumAllocations is not 5");
        GEP_ASSERT(stackAllocator.getNumFrees()==3, "getNumFrees is not 3");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==size, "getNumBytesUsed is not %d", size);

        // one more alloc (should be nullptr)
        GEP_ASSERT(nullptr==stackAllocator.allocateMemory(1), "stack is full -> expected nullptr");
        GEP_ASSERT(stackAllocator.getNumAllocations()==5, "getNumAllocations is not 5");
        GEP_ASSERT(stackAllocator.getNumFrees()==3, "getNumFrees is not 3");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==size, "getNumBytesUsed is not %d", size);

        // empty to half capacity and allocate too large
        stackAllocator.freeMemory(p1);
        GEP_ASSERT(nullptr==stackAllocator.allocateMemory((size>>1)+1), "stack is full -> expected nullptr");
        GEP_ASSERT(stackAllocator.getNumAllocations()==5, "getNumAllocations is not 5");
        GEP_ASSERT(stackAllocator.getNumFrees()==4, "getNumFrees is not 4");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==(size>>1), "getNumBytesUsed is not %d", (size>>1));

        // empty stack
        stackAllocator.freeMemory(p0);
        GEP_ASSERT(stackAllocator.getNumAllocations()==5, "getNumAllocations is not 5");
        GEP_ASSERT(stackAllocator.getNumFrees()==5, "getNumFrees is not 5");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==0, "getNumBytesUsed is not %d", 0);

        // free nullptr
        stackAllocator.freeMemory(nullptr);
    }
    SimpleLeakCheckingAllocator::destroyInstance(); // causes a memory leak check

    for (int i=0; i<2; ++i) // front and back
    {
        const size_t count = 4096;
        const size_t size = sizeof(void*) * count;
        StackAllocator stackAllocator(i==0, size, &SimpleLeakCheckingAllocator::instance());

        // massive allocations
        void* pointers[count];
        for (size_t p=0; p<count; ++p)
        {
            pointers[p] = stackAllocator.allocateMemory(sizeof(void*));
            GEP_ASSERT(pointers[p]!=nullptr, "allocateMemory must not return nullptr");
        }
        for (__int64 p=count-1; p>=0; --p)
        {
            stackAllocator.freeMemory(pointers[p]);
        }

        // free to marker
        size_t numFrees = stackAllocator.getNumFrees();
        void* pm0 = stackAllocator.allocateMemory(sizeof(void*));
        for (int i=0; i<9; ++i) stackAllocator.allocateMemory(sizeof(void*));
        void* pm1 = stackAllocator.allocateMemory(sizeof(void*));
        for (int i=0; i<9; ++i) stackAllocator.allocateMemory(sizeof(void*));
        stackAllocator.freeToMarker(pm1);
        GEP_ASSERT(stackAllocator.getNumFrees()==numFrees+10);
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==10*sizeof(void*), "getNumBytesUsed is not %d", 10*sizeof(void*));
        stackAllocator.freeToMarker(pm0);
        GEP_ASSERT(stackAllocator.getNumFrees()==numFrees+20);
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==0, "getNumBytesUsed is not %d", 0);
    }
    SimpleLeakCheckingAllocator::destroyInstance(); // causes a memory leak check

    for (int i=0; i<2; ++i) // front and back
    {
        const size_t size = 512;
        StackAllocator stackAllocator(i==0, size, &SimpleLeakCheckingAllocator::instance());

        // misaligned allocations
        void* p0 = stackAllocator.allocateMemory(1);
        GEP_ASSERT(reinterpret_cast<uintptr_t>(p0)%sizeof(void*)==0, "wrong alignment");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==1*sizeof(void*), "getNumBytesUsed is not %d", 1*sizeof(void*));
        void* p1 = stackAllocator.allocateMemory(1);
        GEP_ASSERT(reinterpret_cast<uintptr_t>(p1)%sizeof(void*)==0, "wrong alignment");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==2*sizeof(void*), "getNumBytesUsed is not %d", 2*sizeof(void*));
        void* p2 = stackAllocator.allocateMemory(1);
        GEP_ASSERT(reinterpret_cast<uintptr_t>(p2)%sizeof(void*)==0, "wrong alignment");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==3*sizeof(void*), "getNumBytesUsed is not %d", 3*sizeof(void*));
        void* p3 = stackAllocator.allocateMemory(1);
        GEP_ASSERT(reinterpret_cast<uintptr_t>(p3)%sizeof(void*)==0, "wrong alignment");
        GEP_ASSERT(stackAllocator.getNumBytesUsed()==4*sizeof(void*), "getNumBytesUsed is not %d", 4*sizeof(void*));

        // misaligned frees
        stackAllocator.freeMemory(p3);
        stackAllocator.freeMemory(p2);
        stackAllocator.freeMemory(p1);
        stackAllocator.freeMemory(p0);
    }
    SimpleLeakCheckingAllocator::destroyInstance(); // causes a memory leak check

    for (int i=0; i<2; ++i) // front and back
    {
        const size_t size = 2043;
        const size_t sizeAligned = sizeof(void*) == 8 ? 2048 : 2044;

        StackAllocator stackAllocator(i==0, size, &SimpleLeakCheckingAllocator::instance());

        // misaligned buffer size
        GEP_ASSERT(stackAllocator.getNumBytesReserved() == sizeAligned + stackAllocator.getDynamicArraySize(), "getNumBytesReserved is wrong");

        void* p0 = stackAllocator.allocateMemory(sizeAligned-sizeof(void*));
        GEP_ASSERT(reinterpret_cast<uintptr_t>(p0)%sizeof(void*)==0, "wrong alignment");
        void* p1 = stackAllocator.allocateMemory(sizeof(void*));
        GEP_ASSERT(reinterpret_cast<uintptr_t>(p1)%sizeof(void*)==0, "wrong alignment");
        GEP_ASSERT(nullptr==stackAllocator.allocateMemory(1), "stack is full -> expected nullptr");

        // now the internally used dynamic array must have reserved some memory, too
        GEP_ASSERT(stackAllocator.getDynamicArraySize()>0, "the dynamic array needs some memory, too!");
        GEP_ASSERT(stackAllocator.getNumBytesReserved() == sizeAligned + stackAllocator.getDynamicArraySize(), "getNumBytesReserved is wrong");

        stackAllocator.freeMemory(p1);
        stackAllocator.freeMemory(p0);
    }
    SimpleLeakCheckingAllocator::destroyInstance(); // causes a memory leak check
}

GEP_UNITTEST_TEST(Allocator, DoubleEndedStackAllocator)
{
    {
        // default parent allocator front
        const size_t size = 4096;
        DoubleEndedStackAllocator deStackAllocator(size);
        GEP_ASSERT(deStackAllocator.getParentAllocator() == &StdAllocator::globalInstance(), "getParentAllocator default is wrong");
    }

    {
        const size_t size = 4096;
        DoubleEndedStackAllocator deStackAllocator(size, &SimpleLeakCheckingAllocator::instance());

        // initial statistics front
        GEP_ASSERT(deStackAllocator.getNumAllocations() == 0, "getNumAllocations is not 0");
        GEP_ASSERT(deStackAllocator.getNumFrees() == 0, "getNumFrees is not 0");
        GEP_ASSERT(deStackAllocator.getNumBytesReserved() == size + deStackAllocator.getDynamicArraysSize(), "getNumBytesReserved is wrong");
        GEP_ASSERT(deStackAllocator.getNumBytesUsed() == 0, "getNumBytesUsed is not %d", 0);
        GEP_ASSERT(deStackAllocator.getParentAllocator() == &SimpleLeakCheckingAllocator::instance(), "getParentAllocator is wrong");

        // default interface must allocate at front
        void *pf0 = deStackAllocator.allocateMemory(512);
        GEP_ASSERT(pf0!=nullptr, "allocateMemory must not return nullptr");
        GEP_ASSERT(deStackAllocator.getNumAllocations()==1, "getNumAllocations is not 1");
        GEP_ASSERT(deStackAllocator.getNumFrees()==0, "getNumFrees is not 0");
        GEP_ASSERT(deStackAllocator.getNumBytesUsed()==512, "getNumBytesUsed is not %d", 512);

        // explicitly allocate at front
        void *pf1 = deStackAllocator.getFront()->allocateMemory(512);
        GEP_ASSERT(pf1!=nullptr, "allocateMemory must not return nullptr");
        GEP_ASSERT(pf0<pf1, "front / back is not handled correctly");
        GEP_ASSERT(deStackAllocator.getNumAllocations()==2, "getNumAllocations is not 2");
        GEP_ASSERT(deStackAllocator.getNumFrees()==0, "getNumFrees is not 0");
        GEP_ASSERT(deStackAllocator.getNumBytesUsed()==1024, "getNumBytesUsed is not %d", 1024);

        // allocate at back
        void *pb0 = deStackAllocator.getBack()->allocateMemory(512);
        GEP_ASSERT(pb0!=nullptr, "allocateMemory must not return nullptr");
        GEP_ASSERT(pb0>pf1, "front / back is not handled correctly");
        GEP_ASSERT(deStackAllocator.getNumAllocations()==3, "getNumAllocations is not 3");
        GEP_ASSERT(deStackAllocator.getNumFrees()==0, "getNumFrees is not 0");
        GEP_ASSERT(deStackAllocator.getNumBytesUsed()==1536, "getNumBytesUsed is not %d", 1536);

        // now both internally used dynamic arrays must have reserved some memory, too
        GEP_ASSERT(deStackAllocator.getDynamicArraysSize()>0, "the dynamic arrays need some memory, too!");
        GEP_ASSERT(deStackAllocator.getNumBytesReserved() == size + deStackAllocator.getDynamicArraysSize(), "getNumBytesReserved is wrong");

        // allocate at back agein
        void *pb1 = deStackAllocator.getBack()->allocateMemory(512);
        GEP_ASSERT(pb1!=nullptr, "allocateMemory must not return nullptr");
        GEP_ASSERT(pb0>pb1, "front / back is not handled correctly");
        GEP_ASSERT(deStackAllocator.getNumAllocations()==4, "getNumAllocations is not 4");
        GEP_ASSERT(deStackAllocator.getNumFrees()==0, "getNumFrees is not 0");
        GEP_ASSERT(deStackAllocator.getNumBytesUsed()==2048, "getNumBytesUsed is not %d", 2048);

        // free two pointers
        deStackAllocator.getFront()->freeMemory(pf1);
        deStackAllocator.getBack()->freeMemory(pb1);
        GEP_ASSERT(deStackAllocator.getNumAllocations()==4, "getNumAllocations is not 4");
        GEP_ASSERT(deStackAllocator.getNumFrees()==2, "getNumFrees is not 2");
        GEP_ASSERT(deStackAllocator.getNumBytesUsed()==1024, "getNumBytesUsed is not %d", 1024);

        // fill too maximum
        pf1 = deStackAllocator.getFront()->allocateMemory(1536);
        GEP_ASSERT(pf1!=nullptr, "allocateMemory must not return nullptr");
        pb1 = deStackAllocator.getBack()->allocateMemory(1536);
        GEP_ASSERT(pb1!=nullptr, "allocateMemory must not return nullptr");
        GEP_ASSERT(deStackAllocator.getNumBytesUsed()==4096, "getNumBytesUsed is not %d", 4096);

        // overlapping
        GEP_ASSERT(deStackAllocator.getFront()->allocateMemory(1)==nullptr, "stack will overlap -> expected nullptr");
        GEP_ASSERT(deStackAllocator.getBack()->allocateMemory(1)==nullptr, "stack will overlap -> expected nullptr");

        // overlapping with gap between stacks
        deStackAllocator.getFront()->freeMemory(pf1);
        GEP_ASSERT(deStackAllocator.getFront()->allocateMemory(1537)==nullptr, "stack will overlap -> expected nullptr");
        GEP_ASSERT(deStackAllocator.getBack()->allocateMemory(1537)==nullptr, "stack will overlap -> expected nullptr");

        // final free
        deStackAllocator.getFront()->freeMemory(pf0);
        deStackAllocator.getBack()->freeMemory(pb1);
        deStackAllocator.getBack()->freeMemory(pb0);
    }
}
