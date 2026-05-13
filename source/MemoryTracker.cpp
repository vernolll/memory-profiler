#include "../include/MemoryTracker.h"

#include <cstdlib>
#include <iostream>


void OnAlloc(void* ptr, std::size_t size)
{
    std::printf("[ALLOC] Address: %p, Size: %llu bytes\n", ptr, (unsigned long long)size);
}

void OnFree(void* ptr)
{
    if (ptr)
    {
        std::printf("[FREE ] Address: %p\n", ptr);
    }
}

void* operator new(std::size_t size) 
{
    void* ptr = std::malloc(size);
    if (!ptr) throw std::bad_alloc();
    OnAlloc(ptr, size);
    return ptr;
}

void operator delete(void* ptr) noexcept 
{
    OnFree(ptr);
    std::free(ptr);
}

void* operator new[](std::size_t size) 
{
    void* ptr = std::malloc(size);
    if (!ptr) throw std::bad_alloc();
    OnAlloc(ptr, size);
    return ptr;
}

void operator delete[](void* ptr) noexcept
{
    OnFree(ptr);
    std::free(ptr);
}


void* operator new(std::size_t size, const std::nothrow_t&) noexcept 
{
    void* ptr = std::malloc(size);
    if (ptr) OnAlloc(ptr, size);
    return ptr;
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept 
{
    OnFree(ptr);
    std::free(ptr);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
    void* ptr = std::malloc(size);
    if (ptr) OnAlloc(ptr, size);
    return ptr;
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept 
{
    OnFree(ptr);
    std::free(ptr);
}