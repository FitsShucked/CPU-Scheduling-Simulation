cmake_minimum_required(VERSION 3.9)
project(CPU_Scheduling_Simulation)

set(CMAKE_CXX_STANDARD 98)

include_directories(.)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -D DEBUG_MODE")

add_executable(CPU_Scheduling_Simulation
        project1.c)
