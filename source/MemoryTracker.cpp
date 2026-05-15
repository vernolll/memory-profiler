#include "../include/MemoryTracker.h"
#include "../include/AllocationRegistry.h"
#include "../include/TrackerState.h"
#include <cstdlib>
#include <iostream>
#include <thread>

thread_local bool g_InTracker = false;

void* operator new(std::size_t size)
{
    if (g_InTracker)
    {
        return std::malloc(size);
    }

    TrackerGuard guard;
    void* ptr = std::malloc(size);
    if (!ptr) throw std::bad_alloc();
    OnAlloc(ptr, size);
    return ptr;
}

int CaptureStack(void** buffer, int maxFrames) 
{
#ifdef _WIN32
    return CaptureStackBackTrace(2, maxFrames, buffer, nullptr);
#elif defined(__linux__)
    int frames = backtrace(buffer, maxFrames);
    return frames;
#else
    return 0;
#endif
}

void OnAlloc(void* ptr, std::size_t size)
{
    if (g_InTracker) return;

    auto& registry = AllocationRegistry::getInstance();

    if (registry.GetRawData() == nullptr) return;

    void* stack[12];
    int frames = CaptureStack(stack, 12);
    registry.Add(ptr, size, stack, frames);
}

void OnFree(void* ptr) 
{
    AllocationRegistry::getInstance().Remove(ptr);
}

void operator delete(void* ptr) noexcept 
{
    OnFree(ptr);
    std::free(ptr);
}

void operator delete[](void* ptr) noexcept
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

void operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
    OnFree(ptr);
    std::free(ptr);
}