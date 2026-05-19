cmake_minimum_required (VERSION 3.8)
project ("profiler")

set(CMAKE_BUILD_TYPE Debug)

if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG:FULL")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} /DEBUG:FULL")

    add_compile_options(/Od /Zi)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

add_executable (profiler "include/MemoryTracker.h" "source/MemoryTracker.cpp" "source/main.cpp" "include/AllocationRegistry.h" "source/AllocationRegistry.cpp"  "include/SharedMemory.h" "include/SymbolResolver.h" "source/SymbolResolver.cpp" "include/TrackerState.h" "include/ui/MainWindow.h" "source/ui/MainWindow.cpp" "include/ui/MonitorWindow.h" "source/ui/MonitorWindow.cpp" "include/ui/MemoryMapWidget.h" "source/ui/MemoryMapWidget.cpp")

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt5::Core 
    Qt5::Widgets 
    Qt5::Gui
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET profiler PROPERTY CXX_STANDARD 20)
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE DbgHelp)
endif()