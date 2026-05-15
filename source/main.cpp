#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include "../include/MemoryTracker.h"
#include "../include/AllocationRegistry.h"
#include "../include/SharedMemory.h"

void BusinessLogic() 
{
    int* data = new int[rand() % 100 + 1];

    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 50));

    if (rand() % 10 > 2) 
    {
        delete[] data;
    }
}

int main() 
{
    srand(static_cast<unsigned int>(time(NULL)));

    int iteration = 0;
    while (true) 
    {
        BusinessLogic();

        if (++iteration % 100 == 0) 
        {
            std::printf("Total operations tracked: %d\n", iteration);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}