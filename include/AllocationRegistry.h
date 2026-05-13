#pragma once
#include <cstdint>
#include <cstddef>
#include <atomic>

struct AllocationRecord 
{
    void* address;                        // Адрес блока (ключ)
    std::size_t size;                     // Размер
    uint64_t timestamp;                   // Метка времени (в мс или тиках)
    void* callstack[12];                  // Глубина стека
    std::atomic<bool> active{ false };    // Флаг занятости ячейки в таблице
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

private:
    size_t Hash(void* ptr) const 
    {
        return (reinterpret_cast<size_t>(ptr) >> 4) % MAX_RECORDS;
    }

    AllocationRecord* m_records;
};

extern AllocationRegistry g_Registry;