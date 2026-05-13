#include <iostream>
#include <vector>
#include "../include/MemoryTracker.h"

int main() 
{
    std::printf("1. Allocating single integers...\n");
    int* a = new int(10);
    int* b = new int(20);

    delete a;

    std::printf("\n2. Allocating arrays (new[])...\n");
    double* arr = new double[100];
    delete[] arr;

    std::printf("\n3. Fragmentation simulation (Small vs Large)...\n");
    void* ptrs[10];
    for (int i = 0; i < 10; ++i) 
    {
        ptrs[i] = new char[64];
    }

    for (int i = 0; i < 10; i += 2)
    {
        delete[](char*)ptrs[i];
    }

    return 0;
}