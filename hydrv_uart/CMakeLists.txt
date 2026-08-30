cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(HYDRV_MCU_FAMILY STREQUAL "F1" OR HYDRV_MCU_FAMILY STREQUAL "F4")
    add_subdirectory(upper_half)
    add_subdirectory(lower_half)
    if(HYDRV_BUILD_EXAMPLES)
        add_subdirectory(example)
    endif()
    
    message(STATUS "hydrv_uart subdirectories added for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
else()
    message(WARNING "hydrv_uart subdirectories are not supported for HYDRV_MCU_FAMILY: ${HYDRV_MCU_FAMILY}")
endif()
