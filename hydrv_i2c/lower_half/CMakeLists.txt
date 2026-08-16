cmake_minimum_required(VERSION 3.10)

set(LIBRARY_NAME HydrodriversI2CLow)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(HYDRV_MCU_FAMILY STREQUAL "F4")
    add_library(${LIBRARY_NAME} INTERFACE)
    target_link_libraries(${LIBRARY_NAME} INTERFACE CMSIS)
    target_link_libraries(${LIBRARY_NAME} INTERFACE HydrodriversGPIO)
    
    target_include_directories(${LIBRARY_NAME} INTERFACE f4/include)

    message(STATUS "${LIBRARY_NAME} target added for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
else()
    message(WARNING "${LIBRARY_NAME} target is not supported for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
endif()
