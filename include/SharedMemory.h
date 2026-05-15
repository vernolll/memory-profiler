#pragma once

#ifdef _WIN32
#include <windows.h>

#include "AllocationRegistry.h"

struct SharedMemoryPayload
{
    volatile uint32_t changeCounter;
    AllocationRecord records[AllocationRegistry::MAX_RECORDS];
};

class SharedMemoryManager 
{
    HANDLE hMapFile;
    SharedMemoryPayload* pPayload;

public:
    SharedMemoryManager() 
    {
        hMapFile = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(SharedMemoryPayload),
            "Global\\MyMemoryTrackerShared"
        );

        pPayload = (SharedMemoryPayload*)MapViewOfFile(hMapFile,
            FILE_MAP_ALL_ACCESS, 
            0, 0, sizeof(SharedMemoryPayload));


        if (pPayload) std::memset(pPayload, 0, 
            sizeof(SharedMemoryPayload));
    }

    void NotifyChange(const AllocationRecord& rec, bool isAlloc)
    {
        if (!pPayload) return;

        pPayload->changeCounter++;
    }
};
#endif