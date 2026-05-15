#pragma once
#include <thread>

extern thread_local bool g_InTracker;

struct TrackerGuard 
{
    TrackerGuard() { g_InTracker = true; }
    ~TrackerGuard() { g_InTracker = false; }
};