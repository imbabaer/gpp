#pragma once

#include "gep/gepmodule.h"
#include "gep/memory/allocator.h"
#include "gep/container/hashmap.h"
#include "gep/threading/mutex.h"
#include "gep/stackWalker.h"

namespace gep
{
    class GEP_API MallocAllocator : public IAllocator
    {
    public:
        virtual void* allocateMemory(size_t size) override;
        virtual void freeMemory(void* mem) override;
    };

    class GEP_API LeakDetector
    {
    private:
        static const size_t LINE_LENGTH = 1024;

        struct AllocationInfo
        {
            size_t size;
            StackWalker::address_t backtrace[20];
            unsigned char backtraceSize;
            bool referenced;

            unsigned int hash() const
            {
                return hashOf(&size, sizeof(size_t) + sizeof(StackWalker::address_t) * backtraceSize);
            }
        };

        struct LeakInfo
        {
            char* resolvedFunctions;
            size_t count;
        };

        MallocAllocator m_allocator;
        Mutex m_mutex;
        Hashmap<void*, AllocationInfo*, PointerHashPolicy> m_allocationMap;

    public:
        LeakDetector();

        /// \brief should be called for every allocation to track
        void trackAllocation(void* mem, size_t size);
        /// \brief should be called for every free to track
        void trackFree(void* mem);
        /// \brief finds and prints memory leaks to the given output stream
        void findLeaks(std::ostream& printTo);
        /// \brief checks if there are leaks or not
        inline bool hasLeaks() const { return m_allocationMap.count() > 0; }
    };


    class GEP_API LeakDetectorAllocator
        : public IAllocator
    {
    private:
        LeakDetector m_leakDetector;
        IAllocator* m_allocator;

    public:
        LeakDetectorAllocator(IAllocator* allocator)
            : m_allocator(allocator)
        {
            GEP_ASSERT(allocator != nullptr);
        }

        virtual void* allocateMemory(size_t size) override;
        virtual void freeMemory(void* mem) override;

        inline bool hasLeaks() const { return m_leakDetector.hasLeaks(); }
        inline void findLeaks(std::ostream& printTo) { m_leakDetector.findLeaks(printTo); }
        inline IAllocator* getWrapped() { return m_allocator; }
    };

    class GEP_API LeakDetectorAllocatorStatistics
        : public IAllocatorStatistics
    {
    private:
        LeakDetector m_leakDetector;
        IAllocatorStatistics* m_allocator;

    public:
        LeakDetectorAllocatorStatistics(IAllocatorStatistics* allocator)
            : m_allocator(allocator)
        {
            GEP_ASSERT(allocator != nullptr);
        }

        virtual void* allocateMemory(size_t size) override;
        virtual void freeMemory(void* mem) override;

        virtual size_t getNumAllocations() const { return m_allocator->getNumAllocations(); }
        virtual size_t getNumFrees() const { return m_allocator->getNumFrees(); }
        virtual size_t getNumBytesReserved() const { return m_allocator->getNumBytesReserved(); }
        virtual size_t getNumBytesUsed() const { return m_allocator->getNumBytesUsed(); }
        virtual IAllocatorStatistics* getParentAllocator() const { return m_allocator->getParentAllocator(); }

        inline bool hasLeaks() const { return m_leakDetector.hasLeaks(); }
        inline void findLeaks(std::ostream& printTo) { m_leakDetector.findLeaks(printTo); }
        inline IAllocatorStatistics* getWrapped() { return m_allocator; }
    };
}
