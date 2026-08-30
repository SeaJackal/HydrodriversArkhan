cmake_minimum_required(VERSION 3.10)

set(LIBRARY_NAME HydrodriversGPIO)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(HYDRV_MCU_FAMILY STREQUAL "F1" OR HYDRV_MCU_FAMILY STREQUAL "F4")
    add_library(${LIBRARY_NAME} INTERFACE)
    target_link_libraries(${LIBRARY_NAME} INTERFACE HydrolibReturnCodes)
    target_link_libraries(${LIBRARY_NAME} INTERFACE CMSIS)

    target_include_directories(${LIBRARY_NAME} INTERFACE upper_half)
    
    if(HYDRV_MCU_FAMILY STREQUAL "F1")
        target_include_directories(${LIBRARY_NAME} INTERFACE f1/include)
    elseif(HYDRV_MCU_FAMILY STREQUAL "F4")
        target_include_directories(${LIBRARY_NAME} INTERFACE f4/include)
    endif()
    
    if(HYDRV_BUILD_EXAMPLES)
        add_subdirectory(example)
    endif()

    message(STATUS "${LIBRARY_NAME} target added for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
else()
    message(WARNING "${LIBRARY_NAME} target is not supported for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
endif()
