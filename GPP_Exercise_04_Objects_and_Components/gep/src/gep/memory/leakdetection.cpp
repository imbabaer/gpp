#include "stdafx.h"
#include "gep/memory/leakDetection.h"
#include <ostream>

void* gep::MallocAllocator::allocateMemory(size_t size)
{
    return malloc(size);
}

void gep::MallocAllocator::freeMemory(void* mem)
{
    return free(mem);
}

gep::LeakDetector::LeakDetector() : m_allocationMap(&m_allocator)
{
}

void gep::LeakDetector::trackAllocation(void* mem, size_t size)
{
    ScopedLock<Mutex> lock(m_mutex);
    #ifdef GEP_ASSERT_ACTIVE
    LeakDetector::AllocationInfo* lastInfo = nullptr;
    if(m_allocationMap.tryGet(mem, lastInfo))
    {
        char* lastAllocation = (char*)alloca(LINE_LENGTH * lastInfo->backtraceSize);
        StackWalker::resolveCallstack(ArrayPtr<StackWalker::address_t>(lastInfo->backtrace, lastInfo->backtraceSize), lastAllocation, LINE_LENGTH);
        OutputDebugStringA("last allocation at:\n");
        for(int i=0; i < static_cast<int>(lastInfo->backtraceSize) - 1; i++)
        {
            char* line = lastAllocation + (LINE_LENGTH * i);
            OutputDebugStringA(line);
            OutputDebugStringA("\n");
        }
        GEP_ASSERT(false, "block allocated twice");
    }
    #endif
    AllocationInfo* info = GEP_NEW(m_allocator, AllocationInfo);
    info->backtraceSize = (unsigned char)StackWalker::getCallstack(2, ArrayPtr<StackWalker::address_t>(info->backtrace));
    info->size = size;
    m_allocationMap[mem] = info;
}

void gep::LeakDetector::trackFree(void* mem)
{
    ScopedLock<Mutex> lock(m_mutex);
    AllocationInfo* info = nullptr;
    if(m_allocationMap.tryGet(mem, info))
    {
        GEP_DELETE(m_allocator, info);
        m_allocationMap.remove(mem);
    }
    else
    {
        GEP_ASSERT(false, "double or invalid free");
    }
}

void gep::LeakDetector::findLeaks(std::ostream& printTo)
{
    Hashmap<AllocationInfo*, LeakInfo, HashMethodPointerPolicy> resolvedCallstacks(&m_allocator);

    void* maxAddr = nullptr;
    void* minAddr = (void*)((size_t)0 - 1);
    size_t leakedMemory = 0;

    // Resolve each identical leak exactly once
    for(auto& leak : m_allocationMap)
    {
        leak.value->referenced = false;
        if(resolvedCallstacks.exists(leak.value))
        {
            resolvedCallstacks[leak.value].count++;
        }
        else
        {
            LeakInfo info = { nullptr, 1 };
            if(leak.value->backtraceSize > 0)
            {
                info.resolvedFunctions = (char*)m_allocator.allocateMemory(LINE_LENGTH * leak.value->backtraceSize);
                StackWalker::resolveCallstack(ArrayPtr<StackWalker::address_t>(leak.value->backtrace, leak.value->backtraceSize), info.resolvedFunctions, LINE_LENGTH);
            }
            resolvedCallstacks[leak.value] = info;
        }

        if(leak.key < minAddr)
            minAddr = leak.key;
        if(leak.key > maxAddr)
            maxAddr = leak.key;
        leakedMemory += leak.value->size;
    }

    // Find the root leaks
    for(auto& leak : m_allocationMap)
    {
        ArrayPtr<void*> possibleReferences((void**)leak.key, leak.value->size / sizeof(void*));
        for(void* possibleReference : possibleReferences)
        {
            if(possibleReference >= minAddr && possibleReference <= maxAddr)
            {
                if(m_allocationMap.exists(possibleReference))
                    m_allocationMap[possibleReference]->referenced = true;
            }
        }
    }

    // First print all the memory leaks
    for(auto& leak : resolvedCallstacks)
    {
        printTo << "------------------------------------------" << std::endl;
        printTo << "Memory leak size " << leak.key->size << " bytes " << leak.value.count << " times" << std::endl;
        if(leak.value.resolvedFunctions == nullptr)
        {
            printTo << "No callstack available" << std::endl;
        }
        else
        {
            char* line = leak.value.resolvedFunctions;
            for(int i = 0; i<leak.key->backtraceSize; ++i, line += LINE_LENGTH)
            {
                printTo << line << std::endl;
            }
        }
        printTo << std::endl;
    }

    // Now print all the root leaks
    printTo << "------------------------------------------" << std::endl;
    printTo << "Root Leaks" << std::endl;
    for(auto& leak : m_allocationMap)
    {
        if(!leak.value->referenced)
        {
            printTo << "------------------------------------------" << std::endl;
            printTo << "Memory leak size " << leak.value->size << std::endl;
            LeakInfo info = { nullptr, 0 };
            resolvedCallstacks.tryGet(leak.value, info);
            if(info.resolvedFunctions == nullptr)
            {
                printTo << "No callstack available" << std::endl;
            }
            else
            {
                char* line = info.resolvedFunctions;
                for(int i = 0; i<leak.value->backtraceSize; ++i, line += LINE_LENGTH)
                {
                    printTo << line << std::endl;
                }
            }
        }
    }

    //Print statistics
    printTo << std::endl;
    printTo << m_allocationMap.count() << " memory leaks found. " << leakedMemory << " bytes memory leaked." << std::endl;
}

void* gep::LeakDetectorAllocator::allocateMemory(size_t size)
{
    void* result = m_allocator->allocateMemory(size);
    if(result != nullptr)
        m_leakDetector.trackAllocation(result, size);
    return result;
}

void gep::LeakDetectorAllocator::freeMemory(void* mem)
{
    if(mem != nullptr)
        m_leakDetector.trackFree(mem);
    m_allocator->freeMemory(mem);
}

void* gep::LeakDetectorAllocatorStatistics::allocateMemory(size_t size)
{
    void* result = m_allocator->allocateMemory(size);
    if(result != nullptr)
        m_leakDetector.trackAllocation(result, size);
    return result;
}

void gep::LeakDetectorAllocatorStatistics::freeMemory(void* mem)
{
    if(mem != nullptr)
        m_leakDetector.trackFree(mem);
    m_allocator->freeMemory(mem);
}
