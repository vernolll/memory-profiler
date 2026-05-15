#pragma once
#include <cstdint>
#include <cstddef>
#include <atomic>

struct AllocationRecord 
{
    void* address;
    std::size_t size;
    uint64_t timestamp;
    void* callstack[12];
    std::atomic<bool> active{ false };
};

class AllocationRegistry 
{
public:
    static const size_t MAX_RECORDS = 1024 * 64;

    AllocationRegistry();
    ~AllocationRegistry();

    void Add(void* ptr, std::size_t size, void** stack, int stackFrames);

    void Remove(void* ptr);

    AllocationRecord* GetRawData() { return m_records; }

    static AllocationRegistry& getInstance() 
    {
        static AllocationRegistry instance;
        return instance;
    }

private:
    size_t Hash(void* ptr) const 
    {
        return (reinterpret_cast<size_t>(ptr) >> 4) % MAX_RECORDS;
    }

    AllocationRecord* m_records;
};