#include <cstdlib>
#include <cstring>
#include <chrono>
#include <windows.h>
#include "SharedMemory.h"
#include "AllocationRegistry.h"
#include "TrackerState.h"

AllocationRegistry g_Registry;

struct WinSharedMem 
{
    HANDLE hMapFile;
    SharedMemoryPayload* payload;
};

static WinSharedMem g_Shm = { NULL, nullptr };

AllocationRegistry::AllocationRegistry() : m_records(nullptr)
{
    TrackerGuard guard;

    size_t shmSize = sizeof(SharedMemoryPayload);

    g_Shm.hMapFile = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        (DWORD)shmSize,
        "Local\\CppMemoryProfilerShm" 
    );

    if (g_Shm.hMapFile == NULL) 
    {
        return;
    }

    g_Shm.payload = (SharedMemoryPayload*)MapViewOfFile(
        g_Shm.hMapFile,
        FILE_MAP_ALL_ACCESS,
        0, 0, shmSize
    );

    if (g_Shm.payload) 
    {
        m_records = g_Shm.payload->records;

        g_Shm.payload->changeCounter = 0;

        for (size_t i = 0; i < MAX_RECORDS; ++i)
        {
            m_records[i].active.store(false);
        }
    }
}

AllocationRegistry::~AllocationRegistry()
{
    if (g_Shm.payload)
    {
        UnmapViewOfFile(g_Shm.payload);
        g_Shm.payload = nullptr;
    }

    if (g_Shm.hMapFile != NULL)
    {
        CloseHandle(g_Shm.hMapFile);
        g_Shm.hMapFile = NULL;
    }
}

void AllocationRegistry::Add(void* ptr, std::size_t size, void** stack, int stackFrames)
{
    if (g_InTracker || m_records == nullptr) return;
    if (!ptr || !g_Shm.payload) return;

    size_t index = Hash(ptr);
    uint64_t now = std::chrono::steady_clock::now().time_since_epoch().count();

    const size_t MAX_PROBES = 32;

    for (size_t i = 0; i < MAX_PROBES; ++i)
    {
        size_t curr = (index + i) % MAX_RECORDS;

        InterlockedIncrement(&g_Shm.payload->totalProbesCount);

        bool expected = false;
        if (m_records[curr].active.compare_exchange_strong(expected, true))
        {
            m_records[curr].address = ptr;
            m_records[curr].size = size;
            m_records[curr].timestamp = now;

            int frames = (stackFrames < 12) ? stackFrames : 12;
            std::memcpy(m_records[curr].callstack, stack, frames * sizeof(void*));

            InterlockedIncrement(&g_Shm.payload->totalAllocsCount);
            InterlockedIncrement(&g_Shm.payload->changeCounter);
            return;
        }
    }
}

void AllocationRegistry::Remove(void* ptr) 
{
    if (g_InTracker || m_records == nullptr) return;
    if (!ptr || !g_Shm.payload) return;

    size_t index = Hash(ptr);

    for (size_t i = 0; i < MAX_RECORDS; ++i) 
    {
        size_t curr = (index + i) % MAX_RECORDS;

        if (m_records[curr].active.load() && m_records[curr].address == ptr)
        {
            m_records[curr].address = nullptr;
            m_records[curr].active.store(false);

            InterlockedIncrement(&g_Shm.payload->changeCounter);
            return;
        }

        if (!m_records[curr].active.load() && m_records[curr].address == nullptr) break;
    }
}