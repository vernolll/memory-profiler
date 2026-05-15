#pragma once
#include <new>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <execinfo.h>
#endif

void OnAlloc(void* ptr, std::size_t size);

void OnFree(void* ptr);

int CaptureStack(void** buffer, int maxFrames);

void* operator new(std::size_t size) noexcept; 

void operator delete(void* ptr) noexcept;

void* operator new[](std::size_t size);

void operator delete[](void* ptr) noexcept;

void* operator new(std::size_t size, const std::nothrow_t&) noexcept;

void operator delete(void* ptr, const std::nothrow_t&) noexcept;

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept;

void operator delete[](void* ptr, const std::nothrow_t&) noexcept;