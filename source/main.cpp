#include <iostream>
#include <vector>
#include "../include/MemoryTracker.h"
#include "../include/AllocationRegistry.h"

void PrintMemoryReport() 
{
    AllocationRecord* records = g_Registry.GetRawData();
    int activeCount = 0;

    for (size_t i = 0; i < AllocationRegistry::MAX_RECORDS; ++i)
    {
        if (records[i].active.load()) 
        {
            activeCount++;
            std::printf("Block [%d]: Address: %p, Size: %zu bytes\n",
                activeCount, records[i].address, records[i].size);

            std::printf("  Callstack: ");
            for (int j = 0; j < 3; ++j) 
            {
                if (records[i].callstack[j]) 
                {
                    std::printf("%p ", records[i].callstack[j]);
                }
            }
            std::printf("\n");
        }
    }

    if (activeCount == 0) 
    {
        std::printf("No active allocations found.\n");
    }
    else
    {
        std::printf("Total active blocks: %d\n", activeCount);
    }
}

int main() 
{
    int* singleInt = new int(42);

    double* myMatrix = new double[10];

    std::printf("Simulating fragmentation...\n");
    char* chunk1 = new char[128];
    char* chunk2 = new char[128];
    char* chunk3 = new char[128];
    char* chunk4 = new char[128];
    char* chunk5 = new char[128];

    delete[] chunk2;
    delete[] chunk4;

    delete singleInt;

    PrintMemoryReport();

    delete[] myMatrix;
    delete[] chunk1;
    delete[] chunk3;
    delete[] chunk5;

    std::printf("Test completed.\n");
    return 0;
}