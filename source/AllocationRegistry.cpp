#include "../include/AllocationRegistry.h"
#include <cstdlib>
#include <cstring>
#include <chrono>

AllocationRegistry g_Registry;

AllocationRegistry::AllocationRegistry() 
{
    m_records = (AllocationRecord*)std::malloc(sizeof(AllocationRecord) * MAX_RECORDS);

    for (size_t i = 0; i < MAX_RECORDS; ++i)
    {
        m_records[i].address = nullptr;
        m_records[i].active.store(false);
    }
}

AllocationRegistry::~AllocationRegistry()
{
    std::free(m_records);
}

void AllocationRegistry::Add(void* ptr, std::size_t size, void** stack, int stackFrames) 
{
    if (!ptr) return;

    size_t index = Hash(ptr);
    uint64_t now = std::chrono::steady_clock::now().time_since_epoch().count();

    for (size_t i = 0; i < MAX_RECORDS; ++i) 
    {
        size_t curr = (index + i) % MAX_RECORDS;

        bool expected = false;
        if (m_records[curr].active.compare_exchange_strong(expected, true))
        {
            m_records[curr].address = ptr;
            m_records[curr].size = size;
            m_records[curr].timestamp = now;

            int frames = (stackFrames < 12) ? stackFrames : 12;
            std::memcpy(m_records[curr].callstack, stack, frames * sizeof(void*));

            return;
        }
    }
}

void AllocationRegistry::Remove(void* ptr) 
{
    if (!ptr) return;

    size_t index = Hash(ptr);

    for (size_t i = 0; i < MAX_RECORDS; ++i) 
    {
        size_t curr = (index + i) % MAX_RECORDS;

        if (m_records[curr].active.load() && m_records[curr].address == ptr) 
        {
            m_records[curr].address = nullptr;
            m_records[curr].active.store(false);
            return;
        }

        if (!m_records[curr].active.load() && m_records[curr].address == nullptr) 
        {
            break;
        }
    }
}