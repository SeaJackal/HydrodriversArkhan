cmake_minimum_required(VERSION 3.10)

set(LIBRARY_NAME HydrodriversTimerLow)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(${LIBRARY_NAME} INTERFACE)
target_link_libraries(${LIBRARY_NAME} INTERFACE HydrolibReturnCodes)
target_link_libraries(${LIBRARY_NAME} INTERFACE HydrodriversGPIO)
target_link_libraries(${LIBRARY_NAME} INTERFACE CMSIS)
target_include_directories(${LIBRARY_NAME} INTERFACE include)
