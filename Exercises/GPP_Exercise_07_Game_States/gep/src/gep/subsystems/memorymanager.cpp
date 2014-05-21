#include "stdafx.h"
#include "gepimpl/subsystems/memoryManager.h"
#include "gep/memory/allocator.h"

void gep::MemoryManager::initialize()
{

}

void gep::MemoryManager::destroy()
{
    GEP_ASSERT(m_allocators.length() == 0, "not all allocators have been deregistered");
}

void gep::MemoryManager::update(float elapsedTime)
{
    //TODO draw statistics here
}

void gep::MemoryManager::registerAllocator(const char* name, IAllocatorStatistics* pAllocator)
{
    m_allocators.append(AllocatorInfo(name, pAllocator));
}

void gep::MemoryManager::deregisterAllocator(IAllocatorStatistics* pAllocator)
{
    size_t i=0;
    for(; i<m_allocators.length(); i++)
    {
        if(m_allocators[i].pAllocator == pAllocator)
            break;
    }
    if(i < m_allocators.length())
        m_allocators.removeAtIndex(i);
}

