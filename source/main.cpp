#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include "../include/MemoryTracker.h"
#include "../include/AllocationRegistry.h"
#include "../include/SharedMemory.h"
#include "../include/SymbolResolver.h"

void InnerFunction() 
{
    int* data = new int[5];
    std::printf("  Allocated 5 ints in InnerFunction at %p\n", (void*)data);
}

void MiddleFunction() 
{
    InnerFunction();
}

void TopFunction()
{
    MiddleFunction();
}

int main() 
{
    SymbolResolver resolver;

    TopFunction();

    AllocationRecord* records = AllocationRegistry::getInstance().GetRawData();

    for (size_t i = 0; i < AllocationRegistry::MAX_RECORDS; ++i) 
    {
        if (records[i].active.load())
        {
            std::printf("Block: %p (%zu bytes)\n", records[i].address, records[i].size);

            std::vector<ResolvedFrame> frames = resolver.Resolve(records[i].callstack, 12);

            for (const auto& frame : frames) 
            {
                if (!frame.functionName.empty()) 
                {
                    std::printf("  -> %s (%s:%u)\n",
                        frame.functionName.c_str(),
                        frame.fileName.c_str(),
                        frame.lineNumber);
                }
            }
        }
    }
    std::getchar();

    return 0;
}