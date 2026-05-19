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

set(COMMON_HEADERS
    "include/SymbolResolver.h"
    "include/SharedMemory.h"
    "include/TrackerState.h"
)
set(COMMON_SOURCES
    "source/main.cpp"
    "source/SymbolResolver.cpp"
)

set(APP_HEADERS
    "include/MemoryTracker.h"
    "include/AllocationRegistry.h"
)
set(APP_SOURCES
    "source/main.cpp"
    "source/MemoryTracker.cpp"
    "source/AllocationRegistry.cpp"
)

set(UI_HEADERS
    "include/ui/MainWindow.h"
    "include/ui/MonitorWindow.h"
    "include/ui/MemoryMapWidget.h"
    "include/ui/CallstackWidget.h"
)
set(UI_SOURCES
    "source/ui/MainWindow.cpp"
    "source/ui/MonitorWindow.cpp"
    "source/ui/MemoryMapWidget.cpp"
    "source/ui/CallstackWidget.cpp"
)

add_executable(profiler_target_app 
    ${APP_SOURCES}
    ${APP_HEADERS}
    ${COMMON_SOURCES}
    ${COMMON_HEADERS}
)
target_include_directories(profiler_target_app PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(profiler_target_app PRIVATE dbghelp)

add_executable(profiler_gui WIN32 
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${COMMON_SOURCES}
    ${COMMON_HEADERS}
)
target_include_directories(profiler_gui PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ui"
)
target_link_libraries(profiler_gui PRIVATE 
    Qt5::Widgets
    dbghelp
)