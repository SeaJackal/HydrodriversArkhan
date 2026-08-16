cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(HydrodriversUARTLowMock INTERFACE)

target_include_directories(HydrodriversUARTLowMock INTERFACE include)