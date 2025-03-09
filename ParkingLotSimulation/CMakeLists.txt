cmake_minimum_required(VERSION 3.16)

project(ParkingLotSimulation)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(parking_sim main.cpp)

message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "C++ standard required: ${CMAKE_CXX_STANDARD_REQUIRED}")

# Add any additional libraries or dependencies here