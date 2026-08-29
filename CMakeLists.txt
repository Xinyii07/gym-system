cmake_minimum_required(VERSION 3.15)
project(gym_system CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gym_system
    src/GymManagementSystem.cpp
)
