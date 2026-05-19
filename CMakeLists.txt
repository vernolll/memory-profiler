cmake_minimum_required(VERSION 3.16)
project(MemoryProfiler VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG:FULL")
    add_compile_options(/Od /Zi)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)


set(PROFILER_HEADERS
    "include/SymbolResolver.h"
    "include/SharedMemory.h"
    "include/TrackerState.h"
    "include/MemoryTracker.h"
    "include/AllocationRegistry.h"
    "include/ui/MainWindow.h"
    "include/ui/MonitorWindow.h"
    "include/ui/MemoryMapWidget.h"
    "include/ui/CallstackWidget.h"
)

set(PROFILER_SOURCES
    "source/main.cpp"
    "source/SymbolResolver.cpp"
    "source/MemoryTracker.cpp"
    "source/AllocationRegistry.cpp"
    "source/ui/MainWindow.cpp"
    "source/ui/MonitorWindow.cpp"
    "source/ui/MemoryMapWidget.cpp"
    "source/ui/CallstackWidget.cpp"
)

add_executable(profiler WIN32 
    ${PROFILER_SOURCES}
    ${PROFILER_HEADERS}
)

target_include_directories(profiler PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ui"
)

target_link_libraries(profiler PUBLIC 
    Qt5::Widgets
    dbghelp
)