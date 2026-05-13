#pragma once
#include <new>

void OnAlloc(void* ptr, std::size_t size);

void OnFree(void* ptr);


void* operator new(std::size_t size); 

void operator delete(void* ptr) noexcept;

void* operator new[](std::size_t size);

void operator delete[](void* ptr) noexcept;

void* operator new(std::size_t size, const std::nothrow_t&) noexcept;

void operator delete(void* ptr, const std::nothrow_t&) noexcept;

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept;

void operator delete[](void* ptr, const std::nothrow_t&) noexcept;