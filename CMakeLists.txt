cmake_minimum_required(VERSION 3.16.3)
project(ex4)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(ex4
        MemoryConstants.h
#        YaaraConstants.h
        PhysicalMemory.cpp
        PhysicalMemory.h
        SimpleTest.cpp
        VirtualMemory.cpp
        VirtualMemory.h
#        YaaraTest.cpp
        MemoryConstants.h)
