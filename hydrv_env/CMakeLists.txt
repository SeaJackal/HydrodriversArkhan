cmake_minimum_required(VERSION 3.10)

set(LIBRARY_NAME HydrodriversEnv)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(${LIBRARY_NAME} INTERFACE)
target_link_libraries(${LIBRARY_NAME} INTERFACE HydrolibReturnCodes)
target_link_libraries(${LIBRARY_NAME} INTERFACE HydrodriversClock)
target_link_libraries(${LIBRARY_NAME} INTERFACE HydrodriversGPIO)

target_include_directories(${LIBRARY_NAME} INTERFACE include)
