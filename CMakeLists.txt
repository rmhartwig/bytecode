cmake_minimum_required(VERSION 4.1)
project(BytecodeVM)

set(CMAKE_CXX_STANDARD 20)

add_executable(BytecodeVM main.cpp
        common.h
        value.h
        value.cpp
        chunk.cpp
        chunk.h
        debug.cpp
        debug.h
        vm.cpp
        vm.h
        compiler.cpp
        compiler.h
        scanner.cpp
        scanner.h
)
