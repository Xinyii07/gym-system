cmake_minimum_required(VERSION 3.15)
project(gym_system CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gym_system
    src/main.cpp
    src/UIUtils.cpp
    src/DataStore.cpp
    src/MemberModule.cpp
    src/BillingModule.cpp
    src/RecommendationModule.cpp
    src/ServiceBookingModule.cpp
    src/LockerModule.cpp
    src/ReportsModule.cpp
    src/LoyaltyModule.cpp
)

target_include_directories(gym_system PRIVATE include)
